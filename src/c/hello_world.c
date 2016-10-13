#include <pebble.h>

#define STANDARD_MARGIN 3
#define TIME_MARGIN 8 * STANDARD_MARGIN
#define ASSIGN_MARGIN 13 * STANDARD_MARGIN
#define INFOBAR_HEIGHT 15
#define ASSIGN_INFO_HEIGHT 15
#define ASSIGN_INFO_WIDTH 80
#define INFO_MARGIN_WIDTH 55
#define ASSIGNMENT_HEIGHT 90
#define BATTERY_WIDTH 30

#define INBOX_SIZE 1024
#define OUTBOX_SIZE 256

static Window *s_main_window;    // parent window
static Layer *s_canvas_layer;    // Canvas layer
static TextLayer *s_time_layer;  // Time label
static int s_battery_level;
static float s_panda_score;

static char first_title_buffer[256];
static char second_title_buffer[256];
static char first_time_buffer[128];
static char second_time_buffer[128];
static char first_points_buffer[128];
static char second_points_buffer[128];
static char date_buffer[64];

/*********************************************
* Time services
*********************************************/
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(date_buffer, sizeof(date_buffer), "%F", tick_time);
  
  text_layer_set_text(s_time_layer, s_buffer);
  
  layer_mark_dirty(s_canvas_layer);
  
  if (tick_time->tm_min % 5 == 0) {
    // Send request to update assignment data
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

/***********************************************
* Battery callback
***********************************************/
static void battery_callback(BatteryChargeState state) {
  //Record the new battery level
  s_battery_level = state.charge_percent;
  
  // Redraw
  layer_mark_dirty(s_canvas_layer);
}

/***********************************************
* Canvas layer services
***********************************************/
static void draw_status_rec(GContext *ctx, GRect bounds) {
    graphics_context_set_stroke_color(ctx, GColorLightGray);
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 5);
    
    GRect status_box = GRect(bounds.origin.x, bounds.origin.y, bounds.size.w, INFOBAR_HEIGHT);
    graphics_draw_rect(ctx, status_box);
    graphics_fill_rect(ctx, status_box, 0, GCornersAll);
}

static void draw_assignment(char title[], char due[], char points[], GContext *ctx, GRect bounds, GRect points_bounds) {
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, 
                     title, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), 
                     bounds, 
                     GTextOverflowModeTrailingEllipsis, 
                     GTextAlignmentLeft, 
                     NULL);
    graphics_draw_text(ctx, 
                     points, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_14), 
                     points_bounds, 
                     GTextOverflowModeTrailingEllipsis, 
                     GTextAlignmentLeft, 
                     NULL);
    GRect time_bounds = GRect(points_bounds.origin.x + INFO_MARGIN_WIDTH, points_bounds.origin.y, ASSIGN_INFO_WIDTH, ASSIGN_INFO_HEIGHT);
    graphics_draw_text(ctx, 
                     due, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), 
                     time_bounds, 
                     GTextOverflowModeTrailingEllipsis, 
                     GTextAlignmentLeft, 
                     NULL);
}

static void draw_battery(GContext *ctx, GRect bounds) {
  char battery_level[] = "0000";
  snprintf(battery_level, sizeof(battery_level), "%3d%%", s_battery_level);
  graphics_draw_text(ctx, 
                     battery_level, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), 
                     GRect(bounds.size.w - BATTERY_WIDTH - STANDARD_MARGIN,0, BATTERY_WIDTH, INFOBAR_HEIGHT), 
                     GTextOverflowModeTrailingEllipsis, 
                     GTextAlignmentLeft, 
                     NULL);
}

static void draw_date(GContext *ctx, GRect bounds) {
  graphics_draw_text(ctx, 
                     date_buffer, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), 
                     GRect(STANDARD_MARGIN, 0, BATTERY_WIDTH * 3, INFOBAR_HEIGHT), 
                     GTextOverflowModeTrailingEllipsis, 
                     GTextAlignmentLeft, 
                     NULL);
}

