
# X-Sendfile（X-Accel-Redirect）で効率的にファイルを配信

この章ではX-Accel-Redirectを使って、ファイル配信の効率化を行なっていきます。


## 概要

認証が必要なファイルをPHPで配信するとき、素朴な実装では `file_get_contents()` や `readfile()` でファイルを読み込み、PHPのメモリを経由してレスポンスを返します。この方法は、大容量ファイルになるほどメモリ消費が増大し、ワーカーが長時間占有されるという問題があります。

**X-Sendfile**（NginxではX-Accel-Redirect）はこの課題を解決する仕組みです。PHPは「このファイルを送れ」という指示をヘッダーで返すだけで、実際のファイル送信はWebサーバー（FrankenPHP/Caddy）が直接行います。


```
通常の配信:
  クライアント → PHP → ファイルをメモリに読み込む → クライアント

X-Sendfileによる配信:
  クライアント → PHP（認証チェックのみ）→ FrankenPHP（ファイルを直接配信）→ クライアント
```

FrankenPHPは内部にCaddyを持つため、Caddyの `intercept` ディレクティブを使ってこの機能を実現できます。


## 設定方法

### ディレクトリ構成

配信したいファイルは `public/` ディレクトリの外に置きます。これにより、URLへの直接アクセスを防げます。


```
/app/
├── public/          ← Webサーバーのドキュメントルート
│   └── index.php
├── private-files/   ← 直接アクセス不可のファイル置き場
│   └── sample.txt
└── Caddyfile
```

### Caddyfileの設定

`intercept` ブロックが核心部分です。PHPから `X-Accel-Redirect` ヘッダーが返ってきたとき、それを検知してファイルを直接配信するよう Caddy に指示します。


```caddy
{
    frankenphp
    order php_server before file_server
}

:{$APP_PORT:-8000} {
    root * /app/public

    tls internal
    encode zstd br gzip

    # PHPへ X-Sendfile の種別（x-accel-redirect）を通知する
    request_header X-Sendfile-Type x-accel-redirect
    # PHPが指定するエイリアス（/private-files）と実ディレクトリのマッピング
    request_header X-Accel-Mapping /app/private-files=/private-files

    intercept {
        @accel header X-Accel-Redirect *
        handle_response @accel {
            root * /app/private-files
            rewrite * {resp.header.X-Accel-Redirect}
            method * GET
            # セキュリティのため、レスポンスから X-Accel-Redirect ヘッダーを除去する
            header -X-Accel-Redirect
            file_server
        }
    }

    php_server
}
```

各設定項目の役割は次のとおりです。

|ディレクティブ|説明|
|:--|:--|
|`request_header X-Sendfile-Type`|PHPに「このサーバーはx-accel-redirectを使う」と伝える|
|`request_header X-Accel-Mapping`|PHPが返すパスエイリアスと実パスを対応づける|
|`intercept { @accel ... }`|X-Accel-Redirectヘッダーを持つレスポンスを横取りする|
|`root * /app/private-files`|配信元ディレクトリをプライベートファイルに切り替える|
|`header -X-Accel-Redirect`|クライアントへの漏洩を防ぐためヘッダーを削除する|

### start-container.sh への反映

カスタム Caddyfile を Laravel Octane に読み込ませるため、`--caddyfile` オプションを指定します。


```sh
exec php artisan octane:frankenphp \
  --host=localhost \
  --port="${APP_PORT:-8000}" \
  --workers="${OCTANE_WORKERS:-4}" \
  --max-requests="${OCTANE_MAX_REQUESTS:-500}" \
  --caddyfile=/app/Caddyfile \
  --watch
```

## PHP側の実装

### 純粋なPHPの場合

最もシンプルな実装です。アクセス権チェックの後、ヘッダーをセットするだけです。


```php
// アクセス権チェック
if (!isAuthenticated()) {
    http_response_code(403);
    exit;
}

// ファイルパスのエイリアスをヘッダーにセットする
// （/private-files/ が Caddy により /app/private-files/ にマッピングされる）
header('X-Accel-Redirect: /private-files/sample.txt');
```

### Laravelの場合（BinaryFileResponse）

LaravelはSymfony HttpFoundationの `BinaryFileResponse` を通じてX-Sendfileをネイティブサポートしています。`trustXSendfileTypeHeader()` を呼び出すことで、Caddyfileで設定した `X-Sendfile-Type` ヘッダーの値を元に適切なヘッダーが自動的に付与されます。


```php
<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\BinaryFileResponse;

class DownloadController extends Controller
{
    public function download(Request $request, string $filename): BinaryFileResponse
    {
        // アクセス権チェック（例: 認証済みユーザーのみ許可）
        // abort_unless($request->user(), 403, 'ログインが必要です。');

        // パストラバーサル対策
        abort_if(str_contains($filename, '..'), 400, '不正なファイルパスです。');

        $path = base_path('private-files/' . $filename);
        abort_unless(file_exists($path), 404, 'ファイルが見つかりません。');

        // X-Sendfile-Type ヘッダーを信頼し、FrankenPHP に配信を委譲する
        BinaryFileResponse::trustXSendfileTypeHeader();

        return response()->file($path, [
            'Content-Disposition' => 'attachment; filename="' . basename($path) . '"',
        ]);
    }
}
```

ルートはシンプルに定義します。


```php
Route::get('/download/{filename}', [DownloadController::class, 'download'])
    ->where('filename', '[a-zA-Z0-9._-]+');
```

`trustXSendfileTypeHeader()` を呼び出すと、Caddyfileの `request_header X-Sendfile-Type x-accel-redirect` で設定した値を Symfony が認識し、レスポンスに `X-Accel-Redirect: /private-files/filename` ヘッダーを自動でセットします。後はCaddyの `intercept` ブロックがそれを検知してファイルを直接配信します。


## 動作フロー

```
1. GET /download/sample.txt
        ↓
2. DownloadController::download() でアクセス権チェック
        ↓
3. BinaryFileResponse が X-Accel-Redirect: /private-files/sample.txt を返す
        ↓
4. Caddy の intercept ブロックがヘッダーを検知
        ↓
5. root を /app/private-files に切り替え、file_server が直接配信
        ↓
6. クライアントがファイルを受信（PHPメモリを介さない）
```

## まとめ

|比較項目|通常のPHP配信|X-Sendfile配信|
|:--|:--|:--|
|PHPメモリ消費|ファイルサイズ分|ほぼゼロ|
|ワーカー占有時間|転送完了まで|認証処理のみ|
|大容量ファイル|不向き|適している|
|アクセス制御|PHPで実装可能|PHPで実装可能|

X-Sendfileは「PHPで認証し、配信はサーバーに任せる」という責務の分離を実現します。会員限定コンテンツや帳票PDFのダウンロードなど、認証が必要なファイル配信の場面では積極的に採用を検討してください。

