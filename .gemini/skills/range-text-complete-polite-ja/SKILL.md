---
name: range-text-complete-polite-ja
description: Complete and refine only the user-specified text range in Japanese prose. Use when asked to fill missing explanations, bridge unfinished sentences, or improve flow within a bounded section, while normalizing output to polite Japanese style (です・ます調). After editing, stage the changed files with git add.
---

# Range Text Complete Polite Ja

## Goal

指定された範囲の文章を補完し、文体をです・ます調に整える。作業後は変更ファイルを `git add` まで実行する。

## Workflow

1. 対象範囲を確定する。
- ユーザーが指定したファイル、見出し、行範囲、段落範囲のみを編集対象にする。
- 指定が曖昧でも、前後文脈から合理的に範囲を確定する。

2. 文章を補完する。
- 途切れた説明、論理の飛躍、接続不足を補い、読み手が理解できる状態まで整える。
- ユーザーの意図を変えない。新規の主張や未確認情報は追加しない。
- 画像パスに関してだけは修正と指摘を行わない

3. 文体を統一する。
- 編集対象範囲をです・ます調にそろえる。
- 用語、句読点、全角半角の表記揺れも必要に応じて整える。

4. 変更をステージする。
- 編集したファイルを `git add <file>` でステージする。
- 関係ないファイルはステージしない。

## Output Rules

- 最終出力では、補完・調整した要点を短く示す。
- `git add` を実行したファイルを明記する。
- 変更がなければ、その旨を明記し `git add` は実行しない。
