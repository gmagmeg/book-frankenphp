# Webページの表示を早くする

この章では、HTTP 103 Early Hints を利用してWebページの表示を速くする方法を説明します。

## 103 Early Hintsとは

HTTP 103 Early Hints は、サーバーが最終レスポンスを返す前に、ブラウザへ予備のHTTPヘッダーを送信できる仕組みです。ブラウザはこの情報をもとに、CSSやJavaScriptなどのリソースを事前に読み込み始めます。その結果、ページの表示速度が向上します。
何か良さげな機能ですが、どうやったら利用できるのでしょうか。Franken phpでこの機能を利用するために、準備を進めていきます。

## HTTP/2、SSLの設定

103 Early Hints は予備のHTTPヘッダーを並列に送る仕組みであるため、HTTP/2であることが前提です。HTTP/2はHTTPS上でのみ動作するため、HTTPS化が必要です。本来であればWebサーバーアプリケーション側に色々と設定が必要なのですが、FrankenPHP（Caddy）はHTTPS証明書の自動生成に対応しており、フラグを1つ追加するだけで有効化できます。
`php artisan octane:frankenphp` の起動コマンドに `--https` フラグを追加します。

```bash
php artisan octane:frankenphp --https --watch
```

### 証明書関連の設定

ここからは証明書関連の設定を行います。

1. 生成された証明書をホスト側で同期するために、`compose.yaml` に次の設定を追加します。
```yaml
volumes:
  - caddy_data:/data  # TLS証明書を永続化し、コンテナ再起動後も再登録が不要になる
```
2. 設定を反映するためにイメージの再作成とコンテナの再起動を行います。
```bash
docker compose build --no-cache
docker compose up -d
```
3.  ローカル CA 証明書を macOS Keychain に登録します。
```
docker cp book-frankenphp-app:/data/caddy/pki/authorities/local/root.crt ~/caddy-local-root.crt
sudo security add-trusted-cert -d -r trustRoot -k /Library/Keychains/System.keychain ~/caddy-local-root.crt
```

ここまでの手順を行うと、ブラウザで証明書の警告が表示されなくなります。なお、`caddy_data` ボリュームが存在する限り同じ証明書が使われ続けるため、Keychain への再登録はコンテナを再作成しない限り不要です。
最後にブラウザで正常にアクセスできることを確認しましょう。

```
https://localhost:8100
```

## 4.3 アプリケーションコードの設定

下準備は出来たので、アプリケーションにコードを差し込みます。といっても書くコードは少なく、これだけで済みます。
```php
    // FrankenPHP の HTTP 103 Early Hints でMercureハブへの接続を事前通知する。
    header('Link: </.well-known/mercure>; rel=preconnect');
    headers_send(103);
```

コードを差し込んだ後は、103 Early Hints が実際に送信されているか確認しましょう。開発者ツールのネットワークタブにEarly Hints Headerが増えているのが確認できるはずです。

![Early Hints Headerあり](early-hints-headers.png)

![Early Hints Headerなし](images/early-hints-no.png)


## この章のまとめ

いかがだったでしょうか。NginxやApacheなどの他のWebサーバーアプリケーションだったらどれくらいの手順が必要なのか、ざっくり想像するだけでも大変な作業です。しかしFrankenPHPでは、起動コマンドに `--https` フラグを1つ追加するだけでHTTPS化とHTTP/2が有効になります。アプリケーション側のコードもわずか数行で済み、手軽にHTTP 103 Early Hintsを導入できます。

この章では、FrankenPHPを使ってHTTP 103 Early Hintsを動作させるまでの手順を確認しました。SSL証明書の設定、HTTP/2の有効化、アプリケーションコードの組み込みから動作確認まで、シンプルな構成で実現できることがお分かりいただけたと思います。