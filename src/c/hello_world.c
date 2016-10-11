#include <pebble.h>

#define STANDARD_MARGIN 3
#define TIME_MARGIN 8 * STANDARD_MARGIN
#define ASSIGN_MARGIN 13 * STANDARD_MARGIN
#define INFOBAR_HEIGHT 15
#define ASSIGNMENT_HEIGHT 90

#define INBOX_SIZE 1024
#define OUTBOX_SIZE 256

static Window *s_main_window;    // parent window
static Layer *s_canvas_layer;    // Canvas layer
static TextLayer *s_time_layer;  // Time label

static char first_title_buffer[256];
static char second_title_buffer[256];
static char first_time_buffer[128];
static char second_time_buffer[128];

/*********************************************
* Time services
*********************************************/
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_buffer);
  layer_mark_dirty(s_canvas_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
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

static void draw_assignment(char title[], char due[], GContext *ctx, GRect bounds, GRect time_bounds) {
    graphics_context_set_text_color(ctx, GColorBlack);
    graphics_draw_text(ctx, 
                     title, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), 
                     bounds, 
                     GTextOverflowModeTrailingEllipsis, 
                     GTextAlignmentLeft, 
                     NULL);
    graphics_draw_text(ctx, 
                     due, 
                     fonts_get_system_font(FONT_KEY_GOTHIC_14), 
                     time_bounds, 
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
  draw_assignment(first_title_buffer, first_time_buffer, ctx, first_bounds, first_time_bounds);
  
  // Second Assignment
  GRect second_bounds = GRect(STANDARD_MARGIN, bounds.size.h - ASSIGNMENT_HEIGHT + ASSIGN_MARGIN, bounds.size.w - STANDARD_MARGIN, 15);
  GRect second_time_bounds =  GRect(STANDARD_MARGIN, bounds.size.h - ASSIGNMENT_HEIGHT + (STANDARD_MARGIN * 20), first_bounds.size.w, first_bounds.size.h);
  draw_assignment(second_title_buffer, second_time_buffer, ctx, second_bounds, second_time_bounds);
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
}

/*********************************************
* AppMessage services
*********************************************/
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *first_title_tuple = dict_find(iterator, MESSAGE_KEY_FIRST_ASSIGN);
  Tuple *second_title_tuple = dict_find(iterator, MESSAGE_KEY_SECOND_ASSIGN);
  Tuple *first_time_tuple = dict_find(iterator, MESSAGE_KEY_FIRST_DUE);
  Tuple *second_time_tuple = dict_find(iterator, MESSAGE_KEY_SECOND_DUE);
  
  if (first_title_tuple && second_title_tuple) {
    snprintf(first_title_buffer, sizeof(first_title_buffer), "%s", first_title_tuple->value->cstring);
    snprintf(second_title_buffer, sizeof(second_title_buffer), "%s", second_title_tuple->value->cstring);
    snprintf(first_time_buffer, sizeof(first_time_buffer), "%s", first_time_tuple->value->cstring);
    snprintf(second_time_buffer, sizeof(second_time_buffer), "%s", second_time_tuple->value->cstring);
    
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
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
}

static void deinit() {
  window_destroy(s_main_window);  
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}