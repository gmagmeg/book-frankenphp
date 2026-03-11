# ページ増補アウトライン案

## 方針

ページ数を増やしつつ、書籍全体の流れを強化するため、次の3点を軸に増補する。

- 導入の補強
- アプリ全体像の可視化
- 運用・比較の具体化

特に次の章は、独立した節や図表を追加しやすく、ページを増やしやすい。

- `articles/md/01-what-is-frankenphp.md`
- `articles/md/04-first-app.md`
- `articles/md/05-103-early-hints.md`
- `articles/md/07-deploy.md`

また、次の2ファイルは現時点で空のため、前書き・後書き相当を追加できる。

- `articles/md/00-kakidashi.md`
- `articles/md/09-matome.md`

## 章別アウトライン

OK
### 1. `articles/md/00-kakidashi.md`

- `# はじめに`
- `## 本書の対象読者`
- `## 本書で作るアプリケーション`
- `## 本書を読むと得られること`
- `## 先に知っておいてほしいこと`

内容メモ:

- FrankenPHP未経験者向けであることを書く
- Laravel経験者がどこに注目するとよいかを書く
- 本書のゴールが、機能紹介だけではなく、実装を通じた導入判断にあることを明示する

### 2. `articles/md/01-what-is-frankenphp.md`

既存の `## PHP実行環境の変遷とFrankenPHPのアプローチ` の後に追加する。

- `## FrankenPHPはどんな場面で選びやすいか`
- `## 他の選択肢との比較`
- `## 本書でFrankenPHPを採用する理由`

図表案:

- `Apache + mod_php / Nginx + PHP-FPM / RoadRunner / Swoole / FrankenPHP` の比較表

内容メモ:

- 導入しやすさ
- 構成の単純さ
- Laravelとの相性
- 常駐アプリ前提の注意点

### 3. `articles/md/02-setup.md`

既存の `## 環境構築` の前後に追加する。

- `## この章で構築する開発環境の全体像`
- `## 各コンテナ・設定ファイルの役割`
- `## 起動コマンドを分解して読む`
- `## トラブルシューティング`

図表案:

- Docker構成図
- 設定ファイル一覧表

内容メモ:

- `compose.yaml`
- `.env`
- `start-container.sh`
- Octane
- PostgreSQL

これらがどうつながっているかを説明する。

### 4. `articles/md/03-jikkou-model.md`

既存の `## FrankenPHPの2つの実行モード` の後に追加する。

- `## 1リクエストが処理される流れ`
- `## Workerモードが効きやすいケース`
- `## Workerモードが難しくなるケース`
- `## Laravel Octaneと組み合わせる意味`

図表案:

- classic mode と Workerモードの時系列図

内容メモ:

- どこで初期化されるか
- どこが再利用されるか
- どの条件で性能差が出やすいか

後続の注意点章への橋渡しも兼ねる。

### 5. `articles/md/04-first-app.md`

既存の `## 作成するアプリケーションの概要` の後に追加する。

- `## システム全体の処理フロー`
- `## 画面と役割`
- `## 今回の実装でFrankenPHPが担当する範囲`
- `## CSV生成からダウンロードまでの流れ`
- `## この章ではまずMercure通知を作る`

図表案:

- 全体アーキテクチャ図
- シーケンス図
- 画面キャプチャ

内容メモ:

- CSV生成
- OTP通知
- ダウンロード

といった全体像を先に見せてから、Mercureの説明に入る構成にする。
本書の中心章として厚くする。

### 6. `articles/md/05-103-early-hints.md`

既存の `## 103 Early Hintsとは` の後に追加する。

- `## 通常の読み込みとの違い`
- `## Mercure接続でEarly Hintsを使う意味`
- `## ブラウザでの確認ポイント`
- `## 使いどころと使わなくてよい場面`

図表案:

- 通常時とEarly Hints利用時のネットワーク時系列図

内容メモ:

- 設定手順だけでなく、なぜMercureのケースで効くのかを補強する

### 7. `articles/md/06-x-sendfile.md`

既存章は十分な分量があるため、追記は少なめでよい。

- `## 本書のCSVダウンロード要件に当てはめる`
- `## OTP認証とX-Sendfileの責務分離`
- `## 実運用で確認したいポイント`

図表案:

- 認証処理とファイル配信の責務分離図

内容メモ:

- 単独の技術解説にとどめず、本書全体のアプリ要件と結びつける

### 8. `articles/md/07-deploy.md`

既存の `## 事前準備` の後に追加する。

- `## Render.comを選ぶ理由`
- `## デプロイ前チェックリスト`
- `## 環境変数一覧`
- `## 初回デプロイで確認するログ`
- `## よくある詰まりどころ`
- `## 公開後の動作確認`

表案:

- 環境変数一覧表
- 確認チェックリスト

内容メモ:

- 実務で必要な確認観点を足す
- デプロイ章はページを増やしても読みやすさを保ちやすい

### 9. `articles/md/08-attention.md`

既存の注意点列挙の前後に追加する。

- `## 導入前に確認したいこと`
- `## 開発中にハマりやすいポイント`
- `## 本番運用で意識したいポイント`
- `## FrankenPHPが向くプロジェクト / 向かないプロジェクト`

表案:

- 導入判断チェックリスト

内容メモ:

- 既知の問題の羅列だけでなく、採用判断の章として読めるようにする

### 10. `articles/md/09-matome.md`

- `# まとめ`
- `## 本書で扱ったFrankenPHPの強み`
- `## 実装を通じて見えた注意点`
- `## どんな案件で試すとよいか`
- `## 次に読むとよい資料`

内容メモ:

- 読後の整理
- 次の行動につなげる締め

## 優先度

### 優先度高

1. `articles/md/04-first-app.md`
2. `articles/md/03-jikkou-model.md`
3. `articles/md/07-deploy.md`
4. `articles/md/01-what-is-frankenphp.md`
5. `articles/md/00-kakidashi.md`
6. `articles/md/09-matome.md`

### 優先度中

1. `articles/md/05-103-early-hints.md`
2. `articles/md/08-attention.md`
3. `articles/md/02-setup.md`

### 優先度低

1. `articles/md/06-x-sendfile.md`

## おすすめ実装順

1. `04-first-app.md`
2. `03-jikkou-model.md`
3. `07-deploy.md`
4. `01-what-is-frankenphp.md`
5. `00-kakidashi.md` と `09-matome.md`

