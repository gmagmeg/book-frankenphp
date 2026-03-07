# メッセージ通知アプリ作成

## 3.1 作成物の概要
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
OCTANE_MERCURE_SUBSCRIBER_JWT_KEY=frankenphp_mercure_local_dev_2026002
```

次に、コントローラーにメッセージをpushするコードを配置します。

```php
// リクエストの検証と、publish実行に必要な値(topic/payload/options)を組み立てる。
['topic' => $topic, 'payload' => $payload, 'options' => $options] = $this->preparePublishRequest($request);

// Mercureへメッセージを送信する。
$result = mercure_publish($topic, $payload, $options);
```

この呼び出しだけでメッセージをpushできます。`Node.js`などのサーバーサイド`JavaScript`を別途用意する必要がありません。こんなに簡単に済むのは、FrankenPHP に SSE（Server-Sent Events）でメッセージを配信する`Mercure Hub`が標準で組み込まれているためです。そのため、アプリケーション側では `mercure_publish()` を呼び出すだけで配信処理を実装できます。
なお`Mercure Hub`による配信先の分離については、後ほどの章で説明しますので、ここでは割愛いたします。

### Mercure Hubからのメッセージを受信する
受信側では、`Mercure Hub` の購読エンドポイントに接続して、配信されたイベントを待ち受けます。ブラウザで確認する場合は、`EventSource` を使うと最小構成で動作確認できます。

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

送信側で `mercure_publish()` を実行すると、同じ `topic` を購読しているクライアントにこのイベントが届きます。まずはコンソールに受信内容を表示し、メッセージが届くことを確認してください。
