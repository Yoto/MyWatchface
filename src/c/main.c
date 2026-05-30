// ---------------------------------------------------------------------------
// レイヤー
// 上段: 月(左) / 日(右)         BITHAM_30_BLACK
// 中段: 時刻                    ROBOTO_BOLD_SUBSET_49
// 下段: 和暦元号(左) / 西暦(右)  BITHAM_30_BLACK
// ---------------------------------------------------------------------------
static Window     *s_window;
static TextLayer  *s_month_layer;
static TextLayer  *s_day_layer;
static TextLayer  *s_time_layer;
static TextLayer  *s_era_layer;
static TextLayer  *s_year_layer;

// ---------------------------------------------------------------------------
// 和暦計算
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
    static char s_time[5];
    strftime(s_time, sizeof(s_time), "%H%M", t);
    text_layer_set_text(s_time_layer, s_time);

    static char s_month[4];
    strftime(s_month, sizeof(s_month), "%b", t);
    for (int i = 0; s_month[i]; i++)
        if (s_month[i] >= 'a') s_month[i] -= 32;
    text_layer_set_text(s_month_layer, s_month);

    static char s_day[3];
    snprintf(s_day, sizeof(s_day), "%d", t->tm_mday);
    text_layer_set_text(s_day_layer, s_day);

    static char s_era[8];
    static char s_year[5];
    get_wareki(t, s_era, sizeof(s_era), s_year, sizeof(s_year));
    text_layer_set_text(s_era_layer,  s_era);
    text_layer_set_text(s_year_layer, s_year);
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
    update_display(tick_time);
}

// ---------------------------------------------------------------------------
// ウィンドウ  (144 × 168 px)
//
// フォントサイズ選定根拠 (BITHAM_30_BLACK):
//   "MAY" ≈ 64px, "30" ≈ 38px  → 合計 ~110px < 144px ✓
//   "R8"  ≈ 39px, "2026" ≈ 76px → 合計 ~123px < 144px ✓
//
// レイアウト:
//   y=  8  h=38  [MAY (left)          30 (right)]  BITHAM_30_BLACK
//   y= 50  h=62  [       1304        ]             ROBOTO_BOLD_SUBSET_49
//   y=116  h=38  [R8 (left)        2026 (right)]   BITHAM_30_BLACK
// ---------------------------------------------------------------------------
static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    int W = layer_get_bounds(root).size.w;  // 144

    window_set_background_color(window, GColorBlack);

    // ── 月 (左上)  "MAY" ≈ 64px → w=82 で余裕あり ──
    s_month_layer = text_layer_create(GRect(2, 8, 82, 38));
    text_layer_set_background_color(s_month_layer, GColorClear);
    text_layer_set_text_color(s_month_layer, GColorWhite);
    text_layer_set_font(s_month_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_month_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_month_layer));

    // ── 日 (右上)  "30" ≈ 38px → w=60 で余裕あり ──
    s_day_layer = text_layer_create(GRect(82, 8, 60, 38));
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_color(s_day_layer, GColorWhite);
    text_layer_set_font(s_day_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_day_layer));

    // ── 時刻 (中央) ────────────────────────────────
    s_time_layer = text_layer_create(GRect(0, 50, W, 62));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer,
        fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_layer));

    // ── 和暦元号 (左下)  "R8" ≈ 39px → w=70 で余裕あり ──
    s_era_layer = text_layer_create(GRect(2, 116, 70, 38));
    text_layer_set_background_color(s_era_layer, GColorClear);
    text_layer_set_text_color(s_era_layer, GColorWhite);
    text_layer_set_font(s_era_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_era_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_era_layer));

    // ── 西暦 (右下)  "2026" ≈ 76px → w=72 で余裕あり ──
    s_year_layer = text_layer_create(GRect(70, 116, 72, 38));
    text_layer_set_background_color(s_year_layer, GColorClear);
    text_layer_set_text_color(s_year_layer, GColorWhite);
    text_layer_set_font(s_year_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_year_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_year_layer));

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
