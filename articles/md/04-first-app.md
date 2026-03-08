# メッセージ通知アプリ作成

## 作成物の概要
この章では、次の2つの画面を用意します。
1. メッセージをpushする画面
2. メッセージを受け取る画面

まず 1. メッセージをpushする画面を作っていきましょう。
今回利用するソースコードはこちらです。
https://github.com/gmagmeg/book-frankenphp-docker/blob/main/app/Http/Controllers/MercureController.php


### Mercure Hubでメッセージをpushする
メッセージをpushするために、FrankenPHPに組み込まれている Mercure Hub を利用します。まずは有効化のために `.env` ファイルを編集します。
なお各`KEY`は開発を想定して平易な値を使用していますが、本運用時はセキュアな文字列に置き換えてください。

```
MERCURE_TRANSPORT_URL=mercure://publisher:frankenphp_mercure_local_dev_20260302@localhost/.well-known/mercure
OCTANE_MERCURE_PUBLISHER_JWT_KEY=frankenphp_mercure_local_dev_20260302
OCTANE_MERCURE_SUBSCRIBER_JWT_KEY=frankenphp_mercure_local_dev_20260302
```

次に、コントローラーにメッセージをpushするコードを配置します。

```php
// リクエストの検証と、publish実行に必要な値(topic/payload/options)を組み立てる。
['topic' => $topic, 'payload' => $payload, 'options' => $options] = $this->preparePublishRequest($request);

// Mercureへメッセージを送信する。
$result = mercure_publish($topic, $payload, $options);
```

この呼び出しだけでメッセージをpushできます。`Node.js`などのサーバーサイド`JavaScript`を別途用意する必要がありません。こんなに簡単に済むのは、FrankenPHP に SSE（Server-Sent Events）でメッセージを配信する`Mercure Hub`が標準で組み込まれているためです。そのため、アプリケーション側では `mercure_publish()` を呼び出すだけで配信処理を実装できます。

### Mercure Hubからのメッセージを受信する
今回利用するソースコードはこちらです。
https://github.com/gmagmeg/book-frankenphp-docker/blob/main/resources/views/mercure/receiver.blade.php

受信側では、ブラウザ標準の `EventSource` API を使うことで `Mercure Hub` の購読エンドポイントへ接続できるので、特に新規ライブラリのインストールは必要ありません。

```js
const topic = "https://example.com/messages/general";
const subscribeUrl = `/.well-known/mercure?topic=${encodeURIComponent(topic)}`;

const eventSource = new EventSource(subscribeUrl);

eventSource.onmessage = (event) => {
  const message = JSON.parse(event.data);
  console.log("受信したメッセージ:", message);
};

eventSource.onerror = () => {
  console.error("Mercure Hubとの接続でエラーが発生しました。");
};
```

FrankenPHPが提供できるのはバックエンドサーバーの世界までなので、ここからはFrankenPHPを離れ、フロントの世界に移ります。
送信側で `mercure_publish()` を実行すると、同じ `topic` を購読しているクライアントにイベントが届きます。`event.data` には送信時のペイロードが文字列として入っているため、JSON として送信した場合は `JSON.parse()` でオブジェクトに変換します。ここまできたらまずはコンソールで受信内容を確認し、メッセージが届くことを確かめてください。

todo：画像差し込むかどうか検討する