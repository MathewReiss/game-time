#include <pebble.h>
#include "main.h"

#ifdef PBL_COLOR
	
#include "tetris.h"
#include "kirby.h"	
#include "pokemon.h"
#include "mario.h"
	
Window *my_window;
Layer *time_layer;

GFont *gameboy_font;

char time_buffer[8], padded_time[16];

GSize gb_char_size;

GBitmap *gb_logo, *pwr_icon, *mario_bg, *dk_bg, *kirby_bg;

#define COLOR 0
#define GAME 1	
	
int current_color = 0, current_game = 0; //Purple + Gameboy are Default
	
#define PURPLE 0
#define BLUE 1
#define YELLOW 2
#define GREEN 3
#define RED 4
#define TRANSPARENT 5
#define NO_COLOR 6
#define TRANSPARENT_DOT 7
#define RANDOM 20
bool randomize_color = false;
	
#define GAMEBOY 0 //
#define MARIO 1 //
#define KIRBY 2 //
#define TETRIS 3
#define POKEMON_RED 4
#define POKEMON_BLUE 5
	
void handle_tick(struct tm *tick_time, TimeUnits units_changed);	
	
void init_time(){
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	handle_tick(t, MINUTE_UNIT);
}

void load_game_fonts(){
	gameboy_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARIAL_32));
}	
	
void unload_current_game(){
	switch(current_game){
		case GAMEBOY: break;
		case MARIO: mario_handle_deinit(); break;
		case POKEMON_RED: pkmn_handle_deinit(); break;
		case POKEMON_BLUE: pkmn_handle_deinit(); break;
		case TETRIS: tetris_deinit(); break;
		case KIRBY: kirby_deinit(); break;
	}
}

void load_current_game(){
	switch(current_game){
		case GAMEBOY: init_time(); break;
		case MARIO: mario_handle_init(time_layer); break;
		case POKEMON_RED: pkmn_handle_init(time_layer, PKMN_RED); break;
		case POKEMON_BLUE: pkmn_handle_init(time_layer, PKMN_BLUE); break;
		case TETRIS: tetris_init(time_layer); break;
		case KIRBY: kirby_init(time_layer); break;
		default: current_game = GAMEBOY; persist_write_int(GAME, GAMEBOY); init_time(); break;
	}
}

void process_tuple(Tuple *t){
	int key = t->key;
	
	if(key == COLOR){
		current_color = atoi(t->value->cstring);
		persist_write_int(COLOR, current_color);
	}
	else if(key == GAME){
		unload_current_game();
		current_game = atoi(t->value->cstring);
		persist_write_int(GAME, current_game);
		load_current_game();
	}
}

void inbox(DictionaryIterator *iter, void *context){
	Tuple *t = dict_read_first(iter);
	if(t) process_tuple(t);
	while(t != NULL){
		t = dict_read_next(iter);
		if(t) process_tuple(t);
	}
	
	layer_mark_dirty(time_layer);
}

static GColor8 get_color(int choice){
	switch(choice){
		case PURPLE: return ((GColor8){.argb = GColorBlackARGB8 + 0b010110});
		case BLUE: return ((GColor8){.argb = GColorBlackARGB8 + 0b001011});
		case YELLOW: return ((GColor8){.argb = GColorBlackARGB8 + 0b111101});
		case GREEN: return ((GColor8){.argb = GColorBlackARGB8 + 0b101100});
		case RED: return ((GColor8){.argb = GColorBlackARGB8 + 0b110001});
		case TRANSPARENT: return ((GColor8){.argb = GColorBlackARGB8 + 0b101010});
		case TRANSPARENT_DOT: return ((GColor8){.argb = GColorBlackARGB8 + 0b100110});
		default: return ((GColor8){.argb = GColorWhiteARGB8});
	}
}

static void draw_color(Layer *layer, GContext *ctx){
	graphics_context_set_fill_color(ctx, get_color(current_color));
	graphics_fill_rect(ctx, GRect(0,0,144,168), 4, GCornersAll);
	
	if(current_color == TRANSPARENT){
		graphics_context_set_stroke_color(ctx, get_color(TRANSPARENT_DOT));
		int fourth_pixel = 0;
		for(int y = 0; y < 168; y++){
			fourth_pixel = (y%2)*2;
			for(int x = 0; x < 144; x++){
				fourth_pixel++;
				if(fourth_pixel%4 == 0) graphics_draw_pixel(ctx, GPoint(x,y));
			}
		}
		graphics_context_set_stroke_color(ctx, get_color(PURPLE));
		graphics_draw_round_rect(ctx, GRect(0,0,144,168), 4);
		graphics_context_set_stroke_color(ctx, get_color(TRANSPARENT_DOT));
		graphics_draw_round_rect(ctx, GRect(1,1,142,166), 4);
	}
}

