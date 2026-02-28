# AGENTS.md

このファイルは、`ReVIEW-Template` リポジトリで作業するエージェント向けの実務ガイドです。

## 目的
- Re:VIEW 5.8 ベースの書籍テンプレートを維持する
- 原稿（`.re` / `md/*.md`）とビルド設定（YAML, Rake, Grunt）を安全に更新する
- 変更後に最低限のビルド確認を行う

## リポジトリ概要
- ルート:
  - `package.json` / `Gruntfile.js`: `npm run <target>` でビルド実行
  - `Gemfile`: `review` / `rake` / `pandoc2review` などの Ruby 依存
- `articles/`:
  - `config.yml`: 書誌情報・出力設定の中心
  - `catalog.yml`: 章構成（PREDEF/CHAPS/APPENDIX/POSTDEF）
  - `*.re`: Re:VIEW 原稿
  - `md/*.md`: Markdown 原稿（自動で `.re` に取り込み）
  - `_refiles/`: 生成物（`pandoc2review` により作成される）
  - `_autogen_md_chapters.re`: 生成物（`md/*.md` から自動生成）
  - `lib/tasks/*.rake`: ビルドフロー定義

## セットアップ
1. Node/Ruby 依存を導入する
   - `npm install`
   - `bundle install`
2. 必要に応じて TeX / Docker 環境を利用する

## 基本コマンド
- PDF: `npm run pdf`
- EPUB: `npm run epub`
- Web: `npm run web`
- Text: `npm run text`
- HTML（簡易確認）: `npm run html`
- Docker ビルド: `./build-in-docker.sh`

## 編集ポリシー
- まず編集対象の一次ソースを変更する
  - 原稿修正: `articles/*.re` または `articles/md/*.md`
  - 設定修正: `articles/config.yml`, `articles/catalog.yml`, `Gruntfile.js`, `articles/lib/tasks/*.rake`
- 原則として生成物は直接編集しない
  - `articles/_refiles/*`
  - `articles/_autogen_md_chapters.re`
  - 各種ビルド成果物（`*.pdf`, `*.epub`, `webroot/`, `*-text/`, `*-idgxml/`）
- 既存の YAML キーや Re:VIEW 記法は、意図がない限り互換性を崩さない
- 文字コードは UTF-8 を維持する

## 変更時の確認
- 変更内容に応じて最低1つ以上のビルドを実行する
- 設定・ビルド系を触った場合は `npm run html` に加え、影響範囲のターゲット（例: `npm run pdf`）も実行する
- 失敗した場合は、再現コマンドとエラー要点を記録する

## 作業範囲外
- 依頼がない限り、ライセンス表記や大規模なツールチェイン更新（Re:VIEWメジャーバージョン変更等）は行わない
- 無関係ファイルの整形・リネーム・削除は行わない

