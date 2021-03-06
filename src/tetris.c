#include <pebble.h>
#include "tetris.h"
	
#ifdef PBL_COLOR	
	
#define DATE_SPACING 2

#define ANIMATION_SPACING_Y (TETRIMINO_MASK_SIZE + 1)
#define ANIMATION_PERIOD_INVIS_FRAMES 1
#define ANIMATION_PERIOD_VIS_FRAMES 2
#define ANIMATION_PERIOD_FRAMES (ANIMATION_PERIOD_INVIS_FRAMES + ANIMATION_PERIOD_VIS_FRAMES)
#define ANIMATION_PERIODS 3
#define ANIMATION_TIMEOUT_MS 100

#define DYNAMIC_ASSEMBLY 0
#define BIDIRECTIONAL_SYNC 0

#define STATE_COUNT 5
#define TIME_TO_SPLIT_SPACING 2
#define SPLIT_TO_DATE_SPACING 2
#define DATE_LINE_SPACING 1

typedef struct {
    int offset_x;
    int offset_y;
    
    char falling;

    int target_value;
    int next_value;
    DigitDef target;
    DigitDef current;

    int action_height;
    int vanishing_frame;

    int restricted_spawn_width;
} DigitState;

static PaletteColor s_bg_color;
static PaletteColor s_fg_color;

static int s_animating;
static Layer* s_layer;

static DigitState s_states[STATE_COUNT];
static int s_show_second_dot = 1;

static void state_step(DigitState* state) {
    if (!state->falling) {
        if (state->next_value != state->target_value) {
            if (state->vanishing_frame > ANIMATION_PERIODS * ANIMATION_PERIOD_FRAMES) {
                //APP_LOG(APP_LOG_LEVEL_INFO, "Digit target changed to %d", state->next_value);
                state->target_value = state->next_value;
                if (DYNAMIC_ASSEMBLY) {
                    reorder_digit(&state->target, &s_digits[state->target_value]);
                } else {
                    state->target = s_digits[state->target_value];
                }
                state->current.size = 0;
                state->falling = 1;
                state->vanishing_frame = 0;
            } else {
                state->vanishing_frame += 1;
                return;
            }
        } else {
            return;
        }
    }
   
    int last_y = TETRIMINO_MASK_SIZE;
    for (int i = 0; i < state->current.size; ++i) {
        TetriminoPos* current_pos = &state->current.tetriminos[i];
        const TetriminoPos* target_pos = &state->target.tetriminos[i];

        const int height_remaining = target_pos->y - current_pos->y;
        const int moves_needed = abs(target_pos->x - current_pos->x);
        int rotations_needed = target_pos->rotation - current_pos->rotation;
        if (rotations_needed < 0) { rotations_needed += 4; }
        const int actions_needed = moves_needed + rotations_needed;

        if (state->action_height >= current_pos->y) {
            int step = height_remaining / (actions_needed + 1);
            state->action_height = current_pos->y + step;
        }

        if (current_pos->y < target_pos->y) {
            current_pos->y += 1;
        }

        if (current_pos->y >= state->action_height) {
            if (moves_needed > rotations_needed) {
                if (current_pos->x < target_pos->x) {
                    current_pos->x += 1;
                } else if (current_pos->x > target_pos->x) {
                    current_pos->x -= 1;
                }
            } else if (rotations_needed) {
                current_pos->rotation = (current_pos->rotation + 1) % 4;
            }
        }
        
     
        last_y = current_pos->y;
    }
    
    
    if (state->current.size < state->target.size) {
        const char target_letter = state->target.tetriminos[state->current.size].letter;
        const TetriminoDef* td = get_tetrimino_def(target_letter);

        const int start_y = -state->offset_y - td->size + 1;
        if (last_y >= (start_y + ANIMATION_SPACING_Y)) {
            TetriminoPos* current_pos = &state->current.tetriminos[state->current.size];
            current_pos->letter = target_letter;
            if (state->restricted_spawn_width) {
                const int spawn_width = 4;
                current_pos->x = rand() % (spawn_width - td->size + 1);
                current_pos->x += (DIGIT_WIDTH - spawn_width) / 2;
            } else {
                current_pos->x = rand() % (DIGIT_WIDTH - td->size + 1);
            }
            current_pos->y = start_y;
            current_pos->rotation = rand() % 4;
            state->action_height = start_y;
            state->current.size += 1;
        }
    }

    if (state->current.size == state->target.size) {
        if (state->current.size == 0 ||
            memcmp(&state->current.tetriminos[state->current.size-1],
                   &state->target.tetriminos[state->target.size-1],
                   sizeof(TetriminoPos)) == 0)
        {
            state->falling = 0;
        }
    }
}

static void draw_tetrimino(const TetriminoPos* tp, int offset_x, int offset_y) {
    const TetriminoDef* td = get_tetrimino_def(tp->letter); 
    const TetriminoMask* tm = &td->rotations[tp->rotation];
	int choice = get_tetrimino_clr(tp->letter);
	PaletteColor clr = COLOR_WHITE;
	
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "Choice: %i", choice);
	
	switch(choice){
		case 0: clr = COLOR_CYAN; break; //Cyan
		case 1: clr = COLOR_BLUE; break; //Blue
		case 2: clr = COLOR_ORANGE; break; //Orange
		case 3: clr = COLOR_YELLOW; break; //Yellow
		case 4: clr = COLOR_LIME; break; //Lime
		case 5: clr = COLOR_MAGENTA; break; //Magenta
		case 6: clr = COLOR_RED; break; //Red
		default: clr = COLOR_BLACK; break; //White
	}
	
    for (int mask_y = 0; mask_y < TETRIMINO_MASK_SIZE; ++mask_y) {
        for (int mask_x = 0; mask_x < TETRIMINO_MASK_SIZE; ++mask_x) {
            if ((*tm)[mask_y][mask_x]) {
                const int x = tp->x + mask_x + offset_x;
                const int y = tp->y + mask_y + offset_y;
                field_draw(x, y, clr);
            }
        }
    }
}

