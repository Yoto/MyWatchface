#include <pebble.h>

// ---------------------------------------------------------------------------
// Stencil JP - Pebble Time 2 (emery, 200 x 228 px) 用ウォッチフェイス
//
// 初代 Pebble 向けに作成した文字盤の移植版。
//   1段目: 月 (左) / 日 (中央) / 曜日 (右)
//   2段目: 時刻 (HHMM, コロンなし・大)
//   3段目: 和暦 (左) / 西暦 (右)
// 文字は Penn Station (時刻 72px / 和暦西暦 39px / 月日曜日 31px) で描画する。
// ---------------------------------------------------------------------------

// レイアウト定数 (emery: 200 x 228)
// Penn Station はグリフ上端がアセンダラインに一致するため、フレームの y が
// そのまま文字上端になる。サイズは最長ケース (0000 / DEC 31 WED / R10 2028)
// が幅 188px に収まる最大値。
// 実機ではベゼル際の数ピクセルが見えないため、上に 12px・下に 6px の
// 安全マージンを確保し (文字の実描画位置: 12..222)、時刻はその間の中央。
#define ROW_DATE_Y     12
#define ROW_DATE_H     40
#define ROW_TIME_Y     84
#define ROW_TIME_H     66
#define ROW_ERA_Y      190
#define ROW_ERA_H      38
#define SIDE_MARGIN    6
#define ERA_SPLIT_X    92    // 和暦/西暦の境界

static Window    *s_window;
static GFont      s_font_big;    // 時刻用 Penn Station 72px
static GFont      s_font_med;    // 和暦西暦用 Penn Station 39px
static GFont      s_font_small;  // 月日曜日用 Penn Station 31px
static TextLayer *s_month_layer;
static TextLayer *s_day_layer;
static TextLayer *s_weekday_layer;
static TextLayer *s_time_layer;
static TextLayer *s_era_layer;
static TextLayer *s_year_layer;

// ---------------------------------------------------------------------------
// 和暦計算 (令和 / 平成 / 昭和 / 大正)
// ---------------------------------------------------------------------------
static void get_wareki(const struct tm *t, char *buf, size_t n) {
    int y = t->tm_year + 1900;
    int m = t->tm_mon + 1;
    int d = t->tm_mday;
    const char *era;
    int ey;

    if (y > 2019 || (y == 2019 && m >= 5)) {
        era = "R";  ey = y - 2018;
    } else if (y > 1989 || (y == 1989 && (m > 1 || d >= 8))) {
        era = "H";  ey = y - 1988;
    } else if (y > 1926 || (y == 1926 && m == 12 && d >= 25)) {
        era = "S";  ey = y - 1925;
    } else {
        era = "T";  ey = y - 1911;
    }
    snprintf(buf, n, "%s%d", era, ey);
}

// ---------------------------------------------------------------------------
// 画面更新
// ---------------------------------------------------------------------------
static void update_display(struct tm *t) {
    static char s_time[6];
    strftime(s_time, sizeof(s_time),
             clock_is_24h_style() ? "%H%M" : "%I%M", t);
    text_layer_set_text(s_time_layer, s_time);

    static char s_month[4];
    strftime(s_month, sizeof(s_month), "%b", t);
    for (int i = 0; s_month[i]; i++) {
        if (s_month[i] >= 'a' && s_month[i] <= 'z') s_month[i] -= 'a' - 'A';
    }
    text_layer_set_text(s_month_layer, s_month);

    static char s_day[3];
    snprintf(s_day, sizeof(s_day), "%d", t->tm_mday);
    text_layer_set_text(s_day_layer, s_day);

    static char s_weekday[4];
    strftime(s_weekday, sizeof(s_weekday), "%a", t);
    for (int i = 0; s_weekday[i]; i++) {
        if (s_weekday[i] >= 'a' && s_weekday[i] <= 'z') s_weekday[i] -= 'a' - 'A';
    }
    text_layer_set_text(s_weekday_layer, s_weekday);

    static char s_era[8];
    get_wareki(t, s_era, sizeof(s_era));
    text_layer_set_text(s_era_layer, s_era);

    static char s_year[6];
    snprintf(s_year, sizeof(s_year), "%d", t->tm_year + 1900);
    text_layer_set_text(s_year_layer, s_year);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
    update_display(tick_time);
}

// ---------------------------------------------------------------------------
// ウィンドウ
// ---------------------------------------------------------------------------
static TextLayer *make_text_layer(Layer *root, GRect frame, GFont font,
                                  GTextAlignment align) {
    TextLayer *layer = text_layer_create(frame);
    text_layer_set_background_color(layer, GColorClear);
    text_layer_set_text_color(layer, GColorWhite);
    text_layer_set_font(layer, font);
    text_layer_set_text_alignment(layer, align);
    layer_add_child(root, text_layer_get_layer(layer));
    return layer;
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);
    const int w = bounds.size.w;

    window_set_background_color(window, GColorBlack);

    s_font_big = fonts_load_custom_font(
        resource_get_handle(RESOURCE_ID_FONT_PENN_STATION_72));
    s_font_med = fonts_load_custom_font(
        resource_get_handle(RESOURCE_ID_FONT_PENN_STATION_39));
    s_font_small = fonts_load_custom_font(
        resource_get_handle(RESOURCE_ID_FONT_PENN_STATION_31));

    // 1段目: 月 (左) / 日 (中央) / 曜日 (右)
    s_month_layer = make_text_layer(root,
        GRect(SIDE_MARGIN, ROW_DATE_Y, w - SIDE_MARGIN * 2, ROW_DATE_H),
        s_font_small, GTextAlignmentLeft);
    s_day_layer = make_text_layer(root,
        GRect(SIDE_MARGIN, ROW_DATE_Y, w - SIDE_MARGIN * 2, ROW_DATE_H),
        s_font_small, GTextAlignmentCenter);
    s_weekday_layer = make_text_layer(root,
        GRect(SIDE_MARGIN, ROW_DATE_Y, w - SIDE_MARGIN * 2, ROW_DATE_H),
        s_font_small, GTextAlignmentRight);

    // 2段目: 時刻 (コロンなし)
    s_time_layer = make_text_layer(root,
        GRect(0, ROW_TIME_Y, w, ROW_TIME_H),
        s_font_big, GTextAlignmentCenter);

    // 3段目: 和暦 (左) / 西暦 (右)
    s_era_layer = make_text_layer(root,
        GRect(SIDE_MARGIN, ROW_ERA_Y, ERA_SPLIT_X - SIDE_MARGIN, ROW_ERA_H),
        s_font_med, GTextAlignmentLeft);
    s_year_layer = make_text_layer(root,
        GRect(ERA_SPLIT_X, ROW_ERA_Y, w - ERA_SPLIT_X - SIDE_MARGIN, ROW_ERA_H),
        s_font_med, GTextAlignmentRight);

    time_t now = time(NULL);
    update_display(localtime(&now));
}

static void window_unload(Window *window) {
    text_layer_destroy(s_month_layer);
    text_layer_destroy(s_day_layer);
    text_layer_destroy(s_weekday_layer);
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_era_layer);
    text_layer_destroy(s_year_layer);
    fonts_unload_custom_font(s_font_big);
    fonts_unload_custom_font(s_font_med);
    fonts_unload_custom_font(s_font_small);
}

static void init(void) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit(void) {
    tick_timer_service_unsubscribe();
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
