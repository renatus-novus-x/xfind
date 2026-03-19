# XFIND 実装計画

## 1. 目的

この文書は、`spec.md` を実装するための作業順、成果物、確認項目を定める。
動作仕様そのものは `spec.md` を正とする。

## 2. 完成物

実装完了時に、少なくとも次を含むこと。

- `src/` 配下の C ソース一式
- ルートの `CMakeLists.txt`
- `x68k/Makefile`
- `.github/workflows/build.yml`
- `README.md`

## 3. ディレクトリ構成

想定構成:

```text
.
├── CMakeLists.txt
├── README.md
├── spec.md
├── impl_plan.md
├── claude_prompt.md
├── .github/
│   └── workflows/
│       └── build.yml
├── src/
│   ├── MXFIDX.C
│   ├── MXFIND.C
│   ├── FSWALK.C
│   ├── FSWALK.H
│   ├── INDEX.C
│   ├── INDEX.H
│   ├── MATCH.C
│   ├── MATCH.H
│   ├── ACTIONS.C
│   ├── ACTIONS.H
│   ├── CONFIG.C
│   ├── CONFIG.H
│   ├── UITUI.C
│   ├── UITUI.H
│   └── PLATFORM.H
└── x68k/
    └── Makefile
```

## 4. フェーズ

### Phase 0: 骨組み作成

- すべてのソースとヘッダを追加する
- `xfidx` と `xfind` が空実装でもビルドできる状態にする
- `PLATFORM.H` に OS 判定の入口を置く

確認:

- Ubuntu でビルドできる
- 将来 X68000 用ファイルを差し込みやすい

### Phase 1: インデックス生成

- 再帰走査を実装する
- テキスト形式インデックス書き出しを実装する
- エラーがあっても継続する流れを作る

確認:

- 複数ルートからインデックスを生成できる
- 出力順が安定している

### Phase 2: インデックス読込と検索

- インデックス読込を実装する
- ベース名 / フルパス検索を実装する
- 安定ソートを実装する

確認:

- 同じクエリで同じ順序の結果が得られる
- 大文字小文字を区別しない検索ができる

### Phase 3: TUI

- 候補一覧表示を実装する
- カーソル移動、Enter、`c`、`q` を実装する

確認:

- 一覧から `open` と `cd` を呼び出せる

### Phase 4: アクション

- `open` を実装する
- `cd` を実装する
- Ubuntu と X68000 の差分を `PLATFORM.H` 経由で処理する

確認:

- ファイルに対する `open` が動く
- ディレクトリに対する `cd` が動く

### Phase 5: ビルド整備

- ルートに `CMakeLists.txt` を追加する
- `x68k/Makefile` を追加する
- GitHub Actions を追加する
- `README.md` にビルド方法と使い方を書く

確認:

- Ubuntu で自動ビルドできる
- X68000 向けビルド手順が明示されている

## 5. モジュール責務

- `MXFIDX.C`: `xfidx` の入口
- `MXFIND.C`: `xfind` の入口
- `FSWALK.C/H`: 再帰走査
- `INDEX.C/H`: インデックス入出力
- `MATCH.C/H`: 検索と並び替え
- `ACTIONS.C/H`: `open` / `cd` 呼び出し
- `CONFIG.C/H`: 設定読込
- `UITUI.C/H`: TUI
- `PLATFORM.H`: OS 依存差分の入口

## 6. 実装上の制約

- なるべく標準 C に寄せる
- 重い外部ライブラリは使わない
- Ubuntu 専用 API をコアに散らさない
- X68000 依存の知識は `PLATFORM.H` を境界に閉じ込める
- 途中段階でもビルド可能な小さな差分で進める

## 7. 受け入れ条件

最低限、次を満たしたら完了とする。

- Ubuntu で `xfidx` が動く
- Ubuntu で `xfind` が検索できる
- TUI から `open` と `cd` を呼べる
- `CMakeLists.txt` が動く
- GitHub Actions で Ubuntu ビルドが通る
- X68000 向けの `x68k/Makefile` が追加されている
