# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 目的・書籍内容

Re:VIEW 5.8 ベースの技術書プロジェクト。FrankenPHP の入門書（日本語）で、以下のアプリケーションを作りながら解説する:
- バックグラウンドでのCSV生成
- ワンタイムパスワード付きpush通知
- X-Sendfile によるセキュアなファイルダウンロード

## ビルドコマンド

```bash
# 初回セットアップ
bundle install
npm install

# ビルド
npm run pdf     # PDF生成
npm run epub    # EPUB生成
npm run web     # HTML生成
npm run text    # プレーンテキスト生成

# CSS再ビルド（.scssを変更した場合）
./rebuild-css.sh

# Docker環境でビルド（TeX環境なしの場合）
./build-in-docker.sh
```

設定変更・ビルド系を触った後は `npm run pdf` で動作確認する。

## アーキテクチャ

### ビルドフロー

```
articles/md/*.md
    ↓ (z00_autogen_md.rake → z01_pandoc2review.rake)
articles/_refiles/*.re
    ↓ (まとめて取り込み)
articles/_autogen_md_chapters.re
    ↓ (review-preproc → review-compile → rake)
PDF / EPUB / HTML
```

### 原稿ソース

- **一次ソース**: `articles/md/*.md`（Markdownで執筆）
- **生成物（直接編集禁止）**: `articles/_refiles/`, `articles/_autogen_md_chapters.re`
- **章構成**: `articles/catalog.yml`（PREDEF/CHAPS/APPENDIX/POSTDEF）
- **書誌情報・出力設定**: `articles/config.yml`

### 主な設定ファイル

| ファイル | 役割 |
|---------|------|
| `articles/config.yml` | 書誌情報、用紙サイズ、texdocumentclass 設定 |
| `articles/catalog.yml` | 章の順序・構成定義 |
| `articles/config-ebook.yml` | 電子書籍向け設定（config.yml を継承） |
| `Gruntfile.js` | ビルドパイプライン（REVIEW_PREFIX等の環境変数対応） |
| `articles/lib/tasks/*.rake` | Rake タスク定義 |

## 編集ポリシー

- 原稿修正は `articles/md/*.md` を編集する（`.re`ファイルは生成物）
- 設定修正は `articles/config.yml`, `articles/catalog.yml`, `Gruntfile.js`
- 生成物（`articles/_refiles/`, `articles/_autogen_md_chapters.re`）は直接編集しない
- 文字コードは UTF-8 を維持する
- 既存の YAML キーや Re:VIEW 記法は互換性を崩さない

## スタイル・校正

- `prh-rules/` に用語統一ルール（prh）があり、`articles/prh.yml` で設定
- `redpen-conf-ja.xml` で日本語テキストのlint設定
- CSS/EPUBスタイルは `articles/style.scss`, `articles/epub_style.scss`, `articles/style-web.scss`
