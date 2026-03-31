= 高速＆高パフォーマンスを支える技術

FrankenPHPの実行モードと、LaravelでWorkerモードを活用するためのOctaneについて説明します。

== 前提知識

FrankenPHPの内部動作を理解するために、プロセスとスレッドの違いを整理しておきます。

=== プロセス・スレッドの違い

@<b>{プロセス}　OSが独立したメモリ空間を割り当てる実行単位です。プロセスごとにメモリが分かれているため、互いの状態が干渉しにくい反面、起動コストは高くなります。

@<b>{スレッド}　1つのプロセス内で並行して動く実行単位です。同じプロセスのメモリ空間を共有するため、起動コストは比較的低い一方で、スレッドセーフでない場合、共有状態の扱いには注意が必要です。

=== NTSとZTS

PHPにはスレッドセーフの観点から、NTS（Non-Thread Safe）とZTS（Zend Thread Safe）という2つのビルドがあります。

@<b>{NTS（Non-Thread Safe）}　スレッドセーフティの機構を持たないビルドです。1リクエストを1プロセスで処理するApache+mod_phpやFastCGI（php-fpm）などの環境では、プロセスが独立したメモリ空間を持つためスレッド間の競合が発生しません。そのため、スレッドセーフティのオーバーヘッドがなく、わずかに高速に動作します。

@<b>{ZTS（Zend Thread Safe）}　複数スレッドが同一プロセスのメモリ空間を共有して動作する環境に対応したビルドです。スレッド間の競合を防ぐ仕組みを持つため、NTSと比べてわずかなオーバーヘッドが生じますが、マルチスレッド環境でも安全に動作します。

FrankenPHPは内部でスレッドを使ってリクエストを処理するため、ZTSビルドで動作します。そのため、ZTSに対応していない一部のPHP拡張機能は利用できない場合があります（詳しくは後の章で説明します）。

== FrankenPHPの2つの実行モード

FrankenPHPには、大きく分けて Classic mode と Workerモードの2つがあります。両モードともマルチスレッドで動作しますが、アプリケーションをリクエストごとに起動するか、起動済みのまま使い回すか、という違いがあります。

=== Classic mode

Workerモードの設定をしない場合、FrankenPHPは Classic mode で動作します。このモードは従来のPHP実行環境に近い使い方ができるモードで、リクエストごとにPHPファイルを直接実行する形式をとります。その性質上、Laravelのようなフレームワークではこの初期化コストが性能に影響してきます。

=== Workerモード

Workerモードは、アプリケーションを一度起動し、その状態をメモリに保持したままリクエストを処理するモードです。毎回アプリケーションを起動し直さずに済むため、DIコンテナやテンプレートエンジンなどのフレームワーク初期化コストを削減できます。

Workerモードの利点はマルチスレッドであること自体よりも、初期化コストを繰り返さずに済む点にあります。本書ではWorkerモードで起動させるために、次のようなコマンドを使っています。

//emlist[][bash]{
php artisan octane:frankenphp --workers=4
//}

//image[exec-mode-flow][Classic mode と Workerモードのリクエスト処理フロー]{
//}

== Octaneの役割

通常のLaravelアプリケーションのエントリーポイントは @<tt>{public/index.php} で、このファイルはリクエストのたびにアプリケーションを初期化する実装になっています。FrankenPHPのWorkerモードで動作させていても、@<tt>{index.php} がリクエストごとに呼ばれる限り、初期化処理が都度走ってしまいます。

Octaneを導入すると、この @<tt>{index.php} をWorkerスクリプトに差し替え、アプリケーションの起動状態をメモリに保持したままリクエストを処理できるようになります。導入は他のLaravelパッケージと同様で、ComposerコマンドとArtisanコマンドだけで完了します。

//emlist[][bash]{
composer require laravel/octane
php artisan octane:install --server=frankenphp
//}

Octaneを導入したLaravelアプリケーションは、サーバー起動時に一度だけ初期化され、その後は起動済みのアプリケーションがメモリに保持されたまま、受け取ったリクエストを順番に処理していきます。差し替えられたWorkerスクリプトはサーバー起動時に一度だけ実行され、Laravelアプリケーションのインスタンスを @<tt>{$app} に保持します。以降のリクエスト処理では、この @<tt>{$app} を使い回すことで、アプリケーションの再起動なしにリクエストを処理できます。

//emlist[][php]{
<?php
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

このようにして Octane + Workerモードでは、初期化コストをサーバー起動時の1回にまとめられるため、リクエストごとのオーバーヘッドを大きく減らせます。
