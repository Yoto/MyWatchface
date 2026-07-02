#include <pebble.h>

// ---------------------------------------------------------------------------
// レイヤー (Pebble Time 2 / emery, 200 x 228 px)
// 1段目: 月(左) / 日(右)         BITHAM_30_BLACK
// 2段目: 時刻                    ROBOTO_BOLD_SUBSET_49
// 3段目: 和暦元号(左) / 西暦(右)  BITHAM_30_BLACK
// 4段目: 現在の天気              GOTHIC_14_BOLD
// 5段目: 約6時間後の予報          GOTHIC_14_BOLD
// ---------------------------------------------------------------------------
static Window     *s_window;
static TextLayer  *s_month_layer;
static TextLayer  *s_day_layer;
static TextLayer  *s_time_layer;
static TextLayer  *s_era_layer;
static TextLayer  *s_year_layer;
static TextLayer  *s_weather_now_layer;
static TextLayer  *s_weather_fc_layer;

static char s_weather_now_buf[32] = "--";
static char s_weather_fc_buf[32]  = "--";

enum {
  PERSIST_WEATHER_NOW = 1,
  PERSIST_WEATHER_FC  = 2,
};

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
// 天気 (PebbleKit JS / OpenWeatherMap から AppMessage 経由で受信)
// ---------------------------------------------------------------------------
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    Tuple *temp_t   = dict_find(iterator, MESSAGE_KEY_KEY_TEMPERATURE);
    Tuple *cond_t   = dict_find(iterator, MESSAGE_KEY_KEY_CONDITIONS);
    Tuple *pres_t   = dict_find(iterator, MESSAGE_KEY_KEY_PRESSURE);
    Tuple *fc_temp  = dict_find(iterator, MESSAGE_KEY_KEY_FC_TEMPERATURE);
    Tuple *fc_cond  = dict_find(iterator, MESSAGE_KEY_KEY_FC_CONDITIONS);
    Tuple *fc_pres  = dict_find(iterator, MESSAGE_KEY_KEY_FC_PRESSURE);

    if (temp_t && cond_t && pres_t) {
        snprintf(s_weather_now_buf, sizeof(s_weather_now_buf), "%dC, %s, %dhPa",
                  (int)temp_t->value->int32, cond_t->value->cstring, (int)pres_t->value->int32);
        text_layer_set_text(s_weather_now_layer, s_weather_now_buf);
        persist_write_string(PERSIST_WEATHER_NOW, s_weather_now_buf);
    }

    if (fc_temp && fc_cond && fc_pres) {
        snprintf(s_weather_fc_buf, sizeof(s_weather_fc_buf), "%dC, %s, %dhPa",
                  (int)fc_temp->value->int32, fc_cond->value->cstring, (int)fc_pres->value->int32);
        text_layer_set_text(s_weather_fc_layer, s_weather_fc_buf);
        persist_write_string(PERSIST_WEATHER_FC, s_weather_fc_buf);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! reason=%d", (int)reason);
}

// ---------------------------------------------------------------------------
// ウィンドウ  (200 x 228 px, Pebble Time 2 / emery)
// ---------------------------------------------------------------------------
static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(root);
    int W = bounds.size.w;  // 200

    window_set_background_color(window, GColorOxfordBlue);

    // ── 月 (左上) ──
    s_month_layer = text_layer_create(GRect(4, 8, 110, 40));
    text_layer_set_background_color(s_month_layer, GColorClear);
    text_layer_set_text_color(s_month_layer, GColorWhite);
    text_layer_set_font(s_month_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_month_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_month_layer));

    // ── 日 (右上) ──
    s_day_layer = text_layer_create(GRect(W - 4 - 90, 8, 90, 40));
    text_layer_set_background_color(s_day_layer, GColorClear);
    text_layer_set_text_color(s_day_layer, GColorWhite);
    text_layer_set_font(s_day_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_day_layer));

    // ── 時刻 (中央) ──
    s_time_layer = text_layer_create(GRect(0, 54, W, 78));
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorWhite);
    text_layer_set_font(s_time_layer,
        fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(root, text_layer_get_layer(s_time_layer));

    // ── 和暦元号 (左下) ──
    s_era_layer = text_layer_create(GRect(4, 136, 90, 40));
    text_layer_set_background_color(s_era_layer, GColorClear);
    text_layer_set_text_color(s_era_layer, GColorWhite);
    text_layer_set_font(s_era_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_era_layer, GTextAlignmentLeft);
    layer_add_child(root, text_layer_get_layer(s_era_layer));

    // ── 西暦 (右下) ──
    s_year_layer = text_layer_create(GRect(94, 136, W - 4 - 94, 40));
    text_layer_set_background_color(s_year_layer, GColorClear);
    text_layer_set_text_color(s_year_layer, GColorWhite);
    text_layer_set_font(s_year_layer,
        fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
    text_layer_set_text_alignment(s_year_layer, GTextAlignmentRight);
    layer_add_child(root, text_layer_get_layer(s_year_layer));

    // ── 現在の天気 ──
    s_weather_now_layer = text_layer_create(GRect(6, 182, W - 12, 20));
    text_layer_set_background_color(s_weather_now_layer, GColorClear);
    text_layer_set_text_color(s_weather_now_layer, GColorWhite);
    text_layer_set_font(s_weather_now_layer,
        fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_weather_now_layer, GTextAlignmentLeft);
    text_layer_set_text(s_weather_now_layer, s_weather_now_buf);
    layer_add_child(root, text_layer_get_layer(s_weather_now_layer));

    // ── 約6時間後の予報 ──
    s_weather_fc_layer = text_layer_create(GRect(6, 204, W - 12, 20));
    text_layer_set_background_color(s_weather_fc_layer, GColorClear);
    text_layer_set_text_color(s_weather_fc_layer, GColorWhite);
    text_layer_set_font(s_weather_fc_layer,
        fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
    text_layer_set_text_alignment(s_weather_fc_layer, GTextAlignmentLeft);
    text_layer_set_text(s_weather_fc_layer, s_weather_fc_buf);
    layer_add_child(root, text_layer_get_layer(s_weather_fc_layer));

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
    text_layer_destroy(s_weather_now_layer);
    text_layer_destroy(s_weather_fc_layer);
}

static void init(void) {
    if (persist_exists(PERSIST_WEATHER_NOW)) {
        persist_read_string(PERSIST_WEATHER_NOW, s_weather_now_buf, sizeof(s_weather_now_buf));
    }
    if (persist_exists(PERSIST_WEATHER_FC)) {
        persist_read_string(PERSIST_WEATHER_FC, s_weather_fc_buf, sizeof(s_weather_fc_buf));
    }

    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers){
        .load   = window_load,
        .unload = window_unload,
    });
    window_stack_push(s_window, true);
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
    window_destroy(s_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
