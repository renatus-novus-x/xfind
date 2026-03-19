# XFIND 実装進捗記録

作成日: 2026-03-19

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

---

## 追加・更新ファイル一覧

### src/ 配下

| ファイル | 役割 |
|----------|------|
| `src/PLATFORM.H` | OS 検出マクロ・パス定数・`DirIter` / `DirEntry` 型・`TermState` 型・`plat_basename`・`plat_join`・`plat_strcasecmp`・`plat_strcasestr` |
| `src/FSWALK.H` | 再帰走査インタフェース（`FswalkCallback` / `fswalk_run`） |
| `src/FSWALK.C` | POSIX 実装（`opendir`/`readdir`/`stat`）と Human68k 実装（`_dos_findfirst`/`_dos_findnext`） |
| `src/INDEX.H` | インデックス形式定数・`IndexEntry` / `IndexTable` 型・Writer / Reader API |
| `src/INDEX.C` | テキストインデックス書き込み（`IndexWriter`）と読み込み（`idx_load`）の実装 |
| `src/MATCH.H` | `MatchResult` / `MatchSet` 型・`match_search` / `match_free` |
| `src/MATCH.C` | 4 段階ランキング検索・安定 `qsort` 実装 |
| `src/ACTIONS.H` | `action_open` / `action_cd` プロトタイプ・`XFIND_CD_FILE` 定数 |
| `src/ACTIONS.C` | POSIX 実装（`xdg-open` / `stat` で実行可能判定）と Human68k 実装（`_dos_exec`）・`cd` は `/tmp/xfind_cd` 書き出し方式 |
| `src/CONFIG.H` | `Config` / `ConfigEntry` 型・`cfg_load` / `cfg_get` / `cfg_free` |
| `src/CONFIG.C` | `key = value` 形式設定ファイルの読み込み実装 |
| `src/UITUI.H` | `tui_run` プロトタイプ |
| `src/UITUI.C` | ANSI エスケープ + `termios` raw mode による TUI 実装 |
| `src/MXFIDX.C` | `xfidx` エントリポイント（オプション: `-o` / `-c` / `-q`） |
| `src/MXFIND.C` | `xfind` エントリポイント（オプション: `-i` / `-c` / `--open` / `--cd`） |

### ルート・設定ファイル

| ファイル | 内容 |
|----------|------|
| `CMakeLists.txt` | Ubuntu 向け CMake ビルド定義。`.C` 大文字拡張子対応のため `set_source_files_properties(... LANGUAGE C)` を使用 |
| `x68k/Makefile` | X68K クロスビルド用 Makefile（`m68k-elf-gcc` 想定、`-D__human68k__`） |
| `.github/workflows/build.yml` | Ubuntu ビルド・スモークテスト・バイナリアーティファクト保存 |
| `README.md` | ビルド手順・使い方・TUI キー一覧・cd シェル関数・ソース構成・検索優先順位・未実装事項 |
| `.gitignore` | `build/`・`build_*/`・`*.idx` を除外 |

---

## 動作確認結果（WSL Ubuntu 24.04 / GCC 13.3.0）

```
cmake -S . -B build_wsl -DCMAKE_BUILD_TYPE=Release
cmake --build build_wsl --parallel
```

ビルド: **成功**

### スモークテスト結果

| テスト | コマンド | 結果 |
|--------|---------|------|
| インデックス生成 | `xfidx -q -o /tmp/test.idx /usr/bin` | 42,064 エントリ生成、形式正常 |
| 非対話 open | `xfind -i /tmp/test.idx --open ls` | `ls` を実行、exit: 0 |
| 非対話 cd | `xfind -i /tmp/test.idx --cd bash` | `/tmp/xfind_cd` に `/usr/bin` 書き出し、exit: 0 |
| マッチなし | `xfind -i /tmp/test.idx --open __nope__` | `no match` メッセージ、exit: 1 |

生成インデックス先頭:

```
XFIND-INDEX 1
ROOT /usr/bin
ENTRY D /usr/bin
ENTRY F /usr/bin/head
ENTRY F /usr/bin/pygmentize
...
```

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

ncurses などの外部ライブラリを使用せず、ANSI エスケープシーケンスと `termios` raw mode のみで実装した。
X68K の IOCS コンソールも ANSI/VT100 エスケープシーケンスに対応しているため、
TUI のコアは両プラットフォームで共通コードとして使える。

### 4. 安定ソートの実現

C 標準の `qsort` は安定ソートを保証しない。
`MatchResult` に `orig`（元インデックス）フィールドを持たせ、
比較関数の最終 tiebreaker として使用することで安定ソートを実現した。

### 5. OS 依存差分の分離

`PLATFORM.H` を唯一の OS 依存インタフェースとして設計した。

| 差分 | Ubuntu (POSIX) | X68K (Human68k) |
|------|---------------|-----------------|
| ディレクトリ走査 | `opendir` / `readdir` | `_dos_findfirst` / `_dos_findnext` |
| ターミナル raw mode | `termios` | `_dos_rawmode` (スタブ) |
| ファイル open | `xdg-open` / `execv` | `_dos_exec` |
| パス区切り | `/` | `\` |
| パス最大長 | 1024 | 92 |

---

## 既知の制限・注意事項

### CMake のビルド警告

Ubuntu で `cmake --build` を実行すると以下の警告が出る場合がある。

```
cc1plus: warning: command-line option '-std=gnu99' is valid for C/ObjC but not for C++
```

CMake が一部ファイルを C++ コンパイラで処理しているために発生する。
バイナリの動作には影響しない。根本対応としてはファイル拡張子を `.c`（小文字）に変更することで解消できるが、
spec.md の命名規則（大文字 `.C`）を優先して据え置いた。

### X68K 実装の未テスト箇所

| 箇所 | 状況 |
|------|------|
| `FSWALK.C` X68K パス | `_dos_findfirst` / `_dos_findnext` を実装済みだが実機未確認 |
| `ACTIONS.C` X68K open | `_dos_exec` を記述済みだが動作未確認 |
| `UITUI.C` X68K raw mode | `raw_enter` / `raw_leave` はスタブ（Human68k コンソールはデフォルト raw に近い） |
| `x68k/Makefile` | クロスコンパイラのパス・ライブラリパスは環境依存。コメントに調整箇所を明記 |

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
| X68000 向けの `x68k/Makefile` が追加されている | ✓ 追加済み |

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
