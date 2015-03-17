#include <pebble.h>
#include "main_bw.h"

#ifndef PBL_COLOR	
	
Window *my_window;
Layer *time_layer;

GFont *gameboy_font, *kirby_font;

char time_buffer[8], padded_time[16];

GBitmap *gb_logo, *pwr_icon;

#define GAME 1
	
int current_game = 0; //Gameboy is Default
	
#define GAMEBOY 0
	
void load_game_fonts(){
	gameboy_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARIAL_32));
}		
	
void inbox(DictionaryIterator *iter, void *context){}

static void draw_color(Layer *layer, GContext *ctx){
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(0,0,144,168), 4, GCornersAll);
}

static void draw_overlay(Layer *layer, GContext *ctx){
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_context_set_stroke_color(ctx, GColorBlack); 
	graphics_context_set_text_color(ctx, GColorBlack);	
	
	//Draw Black Box
	graphics_fill_rect(ctx, GRect(10,10,124,124), 2, GCornersAll);
	
	for(int y = 0; y < 168; y++){
		for(int x = 0; x < 144; x++){
			if(x%2 + y%2 == 0) graphics_draw_pixel(ctx, GPoint(x,y));
		}
	}
	
	//Draw Gameboy Color Logo - GBitmap
	graphics_draw_bitmap_in_rect(ctx, gb_logo, GRect(22,122,100,8));

	//Draw Power Indicator - GBitmap
	graphics_draw_bitmap_in_rect(ctx, pwr_icon, GRect(12,50,10,8));
	
	//Draw Nintendo Logo	
	graphics_draw_text(ctx, "Nintendo", fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(0,138,144,168), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_round_rect(ctx, GRect(40, 142, 64, 17), 8);
}

static void draw_game(Layer *layer, GContext *ctx){
	GRect game_rect = GRect(22,18,100,100);
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, game_rect, 0, GCornerNone);
	
	switch(current_game){
		case GAMEBOY: 
			graphics_context_set_text_color(ctx, GColorBlack);
			graphics_draw_text(ctx, time_buffer, gameboy_font, GRect(20, 48, 100, 48), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
		break;
	}
}

static void draw_time(Layer *layer, GContext *ctx){
	draw_color(layer, ctx);
	draw_overlay(layer, ctx);
	draw_game(layer, ctx);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed){
	if(clock_is_24h_style()){
		strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
	}
	else{
		if(tick_time->tm_hour == 0 || (tick_time->tm_hour > 9 && tick_time->tm_hour < 13) || tick_time->tm_hour > 21){ 
			strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
		}
		else{
			char zero_padded_time[8];
			strftime(zero_padded_time, sizeof(zero_padded_time), "%I:%M", tick_time);
			for(uint8_t i = 1; i < sizeof(zero_padded_time); i++){
				time_buffer[i-1] = zero_padded_time[i];
			}
		}
	}
	
	layer_mark_dirty(time_layer);
}

void handle_init_bw(void){
	app_message_open(8,8);
	app_message_register_inbox_received(inbox);
	
	my_window = window_create();
	window_set_background_color(my_window, GColorBlack);
	
	if(persist_exists(GAME)) current_game = persist_read_int(GAME);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	handle_tick(t, MINUTE_UNIT);
	
	load_game_fonts();
	
	gb_logo = gbitmap_create_with_resource(RESOURCE_ID_GB_LOGO);
	pwr_icon = gbitmap_create_with_resource(RESOURCE_ID_POWER_ICON);
	
	time_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(time_layer, draw_time);
	
	layer_add_child(window_get_root_layer(my_window), time_layer);
	window_stack_push(my_window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

void handle_deinit_bw(void){
	window_destroy(my_window);
	layer_destroy(time_layer);
	
	fonts_unload_custom_font(gameboy_font);
	
	gbitmap_destroy(gb_logo);
	gbitmap_destroy(pwr_icon);
	
	app_message_deregister_callbacks();
	tick_timer_service_unsubscribe();
}

#endif