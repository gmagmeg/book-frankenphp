# 1.1 FrankenPHPとは何か
PHP実行環境
Go言語製のWebサーバー「Caddy」に、PHPを組み込んだ。
従来の実行環境Nginx＋PHP-FPMや、Apache＋mod_phpより軽量かつ高速。2025年5月にPHP Foundationのサポートも受けるようになった。
https://thephp.foundation/blog/2025/05/15/frankenphp/


### 1. FrankenPHPとは何か

FrankenPHPは、Go言語で開発されたモダンなWebサーバー、Caddyに、PHPの実行環境を直接組み込んだ、モダンなPHPアプリケーションサーバーです。

## 1.1 PHP実行環境の変遷とFrankenPHPのアプローチ
　かつて主流だった Apache + mod_php の構成では、Webサーバー内部にPHPが組み込まれており、プロセス内で直接実行されていました。その後、より高い同時接続数やパフォーマンスを求めて、Nginx + PHP-FPM の構成が標準となりました。この構成ではWebサーバーとPHPプロセスが分離され、その間をFastCGIプロトコルで通信させています。
FrankenPHPは、この分離された構成に対する現代的な解決策です。Nginx + PHP-FPMで発生していたプロセス間通信（FastCGI）のオーバーヘッドを解消するため、かつてのApache + mod_phpのように、Webサーバー（Caddy/Go）内部でPHPを直接動作させる「Webサーバー統合型」のアーキテクチャを再び採用しました。これにより、構成の簡素化とパフォーマンスの向上を両立させています。
そしてFrankenPHPを巡る大きな動きとして、2025年5月、PHPを支援する非営利組織「The PHP Foundation」がFrankenPHPを公式にサポートすることを発表しました。これからのPHPの実行環境の選択肢としての有力候補の1つになった瞬間です。
