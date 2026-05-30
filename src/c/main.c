#include <pebble.h>

// ---------------------------------------------------------------------------
// レイヤー
// ---------------------------------------------------------------------------
static Window     *s_window;
static TextLayer  *s_time_layer;
static TextLayer  *s_month_layer;
static TextLayer  *s_day_layer;
static TextLayer  *s_era_layer;

// ---------------------------------------------------------------------------
// 和暦計算
// 令和: 2019-05-01〜
// 平成: 1989-01-08〜2019-04-30
// 昭和: 1926-12-25〜1989-01-07
// ---------------------------------------------------------------------------
static void get_wareki(const struct tm *t, char *buf, size_t n) {
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
    snprintf(buf, n, "%s%d %d", era, ey, y);
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

    // 日 (先頭 0 なし)
    static char s_day[3];
    snprintf(s_day, sizeof(s_day), "%d", t->tm_mday);
    text_layer_set_text(s_day_layer, s_day);

    // 和暦 + 西暦
    static char s_era[16];
    get_wareki(t, s_era, sizeof(s_era));
    text_layer_set_text(s_era_layer, s_era);
}

// ---------------------------------------------------------------------------
// 分タイマー
// ---------------------------------------------------------------------------
static void tick_handler(struct tm *tick_time, TimeUnits changed) {
    update_display(tick_time);
}

// ---------------------------------------------------------------------------
// ウィンドウ
// ---------------------------------------------------------------------------
static void window_load(Window *window) {
    Layer  *root   = window_get_root_layer(window);
    GRect   bounds = layer_get_bounds(root);
    int     W      = bounds.size.w;   // 144

    window_set_background_color(window, GColorBlack);

    // ── 月 (左上) ──────────────────────────────────────
    s_month_layer = text_layer_create(GRect(2, 4, 90, 34));
    text_layer_set_background_color(s_month_layer, GColorClear);
    text_layer_set_text_color(s_month_layer, GColorWhite);
    text_layer_set_font(s_month_layer,
        fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_month_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_month_layer));

    // ── 日 (右上) ──────────────────────────────────────
    s_day_layer = text_layer_create(GRect(80, 4, 62, 34));
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_color(s_day_layer, GColorWhite);
    text_layer_set_font(s_day_layer,
        fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_day_layer));

    // ── 時刻 (大) ──────────────────────────────────────
    // ROBOTO_BOLD_SUBSET_49 は数字専用フォント（コロン含む）
    s_time_layer = text_layer_create(GRect(0, 36, W, 58));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer,
        fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_layer));

    // ── 和暦 + 西暦 ────────────────────────────────────
    s_era_layer = text_layer_create(GRect(0, 98, W, 34));
    text_layer_set_background_color(s_era_layer, GColorClear);
    text_layer_set_text_color(s_era_layer, GColorWhite);
    text_layer_set_font(s_era_layer,
        fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_alignment(s_era_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_era_layer));

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
