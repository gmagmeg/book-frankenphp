# Webページの表示を早くする

この章では、HTTP 103 Early Hints を利用してWebページの表示を速くする方法を説明します。

## 103 Early Hintsとは

HTTP 103 Early Hints は、サーバーが最終レスポンスを返す前に、ブラウザへ予備のHTTPヘッダーを送信できる仕組みです。ブラウザはこの情報をもとに、CSSやJavaScriptなどのリソースを事前に読み込み始め、最終的なHTMLが届く前から、表示に必要な準備を先回りして進められるようになります。つまりページの表示速度を向上させられるということです。
![103 Early Hints のリクエスト処理フロー比較](images/early-hints-flow.png)

便利そうな機能ですが、どのように利用できるのでしょうか。FrankenPHPでは、この機能を比較的少ない手順で試せます。この節では、そのための準備を進めていきます。

## HTTP/2、SSLの設定

103 Early Hints は予備のHTTPヘッダーを並列に送る仕組みであるため、HTTPSかつHTTP/2であることが前提です。この環境を整えるには本来であればWebサーバーアプリケーション側にいろいろと設定が必要なのですが、FrankenPHP（Caddy）はHTTPS証明書の自動生成に対応しており、フラグを1つ追加するだけで有効化できます。
`php artisan octane:frankenphp` の起動コマンドに `--https` フラグを追加します。

```bash
php artisan octane:frankenphp --https --watch
```

FrankenPHP は内部で、Caddy が受け取ったリクエスト情報を CGI 環境変数に変換し、PHP の `$_SERVER` へ渡しています。`--https` フラグを付けて起動すると、この処理の中で `HTTPS=on` が自動的にセットされます。以下は `frankenphp.go` の該当箇所の抜粋です。

```go
// frankenphp/frankenphp.go（抜粋）

/**
 * Caddy のリクエストコンテキストから CGI 環境変数を生成する。
 * 生成した変数は PHP スレッドの $_SERVER にマッピングされる。
 * CGI/1.1 標準（RFC 3875）に準拠した変数名を使用している。
 */
func buildEnv(r *http.Request, fc *FrankenPHPContext) (map[string]string, error) {
    env := make(map[string]string)

    // TLS 接続、または X-Forwarded-Proto が https の場合に HTTPS=on をセット。
    // PHP 側では $_SERVER['HTTPS'] === 'on' で判定できる。
    if r.TLS != nil || fc.trustForwardedProtoHeader &&
        r.Header.Get("X-Forwarded-Proto") == "https" {
        env["HTTPS"] = "on"
    }

    // ...（SERVER_NAME, REQUEST_METHOD など CGI 標準変数の設定）

    return env, nil
}
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
3. ローカル CA 証明書を macOS Keychain に登録します。
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

下準備ができたので、アプリケーションにコードを差し込みます。といっても、書くコードは少なく、これだけで済みます。
```php
    // FrankenPHP の HTTP 103 Early Hints でMercureハブへの接続を事前通知する。
    header('Link: </.well-known/mercure>; rel=preconnect');
    headers_send(103);
```

コードを差し込んだ後は、103 Early Hints が実際に送信されているか確認しましょう。開発者ツールのネットワークタブに Early Hints Header が増えていることを確認できるはずです。

![Early Hints Headerあり](early-hints-headers.png)

![Early Hints Headerなし](images/early-hints-no.png)


## この章のまとめ

ここまで簡単に設定を済ませてきましたが、NginxやApacheなどの他のWebサーバーアプリケーションであるともっと煩雑な作業が発生します。しかしここがFrankenPHPの良いところで、起動コマンドに `--https` フラグを1つ追加するだけでHTTPS化とHTTP/2が有効になります。アプリケーション側のコードもわずか数行で済み、手軽にHTTP 103 Early Hintsを導入できます。シンプルな設定で諸々の機能が実現できる、FrankenPHP の魅了の1側面が伝わっていれば幸いです。
