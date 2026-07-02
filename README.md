# MyWatchface

写真から復元した Pebble Time 2 用ウォッチフェイスです。

## レイアウト

```
+----------------------------------+
| JUN                            9 |  ← 月(左) / 日(右)   BITHAM_30_BLACK
|                                  |
|             2154                 |  ← 時刻 HHMM         ROBOTO_BOLD_SUBSET_49
|                                  |
| H29                          2017|  ← 和暦 + 西暦        BITHAM_30_BLACK
| 17C, Clouds, 1007hPa             |  ← 現在天気           GOTHIC_14_BOLD
| 14C, Clear, 1019hPa              |  ← 約6時間後の予報     GOTHIC_14_BOLD
+----------------------------------+
        200 × 228 px (emery / Pebble Time 2)
```

## 対応プラットフォーム

このウォッチフェイスは **Pebble Time 2 (emery, 200×228 カラー)** 専用にレイアウトされています。

| プラットフォーム | 機種 |
|---|---|
| emery   | **Pebble Time 2** |

他機種 (aplite / basalt / chalk / diorite) で使う場合は `package.json` の
`targetPlatforms` に追加し、`src/c/main.c` の座標をその機種の解像度
(例: basalt/Pebble Time は 144×168) に合わせて調整してください。

## セットアップ

### 1. Rebble ツールチェーンのインストール

```bash
# Docker 版（推奨）
docker pull rebble/pebble-sdk
# または公式 SDK インストーラー
# https://developer.rebble.io/developer.pebble.com/sdk/index.html
```

Emery (Pebble Time 2) プラットフォームのビルドには SDK 4.2 以降が必要です。

### 2. OpenWeatherMap API キーの取得

1. https://openweathermap.org/api でアカウント作成（無料）
2. API Keys ページから Free tier のキーをコピー
3. `src/pkjs/index.js` の以下の行を書き換える

```javascript
var API_KEY = 'YOUR_OPENWEATHERMAP_API_KEY';
//             ↑ ここに実際のキーを貼り付ける
```

### 3. UUID の再生成（任意）

他の watchface との競合を避けるため、`package.json` の UUID を変更することを推奨します。

```bash
# UUID 生成例
python3 -c "import uuid; print(uuid.uuid4())"
```

### 4. ビルド & インストール

```bash
pebble build
pebble install --phone <スマートフォンのIPアドレス>
```

インストール後、Pebble アプリでこのウォッチフェイスに位置情報の使用を許可してください
（現在地の天気を取得するために使用します）。

## 仕様メモ

- 時刻: 24時間表示、コロンなし（例: 2154 = 21:54）
- 和暦: 令和(R) / 平成(H) / 昭和(S) / 大正(T) を自動判定
- 天気更新: 起動時 + 30分ごとに自動取得（PebbleKit JS 経由）
- 天気データ: OpenWeatherMap 現在値 + 約6時間後の予報（3時間刻みの予報から最も近い時刻を選択）
- 直近に受信した天気は端末に保存され、再起動直後もそれまでの値を表示します
- 背景色は emery のカラー e-paper に合わせて `GColorOxfordBlue`（濃紺）を使用
