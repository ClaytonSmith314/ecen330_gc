#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "missile.h"
#include "lcd.h" // coord_t
#include "config.h"


#define MISSILE_PLR_START_Y LCD_H
#define X_MISSILE_BUCKETS 3
#define ENEMY_LOWEST_START_ALTITUDE 50

#define ENEMY_DEST_Y (LCD_H-1)

#define DIST(dx,dy) (sqrt((dx)*(dx)+(dy)*(dy)))

// The same missile structure is used for all missiles in the game.
// All state variables for a missile are contained in the missile structure.
// There are no global variables maintained in the missile.c file.
// Each missile function requires a pointer argument to a missile struct.



typedef enum {
	MISSILE_ST_IDLE,
	MISSILE_ST_MOVING,
	MISSILE_ST_GROW,
	MISSILE_ST_SHRINK,
	MISSILE_ST_IMPACT
} missile_states_t;


/******************** Missile Init & Launch Functions ********************/

// Different _launch_ functions are used depending on the missile type.

// Initialize the missile as an idle missile. When initialized to the idle
// state, a missile doesn't appear nor does it move. The launch flag should
// also be set to false. Other missile_t members will be set up at launch.
void missile_init(missile_t *missile) {
	missile->currentState = MISSILE_ST_IDLE;
	missile->launch = false;
}

coord_t select_plr_x_origin(coord_t x_dest) {
	uint32_t bucket = x_dest*X_MISSILE_BUCKETS / LCD_W; //bucket between 0 and 2 inclusive
	coord_t x_origin = (bucket*LCD_W+LCD_W/2)/X_MISSILE_BUCKETS; //split screen into buckets & select midpoint of bucket
	return x_origin;
}

coord_t missile_select_enemy_x() {
	return rand()%LCD_W;
}

coord_t missile_select_enemy_y_origin() {
	return rand()%ENEMY_LOWEST_START_ALTITUDE;
}

void missile_complete_setup(missile_t *missile) {
	missile->currentState = MISSILE_ST_MOVING;
	missile->x_current = missile->x_origin;
	missile->y_current = missile->y_origin;
	missile->length = 0;
	missile->total_length = DIST(missile->x_dest-missile->x_origin, missile->y_dest- missile->y_origin);
	missile->launch = true;
	missile->explode_me = false;
}

color_t missile_get_color(missile_type_t type) {
	switch(type) {
		case MISSILE_TYPE_PLAYER: return CONFIG_COLOR_PLAYER_MISSILE;
		case MISSILE_TYPE_ENEMY: return CONFIG_COLOR_ENEMY_MISSILE;
		case MISSILE_TYPE_PLANE: return CONFIG_COLOR_PLANE_MISSILE;
	}
	return CONFIG_COLOR_PLAYER_MISSILE;
}


// Launch the missile as a player missile. This function takes an (x, y)
// destination of the missile (as specified by the user). The origin is the
// closest "firing location" to the destination (there are three firing
// locations evenly spaced along the bottom of the screen).
void missile_launch_player(missile_t *missile, coord_t x_dest, coord_t y_dest) {
	missile->type = MISSILE_TYPE_PLAYER;
	missile->x_origin = select_plr_x_origin(x_dest);
	missile->y_origin = MISSILE_PLR_START_Y;
	missile->x_dest = x_dest;
	missile->y_dest = y_dest;
	missile_complete_setup(missile);
}

// Launch the missile as an enemy missile. This will randomly choose the
// origin and destination of the missile. The origin is somewhere near the
// top of the screen, and the destination is the very bottom of the screen.
void missile_launch_enemy(missile_t *missile) {
	missile->type = MISSILE_TYPE_ENEMY;
	missile->x_origin = missile_select_enemy_x();
	missile->y_origin = missile_select_enemy_y_origin();
	missile->x_dest = missile_select_enemy_x();
	missile->y_dest = ENEMY_DEST_Y;
	missile_complete_setup(missile);
}

// Launch the missile as a plane missile. This function takes the (x, y)
// location of the plane as an argument and uses it as the missile origin.
// The destination is randomly chosen along the bottom of the screen.
void missile_launch_plane(missile_t *missile, coord_t x_orig, coord_t y_orig) {
	missile->type = MISSILE_TYPE_PLANE;
	missile->x_origin = x_orig;
	missile->y_origin = y_orig;
	missile->x_dest = missile_select_enemy_x();
	missile->y_dest = LCD_H;
	missile_complete_setup(missile);
}

