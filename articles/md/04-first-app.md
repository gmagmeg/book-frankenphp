# メッセージ通知アプリの作成

3章で下準備が完了したので、ここからはアプリケーション開発を通してFrankenPHPの組み込み機能を学んでいきましょう。次の要件を満たすアプリケーションを作成していきます。

## 作成するアプリケーションの概要
ユーザーがCSV生成をリクエストしてからダウンロードするまでの一連の流れを実装します。

 1. **CSV生成リクエスト**: ユーザーはCSVの生成をリクエストする。 
 2. **バックグラウンド処理**: リクエストを受けると、非同期でCSVの生成を開始します。
 3. **リアルタイム通知**: 生成完了後、ユーザーにワンタイムパスワード付きのプッシュ通知を送信します。
 4. **セキュアなダウンロード**: ユーザーは通知されたパスワードで認証を行い、ファイルをダウンロードします。

1.と2.はFrankenPHP固有の機能と直接関係しないため、ソースコードの紹介にとどめて簡潔に扱います。

![メッセージ発行画面](images/hakkou.png)  

![メッセージ受信画面](images/zyushin.png)

## 作成物の概要
この章では、次の2つの画面を用意します。
1. メッセージをプッシュする画面
2. メッセージを受け取る画面

まずは 1. メッセージをプッシュする画面を作っていきましょう。
今回利用するソースコードはこちらです。
https://github.com/gmagmeg/book-frankenphp-docker/blob/main/app/Http/Controllers/MercureController.php


### Mercure Hubでメッセージをプッシュする
メッセージをプッシュするために、FrankenPHPに組み込まれている `Mercure Hub` を利用します。まずは、有効化のために `.env` ファイルを編集します。
なお、各キーは開発を想定して平易な値を使用していますが、本番運用時はセキュアな文字列に置き換えてください。

* 紙面の都合上、改行を入れていますが、実際に記載するときは１行に繋げてください
```
MERCURE_TRANSPORT_URL=
mercure://publisher:frankenphp_mercure_local_dev_20260302@
localhost/.well-known/mercure

OCTANE_MERCURE_PUBLISHER_JWT_KEY=
frankenphp_mercure_local_dev_20260302

OCTANE_MERCURE_SUBSCRIBER_JWT_KEY=
frankenphp_mercure_local_dev_20260302
```

次に、コントローラーにメッセージをプッシュするコードを配置します。

```php
// リクエストの検証と、publish実行に必要な値(topic/payload/options)を組み立てる。
['topic' => $topic, 'payload' => $payload, 'options' => $options]
  = $this->preparePublishRequest($request);

// Mercureへメッセージを送信する。
$result = mercure_publish($topic, $payload, $options);
```

この呼び出しだけでメッセージをプッシュできます。`Node.js` などのサーバーサイド `JavaScript` を別途用意する必要がありません。これほど簡単に済むのは、FrankenPHPにSSE（Server-Sent Events）でメッセージを配信する `Mercure Hub` が標準で組み込まれているためです。そのため、アプリケーション側では `mercure_publish()` を呼び出すだけで配信処理を実装できます。

#### OTP（ワンタイムパスワード）の生成

送信するペイロードには、ダウンロード認証用のOTPを含めます。OTPには `lcobucci/jwt` で生成したJWTを採用します。JWTは署名済みのトークンで、有効期限やダウンロード対象ファイル名などを埋め込めます。これにより、受け取ったユーザーだけが使用できる短命な認証情報として機能します。

FrankenPHPの公式ドキュメントでも `symfony/mercure` と `lcobucci/jwt` の組み合わせが紹介されており、本書でもこれに沿って実装します。まずパッケージをインストールします。

```bash
composer require symfony/mercure lcobucci/jwt
```

次のコードでOTPを生成し、ペイロードに含めます。

