#include <pebble.h>
#include "kirby.h"
	
#ifdef PBL_COLOR	
	
//Window *my_window;
Layer *kirby_layer;

GBitmap *k_bg;
GFont kirby_font;

int ball_x = 50, ball_y = 50, initial_x = -20000, initial_y = -20000, diff_x = 0, diff_y = 0, diff_x_val = 0, diff_y_val = 0;

char padded_time[8];

//int ball_x = 50, ball_y = 50, initial_x = -20000, initial_y = -20000, diff_x = 0, diff_y = 0, diff_x_val = 0, diff_y_val = 0;

GBitmap *kirby_rolling[7];
int current_kirby = 0, tick_mod = 0;

void draw_ball(Layer *layer, GContext *ctx){	
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
graphics_draw_bitmap_in_rect(ctx, k_bg, GRect(0,0,100,100));		

	//graphics_context_set_fill_color(ctx, GColorBlack);
	//graphics_fill_circle(ctx, GPoint(ball_x, ball_y), 10);
	graphics_draw_bitmap_in_rect(ctx, kirby_rolling[current_kirby], GRect(ball_x, ball_y, 21, 21));
	

				graphics_context_set_text_color(ctx, GColorBlack);
			graphics_draw_text(ctx, padded_time, kirby_font, GRect(-1, 36-1, 100, 32), GTextOverflowModeFill, GTextAlignmentCenter, NULL);		
			graphics_draw_text(ctx, padded_time, kirby_font, GRect(-1, 36+1, 100, 32), GTextOverflowModeFill, GTextAlignmentCenter, NULL);		
			graphics_draw_text(ctx, padded_time, kirby_font, GRect(1, 36-1, 100, 32), GTextOverflowModeFill, GTextAlignmentCenter, NULL);		
			graphics_draw_text(ctx, padded_time, kirby_font, GRect(1, 36+1, 100, 32), GTextOverflowModeFill, GTextAlignmentCenter, NULL);	
		
			graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b110101});
			graphics_draw_text(ctx, padded_time, kirby_font, GRect(0, 36, 100, 32), GTextOverflowModeFill, GTextAlignmentCenter, NULL);	
}

void kirby_accel_raw_handler(AccelData *data, int num_samples){
	if(initial_x == -20000) initial_x = data[0].x;
	if(initial_y == -20000) initial_y = data[0].y;
	
	diff_x = data[0].x - initial_x;
	diff_y = data[0].y - initial_y;
	
	if(diff_x < -100 && ball_x > 3){ ball_x--; diff_x_val = -1; }
	else if(diff_x > 100 && ball_x < 76){ ball_x++; diff_x_val = 1; }
	else diff_x_val = 0;
	
	if(diff_y < -100 && ball_y < 76){ ball_y++; diff_y_val = 1;}
	else if(diff_y > 100 && ball_y > 3){ ball_y--; diff_y_val = -1; }
	else diff_y_val = 0;

	layer_mark_dirty(kirby_layer);
}

static void kirby_timer_tick(){
	AccelData accel = {false, 0, 0, 0, 0};
	
	accel_service_peek(&accel);
	
	tick_mod++;
	
	if(tick_mod == 10){
		current_kirby++;
		if(current_kirby > 6) current_kirby = 0;
		tick_mod = 0;
	}
	
	kirby_accel_raw_handler(&accel, 1);
	
	app_timer_register(10, kirby_timer_tick, NULL);
}

void kirby_handle_minute(struct tm *tick_time, TimeUnits units){
	if(clock_is_24h_style()) strftime(padded_time, sizeof(padded_time), "%H %M", tick_time);
	else strftime(padded_time, sizeof(padded_time), "%I %M", tick_time);
}

void kirby_init(Layer *layer) {

  //my_window = window_create();
	//window_set_background_color(my_window, GColorBlack);
	
	kirby_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_KIRBY_28));
	k_bg = gbitmap_create_with_resource(RESOURCE_ID_KIRBY_BG);
	
	accel_data_service_subscribe(1, NULL);
	
	app_timer_register(10, kirby_timer_tick, NULL);

	kirby_layer = layer_create(GRect(22,18,100,100));
	layer_set_update_proc(kirby_layer, draw_ball);
	layer_add_child(layer, kirby_layer);

	kirby_rolling[0] = gbitmap_create_with_resource(RESOURCE_ID_K0);
	kirby_rolling[1] = gbitmap_create_with_resource(RESOURCE_ID_K1);
	kirby_rolling[2] = gbitmap_create_with_resource(RESOURCE_ID_K2);
	kirby_rolling[3] = gbitmap_create_with_resource(RESOURCE_ID_K4);
	kirby_rolling[4] = gbitmap_create_with_resource(RESOURCE_ID_K3);
	kirby_rolling[5] = gbitmap_create_with_resource(RESOURCE_ID_K5);
	kirby_rolling[6] = gbitmap_create_with_resource(RESOURCE_ID_K6);
	
	//tick_timer_service_subscribe(SECOND_UNIT, kirby_handle_minute);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	kirby_handle_minute(t, MINUTE_UNIT);
	
  //window_stack_push(my_window, true);
}

void kirby_deinit(void) {
	//window_destroy(my_window);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit");
	accel_data_service_unsubscribe();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "accel unsub");
	layer_remove_from_parent(kirby_layer);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "kirby layer removed");
	
	layer_destroy(kirby_layer);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "kirby layer destroyed");
	
	for(uint8_t i = 0; i < 7; i++){
		gbitmap_destroy(kirby_rolling[i]);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "kirby image %i destroyed", i);
	}
	
	gbitmap_destroy(k_bg);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "bg destroyed");
	
	fonts_unload_custom_font(kirby_font);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "font unloaded");
}

#endif