static void draw_overlay(Layer *layer, GContext *ctx){
	//Draw Black Box
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, GRect(10,10,124,124), 2, GCornersAll);
	
	//Draw Gameboy Color Logo - GBitmap
	graphics_draw_bitmap_in_rect(ctx, gb_logo, GRect(22,122,100,8));

	//Draw Power Indicator - GBitmap
	graphics_draw_bitmap_in_rect(ctx, pwr_icon, GRect(12,50,10,8));
	
	//Draw Nintendo Logo
	switch(current_color){
		case PURPLE: 
			graphics_context_set_stroke_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b010011}); 
			graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b010011});
		break;
		
		case BLUE: 
			graphics_context_set_stroke_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b001111}); 
			graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b001111});		
		break;
		
		case YELLOW: 
			graphics_context_set_stroke_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b111001}); 
			graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b111001});		
		break;
		
		case GREEN: 
			graphics_context_set_stroke_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b011100}); 
			graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b011100});		
		break;
		
		case RED: 
			graphics_context_set_stroke_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b110101}); 
			graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b110101});		
		break;
		
		case TRANSPARENT: 
			graphics_context_set_stroke_color(ctx, get_color(PURPLE));
			graphics_context_set_text_color(ctx, get_color(PURPLE));
		break;
	}
	
	graphics_draw_text(ctx, "Nintendo", fonts_get_system_font(FONT_KEY_GOTHIC_18), GRect(0,138,144,168), GTextOverflowModeFill, GTextAlignmentCenter, NULL);
	graphics_draw_round_rect(ctx, GRect(40, 142, 64, 18), 8);
}

bool long_time = false;
char current[1], mario_time[10];
int gap = 0;

static void draw_game(Layer *layer, GContext *ctx){
	GRect game_rect = GRect(22,18,100,100);
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, game_rect, 0, GCornerNone);

	switch(current_game){
		case GAMEBOY: 

			if(long_time){
				for(uint8_t i = 0; i < 5; i++){
                    current[0] = time_buffer[i];
					
					gap = i >= 3 ? gb_char_size.w/2 : 0;
				
					graphics_context_set_text_color(ctx, GColorLightGray);
					graphics_draw_text(ctx, current, gameboy_font, GRect(22+gb_char_size.w*i-gap+1, 48+1, 100, 48), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
					
			        switch(i){
						case 0: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b010110}); break;
						case 1: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b001001}); break;
						case 2: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b100001}); break;
						case 3: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b100001}); break;
						case 4: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b111001}); break;
					}
					
					graphics_draw_text(ctx, current, gameboy_font, GRect(22+gb_char_size.w*i-gap, 48, 100, 48), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
				}
			}
		    else{
				for(uint8_t i = 0; i < 4; i++){
					current[0] = time_buffer[i];

					gap = i >= 2 ? gb_char_size.w/2 : 0;					
					
					graphics_context_set_text_color(ctx, GColorLightGray);
					graphics_draw_text(ctx, current, gameboy_font, GRect(22+gb_char_size.w*i-gap+1, 48+1, 100, 48), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
	
					switch(i){
						case 0: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b010110}); break;
						case 1: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b001001}); break;
						case 2: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b100001}); break;
						case 3: graphics_context_set_text_color(ctx, (GColor8){.argb = GColorBlackARGB8 + 0b111001}); break;
					}

					graphics_draw_text(ctx, &time_buffer[i], gameboy_font, GRect(22+gb_char_size.w/2+gb_char_size.w*i-gap, 48, 100, 48), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
				}
			}
		break;
		
		case TETRIS: 
			graphics_context_set_fill_color(ctx, GColorDarkGray);
			graphics_fill_rect(ctx, GRect(22,18,100,100), 0, GCornersAll);
			graphics_context_set_fill_color(ctx, GColorBlack);
			graphics_fill_rect(ctx, GRect(23,19,98,98), 0, GCornersAll);
		break;
	}	
}


