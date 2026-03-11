
# アプリケーションを作成する下準備

## 事前確認

この章では、この本で利用するFrankenPHPの開発環境を構築します。開発環境はこちらに用意しているので、`git clone` して進めてください。https://github.com/gmagmeg/book-frankenphp-docker

環境構築にあたり、次の環境が整っている前提で進めます。

前提環境: - macOS（またはLinux）- Git - ターミナル- Docker Engine が動作していること


* Windows を利用する場合は、WSL2上で作業する前提で進めます。

## 環境構築

構成は次のとおりです。- アプリケーションサーバー: FrankenPHP - フレームワーク: Laravel 12 - データベース: PostgreSQL - 公開ポート: `8100`

FrankenPHP の能力を最大限に発揮するために、Octane 経由で Workerモードを利用して起動します。Octane と Workerモードは次の章で詳しく説明します。本書のコンテナでは、`start-container.sh` から次のように起動します。


```bash
php artisan octane:frankenphp \
  --host=localhost \
  --port="${APP_PORT:-8100}" \
  --workers="${OCTANE_WORKERS:-4}" \
  --max-requests="${OCTANE_MAX_REQUESTS:-500}"
```

`--port="${APP_PORT:-8100}"` は、`APP_PORT` が未設定の場合に `8100` を使う指定です。
本書では `.env` で `APP_PORT=8100` を設定して進めるため、実際の待受ポートは `8100` になります。

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


## ここまでの確認ポイント

この時点で、次を満たしていれば本章の開発準備は完了です。- `book-frankenphp-app` と `book-frankenphp-db` が起動している- ホストの `8100` ポートで Laravel アプリケーションへアクセスできる- マイグレーションが適用済みである

次の章では、FrankenPHP の大きな特徴である Workerモードと、FrankenPHP 上で起動済みの Laravel アプリケーションを効率よく扱う、拡張パッケージOctane について説明します。アプリケーションの実装まで、もう少々お待ちください。

