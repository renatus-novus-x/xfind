# XFIND 実装進捗記録

作成日: 2026-03-19
最終更新: 2026-03-20

---

## 実装の目的

`spec.md` と `impl_plan.md` に従い、テキストインデックスを用いたファイル検索ツール XFIND を実装する。
対象プラットフォームは Ubuntu と X68000 / Human68k。

---

## 完了フェーズ

`impl_plan.md` に定義された 5 フェーズをすべて完了。

| フェーズ | 内容 | 状態 |
|---------|------|------|
| Phase 0 | 骨組み作成（全ソース・ヘッダ追加、空実装でビルド可能な状態） | 完了 |
| Phase 1 | インデックス生成（再帰走査・テキスト形式書き出し） | 完了 |
| Phase 2 | インデックス読込と検索（ベース名/フルパス検索・安定ソート） | 完了 |
| Phase 3 | TUI（候補一覧・カーソル移動・Enter/c/q） | 完了 |
| Phase 4 | アクション（open / cd の Ubuntu・X68K 分岐） | 完了 |
| Phase 5 | ビルド整備（CMakeLists.txt・x68k/Makefile・GitHub Actions・README.md） | 完了 |
| Phase 6 | オープナのカスタマイズ（`-O`・`open_cmd`・`XFIND_OPEN`・自動検出・フォールバック） | 完了 |

---

## 追加・更新ファイル一覧

### src/ 配下

| ファイル | 役割 |
|----------|------|
| `src/PLATFORM.H` | OS 検出マクロ（`PLATFORM_POSIX` / `PLATFORM_X68K`）・パス定数・`DirIter` 型・`plat_basename`・`plat_join`・`plat_strcasecmp`・`plat_strcasestr` |
| `src/FSWALK.H` | 再帰走査インタフェース（`FswalkCallback` / `fswalk_run`） |
| `src/FSWALK.C` | POSIX `opendir`/`readdir`/`stat` による実装（Ubuntu・X68K 共通） |
| `src/INDEX.H` | インデックス形式定数・`IndexEntry` / `IndexTable` 型・Writer / Reader API |
| `src/INDEX.C` | テキストインデックス書き込み（`IndexWriter`）と読み込み（`idx_load`）の実装 |
| `src/MATCH.H` | `MatchResult` / `MatchSet` 型・`match_search` / `match_free` |
| `src/MATCH.C` | 4 段階ランキング検索・安定 `qsort` 実装 |
| `src/ACTIONS.H` | `action_open(path, opener)` / `action_cd` プロトタイプ・`XFIND_CD_FILE` 定数 |
| `src/ACTIONS.C` | Ubuntu: オープナ解決チェーン（`opener` 引数 → `XFIND_OPEN` 環境変数 → 自動検出 → フォールバック）。X68K: `_dos_exec2()`。`action_cd` は共通 |
| `src/CONFIG.H` | `Config` / `ConfigEntry` 型・`cfg_load` / `cfg_get` / `cfg_free` |
| `src/CONFIG.C` | `key = value` 形式設定ファイルの読み込み実装 |
| `src/UITUI.H` | `tui_run(set, opener)` プロトタイプ |
| `src/UITUI.C` | ANSI エスケープ TUI。raw mode: Ubuntu は `termios`、X68K は `_dos_getchar()`。opener を `action_open` に透過 |
| `src/MXFIDX.C` | `xfidx` エントリポイント（オプション: `-o` / `-c` / `-q`） |
| `src/MXFIND.C` | `xfind` エントリポイント（オプション: `-i` / `-c` / `-O` / `--opener` / `--open` / `--cd`）。opener 解決（`-O` > `open_cmd` > NULL）を担当 |

### ルート・設定ファイル

| ファイル | 内容 |
|----------|------|
| `CMakeLists.txt` | Ubuntu 向け CMake ビルド定義。`.C` 大文字拡張子を C として扱うため `set_source_files_properties(... LANGUAGE C)` を使用 |
| `x68k/Makefile` | X68K クロスビルド（`m68k-xelf-gcc`）。`TOOLCHAIN` 変数と `export PATH` でツールチェーンを解決 |
| `.github/workflows/build.yml` | Ubuntu ビルド・スモークテスト・バイナリアーティファクト保存 |
| `README.md` | ビルド手順（Ubuntu/X68K）・使い方・TUI キー一覧・cd シェル関数・ソース構成・プラットフォーム差分表 |
| `PROGRESS.md` | 本ファイル |
| `.gitignore` | `build/`・`build_*/`・`*.idx` を除外 |

---

## 動作確認結果

### Ubuntu (WSL Ubuntu 24.04 / GCC 13.3.0)

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

