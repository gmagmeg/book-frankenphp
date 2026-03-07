# アプリケーションを作成する下準備

## 2.1 事前確認
この章では、FrankenPHPの開発環境を構築していきます。
今回の書籍で使う環境情報はこちらに用意しているので、`git clone` して進めてください。
https://github.com/gmagmeg/book-frankenphp-docker

環境構築にあたり、次の環境が整っている前提で進めます。

環境情報:
- macOS（またはLinux）
- Git
- ターミナル
- Docker Engine が動作していること

* Windowsを利用する場合は、WSL2上で作業する前提で進めます。

## 2.2 環境方針

構成は次のとおりです。
- アプリケーションサーバー: FrankenPHP（Laravel Octane worker モード）
- フレームワーク: Laravel 12
- データベース: PostgreSQL
- 公開ポート: `8100`

## 2.3 Laravel + 拡張パッケージ Octane
FrankenPHP の能力を最大限に発揮するために、Octane 経由で worker モードで起動します。本書のコンテナでは、`start-container.sh` から次のように起動します。

```bash
php artisan octane:frankenphp \
  --host=0.0.0.0 \
  --port="${APP_PORT:-8100}" \
  --workers="${OCTANE_WORKERS:-4}" \
  --max-requests="${OCTANE_MAX_REQUESTS:-500}"
```

`--port="${APP_PORT:-8100}"` は、`APP_PORT` が未設定の場合に `8100` を使う指定です。  
本書では `.env` で `APP_PORT=8100` を設定して進めるため、実際の待受ポートは `8100` になります。

## 2.4 環境を起動する
`frankenphp` ディレクトリで、次の順に実行します。

```bash
docker compose up -d --build
docker compose exec app php artisan key:generate --force
docker compose exec app php artisan migrate --force
```

起動後、次のコマンドで動作確認します。

```bash
docker compose ps
curl -i http://127.0.0.1:8100
```

`HTTP/1.1 200` が返れば準備完了です。

## 2.5 ここまでの確認ポイント
この時点で、次を満たしていれば本章の開発準備は完了です。
- `book-frankenphp-app` と `book-frankenphp-db` が起動している
- ホストの `8100` ポートで Laravel アプリケーションへアクセスできる
- マイグレーションが適用済みである

次の章から、いよいよアプリケーションの実装に入ります。