```php
use Lcobucci\JWT\Configuration;
use Lcobucci\JWT\Signer\Hmac\Sha256;
use Lcobucci\JWT\Signer\Key\InMemory;

// HMAC-SHA256で署名したJWTを生成する設定
$jwtConfig = Configuration::forSymmetricSigner(
    new Sha256(),
    InMemory::plainText(config('services.jwt.secret'))
);

// 有効期限10分・ダウンロード対象ファイル名を埋め込んだOTPを生成する
$otp = $jwtConfig->builder()
    ->issuedAt(new \DateTimeImmutable())
    ->expiresAt(new \DateTimeImmutable('+10 minutes'))
    ->withClaim('file', $filename)
    ->getToken($jwtConfig->signer(), $jwtConfig->signingKey())
    ->toString();

// OTPをMercureのペイロードに含めて送信する
$payload = json_encode([
    'message' => 'CSVの生成が完了しました',
    'otp'     => $otp,
    'file'    => $filename,
]);
$result = mercure_publish($topic, $payload);
```

このOTPは `lcobucci/jwt` が持つJWT標準の有効期限・署名検証の仕組みを活用しており、有効期限切れや改ざんされたトークンは次章のダウンロード処理で自動的に弾かれます。

### Mercure Hubからのメッセージを受信する
今回利用するソースコードはこちらです。
https://github.com/gmagmeg/book-frankenphp-docker/blob/main/resources/views/mercure/receiver.blade.php

FrankenPHPが提供できるのはバックエンドサーバーの世界までなので、ここからはFrankenPHPを離れ、フロントの世界に移ります。
送信側で `mercure_publish()` を実行すると、同じ `topic` を購読しているクライアントにイベントが届きます。`event.data` には送信時のペイロードが文字列として入っているため、JSON として送信した場合は `JSON.parse()` でオブジェクトに変換します。ここまできたらまずはコンソールで受信内容を確認し、メッセージが届くことを確かめてください。

受信側では、ブラウザ標準の `EventSource` API を使うことで `Mercure Hub` の購読エンドポイントへ接続できるので、特に新規ライブラリのインストールは必要ありません。

```js
const topic = "https://example.com/messages/general";
const subscribeUrl = `/.well-known/mercure?topic=${encodeURIComponent(topic)}`;

const eventSource = new EventSource(subscribeUrl);
eventSource.onmessage = (event) => {
    const data = JSON.parse(event.data);
    // 受信したOTPとファイル名を画面に表示する
    document.getElementById('otp-display').textContent = data.otp;
    document.getElementById('filename-display').textContent = data.file;
    document.getElementById('download-area').classList.remove('hidden');
};
eventSource.onerror = () => {
    console.error('Mercureとの接続でエラーが発生しました。');
};
```

ユーザーは画面に表示されたOTPをコピーし、ダウンロードフォームに入力します。OTPの検証とファイル配信については次章で説明します。

### バックグラウンドでのCSV生成

CSV生成はFrankenPHP固有の機能と直接関係しないため、実装の詳細はソースコードを参照してください。
https://github.com/gmagmeg/book-frankenphp-docker/blob/main/app/Jobs/GenerateCsvJob.php

Laravelの `dispatch()` でジョブをキューに投入するだけで、リクエスト完了後にバックグラウンドで処理が進みます。CSV生成が完了したタイミングで、上記のMercure pushとOTP生成を呼び出しています。

## この章のまとめ

この章では、FrankenPHPに組み込まれたMercure Hubを使って、リアルタイム通知機能を実装しました。

- **Mercure Hub**: FrankenPHPに内蔵されており、`mercure_publish()` を呼び出すだけでSSEによるリアルタイム通知を実現できます。Node.jsなどの別サーバーは不要です。
- **OTP**: `lcobucci/jwt` を用いたJWTをOTPとして生成し、Mercureのペイロードに含めて送信します。有効期限付きの署名済みトークンとして機能するため、短命な認証情報に適しています。
- **EventSource API**: ブラウザ標準のAPIのみで購読でき、新規ライブラリのインストールは不要です。

次の章では、受け取ったOTPを使ってCSVをセキュアにダウンロードする、X-Sendfileの実装を見ていきます。