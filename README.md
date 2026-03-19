# XFIND

テキストインデックスを使って高速にファイルやディレクトリを検索する CLI/TUI ツールです。
Ubuntu と X68000 / Human68k の両プラットフォームに対応した設計になっています。

## コマンド

| コマンド | 役割 |
|----------|------|
| `xfidx`  | ディレクトリを再帰走査してインデックスを生成する |
| `xfind`  | インデックスを検索し、結果を TUI で選択して開く  |

## クイックスタート (Ubuntu)

```sh
# ビルド
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# インデックス生成
build/xfidx /usr/bin /home/$USER/src

# TUI で検索・選択
build/xfind readme
```

## `xfidx` 使い方

```text
xfidx [-o OUTFILE] [-c CONFIG] [-q] ROOT [ROOT ...]

  -o OUTFILE   出力インデックスファイル (デフォルト: xfind.idx)
  -c CONFIG    設定ファイル
  -q           進捗表示を抑制
```

例:

```sh
xfidx /usr/bin
xfidx -o ~/my.idx ~/src ~/doc
```

## `xfind` 使い方

```text
xfind [-i INDEXFILE] [-c CONFIG] [--open|--cd] [QUERY]

  -i INDEXFILE   インデックスファイル (デフォルト: xfind.idx → ~/.xfind.idx の順に探索)
  -c CONFIG      設定ファイル
  --open         非対話: ベストマッチを開く
  --cd           非対話: ベストマッチへ cd する (シェルラッパー必要、後述)
  QUERY          検索語 (省略時は全件表示)
```

### TUI キー操作

| キー | 操作 |
|------|------|
| `j` / `↓` | カーソルを下に移動 |
| `k` / `↑` | カーソルを上に移動 |
| `Enter`   | 選択項目を `open` する |
| `c`       | 選択項目へ `cd` する (シェルラッパー必要) |
| `q` / `ESC` | 終了 |

## `cd` の設定

`xfind` は子プロセスなので親シェルのカレントディレクトリを直接変更できません。
`cd` キーを押すと `/tmp/xfind_cd` にパスを書き出します。
以下のシェル関数を `~/.bashrc` または `~/.zshrc` に追加してください。

```sh
xcd() {
    xfind "$@"
    local p
    p=$(cat /tmp/xfind_cd 2>/dev/null)
    if [ -n "$p" ]; then
        cd "$p" && rm -f /tmp/xfind_cd
    fi
}
```

以後 `xfind` の代わりに `xcd readme` と打つと、`cd` アクションで実際にシェルが移動します。

## インデックス形式

テキスト形式で人間が読める内容です。

```text
XFIND-INDEX 1
ROOT /usr/bin
ENTRY F /usr/bin/ls
ENTRY F /usr/bin/grep
ENTRY D /usr/bin
```

## ビルド

### Ubuntu (CMake)

```sh
cmake -S . -B build
cmake --build build
# インストール (省略可)
sudo cmake --install build
```

### X68000 / Human68k

`m68k-elf-gcc` などのクロスコンパイラが必要です。

```sh
cd x68k
make
# → xfidx.x  xfind.x が生成される
```

`x68k/Makefile` の `CC` と `LDFLAGS` をツールチェーンに合わせて編集してください。

## ソース構成

```text
src/
  PLATFORM.H   OS 依存差分の入口 (パス区切り、termios/DOS raw mode など)
  FSWALK.H/C   再帰ディレクトリ走査
  INDEX.H/C    インデックスの読み書き
  MATCH.H/C    検索とランキング
  ACTIONS.H/C  open / cd アクション
  CONFIG.H/C   設定ファイル読み込み
  UITUI.H/C    TUI (ANSI エスケープシーケンスベース)
  MXFIDX.C     xfidx のエントリポイント
  MXFIND.C     xfind のエントリポイント
```

## 検索の優先順位

1. ベース名の完全一致
2. ベース名の前方一致
3. ベース名の部分一致
4. フルパスの部分一致
5. ベース名が短いもの優先
6. フルパスが短いもの優先
7. 辞書順

比較はすべて大文字小文字を区別しません。

## 未実装 / スコープ外

- バイナリインデックス
- 内容全文検索
- 正規表現検索
- 学習ランキング
- 複数選択
- 常駐監視・自動再索引
- GUI
- 実機 X68000 でのテスト
