# Stencil JP — Pebble Time 2 ウォッチフェイス

初代 Pebble 向けに作成したオリジナル文字盤を、Pebble Time 2 (emery / 200×228px カラー) 向けに作り直したウォッチフェイスです。

## 画面構成

```
┌────────────────────┐
│ JUN              9 │  月 / 日
│                    │
│       2154         │  時刻 (HHMM・コロンなし)
│                    │
│ R8            2026 │  和暦 / 西暦
├────────────────────┤
│ 17C, Clouds, 1007hPa │  現在の天気
│ 14C, Clear, 1019hPa  │  約6時間後の予報
└────────────────────┘
```

- 大きな文字は同梱の角張ったフォント Khand Bold(時刻 100px、日付・年 54px)で描画し、Pebble Time 2 の 200×228px 画面いっぱいに表示します。
- 大きな文字にはオリジナルのステンシル風フォントを模した**斜め格子模様**をオーバーレイ描画で再現しています。
- 和暦は令和・平成・昭和・大正に対応し、自動計算されます (例: 2026年 → `R8`)。
- 時刻は Pebble 本体の 12/24 時間設定に追従します。
- 天気は電話の位置情報をもとに [OpenWeatherMap](https://openweathermap.org/) から取得し、30 分ごとに更新します。取得結果は本体に保存されるため、再起動直後も前回の値が表示されます。

## セットアップ

1. [OpenWeatherMap](https://openweathermap.org/api) で無料の API キーを取得します。
2. `src/pkjs/index.js` の先頭にある `API_KEY` を取得したキーに書き換えます。

```js
var API_KEY = 'ここに取得したAPIキー';
```

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
| `src/c/main.c` | 文字盤本体 (レイアウト・和暦計算・格子テクスチャ・天気受信) |
| `src/pkjs/index.js` | PebbleKit JS (位置情報の取得と OpenWeatherMap への問い合わせ) |
| `resources/fonts/Khand-Bold.ttf` | 大きな文字用フォント (ビルド時に数字・英大文字のみに絞って収録) |
| `package.json` | プロジェクト定義 (対象プラットフォーム: emery、フォントリソース) |
| `wscript` | ビルドスクリプト |

Khand フォント (Indian Type Foundry) は [SIL Open Font License 1.1](https://fonts.google.com/specimen/Khand/license) で提供されています。
