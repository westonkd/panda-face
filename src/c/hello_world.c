#include <pebble.h>

#define STANDARD_MARGIN 3
#define INFOBAR_HEIGHT 15
#define ASSINGMNET_HEIGHT 90

static Window *s_main_window;    // parent window
static Layer *s_canvas_layer; 
static TextLayer *s_time_layer;  // Time label

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
    
    //graphics_context_set_text_color(ctx, GColorBlack);
    //graphics_draw_text(ctx,"DISCONNECTED", fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(bounds.origin.x, bounds.size.h - 10 - 8, bounds.size.w, bounds.size.h), GTextOverflowModeWordWrap, GTextAlignmentCenter , NULL);
}

static void draw_assingment_info(GContext *ctx, GRect bounds) {
  graphics_context_set_stroke_color(ctx, GColorLightGray);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_width(ctx, 5);
  
  GRect status_box = GRect(bounds.origin.x, bounds.size.h - ASSINGMNET_HEIGHT, bounds.size.w, bounds.size.h);
  graphics_draw_rect(ctx, status_box);
  graphics_fill_rect(ctx, status_box, 0, GCornersAll);
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