#include <pebble.h>

#ifdef PBL_COLOR	
	
void mario_handle_tick(struct tm *tick_time, TimeUnits units);
	
void mario_handle_init(Layer *root);
void mario_handle_deinit(void);

#endif