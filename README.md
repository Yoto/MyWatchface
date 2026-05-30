# MyWatchface
写真から復元した Pebble ウォッチフェイスです。

## レイアウト

```
+---------------------------+
| DEC                    8  |  ← 月(左) / 日(右)  GOTHIC_28_BOLD
|                           |
|          1937             |  ← 時刻 HHMM        ROBOTO_BOLD_SUBSET_49
|                           |
|        R8  2026           |  ← 和暦 + 西暦      GOTHIC_28_BOLD
|             3C, Clear, .. |  ← 現在天気         GOTHIC_14_BOLD
|             4C, Clear, .. |  ← ~6h後の予報      GOTHIC_14_BOLD
+---------------------------+
        144 × 168 px (basalt / Pebble Time 2)
```

## 対応プラットフォーム

| プラットフォーム | 機種 |
|---|---|
| aplite  | Pebble, Pebble Steel |
| basalt  | **Pebble Time, Pebble Time 2** |
| chalk   | Pebble Time Round |
| diorite | Pebble 2 |

## セットアップ

### 1. Rebble ツールチェーンのインストール

```bash
# Docker 版（推奨）
docker pull rebble/pebble-sdk
# または公式 SDK インストーラー
# https://developer.rebble.io/developer.pebble.com/sdk/index.html
```

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
cd minimal-jp-watchface
pebble build
pebble install --phone <スマートフォンのIPアドレス>
```

## 仕様メモ

- 時刻: 24時間表示、コロンなし（例: 1937 = 19:37）
- 和暦: 令和(R) / 平成(H) / 昭和(S) / 大正(T) を自動判定
- 天気更新: 起動時 + 30分ごとに自動取得
- 天気データ: OpenWeatherMap 現在値 + 約6時間後の予報
