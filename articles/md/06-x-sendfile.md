# X-Sendfileで効率的にファイルを配信

FrankenPHPのX-Sendfile機能を使って、ファイル配信を効率化します。

## 概要

認証が必要なファイルをPHPで配信するとき、素朴な実装では `file_get_contents()` や `readfile()` でファイルを読み込み、PHPのメモリを経由してレスポンスを返します。この方法は、大容量ファイルになるほどメモリ消費が増大し、Workerが長時間占有されるという問題があります。

**X-Sendfile**はこの課題を解決する仕組みです。PHPは「このファイルを送れ」という指示をヘッダーで返すだけで、実際のファイル送信はWebサーバー（FrankenPHP）が直接行います。

```
通常の配信:
  クライアント → PHP → ファイルをメモリに読み込む → クライアント

X-Sendfileによる配信:
  クライアント → PHP（認証チェックのみ）→ FrankenPHP（ファイルを直接配信）→ クライアント
```

FrankenPHPはX-Sendfileをネイティブサポートしています。NginxのX-Accel-Redirectのような複雑なサーバー設定は不要で、PHP側の数行の設定だけで利用できます。

## 設定方法

### ディレクトリ構成

配信したいファイルは `public/` ディレクトリの外に置きます。これにより、URLへの直接アクセスを防げます。

```
/app/
├── public/          ← Webサーバーのドキュメントルート
│   └── index.php
├── private-files/   ← 直接アクセス不可のファイル置き場
│   └── sample.txt
```

### FrankenPHPのX-Sendfile対応

FrankenPHPはリクエストをPHPに転送する際、自動的に `X-Sendfile-Type: X-Sendfile` ヘッダーを付与します。PHPがレスポンスに `X-Sendfile` ヘッダーを含めて返すと、FrankenPHPはそのヘッダーで指定されたファイルを直接配信します。

NginxのX-Accel-Redirectでは `intercept` ブロックやパスマッピングの設定が必要でしたが、FrankenPHPではそうした設定は一切不要です。Caddyfileに特別な記述を追加する必要はありません。

## PHP側の実装

### AppServiceProviderの設定

`trustXSendfileTypeHeader()` を `AppServiceProvider` の `boot()` メソッドで呼び出します。これにより、FrankenPHPがリクエストに付与する `X-Sendfile-Type: X-Sendfile` ヘッダーをSymfonyの `BinaryFileResponse` が信頼するようになります。

```php
use Symfony\Component\HttpFoundation\BinaryFileResponse;

public function boot(): void
{
    BinaryFileResponse::trustXSendfileTypeHeader();
}
```

OctaneではWorker起動時に `boot()` が1回だけ実行されます。`trustXSendfileTypeHeader()` は静的フラグをセットするだけなので、リクエストごとに呼ぶ必要はなく、`boot()` に置くのが最適です。

### DownloadController

コントローラーはアクセス権チェックとファイル存在確認のみを行います。ファイルの実際の配信はFrankenPHPに委譲されます。

```php
<?php
class DownloadController extends Controller
{
    public function download(Request $request, string $filename): BinaryFileResponse
    {
        // パストラバーサル対策
        abort_if(str_contains($filename, '..'), 400, '不正なファイルパスです。');
        $path = base_path('private-files/' . $filename);
        abort_unless(file_exists($path), 404, 'ファイルが見つかりません。');

        return new BinaryFileResponse($path);
    }
}
```

`trustXSendfileTypeHeader()` が有効な状態で `BinaryFileResponse` を返すと、Symfonyが自動的に `X-Sendfile: /app/private-files/filename` ヘッダーをレスポンスにセットし、ボディを空にします。FrankenPHPはこのヘッダーを検知してファイルを直接配信します。

### ルート定義

```php
Route::get('/download/{filename}', [DownloadController::class, 'download'])
    ->where('filename', '[a-zA-Z0-9._-]+');
```

## 動作フロー

```
1. クライアント → FrankenPHP: GET /download/{filename}
        ↓
2. FrankenPHP → PHP: リクエスト転送（X-Sendfile-Type: X-Sendfile ヘッダー付与）
        ↓
3. PHP: アクセス権チェック・ファイル存在確認
        ↓
4. PHP → FrankenPHP: BinaryFileResponse（X-Sendfile: /app/private-files/xxx ヘッダー付き、ボディ空）
        ↓
5. FrankenPHP → クライアント: ファイルを直接配信（PHPのメモリを使わない）
```

## 動作確認

`private-files/` にPHPの `memory_limit`（デフォルト128MB）を超えるテストファイルを配置して、ダウンロードが成功することを確認します。

```bash
# 300MB のテストファイルを作成
dd if=/dev/zero of=private-files/test-300mb.bin bs=1m count=300
```

```
GET https://localhost:8100/download/test-300mb.bin
```

128MBを超えるファイルが正常にダウンロードできれば、X-Sendfileが有効に動作しています。PHPがファイルの中身をメモリに読み込まず、FrankenPHPが直接配信しているためです。

| 比較項目 | 通常のPHP配信 | X-Sendfile配信 |
|---|---|---|
| PHPメモリ消費 | ファイルサイズ分 | ほぼゼロ |
| Worker占有時間 | 転送完了まで | 認証処理のみ |
| 大容量ファイル | 不向き | 適している |
| アクセス制御 | PHPで実装可能 | PHPで実装可能 |
| サーバー設定 | 不要 | 不要（FrankenPHPネイティブ対応） |

PHP側は `trustXSendfileTypeHeader()` を呼ぶだけで利用でき、Caddyfileへの追加設定も不要です。会員限定コンテンツや帳票PDFなど、認証が必要なファイル配信の場面で活用してください。
