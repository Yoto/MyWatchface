# Stencil JP — Pebble Time 2 ウォッチフェイス

初代 Pebble 向けに作成したオリジナル文字盤を、Pebble Time 2 (emery / 200×228px カラー) 向けに作り直したウォッチフェイスです。

## 画面構成

```
┌────────────────────┐
│ JUL              2 │  月 / 日
│                    │
│       2009         │  時刻 (HHMM・コロンなし)
│                    │
│ R8            2026 │  和暦 / 西暦
└────────────────────┘
```

- 文字はデジタル時計風のピクセルフォント **Jersey 10**(時刻 112px、日付・年 62px)で描画し、3段が 200×228px の画面全体に広がるよう配置しています。
- 文字にはオリジナルのステンシル風フォントを模した**斜め格子模様**をオーバーレイ描画で再現しています。
- 和暦は令和・平成・昭和・大正に対応し、自動計算されます (例: 2026年 → `R8`)。
- 時刻は Pebble 本体の 12/24 時間設定に追従します。
- 天気表示は現在オフにしています(以前の実装は git 履歴の `src/pkjs/index.js` と AppMessage まわりを参照)。

## ビルドとインストール

[Pebble SDK](https://developer.rebble.io/developer.pebble.com/sdk/index.html)(Rebble 版)が必要です。

```sh
# ビルド
pebble build

# エミュレータで実行 (Pebble Time 2 = emery)
pebble install --emulator emery

# 実機へインストール (Pebble アプリの Developer Connection を有効にする)
pebble install --phone <スマートフォンのIPアドレス>
```

## ファイル構成

| パス | 内容 |
| --- | --- |
| `src/c/main.c` | 文字盤本体 (レイアウト・和暦計算・格子テクスチャ) |
| `resources/fonts/Jersey10-Regular.ttf` | 大きな文字用フォント (ビルド時に数字・英大文字のみに絞って収録) |
| `package.json` | プロジェクト定義 (対象プラットフォーム: emery、フォントリソース) |
| `wscript` | ビルドスクリプト |

Jersey 10 フォント (Sarah Cadigan-Fried) は [SIL Open Font License 1.1](https://fonts.google.com/specimen/Jersey+10/license) で提供されています。