/******************** Missile Control & Tick Functions ********************/

// Used to indicate that a moving missile should be detonated. This occurs
// when an enemy or a plane missile is located within an explosion zone.
void missile_explode(missile_t *missile) {
	missile->explode_me = true;
	missile->radius = 0;
}

// Tick the state machine for a single missile.
void missile_tick(missile_t *missile) {
	switch(missile->currentState) {
		case MISSILE_ST_IDLE: break;
		case MISSILE_ST_MOVING: {
			if(missile->length > missile->total_length){
				if(missile->type==MISSILE_TYPE_PLAYER) {
					missile->currentState = MISSILE_ST_GROW;
				} else {
					missile->currentState = MISSILE_ST_IMPACT;
				}
			}
		} break;
		case MISSILE_ST_GROW: {
			if(missile->radius >= CONFIG_EXPLOSION_MAX_RADIUS) {
				missile->currentState = MISSILE_ST_SHRINK;
			}
		} break;
		case MISSILE_ST_SHRINK: {
			if(missile->radius <= 0) {
				missile->currentState = MISSILE_ST_IDLE;
			}
		} break;
		case MISSILE_ST_IMPACT: {
			missile->currentState = MISSILE_ST_IDLE;
		} break;
	}

	switch(missile->currentState) {
		case MISSILE_ST_IDLE: break;
		case MISSILE_ST_MOVING: {
			uint32_t dist_move;
			switch(missile->type) {
				case MISSILE_TYPE_PLAYER: {
					dist_move = CONFIG_PLAYER_MISSILE_DISTANCE_PER_TICK;
				} break;
				case MISSILE_TYPE_ENEMY: {
					dist_move = CONFIG_ENEMY_MISSILE_DISTANCE_PER_TICK;
				} break;
				case MISSILE_TYPE_PLANE: {
					dist_move = CONFIG_PLANE_DISTANCE_PER_TICK;
				} break;
				default: {dist_move=0;} break;
			}
			missile->length += dist_move;
			missile->x_current = missile->x_origin + ((missile->x_dest-missile->x_origin)*missile->length)/missile->total_length;
			missile->y_current = missile->y_origin + ((missile->y_dest-missile->y_origin)*missile->length)/missile->total_length;

			lcd_drawLine(missile->x_origin, missile->y_origin, missile->x_current, missile->y_current, missile_get_color(missile->type));
		} break;
		case MISSILE_ST_GROW: {
			missile->radius += CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
			lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, missile_get_color(missile->type));
		} break;
		case MISSILE_ST_SHRINK: {
			missile->radius -= CONFIG_EXPLOSION_RADIUS_CHANGE_PER_TICK;
			lcd_fillCircle(missile->x_current, missile->y_current, missile->radius, missile_get_color(missile->type));
		} break;
		case MISSILE_ST_IMPACT: {
			
		} break;
	}
}

/******************** Missile Status Functions ********************/

// Return the current missile position through the pointers *x,*y.
void missile_get_pos(missile_t *missile, coord_t *x, coord_t *y) {
	*x = missile->x_current;
	*y = missile->y_current;
}

// Return the missile type.
missile_type_t missile_get_type(missile_t *missile) {
	return missile->type;
}

// Return whether the given missile is moving.
bool missile_is_moving(missile_t *missile) {
	return missile->currentState == MISSILE_ST_MOVING;
}

// Return whether the given missile is exploding. If this missile
// is exploding, it can explode another intersecting missile.
bool missile_is_exploding(missile_t *missile) {
	return missile->currentState==MISSILE_ST_GROW || missile->currentState==MISSILE_ST_SHRINK;
}

// Return whether the given missile is idle.
bool missile_is_idle(missile_t *missile) {
	return missile->currentState==MISSILE_ST_IDLE;
}

// Return whether the given missile is impacted.
bool missile_is_impacted(missile_t *missile) {
	return missile->currentState==MISSILE_ST_IMPACT;
}

// Return whether an object (e.g., missile or plane) at the specified
// (x,y) position is colliding with the given missile. For a collision
// to occur, the missile needs to be exploding and the specified
// position needs to be within the explosion radius.
bool missile_is_colliding(missile_t *missile, coord_t x, coord_t y) {
	if(missile_is_exploding(missile)) {
		return DIST(x-missile->x_current, y-missile->y_current) <= missile->radius;
	} else {
		return false;
	}
}
