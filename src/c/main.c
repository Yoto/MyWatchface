#include <pebble.h>

// ---------------------------------------------------------------------------
// レイヤー
// 上段: 月(左) / 日(右)       BITHAM_42_BOLD
// 中段: 時刻                  ROBOTO_BOLD_SUBSET_49
// 下段: 和暦元号(左) / 西暦(右) BITHAM_42_BOLD
// ---------------------------------------------------------------------------
static Window     *s_window;
static TextLayer  *s_month_layer;
static TextLayer  *s_day_layer;
static TextLayer  *s_time_layer;
static TextLayer  *s_era_layer;   // "R8" など
static TextLayer  *s_year_layer;  // "2026" など

// ---------------------------------------------------------------------------
// 和暦計算（元号+年 / 西暦 を別バッファに返す）
// ---------------------------------------------------------------------------
static void get_wareki(const struct tm *t,
                       char *era_buf,  size_t era_n,
                       char *year_buf, size_t year_n) {
    int y = t->tm_year + 1900;
    int m = t->tm_mon  + 1;
    int d = t->tm_mday;
    const char *era;
    int ey;

    if (y > 2019 || (y == 2019 && m > 4)) {
        era = "R";  ey = y - 2018;
    } else if (y > 1989 || (y == 1989 && (m > 1 || (m == 1 && d >= 8)))) {
        era = "H";  ey = y - 1988;
    } else if (y > 1926 || (y == 1926 && m == 12 && d >= 25)) {
        era = "S";  ey = y - 1925;
    } else {
        era = "T";  ey = y - 1911;
    }
    snprintf(era_buf,  era_n,  "%s%d", era, ey);
    snprintf(year_buf, year_n, "%d",   y);
}

// ---------------------------------------------------------------------------
// 画面更新
// ---------------------------------------------------------------------------
static void update_display(struct tm *t) {
    // 時刻 (HHMM)
    static char s_time[5];
    strftime(s_time, sizeof(s_time), "%H%M", t);
    text_layer_set_text(s_time_layer, s_time);

    // 月 (大文字 3 字)
    static char s_month[4];
    strftime(s_month, sizeof(s_month), "%b", t);
    for (int i = 0; s_month[i]; i++)
        if (s_month[i] >= 'a') s_month[i] -= 32;
    text_layer_set_text(s_month_layer, s_month);

    // 日
    static char s_day[3];
    snprintf(s_day, sizeof(s_day), "%d", t->tm_mday);
    text_layer_set_text(s_day_layer, s_day);

    // 和暦 + 西暦
    static char s_era[8];
    static char s_year[5];
    get_wareki(t, s_era, sizeof(s_era), s_year, sizeof(s_year));
    text_layer_set_text(s_era_layer,  s_era);
    text_layer_set_text(s_year_layer, s_year);
}

// ---------------------------------------------------------------------------
// 分タイマー
// ---------------------------------------------------------------------------
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
    update_display(tick_time);
}

// ---------------------------------------------------------------------------
// ウィンドウ  (144 × 168 px)
//
//  y=  4  h=52  [MAY          30]   BITHAM_42_BOLD
//  y= 54  h=62  [   1240        ]   ROBOTO_BOLD_SUBSET_49
//  y=114  h=52  [R8         2026]   BITHAM_42_BOLD
// ---------------------------------------------------------------------------
static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    int W = layer_get_bounds(root).size.w;  // 144

    window_set_background_color(window, GColorBlack);

    // ── 月 (左上) ──────────────────────────────────────
    s_month_layer = text_layer_create(GRect(2, 4, 80, 52));
    text_layer_set_background_color(s_month_layer, GColorClear);
    text_layer_set_text_color(s_month_layer, GColorWhite);
    text_layer_set_font(s_month_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_month_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_month_layer));

    // ── 日 (右上) ──────────────────────────────────────
    s_day_layer = text_layer_create(GRect(62, 4, 80, 52));
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_color(s_day_layer, GColorWhite);
    text_layer_set_font(s_day_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_day_layer));

    // ── 時刻 (大・中央) ────────────────────────────────
    s_time_layer = text_layer_create(GRect(0, 54, W, 62));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer,
        fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_layer));

    // ── 和暦元号 (左下) ────────────────────────────────
    s_era_layer = text_layer_create(GRect(2, 114, 80, 52));
    text_layer_set_background_color(s_era_layer, GColorClear);
    text_layer_set_text_color(s_era_layer, GColorWhite);
    text_layer_set_font(s_era_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_era_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_era_layer));

    // ── 西暦 (右下) ────────────────────────────────────
    s_year_layer = text_layer_create(GRect(62, 114, 80, 52));
    text_layer_set_background_color(s_year_layer, GColorClear);
    text_layer_set_text_color(s_year_layer, GColorWhite);
    text_layer_set_font(s_year_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text_alignment(s_year_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_year_layer));

    // 初回表示
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    update_display(t);
}

static void window_unload(Window *window) {
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_month_layer);
    text_layer_destroy(s_day_layer);
    text_layer_destroy(s_era_layer);
    text_layer_destroy(s_year_layer);
}

// ---------------------------------------------------------------------------
// 初期化
// ---------------------------------------------------------------------------
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
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
