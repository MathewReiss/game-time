#include <pebble.h>
#include "main.h"
#include "main_bw.h"

int main(void){
	#ifdef PBL_COLOR
		handle_init();
		app_event_loop();
		handle_deinit();
	#else
		handle_init_bw();
		app_event_loop();
		handle_deinit_bw();
	#endif
}