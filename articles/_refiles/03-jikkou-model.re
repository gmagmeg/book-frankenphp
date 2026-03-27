= 高速＆高パフォーマンスを支える技術

この章では、FrankenPHPの実行モデルと、Laravelでそれを活用するためのOctaneについて説明します。まずFrankenPHPの内部動作を理解しやすくするために、プロセスとスレッドの違いを整理しておきます。

=== プロセス・スレッドの違い

 * @<b>{プロセス} OSが独立したメモリ空間を割り当てる実行単位です。プロセスごとにメモリが分かれているため、互いの状態が干渉しにくい反面、起動コストは高くなります。
 * @<b>{スレッド} 1つのプロセス内で並行して動く実行単位です。同じプロセスのメモリ空間を共有するため、起動コストは比較的低い一方で、スレッドセーフで無い場合、共有状態の扱いには注意が必要です。

=== NTSとZTS

PHPにはスレッドセーフの観点から、NTS（Non-Thread Safe）とZTS（Zend Thread Safe）という2つのビルドがあります。

 * @<b>{NTS（Non-Thread Safe）}スレッドセーフティの機構を持たないビルドです。1リクエストを1プロセスで処理するApache+mod_phpやFastCGI（php-fpm）などの環境では、プロセスが独立したメモリ空間を持つためスレッド間の競合が発生しません。そのため、スレッドセーフティのオーバーヘッドがなく、わずかに高速に動作します。
 * @<b>{ZTS（Zend Thread Safe）}複数スレッドが同一プロセスのメモリ空間を共有して動作する環境に対応したビルドです。スレッド間の競合を防ぐ仕組みを持つため、NTSと比べてわずかなオーバーヘッドが生じますが、マルチスレッド環境でも安全に動作します。

FrankenPHPは内部でスレッドを使ってリクエストを処理するため、ZTSビルドで動作します。そのため、ZTSに対応していない一部のPHP拡張機能は利用できない場合があります（詳しくは後の章で説明します）。

== FrankenPHPの2つの実行モード

FrankenPHPには、大きく分けて classic mode と Workerモードの2つがあります。両モードともマルチスレッドで動作しますが、アプリケーションをリクエストごとに起動するか、起動済みのまま使い回すか、という違いがあります。

=== classic mode

Workerモードの設定をしない場合、FrankenPHPは classic mode で動作します。このモードは従来のPHP実行環境に近い使い方ができるモードで、リクエストごとにPHPファイルを直接実行する形式をとります。内部的にスレッドプールを用いてリクエストを処理することはWorkerモードと共通しており、単純にシングルスレッドで動くわけではありません（筆者はここの棲み分けで随分悩みました）。その性質上、Laravelのようなフレームワークではこの初期化コストが性能に影響してきます。

=== Workerモード

Workerモードは、アプリケーションを一度起動し、その状態をメモリに保持したままリクエストを処理するモードです。毎回アプリケーションを起動し直さずに済むため、DIコンテナやテンプレートエンジンなどのフレームワーク初期化コストを削減できます。

なお、FrankenPHPは classic mode でも Workerモードでも内部ではスレッドを使ってリクエストを処理します。Workerモードの利点はマルチスレッドであること自体よりも、初期化コストを繰り返さずに済む点にあります。@<br>{}本書ではWorkerモードで起動させるために、次のようなコマンドを使っています。

//emlist[][bash]{
php artisan octane:frankenphp \
  --workers=4
//}

//image[exec-mode-flow][classic mode と Workerモードのリクエスト処理フロー]{
//}

== Octaneの役割

@<b>{Octaneの導入} FrankenPHPのWorkerモードを、Laravelでも最大限に活かすうえで重要なのが、Octaneです。通常のLaravelアプリケーションのエントリーポイントは @<tt>{public/index.php} ですが、このファイルはリクエストのたびにアプリケーションを初期化する実装になっています。そのため、FrankenPHPのWorkerモードで動作させていても、@<tt>{index.php} がリクエストごとに呼ばれる限り、初期化処理が都度走ってしまいます。Octaneを導入すると、この @<tt>{index.php} をWorkerスクリプトに差し替え、アプリケーションの起動状態をメモリに保持したままリクエストを処理できるようになります。導入は他のLaravelパッケージと同様に、ComposerコマンドとArtisanコマンドで行えます。

//emlist[][bash]{
composer require laravel/octane
php artisan octane:install --server=frankenphp
//}

これだけでOctaneの導入は完了し、FrankenPHP のWorkerモードを最大限利用する準備が整います。

@<b>{Octane導入後の仕組み}ここからはWorkerスクリプトの実装を追って、起動状態をどう保持していくかを追っていきます。@<br>{}Octaneを導入したLaravelアプリケーションは、サーバー起動時に一度だけ初期化され、その後は起動済みのアプリケーションがメモリに保持されたまま、受け取ったリクエストを順番に処理していきます。差し替えられたWorkerスクリプトはサーバー起動時に一度だけ実行され、Laravelアプリケーションのインスタンスを @<tt>{$app} に保持します。以降のリクエスト処理では、この @<tt>{$app} を使い回すことで、アプリケーションの再起動なしにリクエストを処理できます。

//emlist[][php]{
<?php

use Laravel\Octane\FrankenPhp\FrankenPhpClient;
use Laravel\Octane\FrankenPhp\ServerStateFile;
use Laravel\Octane\Worker;

// アプリケーションを一度だけ起動し、$app に保持する
$app = require __DIR__.'/../bootstrap/app.php';

$worker = tap(new Worker(
    new \Laravel\Octane\ApplicationFactory(__DIR__.'/../bootstrap/app.php'),
    $client = new FrankenPhpClient(・・・)
))->boot();

// リクエストをループで処理する（$app は再起動しない）
while (
    frankenphp_handle_request(function () use ($worker, $client): void {
        $worker->handle(・・・);
    })
);

$worker->terminate();
//}

@<tt>{$worker->boot()} の内部では、@<tt>{Worker} クラスが @<tt>{$this->app} プロパティにアプリケーションのインスタンスを保持します。以下は @<tt>{vendor/laravel/octane/src/Worker.php} の抜粋です。

//emlist[][php]{
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
//}

このようにして Octane + Workerモードでは、初期化コストをサーバー起動時の1回にまとめられるため、リクエストごとのオーバーヘッドを大きく減らせます。Octaneを導入することで、FrankenPHPのWorkerモードが持つ起動コスト削減の効果を、Laravelアプリケーションでも活かせるようになりました。

== この章のまとめ

この章では、FrankenPHPの実行モードと、LaravelでWorkerモードを活用するためのOctaneについて説明しました。

 * @<b>{classic mode}：FrankenPHPの標準的な実行モードです。内部ではスレッドプールを使ってリクエストを処理しますが、アプリケーションの初期化はリクエストごとに発生します。
 * @<b>{Workerモード}：アプリケーションを一度起動し、その状態をメモリに保持したままリクエストを処理するモードです。性能面での利点は、マルチスレッドであること自体よりも、初期化コストを繰り返さずに済む点にあります。
 * @<b>{Octane}：Laravelアプリケーションを一度起動し、その状態を保持したままリクエストを処理するための拡張パッケージです。FrankenPHPと組み合わせることで、LaravelでもWorkerモードの利点を活かしやすくなります。

FrankenPHPとOctaneを組み合わせることで、通常のPHP実行環境と比べて高いスループットと低いレイテンシが期待できます。
