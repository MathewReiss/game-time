#include <pebble.h>
	
#ifdef PBL_COLOR

#define PKMN_RED 0
#define PKMN_BLUE 1	
	
void pkmn_handle_tick(struct tm *tick_time, TimeUnits units);
void pkmn_handle_init(Layer *root, int set_color);
void pkmn_handle_deinit(void);

#endif