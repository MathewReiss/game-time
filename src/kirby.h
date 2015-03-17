#include <pebble.h>
	
#ifdef PBL_COLOR	
	
void kirby_handle_minute(struct tm *tick_time, TimeUnits units);

void kirby_init(Layer *layer);
void kirby_deinit(void);

#endif