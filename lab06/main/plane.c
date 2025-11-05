
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "missile.h"
#include "plane.h"
#include "lcd.h" // coord_t
#include "config.h"

#define PLANE_ALTITUDE (LCD_H/4)
#define PLANE_OFF_SCREEN 

typedef enum {
	PLANE_ST_IDLE,
	PLANE_ST_FLYING
} plane_states_t;

plane_states_t plane_state;

missile_t *plane_missile_;
bool explode_me = false;
bool plane_missile_launched = false;

coord_t plane_x_pos;
uint32_t plane_idle_ticks=0;


/******************** Plane Init Function ********************/

// Initialize the plane state machine. Pass a pointer to the missile
// that will be (re)launched by the plane. It will only have one missile.
void plane_init(missile_t *plane_missile) {
    plane_state = PLANE_ST_FLYING;
    plane_missile_ = plane_missile;
    plane_x_pos = LCD_W + CONFIG_PLANE_WIDTH/2;
    plane_missile_launched = false;
}

/******************** Plane Control & Tick Functions ********************/

// Trigger the plane to explode.
void plane_explode(void) {
    explode_me = true;
}

// State machine tick function.
void plane_tick(void) {
    // state switch + mealy actions
    switch(plane_state) {
        case PLANE_ST_IDLE: {
            if(plane_idle_ticks >= CONFIG_PLANE_IDLE_TIME_TICKS) {
                plane_state = PLANE_ST_FLYING;
                plane_x_pos = LCD_W + CONFIG_PLANE_WIDTH/2;
                plane_missile_launched = false;
            }
        } break;
        case PLANE_ST_FLYING: {
            if(explode_me || plane_x_pos <= -CONFIG_PLANE_WIDTH/2) {
                plane_state = PLANE_ST_IDLE;
                plane_idle_ticks = 0;
                explode_me = false;
            }

        } break;
    }
    // actions
    switch(plane_state) {
        case PLANE_ST_IDLE: {
            plane_idle_ticks++;
        } break;
        // draw and move plane
        case PLANE_ST_FLYING: {
            lcd_fillTriangle(
                plane_x_pos-CONFIG_PLANE_WIDTH/2, PLANE_ALTITUDE,
                plane_x_pos+CONFIG_PLANE_WIDTH/2, PLANE_ALTITUDE-CONFIG_PLANE_HEIGHT/2,
                plane_x_pos+CONFIG_PLANE_WIDTH/2, PLANE_ALTITUDE+CONFIG_PLANE_HEIGHT/2,
                CONFIG_COLOR_PLANE
            );
            plane_x_pos -= CONFIG_PLANE_DISTANCE_PER_TICK;
            if((!plane_missile_launched) && (plane_x_pos<LCD_W/2)) {
                missile_launch_plane(plane_missile_, plane_x_pos, PLANE_ALTITUDE);
                plane_missile_launched=true;
            }
        } break;
    }
}

/******************** Plane Status Functions ********************/

// Return the current plane position through the pointers *x,*y.
void plane_get_pos(coord_t *x, coord_t *y) {
    *x = plane_x_pos;
    *y = PLANE_ALTITUDE;
}

// Return whether the plane is flying.
bool plane_is_flying(void) {
    return plane_state==PLANE_ST_FLYING;
}