static void draw_digit_def(const DigitDef* def, int offset_x, int offset_y) {
    for (int i = 0; i < def->size; ++i) {
        draw_tetrimino(&def->tetriminos[i], offset_x, offset_y);
    }
}

static void draw_digit_state(const DigitState* state) {
    if (state->vanishing_frame) {
        const int in_period = (state->vanishing_frame - 1) % ANIMATION_PERIOD_FRAMES;
        if (in_period < ANIMATION_PERIOD_INVIS_FRAMES) {
            return;
        }
    }
    draw_digit_def(&state->current, state->offset_x, state->offset_y);
}

static void layer_draw(Layer* layer, GContext* ctx) {
    for (int i = 0; i < 4; ++i) {
        draw_digit_state(&s_states[i]);
    }
    if (s_show_second_dot || s_states[4].falling || s_states[4].vanishing_frame) {
        draw_digit_state(&s_states[4]);
    }
    //draw_date();
    field_flush(layer, ctx);
}


static void process_animation(void* data) {
    //
    static int st = 0;
    //APP_LOG(APP_LOG_LEVEL_INFO, "Step %d", st);
    st += 1;
    //
    
    s_animating = 1;
    for (int i = 0; i < STATE_COUNT; ++i) {
        state_step(&s_states[i]);
    }
    layer_mark_dirty(s_layer);
    for (int i = 0; i < STATE_COUNT; ++i) {
        if (s_states[i].falling || s_states[i].vanishing_frame) {
            app_timer_register(ANIMATION_TIMEOUT_MS, process_animation, NULL);
            return;
        }
    }
    s_animating = 0;
}

static void main_window_load(Layer *root) {
 //  s_layer = window_get_root_layer(window);

	//layer_set_frame(s_layer, GRect(22,22,100,100));
	
    //field_reset(s_bg_color);
   // layer_set_update_proc(s_layer, layer_draw);

	s_layer = layer_create(GRect(22,18,100,100));
	field_reset(COLOR_BLACK);
	layer_set_update_proc(s_layer, layer_draw);
	
	//window_set_background_color(window, GColorBlack);
	
   layer_add_child(root, s_layer);
}

static void main_window_unload() {
	layer_remove_from_parent(s_layer);
	layer_destroy(s_layer);
}

void tetris_tick_handler(struct tm* tick_time, TimeUnits units_changed) {
    const int clock24 = clock_is_24h_style();
    
    int digit_values[STATE_COUNT];
    int hour = tick_time->tm_hour;
    
    if (!clock24) {
        hour = hour % 12;
        if (hour == 0) {
            hour = 12;
        }
    }
    digit_values[0] = hour / 10;
    digit_values[1] = hour % 10;
    digit_values[2] = tick_time->tm_min / 10;
    digit_values[3] = tick_time->tm_min % 10;
    digit_values[4] = 10;

    if (!clock24 && digit_values[0] == 0) {
        digit_values[0] = DIGIT_COUNT;
    }
    
    int changed = 0;
    for (int i = 0; i < STATE_COUNT; ++i) {
        const int value = digit_values[i];
        if (s_states[i].next_value != value) {
            s_states[i].next_value = value;
            //APP_LOG(APP_LOG_LEVEL_INFO, "Digit %d scheduled to be %d", i, value);
            changed = 1;
        }
    }

    if (changed && !s_animating) {
        process_animation(NULL);
    }

    if (s_settings[ANIMATE_SECOND_DOT]) {
        s_show_second_dot = tick_time->tm_sec % 2;
        layer_mark_dirty(s_layer);
    }
}

static void on_settings_changed() {
    int digits_mode = s_settings[DIGITS_MODE];
    if (digits_mode == DM_ASYMMETRIC && clock_is_24h_style()) {
        digits_mode = DM_DENSE;
	}
        s_states[0].offset_x = 2;
        s_states[1].offset_x = 9;
        s_states[2].offset_x = 19;
        s_states[3].offset_x = 26;
        s_states[4].offset_x = 14;

    int offset_y = (FIELD_HEIGHT - DIGIT_HEIGHT) / 2;

    for (int i = 0; i < STATE_COUNT; ++i) {
        s_states[i].offset_y = offset_y;
    }

	    s_bg_color = COLOR_BLACK;
        s_fg_color = COLOR_WHITE;

	//tick_timer_service_subscribe(MINUTE_UNIT, tetris_tick_handler);

    field_reset(s_bg_color);
    if (s_layer) {
        layer_mark_dirty(s_layer);
    }
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	tetris_tick_handler(t, MINUTE_UNIT);
}
  
void tetris_init(Layer *root_layer) {
    srand(time(NULL));
    
#if USE_RAW_DIGITS == 1
    for (int i = 0; i < DIGIT_COUNT; ++i) {
        DigitDef def;
        if (!parse_raw_digit(&def, &s_raw_digits[i])) {
            return;
        }
        reorder_digit(&s_digits[i], &def);
    }
#endif

    for (int i = 0; i < STATE_COUNT; ++i) {
        s_states[i].next_value = -1;
        s_states[i].target_value = -1;
    }
    s_states[4].restricted_spawn_width = 1;
    
    // Create main Window element and assign to pointer
   // s_window = window_create();

    // Set handlers to manage the elements inside the Window
  	main_window_load(root_layer);

    on_settings_changed();
}

void tetris_deinit() {
    // Destroy Window
	//main_window_unload(s_window);
    //window_destroy(s_window);
	main_window_unload();
}

#endif