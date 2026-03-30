# Webページの表示を早くする

HTTP 103 Early Hints を利用してWebページの表示を速くする方法を説明します。

## 103 Early Hintsとは

HTTP 103 Early Hints は、サーバーが最終レスポンスを返す前に、ブラウザへ予備のHTTPヘッダーを送信できる仕組みです。ブラウザはこの情報をもとに、CSSやJavaScriptなどのリソースを事前に読み込み始めて最終的なHTMLが届く前から、表示に必要な準備を先回りして進められるようになり、ページの表示速度を向上させられます。
![103 Early Hints のリクエスト処理フロー比較](images/early-hints-flow.png)

FrankenPHPでは、この機能を比較的少ない手順で試せます。

## HTTP/2、SSLの設定

103 Early Hints は最終レスポンスに先立って予備のHTTPヘッダーを送る仕組みであるため、HTTPSかつHTTP/2であることが前提です。HTTP/2では1つのTCP接続上で複数のストリームを多重化できるため、最終レスポンスの送信を待たずに予備ヘッダーを別ストリームで先行して届けられます。この環境を整えるには本来であればWebサーバーアプリケーション側にいろいろと設定が必要なのですが、FrankenPHP（Caddy）はHTTPS証明書の自動生成に対応しており、フラグを1つ追加するだけで有効化できます。2章ですでにこの`--https`オプションは加えて起動しているので、証明書の生成とHTTPSの有効化は済んでいます。

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

## アプリケーションコードの設定

アプリケーションにコードを差し込みます。`headers_send(103)` はFrankenPHPが提供する独自関数で、PHP標準には存在しません。この関数を呼び出すことで、最終レスポンスを返す前に103 Early Hintsをブラウザへ送信できます。

### preconnectとpreloadの使い分け

Early Hintsで指定できるリソースヒントには、主に `preconnect` と `preload` の2種類があります。

- **`preconnect`**: 指定したオリジンへのTCP接続・TLSハンドシェイクを事前に行います。外部サービスとの接続で効果的です。
- **`preload`**: 指定したリソースのダウンロードを事前に開始します。CSSやJavaScriptなど、ページ表示に必要なアセットに有効です。

本書のアプリケーションでは、Mercure Hubへの事前接続と、アセットの事前読み込みを組み合わせて使います。

```php
// Mercure Hubへの事前接続（外部エンドポイントとのコネクション確立）
header('Link: </.well-known/mercure>; rel=preconnect');
// CSS・JSのそれぞれの事前読み込み
header('Link: </build/app.css>; rel=preload; as=style', false);
header('Link: </build/app.js>; rel=preload; as=script', false);
headers_send(103);
```

`header()` の第2引数に `false` を渡すことで、前のLinkヘッダーを上書きせずに複数のヒントを追加できます。

### 動作確認

コードを差し込んだ後は、103 Early Hints が実際に送信されているか確認しましょう。ブラウザの開発者ツールを開き、Networkタブで対象のリクエストを選択してください。Headersの中に `103 Early Hints` のセクションが表示され、指定した `Link` ヘッダーが含まれていれば正しく動作しています。

![Early Hints Headerあり](images/early-hints-headers.png)

![Early Hints Headerなし](images/early-hints-no.png)

さらにTimingタブを確認すると、Early Hintsありの場合はリソースの読み込み開始タイミングが早まっていることがわかります。CSSやJSの取得がHTMLの受信完了を待たずに始まるため、ページの表示速度の改善につながります。

他のWebサーバーであれば煩雑な設定作業が発生しますが、FrankenPHPでは起動コマンドに `--https` フラグを1つ追加するだけでHTTPS化とHTTP/2が有効になります。アプリケーション側のコードもわずか数行で、手軽にHTTP 103 Early Hintsを導入できます。
