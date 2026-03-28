# X-Sendfile 対応の変更内容

## 概要

FrankenPHP（Caddy）の X-Sendfile 機能を利用し、プライベートファイルの配信を PHP からウェブサーバーに委譲する仕組みを実装した。
これにより、PHP のメモリを消費せずに大容量ファイルを配信できる。

## 変更ファイル

### 1. `app/Providers/AppServiceProvider.php`

`boot()` メソッドに `BinaryFileResponse::trustXSendfileTypeHeader()` を追加。

```php
use Symfony\Component\HttpFoundation\BinaryFileResponse;

public function boot(): void
{
    BinaryFileResponse::trustXSendfileTypeHeader();
}
```

- **目的**: FrankenPHP がリクエストに付与する `X-Sendfile-Type: X-Sendfile` ヘッダーを Symfony の `BinaryFileResponse` が信頼するようにする
- **Octane との関係**: Octane ではワーカー起動時に `boot()` が1回だけ実行される。静的フラグなのでリクエストごとに呼ぶ必要はない

### 2. `app/Http/Controllers/DownloadController.php`

コントローラーから `trustXSendfileTypeHeader()` の呼び出しを削除し、AppServiceProvider に集約した。

```php
public function download(Request $request, string $filename): BinaryFileResponse
{
    abort_if(str_contains($filename, '..'), 400, '不正なファイルパスです。');

    $path = base_path('private-files/' . $filename);

    abort_unless(file_exists($path), 404, 'ファイルが見つかりません。');

    return new BinaryFileResponse($path);
}
```

## X-Sendfile の動作フロー

```
1. クライアント → FrankenPHP: GET /api/csv/download/{filename}
2. FrankenPHP → PHP: リクエスト転送（X-Sendfile-Type: X-Sendfile ヘッダー付与）
3. PHP: アクセス権チェック・ファイル存在確認
4. PHP → FrankenPHP: BinaryFileResponse（X-Sendfile: /app/private-files/xxx ヘッダー付き、ボディ空）
5. FrankenPHP → クライアント: ファイルを直接配信（PHP のメモリを使わない）
```

## 動作確認方法

`private-files/` に大容量のテストファイルを配置してダウンロードする。

```bash
# 300MB のテストファイルを作成
dd if=/dev/zero of=private-files/test-100mb.bin bs=1m count=300
```

```
GET https://localhost:8100/api/csv/download/test-100mb.bin
```

PHP の `memory_limit`（デフォルト 128MB）を超えるファイルが正常にダウンロードできれば、X-Sendfile が有効に動作している。
PHP がファイルの中身をメモリに読み込まず、FrankenPHP が直接配信しているためである。