static void draw_assingment_info(GContext *ctx, GRect bounds) {
  // Background
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 5);
  GRect status_box = GRect(bounds.origin.x, bounds.size.h - ASSIGNMENT_HEIGHT, bounds.size.w, bounds.size.h);
  graphics_draw_rect(ctx, status_box);
  graphics_fill_rect(ctx, status_box, 0, GCornersAll);
 
  // First Assignment
  GRect first_bounds = GRect(STANDARD_MARGIN, bounds.size.h - ASSIGNMENT_HEIGHT + STANDARD_MARGIN, bounds.size.w - STANDARD_MARGIN, 15);
  GRect first_time_bounds =  GRect(STANDARD_MARGIN, bounds.size.h - ASSIGNMENT_HEIGHT + TIME_MARGIN, first_bounds.size.w, first_bounds.size.h);
  draw_assignment(first_title_buffer, first_time_buffer, first_points_buffer, ctx, first_bounds, first_time_bounds);
  
  // Second Assignment
  GRect second_bounds = GRect(STANDARD_MARGIN, bounds.size.h - ASSIGNMENT_HEIGHT + ASSIGN_MARGIN, bounds.size.w - STANDARD_MARGIN, 15);
  GRect second_time_bounds =  GRect(STANDARD_MARGIN, bounds.size.h - ASSIGNMENT_HEIGHT + (STANDARD_MARGIN * 20), first_bounds.size.w, first_bounds.size.h);
  draw_assignment(second_title_buffer, second_time_buffer, second_points_buffer, ctx, second_bounds, second_time_bounds);
}

static void draw_panda_score(GContext *ctx, GRect bounds, float score) {
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  // Set the fill color
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  
  GRect bar_bounds = GRect(0, bounds.size.h - ASSIGNMENT_HEIGHT - (INFOBAR_HEIGHT / 3), ((float) bounds.size.w) * score, INFOBAR_HEIGHT / 3);
  graphics_draw_rect(ctx, bar_bounds);
  graphics_fill_rect(ctx, bar_bounds, 0, GCornersAll);
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
  // Get bounds
  GRect bounds = layer_get_bounds(layer);
  
  // Set the line color
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  // Draw the status
  draw_status_rec(ctx, bounds);
  
  // Draw assignment info
  draw_assingment_info(ctx, bounds);
  
  // Draw battery
  draw_battery(ctx, bounds);
  
  // Draw date
  draw_date(ctx, bounds);
  
  // Draw Panda score
  draw_panda_score(ctx, bounds, s_panda_score);
}

/*********************************************
* AppMessage services
*********************************************/
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *first_title_tuple = dict_find(iterator, MESSAGE_KEY_FIRST_ASSIGN);
  Tuple *second_title_tuple = dict_find(iterator, MESSAGE_KEY_SECOND_ASSIGN);
  Tuple *first_time_tuple = dict_find(iterator, MESSAGE_KEY_FIRST_DUE);
  Tuple *second_time_tuple = dict_find(iterator, MESSAGE_KEY_SECOND_DUE);
  Tuple *first_points_tuple = dict_find(iterator, MESSAGE_KEY_FIRST_POINTS);
  Tuple *second_points_tuple = dict_find(iterator, MESSAGE_KEY_SECOND_POINTS);
  Tuple *complete_tuple = dict_find(iterator, MESSAGE_KEY_COMPLETE);
  Tuple *total_tuple = dict_find(iterator, MESSAGE_KEY_TOTAL);
  
  if (first_title_tuple && second_title_tuple && first_time_tuple && second_time_tuple &&
      first_points_tuple && second_points_tuple && complete_tuple && total_tuple) {
    snprintf(first_title_buffer, sizeof(first_title_buffer), "%s", first_title_tuple->value->cstring);
    snprintf(second_title_buffer, sizeof(second_title_buffer), "%s", second_title_tuple->value->cstring);
    snprintf(first_time_buffer, sizeof(first_time_buffer), "%s", first_time_tuple->value->cstring);
    snprintf(second_time_buffer, sizeof(second_time_buffer), "%s", second_time_tuple->value->cstring);
    snprintf(first_points_buffer, sizeof(first_points_buffer), "%3s Pts. - ", first_points_tuple->value->cstring);
    snprintf(second_points_buffer, sizeof(second_points_buffer), "%3s Pts. - ", second_points_tuple->value->cstring);
    
    s_panda_score = (float) complete_tuple->value->int32 / (float) total_tuple->value->int32;    
      
    layer_mark_dirty(s_canvas_layer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


/*********************************************
* Main window services
*********************************************/
static void main_window_load(Window *window) {  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create canvas layer
  s_canvas_layer = layer_create(bounds);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, INFOBAR_HEIGHT + STANDARD_MARGIN, bounds.size.w, 50));

  window_set_background_color(s_main_window, GColorBlack);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(window), s_canvas_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Check battery
  battery_callback(battery_state_service_peek());
  
  layer_mark_dirty(s_canvas_layer);
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
  // AppMessage callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
                                   
  // Open AppMessage
  app_message_open(INBOX_SIZE, OUTBOX_SIZE);
  
  // Subscribe to tick handler
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
  
  // Register Battery handler and initialize 
  battery_state_service_subscribe(battery_callback);
}

static void deinit() {
  window_destroy(s_main_window);  
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}