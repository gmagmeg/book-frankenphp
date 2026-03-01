# 1.1 FrankenPHPとは何か

FrankenPHPは、Go言語製Webサーバー「Caddy」にPHPランタイムを統合したPHPアプリケーションサーバーで、作者はKévin Dunglas、公式ソースは `https://github.com/php/frankenphp` です。GitHubのURLを見てわかる通り、実は2025年5月、PHPを支援する非営利組織 The PHP Foundation からの公式サポートが公表され、今日ではPHP実行環境の有力な選択肢の1つとして、注目されています。
出典: https://thephp.foundation/blog/2025/05/15/frankenphp/

## PHP実行環境の変遷とFrankenPHPのアプローチ

かつて主流だった `Apache + mod_php` では、Webサーバー内部でPHPを直接実行していました。
その後、同時接続数や運用性の観点から、WebサーバーとPHPプロセスを分離した `Nginx + PHP-FPM` が広く普及しました。この構成では、両者はFastCGIで通信します。

FrankenPHPは、この分離構成で発生するFastCGI通信のオーバーヘッドや構成の複雑さに対するアプローチとして、Webサーバー統合型の実行モデルを現代的に再構成したものです。
これにより、構成の単純化と性能改善が期待できます。
