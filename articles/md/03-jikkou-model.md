# 高速＆高パフォーマンスを支える技術

この章では、FrankenPHPの実行モデルと、Laravelでそれを活用するためのOctaneについて説明します。
まずFrankenPHPの内部動作を理解しやすくするために、プロセスとスレッドの違いを整理しておきます。

### プロセス・スレッドの違い
- **プロセス**
  OSが独立したメモリ空間を割り当てる実行単位です。プロセスごとにメモリが分かれているため、互いの状態が干渉しにくい反面、起動コストは高くなります。

- **スレッド**
  1つのプロセス内で並行して動く実行単位です。同じプロセスのメモリ空間を共有するため、起動コストは比較的低い一方で、共有状態の扱いには注意が必要です。

**表：プロセスとスレッドの比較**

| 比較項目 | プロセス | スレッド |
|---|---|---|
| メモリ空間 | 独立（共有なし） | 同一プロセス内で共有 |
| 起動コスト | 高い | 低い |
| データ受け渡し | 専用の仕組みが必要 | 容易 |
| 変数の独立性 | 高い（干渉なし） | 低い（排他制御が必要） |

## FrankenPHPの2つの実行モード
FrankenPHPには、大きく分けて classic mode と Workerモードの2つがあります。両モードともマルチスレッドで動作しますが、アプリケーションをリクエストごとに起動するか、起動済みのまま使い回すか、という違いがあります。

### classic mode
Workerモードの設定をしない場合、FrankenPHPは classic mode で動作します。このモードは従来のPHP実行環境に近い使い方ができるモードで、リクエストごとにPHPファイルを直接実行する形式をとります。内部的にスレッドプールを用いてリクエストを処理することはWorkerモードと共通しており、単純にシングルスレッドで動くわけではありません（筆者はここの棲み分けで随分悩みました）。
大きな違いとして、classic mode ではリクエストごとにアプリケーションの初期化が発生します。Laravelのようなフレームワークを利用する場合は、設定やサービスコンテナの読み込みを毎回行うため、この初期化コストが性能に影響してきます。

### Workerモード
Workerモードは、アプリケーションを一度起動し、その状態をメモリに保持したままリクエストを処理するモードです。OPcacheがPHPコードを効率よく再利用する仕組みであるのに対し、Workerモードは起動済みのアプリケーション全体を使い回せる点に特徴があります。毎回アプリケーションを起動し直さずに済むため、設定配列や設定オブジェクト、DIコンテナ、テンプレートエンジンなどなどのフレームワークの初期化コストを削減できます。
このようにWorkerモードの本質は起動済みのアプリケーションを使い回せることにあり、高速＆高パフォーマンスを支える重要な技術となっています。

FrankenPHPは classic mode でも Workerモードでも、内部ではスレッドを使ってリクエストを処理します。そのため、Workerモードを理解するうえでは、スレッドの存在そのものよりも、アプリケーションの起動タイミングに注目することが重要です。

![classic mode と Workerモードのリクエスト処理フロー](images/exec-mode-flow.png)

**表：classic mode と Workerモードの比較**

| 比較項目 | classic mode | Workerモード |
|---|---|---|
| アプリケーションの起動 | リクエストごとに起動する | 起動済みの状態を再利用する |
| 初期化コスト | リクエストごとに発生する | 起動時にまとめやすい |
| PHP実行環境としての性格 | 従来のPHP実行環境に近い | アプリケーションを常駐させて扱う |
| フレームワーク利用時の特徴 | 毎回の初期化が性能に影響しやすい | 初期化コストを抑えやすい |

## 本書での起動方法
本書ではLaravelを扱うため、FrankenPHPをOctane経由で起動します。実際のコンテナでは、次のようなコマンドを使っています。

```bash
php artisan octane:frankenphp \
  --workers=4
```

Laravel Octaneでは、`--workers` によってリクエストを処理するワーカー数を指定できます。本書では、この構成を使って起動済みのLaravelアプリケーションを活かしながらリクエストを処理していきます。

## Octaneの役割
**Octaneの導入**
FrankenPHPのWorkerモードをLaravelで活かすうえで重要なのがOctaneです。Octaneは、Laravelアプリケーションを一度起動し、その起動状態をメモリに保持したままリクエストを処理する仕組みを提供します。導入は他のLaravelパッケージと同様に、ComposerコマンドとArtisanコマンドで行えます。