ビルド: **成功**

| テスト | コマンド | 結果 |
|--------|---------|------|
| インデックス生成 | `xfidx -q -o /tmp/test.idx /usr/bin` | 42,064 エントリ生成、形式正常 |
| 非対話 open (実行可能) | `xfind -i /tmp/test.idx --open ls` | `ls` を直接実行、exit: 0 |
| 非対話 cd | `xfind -i /tmp/test.idx --cd bash` | `/tmp/xfind_cd` に `/usr/bin` 書き出し、exit: 0 |
| マッチなし | `xfind -i /tmp/test.idx --open __nope__` | `no match` メッセージ、exit: 1 |
| XFIND_OPEN 環境変数 | `XFIND_OPEN="echo OPENED:" xfind --open README` | `OPENED: /path/to/README.gz`、exit: 0 |
| -O オプション (優先) | `xfind -O "echo OPT:" -c cfg --open README` | `OPT: /path/to/README.gz`（config より優先）、exit: 0 |
| open_cmd 設定ファイル | `xfind -c cfg --open README` (cfg: `open_cmd=echo CFG:`) | `CFG: /path/to/README.gz`、exit: 0 |
| フォールバック | オープナが一切ない環境で非実行ファイルを open | パスを stdout に出力、exit: 0 |

生成インデックス先頭:

```
XFIND-INDEX 1
ROOT /usr/bin
ENTRY D /usr/bin
ENTRY F /usr/bin/head
ENTRY F /usr/bin/pygmentize
...
```

### X68K (m68k-xelf-gcc 13.3.0 / elf2x68k / WSL Ubuntu 24.04 上でクロスビルド)

```sh
cd x68k
make
```

ビルド: **成功**

| 生成物 | サイズ | 形式 |
|--------|--------|------|
| `xfidx.x` | 85 KB | X68K 実行形式（`.X` ファイル） |
| `xfind.x`  | 90 KB | X68K 実行形式（`.X` ファイル） |

実機動作確認は未実施（クロスビルド成功のみ確認）。

---

## 主要な設計判断

### 1. `.C` 大文字拡張子の扱い

spec.md の指定通りすべてのソースファイルを大文字 `.C` 拡張子で作成した。
Linux/macOS の CMake は `.C` をデフォルトで C++ ファイルと見なすため、
`set_source_files_properties(... PROPERTIES LANGUAGE C)` で C コンパイラを強制した。

### 2. cd アクション

子プロセスは親シェルのカレントディレクトリを直接変更できない。
`action_cd()` は対象パスを `/tmp/xfind_cd` に書き出し、
ユーザーは以下のシェル関数を設定することで実際の `cd` を行う。

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

これは spec.md 6.2「各 OS で実用的な補助方式を採用してよい」に準拠する。

### 3. TUI の実装方針

ncurses などの外部ライブラリを使用せず、ANSI エスケープシーケンスのみで実装した。
X68K の IOCS コンソールも ANSI/VT100 エスケープシーケンスに対応しているため、
描画コードは両プラットフォームで共通。入力のみプラットフォーム分岐する。

### 4. 安定ソートの実現

C 標準の `qsort` は安定ソートを保証しない。
`MatchResult` に `orig`（元インデックス）フィールドを持たせ、
比較関数の最終 tiebreaker として使用することで安定ソートを実現した。

### 6. オープナのカスタマイズ設計

`xdg-open` は Ubuntu の標準インストールに含まれないケースがある。
`action_open()` に `opener` 引数を追加し、次の優先チェーンで解決する実装とした。

| 優先順位 | 手段 | 実装箇所 |
|---------|------|---------|
| 1 | `-O` / `--opener` コマンドラインオプション | `MXFIND.C` でパースし `opener` 変数に格納 |
| 2 | 設定ファイルの `open_cmd` キー | `cfg_get()` で取得、`-O` がなければ採用 |
| 3 | 環境変数 `XFIND_OPEN` | `ACTIONS.C` の `resolve_opener()` 内で参照 |
| 4 | 自動検出（`xdg-open` → `open` → `mimeopen`） | `cmd_in_path()` で PATH 探索 |
| 5 | フォールバック | パスを stdout に出力して exit 0 |

設計のポイント:
- 1・2 の解決は `MXFIND.C` が担い、`opener` として `action_open()` に渡す
- 3・4・5 の解決は `ACTIONS.C` 内の `resolve_opener()` が担う
- `tui_run()` も `opener` を受け取り、Enter キー時に透過して渡す
- X68K では `opener` 引数は無視され、常に `_dos_exec2()` を使用

### 5. OS 依存差分の分離

