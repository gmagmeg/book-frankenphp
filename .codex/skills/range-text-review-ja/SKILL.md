---
name: range-text-review
description: Review and edit only the user-specified text range in prose. Use when asked to proofread a chapter/section/line range, fix typos and notation inconsistencies, and keep the original intent. If statements may be factually wrong or misleading, do not rewrite those claims; list them as findings.
---

# Range Text Review

## Goal

指定された範囲の文章だけをレビューし、誤字脱字・表記揺れを修正する。内容の正誤が疑わしい箇所は本文を直さず、指摘として分離して返す。

## Workflow

1. 対象範囲を特定する。
- ユーザー指定のファイル、見出し、行範囲、段落範囲以外は編集対象にしない。
- 範囲が曖昧な場合は、直前の指示とファイル構造から合理的に解釈する。

2. 文面を校正する。
- 誤字脱字、送り仮名、記号、全角半角、用語統一などの表記揺れを修正する。
- 文意を変えない。構成変更や大幅なリライトを避ける。
- 可読性のための改行位置の調整は許可する。

3. 事実性の懸念を抽出する。
- 日付、固有名詞、仕様、因果関係などで誤認の可能性がある記述を見つける。
- その箇所は本文に反映せず、指摘として列挙する。

4. 結果を2部構成で返す。
- `修正点`: どこをどう直したかを箇条書きで列挙する（全文は出力しない）。
- `要確認事項`: 事実誤認や根拠不足の可能性がある箇所の指摘（理由を1行で添える）。

## Output Rules

- 修正文の全文は出力しない。
- 修正不要なら `修正点` に「修正なし」と明記する。
- `要確認事項` がなければ「該当なし」と明記する。
- 変更理由は冗長に書かず、差分が分かる最小限にする。