```bash
composer require laravel/octane
php artisan octane:install --server=frankenphp
```

続いて、次のコマンドでサーバーを起動します。

```bash
php artisan octane:frankenphp
```

これでOctaneの導入は完了です。

**Octane導入後の仕組み**

Octaneを導入したLaravelアプリケーションは、サーバー起動時に一度だけ初期化されます。その後は、起動済みのアプリケーションがメモリに保持されたまま、受け取ったリクエストを順番に処理していきます。

この仕組みを実現するために、OctaneはLaravelの標準エントリーポイントである `public/index.php` をWorkerスクリプトに差し替えます。Workerスクリプトはサーバー起動時に一度だけ実行され、Laravelアプリケーションのインスタンスを `$app` に保持します。以降のリクエスト処理では、この `$app` を使い回すことで、アプリケーションの再起動なしにリクエストを処理できます。

`php artisan octane:install --server=frankenphp` を実行すると、次のようなWorkerスクリプトが `public/frankenphp-worker.php` に生成されます。

```php
<?php

use Laravel\Octane\FrankenPhp\FrankenPhpClient;
use Laravel\Octane\FrankenPhp\ServerStateFile;
use Laravel\Octane\Worker;

// アプリケーションを一度だけ起動し、$app に保持する
$app = require __DIR__.'/../bootstrap/app.php';

$worker = tap(new Worker(
    new \Laravel\Octane\ApplicationFactory(__DIR__.'/../bootstrap/app.php'),
    $client = new FrankenPhpClient(
        new ServerStateFile($_SERVER['STATE_FILE']
            ?? sys_get_temp_dir().'/frankenphp-octane-server-state.json'),
    )
))->boot();

// リクエストをループで処理する（$app は再起動しない）
while (
    frankenphp_handle_request(function () use ($worker, $client): void {
        $worker->handle(...$client->marshalRequest(new \Laravel\Octane\RequestContext));
    })
);

$worker->terminate();
```

`$worker->boot()` の内部では、`Worker` クラスが `$this->app` プロパティにアプリケーションのインスタンスを保持します。以下は `vendor/laravel/octane/src/Worker.php` の抜粋です。

```php
// vendor/laravel/octane/src/Worker.php（抜粋）
class Worker
{
    /** 起動済みのアプリケーションインスタンスを保持するプロパティ */
    protected $app;

    /**
     * サーバー起動時に1回だけ呼ばれる。
     * アプリケーションを生成し $this->app に格納する。
     */
    public function boot(array $initialInstances = []): static
    {
        $this->app = $app = $this->appFactory->createApplication();
        // ...
    }

    /**
     * リクエストごとに呼ばれる。
     * $this->app を再利用してリクエストを処理する。
     */
    public function handle(Request $request, mixed $context = null): void
    {
        CurrentApplication::set($this->app);
        // ...
    }
}
```

通常のPHP実行環境では、リクエストごとにフレームワークや設定ファイルの読み込みが発生します。これに対して Octane + Workerモードでは、初期化コストをサーバー起動時の1回にまとめられるため、リクエストごとのオーバーヘッドを大きく減らせます。Octaneを導入することで、FrankenPHPのWorkerモードが持つ起動コスト削減の効果を、Laravelアプリケーションでも活かせるようになります。

## この章のまとめ

この章では、FrankenPHPの実行モードと、LaravelでWorkerモードを活用するためのOctaneについて説明しました。

- **classic mode**：FrankenPHPの標準的な実行モードです。内部ではスレッドプールを使ってリクエストを処理しますが、アプリケーションの初期化はリクエストごとに発生します。
- **Workerモード**：アプリケーションを一度起動し、その状態をメモリに保持したままリクエストを処理するモードです。性能面での利点は、マルチスレッドであること自体よりも、初期化コストを繰り返さずに済む点にあります。
- **Octane**：Laravelアプリケーションを一度起動し、その状態を保持したままリクエストを処理するための拡張パッケージです。FrankenPHPと組み合わせることで、LaravelでもWorkerモードの利点を活かしやすくなります。

FrankenPHPとOctaneを組み合わせることで、通常のPHP実行環境と比べて高いスループットと低いレイテンシが期待できます。
