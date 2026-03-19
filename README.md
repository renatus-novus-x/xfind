# XFIND

テキストインデックスを使って高速にファイルやディレクトリを検索する CLI/TUI ツールです。
**Ubuntu** と **X68000 / Human68k** の両プラットフォームに対応しています。

## コマンド

| コマンド | 役割 |
|----------|------|
| `xfidx`  | ディレクトリを再帰走査してインデックスを生成する |
| `xfind`  | インデックスを検索し、結果を TUI で選択して開く  |

---

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

## クイックスタート (X68000)

```sh
# m68k-xelf-gcc が PATH にある場合
cd x68k
make

# ツールチェーンの場所を明示する場合
make TOOLCHAIN=/path/to/m68k-xelf

# X68000 実機またはエミュレータ上で
XFIDX.X C:\BIN C:\DOC
XFIND.X readme
```

---

## `xfidx` 使い方

```text
xfidx [-o OUTFILE] [-c CONFIG] [-q] ROOT [ROOT ...]

  -o OUTFILE   出力インデックスファイル (デフォルト: xfind.idx)
  -c CONFIG    設定ファイル
  -q           進捗表示を抑制
```

例:

```sh
# Ubuntu
xfidx /usr/bin
xfidx -o ~/my.idx ~/src ~/doc

# X68000
XFIDX.X -o MYIDX.IDX C:\BIN C:\DOC
```

## `xfind` 使い方

```text
xfind [-i INDEXFILE] [-c CONFIG] [-O OPENER] [--open|--cd] [QUERY]

  -i INDEXFILE        インデックスファイル (デフォルト: xfind.idx → ~/.xfind.idx)
  -c FILE             設定ファイル
  -O CMD, --opener CMD  オープナコマンド (後述の優先順位を上書き)
  --open              非対話: ベストマッチを開く
  --cd                非対話: ベストマッチへ cd する (シェルラッパー必要、後述)
  QUERY               検索語 (省略時は全件表示)
```

### TUI キー操作

| キー | 操作 |
|------|------|
| `j` / `↓` | カーソルを下に移動 |
| `k` / `↑` | カーソルを上に移動 |
| `Enter`   | 選択項目を `open` する |
| `c`       | 選択項目へ `cd` する (後述のシェルラッパー必要) |
| `q` / `ESC` | 終了 |

---

## `cd` の設定

`xfind` は子プロセスなので親シェルのカレントディレクトリを直接変更できません。
`c` キーを押すと `/tmp/xfind_cd` にパスを書き出します。
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

以後 `xfind` の代わりに `xcd readme` と打つと、`c` アクションで実際にシェルが移動します。

---

## オープナの設定

非実行ファイルを開く際に使うコマンド（オープナ）は次の順で決定します。

| 優先順位 | 手段 | 例 |
|---------|------|----|
| 1 | `-O` / `--opener` オプション | `xfind -O zathura readme.pdf` |
| 2 | 設定ファイルの `open_cmd` | `open_cmd = evince` |
| 3 | 環境変数 `XFIND_OPEN` | `export XFIND_OPEN=less` |
| 4 | 自動検出 | `xdg-open` → `open` → `mimeopen` の順 |
| 5 | フォールバック | パスを標準出力に表示して終了 (exit 0) |

`xdg-open` は Ubuntu の標準インストールに含まれない場合があります。
その場合は設定ファイルまたは `XFIND_OPEN` でオープナを指定してください。

設定ファイル (`xfind.cfg`) の例:

```text
open_cmd = xdg-open
```

環境変数での設定例:

```sh
export XFIND_OPEN=xdg-open
```

コマンドラインでの一時指定:

```sh
xfind -O "evince" report
xfind --opener "less" readme
```

---

## インデックス形式

テキスト形式で人間が読める内容です。Ubuntu / X68000 で共通です。