static void draw_time(Layer *layer, GContext *ctx){
	draw_color(layer, ctx);
	draw_overlay(layer, ctx);
	draw_game(layer, ctx);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed){
	if(false && tick_time->tm_min%3 == 0){
		if(current_game == TETRIS){
			tetris_deinit();
			kirby_init(time_layer);
			current_game = KIRBY;
		}
		else if(current_game == KIRBY){
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Current Game: KIRBY");
			kirby_deinit();
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "KIRBY Deinit");			
			pkmn_handle_init(time_layer, RED);
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "POKEMON Init");
			current_game = POKEMON_RED;
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Current Game: POKEMON");
		}
		else if(current_game == POKEMON_RED){
			pkmn_handle_deinit();
			pkmn_handle_init(time_layer, BLUE);
			current_game = POKEMON_BLUE;
		}
		else if(current_game == POKEMON_BLUE){
			pkmn_handle_deinit();
			mario_handle_init(time_layer);
			current_game = MARIO;
		}
		else if(current_game == MARIO){
			mario_handle_deinit();
			current_game = GAMEBOY;
		}
		else if(current_game == GAMEBOY){
			tetris_init(time_layer);
			current_game = TETRIS;
		}
		//current_color = current_color < 5 ? current_color+1 : 0;
		//current_game = current_game == TETRIS ? GAMEBOY : TETRIS;
	}
	
	if(current_game == TETRIS){
		tetris_tick_handler(tick_time, units_changed);
		layer_mark_dirty(time_layer);
		return;
	}
	
	if(current_game == KIRBY){ 
		kirby_handle_minute(tick_time, units_changed);
		layer_mark_dirty(time_layer);
		return;
	}
	
	if(current_game == MARIO){
		mario_handle_tick(tick_time, units_changed);
		layer_mark_dirty(time_layer);
		return;
	}
	
	if(current_game == POKEMON_RED || current_game == POKEMON_BLUE){
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Current Game: POKEMON");
		pkmn_handle_tick(tick_time, units_changed);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "POKEMON Handle Tick");
		layer_mark_dirty(time_layer);
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "layer dirty");
		return;
	}
	
	//if(randomize_color && tick_time->tm_min%10 == 0) current_color = current_color < 5 ? current_color+1 : 0;
	
	if(clock_is_24h_style()){
		strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
		long_time = true;
	}
	else{
		if(tick_time->tm_hour == 0 || (tick_time->tm_hour > 9 && tick_time->tm_hour < 13) || tick_time->tm_hour > 21){ 
			long_time = true; 
			strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
		}
		else{
			char zero_padded_time[8];
			strftime(zero_padded_time, sizeof(zero_padded_time), "%I:%M", tick_time);
			for(uint8_t i = 1; i < sizeof(zero_padded_time); i++){
				time_buffer[i-1] = zero_padded_time[i];
			}
			long_time = false;
		}
	}
	
	layer_mark_dirty(time_layer);
}

void handle_init(void){
	app_message_open(8,8);
	app_message_register_inbox_received(inbox);
	
	my_window = window_create();
	window_set_background_color(my_window, GColorBlack);
	
	if(persist_exists(GAME)) current_game = persist_read_int(GAME);
	if(persist_exists(COLOR)) current_color = persist_read_int(COLOR);
	
	load_game_fonts();
	gb_char_size = graphics_text_layout_get_content_size("1", gameboy_font, GRect(0,0,32,32), GTextOverflowModeFill, GTextAlignmentLeft);
	
	gb_logo = gbitmap_create_with_resource(RESOURCE_ID_GB_LOGO);
	pwr_icon = gbitmap_create_with_resource(RESOURCE_ID_POWER_ICON);
	
	time_layer = layer_create(GRect(0,0,144,168));
	layer_set_update_proc(time_layer, draw_time);
	
	layer_add_child(window_get_root_layer(my_window), time_layer);
	window_stack_push(my_window, true);
	
	//TESTING ONLY
	//light_enable(true);
	
	if(current_game == TETRIS){
		tetris_init(time_layer);
	}
	else if(current_game == KIRBY){
		kirby_init(time_layer);
	}
	else if(current_game == POKEMON_RED){
		pkmn_handle_init(time_layer, 0);
	}
	else if(current_game == POKEMON_BLUE){
		pkmn_handle_init(time_layer, 1);
	}
	else if(current_game == MARIO){
		mario_handle_init(time_layer);
	}
	
	tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
	
	init_time();
}

void handle_deinit(void){
	window_destroy(my_window);
	layer_destroy(time_layer);
	
	fonts_unload_custom_font(gameboy_font);
	
	gbitmap_destroy(gb_logo);
	gbitmap_destroy(pwr_icon);
	
	app_message_deregister_callbacks();
	tick_timer_service_unsubscribe();
	
	switch(current_game){
		case GAMEBOY: break;
		case MARIO: mario_handle_deinit(); break;
		case POKEMON_RED:
		case POKEMON_BLUE: pkmn_handle_deinit(); break;
		case TETRIS: tetris_deinit(); break;
		case KIRBY: kirby_deinit(); break;
	}
}

#endif