
# Render.com にデプロイする

## 概要

FrankenPHP を利用したアプリケーションを公開するにあたって、デプロイ先として有力な選択肢の1つが「Render.com」です。Render.com は、GitHub と連携してビルドやデプロイを自動化できる PaaS（Platform as a Service）で、Docker への対応が非常にスムーズです。

FrankenPHP は Web サーバー（Caddy）と PHP ランタイムが1つのバイナリ・コンテナに統合されているため、Render.com の Web Service（Docker ランタイム）を利用すれば、複雑なサーバー構築なしに、たった1つのコンテナを立ち上げるだけでアプリケーションを公開できます。

この章では、Render.com にアプリケーションをデプロイする手順を解説します。


## 事前準備

デプロイを始める前に、次の準備を済ませておいてください。


* GitHub リポジトリの作成: 開発したコードを GitHub にプッシュしておきます。
* Render.com のアカウント作成: 公式サイト（https://render.com/）から登録を済ませます。
* PostgreSQL の作成: Render.com のダッシュボードから「New -> PostgreSQL」を選択し、データベースを作成しておきます。

## Web Service の作成

Render.com のダッシュボードから、新しく Web Service を作成します。


### 1. リポジトリの選択

「New -> Web Service」を選択し、デプロイしたい GitHub リポジトリを連携・選択します。


### 2. インスタンス設定

「Runtime」に **Docker** を選択します。Render.com は `Dockerfile` を自動的に検出し、ビルドを開始します。


### 3. 環境変数（Environment Variables）の設定

Laravel アプリケーションを動作させるために必要な環境変数を設定します。特に重要なのは次の項目です。


* `APP_KEY`: `php artisan key:generate` で生成されるキー
* `APP_ENV`: `production`
* `DATABASE_URL`: Render.com で作成した PostgreSQL の接続文字列（Internal Database URL）
* `APP_PORT`: `8000` (Dockerfile の EXPOSE と合わせる)
* `PORT`: `8000` (Render.com がコンテナを外部に公開するために使用するポート)

### 4. ポートの設定について

Render.com は、デフォルトで `PORT` という環境変数を使用して、外部からのリクエストをコンテナのどのポートに流すかを決定します。本書の `Dockerfile` では `EXPOSE 8000` を指定しているため、Render 側の設定でポートを `8000` に合わせるか、環境変数 `PORT=8000` を追加することで、正しくトラフィックがルーティングされます。


## デプロイの実行

設定が完了すると、自動的にビルドとデプロイが始まります。Render.com のログ画面で、Docker イメージのビルドが成功し、FrankenPHP (Octane) が正常に起動することを確認してください。

正常に起動すると、次のようなログが出力されます。


```text
INFO  Server running at http://localhost:8000
INFO  4 workers are running.
```

## 動作確認

デプロイ完了後、Render.com から発行された URL（例: `https://your-app.onrender.com`）にアクセスします。Laravel のウェルカム画面や、作成したアプリケーションの画面が表示されれば、デプロイ成功です。


## まとめ

Render.com を利用することで、FrankenPHP の「Web サーバーと実行環境の統合」という強みを最大限に活かし、シンプルな Docker コンテナデプロイでアプリケーションを公開できました。

次の章では、運用時に注意すべき点や、さらなる最適化について見ていきましょう。

