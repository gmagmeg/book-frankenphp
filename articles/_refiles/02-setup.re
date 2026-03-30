= アプリケーションを作成する下準備

== 事前確認

この章では、この本で利用するFrankenPHPの開発環境を構築します。開発環境はこちらに用意しているので、@<tt>{git clone} して進めてください。@<br>{}@<tt>{git clone https://github.com/gmagmeg/book-frankenphp-docker}

環境構築にあたり、次の環境が整っている前提で進めます。

 * macOS（またはLinux）
 * Git
 * ターミナル
 * Docker Engine が動作していること
 * Windows を利用する場合は、WSL2上で作業する前提で進めます。

== 環境構築

構成は次のとおりです。

 * アプリケーションサーバー: FrankenPHP
 * フレームワーク: Laravel 12
 * データベース: PostgreSQL
 * 公開ポート: @<tt>{8100}

Laravel で FrankenPHP の能力を最大限に発揮するために、Octane 経由で Workerモードを利用して起動します。Octane と Workerモードについては次の章で詳しく説明しますので、この章では省きます。また本書のコンテナでは、@<tt>{start-container.sh} から次のように起動します。

//emlist[][bash]{
php artisan octane:frankenphp \
  --host=localhost \
  --port="${APP_PORT:-8000}" \
  --workers="${OCTANE_WORKERS:-4}" \
  --max-requests="${OCTANE_MAX_REQUESTS:-500}" \
  --caddyfile=/app/Caddyfile \
  --watch
//}

本書では @<tt>{.env} で @<tt>{APP_PORT=8100} を設定して進めるため、実際の待受ポートは @<tt>{8100} になります（未設定では8000番が使われます）。

@<tt>{frankenphp} ディレクトリで、次のコマンドを実行します。

//emlist[][bash]{
docker compose up -d --build
docker compose exec app php artisan key:generate --force
docker compose exec app php artisan migrate --force
//}

コンテナを起動後、Caddyが作成した自己署名ルート証明書をOSに信頼済みとして登録します。こうすることで、ブラウザからアクセスしたときにセキュリティ関連の警告が消えます。

//emlist[][bash]{
# macOSの場合
sudo security add-trusted-cert -d -r \
trustRoot -k /Library/Keychains/System.keychain \
/tmp/caddy-local-root.crt
//}

//emlist[][bash]{
# Linux（Ubuntu/Debian）の場合
sudo cp /tmp/caddy-local-root.crt \
/usr/local/share/ca-certificates/caddy-local-root.crt
sudo update-ca-certificates
//}

ブラウザでアクセスすれば、Laravelの初期画面が映るはずです。

//emlist[][bash]{
https://localhost:8100/
//}

//image[laravel-init-img][Laravel 起動画面][scale=0.5]{
//}

本書の環境では SSE を利用するにあたり、開発環境でも @<tt>{tls internal} を設定し、 @<b>{HTTPS} で動作するようになっています。そのため @<tt>{curl} には自己署名証明書を許容する @<tt>{-k} オプションが必要です。ブラウザでアクセスする場合も、自己署名証明書に関するセキュリティ警告が表示されますので、ご注意ください。

== ここまでの確認ポイント

この時点で、次を満たしていれば本章の開発準備は完了です。

 * @<tt>{book-frankenphp-app} と @<tt>{book-frankenphp-db} が起動している
 * ホストの @<tt>{8100} ポートで Laravel アプリケーションへアクセスできる
 * マイグレーションが適用済みである

次の章ではWorkerモードと、拡張パッケージOctane について説明します。アプリケーションの実装まで、もう少々お待ちください。
