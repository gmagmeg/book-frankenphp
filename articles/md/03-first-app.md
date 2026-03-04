# メッセージ通知アプリ作成

## 3.1 作成物の概要
この章では、次の2つの画面を用意します。
1. メッセージを送る画面
2. メッセージを受け取る画面

まず 1. メッセージを送る画面を作っていきましょう。

### Mercure Hubを有効にする
メッセージを送るためにFranken PHPに組み込まれているMercure HUbを利用します。有効にするために.envファイルを編集します。

```
MERCURE_TRANSPORT_URL=mercure://publisher:frankenphp_mercure_local_dev_20260302@localhost/.well-known/mercure
OCTANE_MERCURE_PUBLISHER_JWT_KEY=frankenphp_mercure_local_dev_20260302
OCTANE_MERCURE_SUBSCRIBER_JWT_KEY=frankenphp_mercure_local_dev_20260
```


該当のコードはこちらです。
https://github.com/gmagmeg/book-frankenphp-docker/blob/main/app/Http/Controllers/MercureController.php

FrankenPHPでMercureを有効化し、`OCTANE_MERCURE_*` の環境変数を設定する」必要があることを明示しています。
つまり、アプリ側の公開処理は `mercure_publish()` を呼ぶだけに寄せ、実際の配信先（Hub）設定は環境変数で切り替える構成です。

サンプルとして、設定に関わる部分を抜粋すると次のようになります。

```php
$result = mercure_publish($validated['topic'], $payload, $options);
```

MercureHub は、サーバー側がトピック単位でイベントを publish し、クライアントが Server-Sent Events（SSE）で subscribe して受信するためのリアルタイム配信ハブです。`private` 更新や JWT による購読制御を組み合わせることで、通知の公開範囲を制御できます。
