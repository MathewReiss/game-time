#include <pebble.h>

#ifdef PBL_COLOR	
	
Layer *mario_layer;

GBitmap *ground, *block, *mario;

int min_height = 0, tick = 0, mario_height = 0;

char hour_buffer[3], min_buffer[3], next_hour[3], next_min[3];

static void mario_timer_tick(void *context){
	if(tick == 0){
		gbitmap_destroy(mario);
		mario = gbitmap_create_with_resource(RESOURCE_ID_MARIO_JUMP);
	}
	
	if(tick < 10){
		mario_height--;
	}
	else if(tick >= 10 && tick < 20){
		min_height--;
		mario_height++;	
	}
	else if(tick > 20 && tick <= 30){
		min_height++;
	}
	
	if(tick == 20){
		gbitmap_destroy(mario);
		mario = gbitmap_create_with_resource(RESOURCE_ID_MARIO_STAND);
		strncpy(hour_buffer, next_hour, sizeof(hour_buffer));
		strncpy(min_buffer, next_min, sizeof(min_buffer));
	}
	
	tick++;
	
	if(tick <= 30){
		app_timer_register(25, mario_timer_tick, NULL);
	}
	else{
		tick = 0;
		//app_timer_cancel(jump_timer);
	}

	layer_mark_dirty(mario_layer);
}

void mario_handle_tick(struct tm *tick_time, TimeUnits units){
	if(clock_is_24h_style()) strftime(next_hour, sizeof(next_hour), "%H", tick_time);
	else strftime(next_hour, sizeof(next_hour), "%I", tick_time);
	
	strftime(next_min, sizeof(next_min), "%M", tick_time);
	
	//Start App Timer
	app_timer_register(25, mario_timer_tick, NULL);
}

void draw_mario(Layer *layer, GContext *ctx){
	graphics_context_set_fill_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b101011});
	graphics_fill_rect(ctx, GRect(0,0,100,100), 0, GCornerNone);
	
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	
	graphics_draw_bitmap_in_rect(ctx, ground, GRect(0, 64, 100, 36));
	
	graphics_draw_bitmap_in_rect(ctx, mario, GRect(18+32,52+mario_height, 32,32));
	
	graphics_draw_bitmap_in_rect(ctx, block, GRect(18,12,32,32));
	graphics_draw_bitmap_in_rect(ctx, block, GRect(18+32,12+min_height,32,32));
	
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(ctx, hour_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(23,10,32,32), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, min_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(32+23,10+min_height,32,32), GTextOverflowModeFill, GTextAlignmentLeft, NULL);

	graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b100100});
	graphics_draw_text(ctx, hour_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(22,9,32,32), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	graphics_draw_text(ctx, min_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GRect(32+22,9+min_height,32,32), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

void mario_handle_init(Layer *root) {
  mario_layer = layer_create(GRect(22, 18, 100, 100));
	layer_set_update_proc(mario_layer, draw_mario);
	
	ground = gbitmap_create_with_resource(RESOURCE_ID_GROUND);
	block = gbitmap_create_with_resource(RESOURCE_ID_BLOCK);
	mario = gbitmap_create_with_resource(RESOURCE_ID_MARIO_STAND);
	
	layer_add_child(root, mario_layer);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	mario_handle_tick(t, MINUTE_UNIT);
}

void mario_handle_deinit(void) {
	layer_remove_from_parent(mario_layer);	

  layer_destroy(mario_layer);
	
	gbitmap_destroy(ground);
	gbitmap_destroy(block);
	gbitmap_destroy(mario);
}


#endif