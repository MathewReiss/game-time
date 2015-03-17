#include <pebble.h>
#include "pokemon.h"

#ifdef PBL_COLOR
	
Layer *battle_layer;
	
int color = 1, exp_pts = 3;

//Lvl: 99
char exp_pts_buffer[10];

GFont pokefont, pokefont_small;

int b_front_x = 44, b_front_y = -10, b_back_x = 0, b_back_y = 10;
int min_ally = 0, min_enemy = 0;

char hour_buffer[3], min_buffer[3], time_buffer[16];

GBitmap *current_front, *current_back, *corner;

void draw_battle(Layer *layer, GContext *ctx){
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, GRect(0,0,100,100), 0, GCornerNone);
		
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_draw_round_rect(ctx, GRect(1,67,98,32), 2);
	
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	
	graphics_draw_bitmap_in_rect(ctx, corner, GRect(0,66,6,6));
	graphics_draw_bitmap_in_rect(ctx, corner, GRect(94,66,6,6));
	graphics_draw_bitmap_in_rect(ctx, corner, GRect(0,94,6,6));
	graphics_draw_bitmap_in_rect(ctx, corner, GRect(94,94,6,6));
	
	graphics_draw_bitmap_in_rect(ctx, current_back, GRect(b_back_x,b_back_y,56,56));
	graphics_draw_bitmap_in_rect(ctx, current_front, GRect(b_front_x,b_front_y,56,56));
	
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, time_buffer, pokefont, GRect(2,72,96,32), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	
	//Add Level
	graphics_draw_text(ctx, exp_pts_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, 0, 50, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, exp_pts_buffer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK), GRect(60, 43, 50, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	
	//Add Dynamic Health Bars
	if(min_enemy < 15)
		graphics_context_set_fill_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b001001});
	else if(min_enemy < 24)
		graphics_context_set_fill_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b111001});
	else
		graphics_context_set_fill_color(ctx, GColorRed);
	graphics_fill_rect(ctx, GRect(10,16,30-min_enemy,4), 0, GCornerNone);
	graphics_draw_round_rect(ctx, GRect(10,15, 30, 6), 2);
	
	if(min_ally < 15)
		graphics_context_set_fill_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b001001});
	else if(min_ally < 24)
		graphics_context_set_fill_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b111001});
	else
		graphics_context_set_fill_color(ctx, GColorRed);
	graphics_fill_rect(ctx, GRect(60,59,30-min_ally,4), 0, GCornerNone);
	graphics_draw_round_rect(ctx, GRect(60, 58, 30, 6), 2);
}

void init_battle(struct tm *tick_time){
	min_enemy = tick_time->tm_min/2;
	if(min_enemy == 30) min_enemy = 29;
	min_ally = tick_time->tm_min/2;
	if(min_ally == 29) min_ally = 28;
	snprintf(exp_pts_buffer, sizeof(exp_pts_buffer), "Lvl: %i", exp_pts);
}

void pkmn_handle_tick(struct tm *tick_time, TimeUnits units){
	if(clock_is_24h_style())
		strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
	else
		strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
	
	if(tick_time->tm_min%2){
		min_enemy++;
		if(min_enemy == 30) min_enemy = 29;
	}
	else{
		min_ally++;
		if(min_ally == 29) min_ally = 28;
	}
	if(tick_time->tm_min == 0){
		min_enemy = 0;
		min_ally = 0;
		exp_pts++;
		if(exp_pts == 101) exp_pts = 100;
		snprintf(exp_pts_buffer, sizeof(exp_pts_buffer), "Lvl: %i", exp_pts);
	}
	
	layer_mark_dirty(battle_layer);
}

void pkmn_handle_init(Layer *root, int set_color) {
  	
  battle_layer = layer_create(GRect(22, 18, 100, 100));
	layer_set_update_proc(battle_layer, draw_battle);
	
	//tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	
	layer_add_child(root, battle_layer);
	
	color = set_color;
	
	if(color == PKMN_RED){
		current_front = gbitmap_create_with_resource(RESOURCE_ID_SQ_FRONT);
		current_back = gbitmap_create_with_resource(RESOURCE_ID_CHAR_BACK);
	}
	else{
		current_front = gbitmap_create_with_resource(RESOURCE_ID_FRONT_RB);
		current_back = gbitmap_create_with_resource(RESOURCE_ID_SQ_BACK);
	}
	
	corner = gbitmap_create_with_resource(RESOURCE_ID_CORNER);
	
	pokefont = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKEMON_18));
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	pkmn_handle_tick(t, MINUTE_UNIT);
	init_battle(t);
}

void pkmn_handle_deinit(void) {
  layer_remove_from_parent(battle_layer);
	layer_destroy(battle_layer);
	
	gbitmap_destroy(corner);
	gbitmap_destroy(current_front);
	gbitmap_destroy(current_back);
	
	fonts_unload_custom_font(pokefont);
	
	exp_pts = 3;
	snprintf(exp_pts_buffer, sizeof(exp_pts_buffer), "Lvl: %i", exp_pts);
}

#endif