#include <pebble.h>

// UI stuff
static Window *s_window;
static GFont s_res_font_roboto_condensed_bold_reduced_60;
static GFont s_res_roboto_condensed_21;
static TextLayer *s_textlayer_date;
static TextLayer *s_textlayer_day;
static TextLayer *s_textlayer_time;
static TextLayer *s_textlayer_utc;

// private functions
static void update(struct tm *tick_time, TimeUnits units_changed);
static void initialise_ui(void);
static void destroy_ui(Window* window);

static void update(struct tm *tick_time, TimeUnits units_changed) {
  char time_fmt[sizeof("%I:%M")];

  static char localdate[sizeof("9999-99-99")];
  static char localday[sizeof("Wednesday")];
  static char localtime[sizeof("99:99")];
  static char utc[sizeof("99-99 99:99Z")];

  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strcpy(time_fmt, "%R");
  } else {
    // Use 12 hour format
    strcpy(time_fmt, "%I:%M");
  }

  // only update day/date if needed
  if (units_changed & DAY_UNIT) {
    strftime(localdate, sizeof(localdate), "%F", tick_time);
    strftime(localday, sizeof(localday), "%A", tick_time);

    text_layer_set_text(s_textlayer_date, localdate);
    text_layer_set_text(s_textlayer_day, localday);
  }

  strftime(localtime, sizeof(localtime), time_fmt, tick_time);
  mktime(tick_time); // is this ok to rely on?
  strftime(utc, sizeof(utc), "%m-%d %RZ", tick_time);

  text_layer_set_text(s_textlayer_time, localtime);
  text_layer_set_text(s_textlayer_utc, utc);
}

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, 1);
  #endif
  
  s_res_font_roboto_condensed_bold_reduced_60 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_font_roboto_condensed_bold_reduced_60));
  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  
  // s_textlayer_date
  s_textlayer_date = text_layer_create(GRect(0, 29, 143, 26));
  text_layer_set_background_color(s_textlayer_date, GColorClear);
  text_layer_set_text_color(s_textlayer_date, GColorWhite);
  text_layer_set_text_alignment(s_textlayer_date, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_date, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_date);
  
  // s_textlayer_day
  s_textlayer_day = text_layer_create(GRect(0, 5, 143, 26));
  text_layer_set_background_color(s_textlayer_day, GColorClear);
  text_layer_set_text_color(s_textlayer_day, GColorWhite);
  text_layer_set_text_alignment(s_textlayer_day, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_day, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_day);
  
  // s_textlayer_time
  s_textlayer_time = text_layer_create(GRect(0, 44, 143, 64));
  text_layer_set_background_color(s_textlayer_time, GColorClear);
  text_layer_set_text_color(s_textlayer_time, GColorWhite);
  text_layer_set_text_alignment(s_textlayer_time, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_time, s_res_font_roboto_condensed_bold_reduced_60);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_time);
  
  // s_textlayer_utc
  s_textlayer_utc = text_layer_create(GRect(0, 143, 143, 26));
  text_layer_set_background_color(s_textlayer_utc, GColorClear);
  text_layer_set_text_color(s_textlayer_utc, GColorWhite);
  text_layer_set_text_alignment(s_textlayer_utc, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_utc, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_utc);
}

static void destroy_ui(Window* window) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_date);
  text_layer_destroy(s_textlayer_day);
  text_layer_destroy(s_textlayer_time);
  text_layer_destroy(s_textlayer_utc);
  fonts_unload_custom_font(s_res_font_roboto_condensed_bold_reduced_60);
}

static void handle_init(void) {
  time_t now = time(NULL);

  initialise_ui();

  // call update to get correct output before showing window
  update(localtime(&now), DAY_UNIT);

  // register window unload funtion
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = destroy_ui,
  });

  // show window
  window_stack_push(s_window, true); 

  // register update as tick timer handler
  tick_timer_service_subscribe(MINUTE_UNIT, update);
}

static void handle_deinit(void) {
  window_stack_remove(s_window, true);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