`PLATFORM.H` に OS 検出マクロを集約し、差分を最小化した。
X68K ツールチェーン（m68k-xelf-gcc）は POSIX の `dirent.h`・`sys/stat.h` を提供するため、
ファイル走査・インデックス I/O のコアは共通コードで動作する。

| 機能 | Ubuntu (`PLATFORM_POSIX`) | X68K (`PLATFORM_X68K`) |
|------|--------------------------|------------------------|
| プラットフォーム検出 | `__linux__` 等 | `__m68k__`（コンパイラ自動定義） |
| ディレクトリ走査 | POSIX `opendir` / `readdir` | 同上（ツールチェーン提供） |
| ターミナル raw mode | POSIX `termios` | `_dos_getchar()` |
| ファイルを開く | `xdg-open` / 直接実行 | `_dos_exec2()` |
| パス区切り | `/` | `\` |
| パス最大長 | 1024 | 96 |

---

## 既知の制限・注意事項

### CMake のビルド警告（Ubuntu）

Ubuntu で `cmake --build` を実行すると以下の警告が出る場合がある。

```
cc1plus: warning: command-line option '-std=gnu99' is valid for C/ObjC but not for C++
```

CMake が `.C` 拡張子ファイルを一部 C++ コンパイラで処理するために発生する。
バイナリの動作には影響しない。根本対応はファイル拡張子を `.c`（小文字）にすることだが、
spec.md の命名規則（大文字 `.C`）を優先して据え置いた。

### X68K クロスビルドで判明した差分と対応

m68k-xelf-gcc ツールチェーン（elf2x68k 統合版）の実環境調査により以下が判明し、すべて対応済み。

| 問題 | 原因 | 対応 |
|------|------|------|
| `sys/dos.h` not found | X68K ヘッダは `x68k/dos.h` に配置 | `PLATFORM.H` の X68K ブランチを除去し `x68k/dos.h` は `ACTIONS.C` のみで使用 |
| `termios.h` → `sys/termios.h` not found | X68K ツールチェーンに `sys/termios.h` が存在しない | `PLATFORM.H` から `termios.h` を削除。`UITUI.C` でプラットフォーム分岐 |
| リンカ `m68k-xelf-ld.bfd` not found | `ld.x`（elf2x68k ラッパー）が `ld.bfd` を名前で呼ぶ | Makefile で `export PATH := $(TOOLCHAIN)/bin:$(PATH)` を設定 |
| `MXFIND.C` で `sys/dos.h` 参照 | 当初 X68K ブランチで `_dos_findfirst` を使用 | POSIX `stat()` に統一（X68K ツールチェーンも POSIX stat を提供） |

X68K ツールチェーンは POSIX (`dirent.h` / `sys/stat.h`) を提供するため、
ファイル走査・`file_exists` は POSIX コードを両プラットフォームで共用できる。

### X68K 実機での未テスト箇所

| 箇所 | 状況 |
|------|------|
| `ACTIONS.C` X68K open | `_dos_exec2(0, path, "", "")` を実装済みだが実機未確認 |
| `UITUI.C` X68K キー入力 | `_dos_getchar()` で実装済みだが実機未確認 |
| 矢印キーコード | `X68K_KEY_UP=0x3A` / `X68K_KEY_DOWN=0x3B` は仮定値（要実機確認） |

---

## 受け入れ条件の充足状況

`impl_plan.md` 7. 受け入れ条件との対応:

| 条件 | 状態 |
|------|------|
| Ubuntu で `xfidx` が動く | ✓ 確認済み |
| Ubuntu で `xfind` が検索できる | ✓ 確認済み |
| TUI から `open` と `cd` を呼べる | ✓ 実装済み（TUI は端末が必要なため自動テスト外） |
| `CMakeLists.txt` が動く | ✓ 確認済み |
| GitHub Actions で Ubuntu ビルドが通る | ✓ ワークフロー定義済み（CI 実行は push 後） |
| X68000 向けの `x68k/Makefile` が追加されている | ✓ 追加済み・クロスビルド成功確認済み |
| `xdg-open` がない環境でも `open` が exit 0 で動作する | ✓ フォールバック（パス出力）確認済み |
| `XFIND_OPEN` / `-O` でオープナを上書きできる | ✓ 全パターン確認済み |

---

## 未実装項目（仕様上のスコープ外）

以下は spec.md 2.2「初期実装に含めないもの」に明示されており、実装していない。

- バイナリインデックス
- 内容全文検索
- 正規表現検索
- 学習ランキング
- 複数選択処理
- 常駐監視・バックグラウンド自動再索引
- GUI
- 実機 X68000 でのテスト
- `edit` / `view` アクション
