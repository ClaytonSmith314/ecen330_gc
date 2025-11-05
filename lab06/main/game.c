
#include <stdio.h>
#include <stdlib.h> // rand

#include "hw.h"
#include "lcd.h"
#include "cursor.h"
#include "sound.h"
#include "pin.h"
#include "missile.h"
#include "plane.h"
#include "game.h"
#include "config.h"

// sound support
#include "missileLaunch.h"

// M2: Define stats constants
#define MISSILE_LAUNCH_BTN HW_BTN_A
#define START_VOLUME 60
#define STATS_STR_BUFF_SIZE 100

// All missiles
missile_t missiles[CONFIG_MAX_TOTAL_MISSILES];

// Alias into missiles array
missile_t *enemy_missiles = missiles+0;
missile_t *player_missiles = missiles+CONFIG_MAX_ENEMY_MISSILES;
missile_t *plane_missile = missiles+CONFIG_MAX_ENEMY_MISSILES+
									CONFIG_MAX_PLAYER_MISSILES;

// M2: Declare stats variables
uint32_t total_shots_fired = 0;
uint32_t total_missiles_impacted = 0;


bool missile_launch_btn_pressed = false;
uint8_t next_plr_missile_to_launch = 0;


// Initialize the game control logic.
// This function initializes all missiles, planes, stats, etc.
void game_init(void)
{
	// Initialize missiles
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
		missile_init(missiles+i);

	// Initialize plane
	plane_init(plane_missile);

	// M2: Initialize stats
	total_shots_fired = 0;
	total_missiles_impacted = 0;

	// M2: Set sound volume
	sound_set_volume(START_VOLUME);
}

// Update the game control logic.
// This function calls the missile & plane tick functions, relaunches
// idle enemy missiles, handles button presses, launches player missiles,
// detects collisions, and updates statistics.
void game_tick(void)
{
	// Tick missiles in one batch
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++)
		missile_tick(missiles+i);

	// Tick plane
	plane_tick();

	// Relaunch idle enemy missiles
	for (uint32_t i = 0; i < CONFIG_MAX_ENEMY_MISSILES; i++)
		if (missile_is_idle(enemy_missiles+i))
			missile_launch_enemy(enemy_missiles+i);

	// M1: Relaunch idle player missiles, !!! remove after Milestone 1 !!!
	// for (uint32_t i = 0; i < CONFIG_MAX_PLAYER_MISSILES; i++)
	// 	if (missile_is_idle(player_missiles+i))
	// 		missile_launch_player(player_missiles+i, rand()%LCD_W, rand()%LCD_H);

	// M2: Check for button press. If so, launch a free player missile.
	if(!missile_launch_btn_pressed) {
		//launch missile if missile launch btn pressed
		if(pin_get_level(MISSILE_LAUNCH_BTN)<=0 && missile_is_idle(player_missiles+next_plr_missile_to_launch)) {
			coord_t x, y;
			cursor_get_pos(&x, &y);
			missile_launch_player(player_missiles+next_plr_missile_to_launch, x, y);
			next_plr_missile_to_launch++;
			next_plr_missile_to_launch = next_plr_missile_to_launch%CONFIG_MAX_PLAYER_MISSILES;
			missile_launch_btn_pressed=true;
			total_shots_fired++;
			sound_start(missileLaunch, MISSILELAUNCH_SAMPLES, false);
		}
	} else {
		if(pin_get_level(MISSILE_LAUNCH_BTN)>0) {
			missile_launch_btn_pressed=false;
		}
	}


	// M2: Check for moving non-player missile collision with an explosion.
	// M2: Check for flying plane collision with an explosion.
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) 
	{
		// only exploding missiles can explode other missiles
		if(missile_is_exploding(missiles+i))
		{
			coord_t x, y;
			// explode missiles
			for (uint32_t j = 0; j < CONFIG_MAX_TOTAL_MISSILES; j++)
			{
				// check if missile is right type and not same as exploding
				if(i!=j && missile_get_type(missiles+j)!=MISSILE_TYPE_PLAYER) 
				{
					missile_get_pos(missiles+j, &x, &y);
					if(missile_is_colliding(missiles+i, x, y)) {
						if(!missile_is_exploding(missiles+j)) {
							missile_explode(missiles+j);
						}
					}
				}
			}
			// explode plane
			plane_get_pos(&x, &y);
			if(missile_is_colliding(missiles+i, x, y)) {
				plane_explode();
			}
		}
	}

	// M2: Count non-player impacted missiles
	for (uint32_t i = 0; i < CONFIG_MAX_TOTAL_MISSILES; i++) {
		if(missile_is_impacted(missiles+i)) {
			total_missiles_impacted++;
		}
	}

	// M2: Draw stats
	static char stats_str[STATS_STR_BUFF_SIZE];
	sprintf(stats_str, "Shots Fired: %ld  Missiles Impacted: %ld", 
	total_shots_fired, total_missiles_impacted);
	lcd_drawString(0, 0, stats_str, CONFIG_COLOR_STATUS);


}