```text
XFIND-INDEX 1
ROOT /usr/bin
ENTRY F /usr/bin/ls
ENTRY F /usr/bin/grep
ENTRY D /usr/bin
```

---

## ビルド

### Ubuntu (CMake)

CMake 3.14 以上と GCC または Clang が必要です。

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# インストール (省略可)
sudo cmake --install build
```

### X68000 / Human68k

[elf2x68k](https://github.com/yunkya2/elf2x68k) ツールチェーン (`m68k-xelf-gcc`) が必要です。

```sh
cd x68k
make
```

`m68k-xelf-gcc` が PATH にない場合は `TOOLCHAIN` でインストール先を指定してください。

```sh
make TOOLCHAIN=/path/to/m68k-xelf
```

`make` が成功すると `x68k/xfidx.x` と `x68k/xfind.x` が生成されます。
これらは X68K 実行形式（`.X` ファイル）で、実機またはエミュレータ上で直接実行できます。

#### ツールチェーンについて

- コンパイラ: `m68k-xelf-gcc`（GCC 13 ベース）
- リンカラッパー: `m68k-xelf-ld.x`（ELF → X68K 形式へ自動変換）
- 変換ツール: `elf2x68k.py`
- X68K ヘッダ: `<x68k/dos.h>`、`<x68k/iocs.h>`
- プラットフォーム検出: コンパイラが `__m68k__` を自動定義

---

## ソース構成

```text
.
├── CMakeLists.txt          Ubuntu 向け CMake ビルド
├── README.md
├── spec.md                 仕様書
├── impl_plan.md            実装計画
├── PROGRESS.md             実装進捗記録
├── .github/
│   └── workflows/
│       └── build.yml       Ubuntu 自動ビルド (GitHub Actions)
├── src/
│   ├── PLATFORM.H          プラットフォーム検出・共通ユーティリティ
│   ├── FSWALK.H/C          再帰ディレクトリ走査 (POSIX opendir/readdir)
│   ├── INDEX.H/C           インデックスの読み書き
│   ├── MATCH.H/C           検索とランキング
│   ├── ACTIONS.H/C         open / cd アクション
│   ├── CONFIG.H/C          設定ファイル読み込み
│   ├── UITUI.H/C           TUI (ANSI エスケープ + termios / _dos_getchar)
│   ├── MXFIDX.C            xfidx のエントリポイント
│   └── MXFIND.C            xfind のエントリポイント
└── x68k/
    └── Makefile            X68000 クロスビルド
```

### プラットフォーム別の実装差分

ファイル走査・インデックス入出力・検索・TUI 表示のコアは両プラットフォームで共通です。
差分は `PLATFORM.H` のマクロで制御します。

| 機能 | Ubuntu | X68000 |
|------|--------|--------|
| プラットフォーム検出 | `PLATFORM_POSIX` | `PLATFORM_X68K` (`__m68k__` 自動定義) |
| ディレクトリ走査 | POSIX `opendir` / `readdir` | 同上（ツールチェーン提供） |
| ターミナル raw mode | POSIX `termios` | `_dos_getchar()` |
| ファイルを開く | `xdg-open` / 直接実行 | `_dos_exec2()` |
| パス区切り | `/` | `\` |
| パス最大長 | 1024 | 96 |
| 生成ファイル名 | `xfidx` / `xfind` | `xfidx.x` / `xfind.x` |

---

## 検索の優先順位

1. ベース名の完全一致
2. ベース名の前方一致
3. ベース名の部分一致
4. フルパスの部分一致
5. ベース名が短いもの優先
6. フルパスが短いもの優先
7. 辞書順

比較はすべて大文字小文字を区別しません。

---

## 未実装 / スコープ外

- バイナリインデックス
- 内容全文検索
- 正規表現検索
- 学習ランキング
- 複数選択
- 常駐監視・自動再索引
- GUI
- 実機 X68000 でのテスト（クロスビルドは確認済み）
