#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lcd.h"
#include "pac.h"

static const char *TAG = "lab01";

#define DELAY_MS(ms) \
	vTaskDelay(((ms)+(portTICK_PERIOD_MS-1))/portTICK_PERIOD_MS)

//----------------------------------------------------------------------------//
// Car Implementation - Begin
//----------------------------------------------------------------------------//

// Car constants
#define CAR_CLR rgb565(220,30,0)
#define WINDOW_CLR rgb565(180,210,238)
#define TIRE_CLR BLACK
#define HUB_CLR GRAY

// Car Shape constants
#define CAR_W 60
#define CAR_H 32

#define BODY_X0 0
#define BODY_Y0 12
#define BODY_X1 59
#define BODY_Y1 24

#define CAB_X0 1
#define CAB_Y0 0
#define CAB_X1 39
#define CAB_Y1 11

#define BACK_WINDOW_X0 3
#define BACK_WINDOW_Y0 1
#define BACK_WINDOW_X1 18
#define BACK_WINDOW_Y1 8

#define FRONT_WINDOW_X0 21
#define FRONT_WINDOW_Y0 1
#define FRONT_WINDOW_X1 37
#define FRONT_WINDOW_Y1 8

#define WINDOW_RADIUS 2

#define HOOD_X0 40
#define HOOD_Y0 9
#define HOOD_X1 39
#define HOOD_Y1 11
#define HOOD_X2 59
#define HOOD_Y2 24

#define BACK_WHEEL_X 11
#define BACK_WHEEL_Y 24
#define FRONT_WHEEL_X 48
#define FRONT_WHEEL_Y 24

#define HUB_RADIUS 4
#define TIRE_RADIUS 7



// TODO: Finish car part constants


/**
 * @brief Draw a car at the specified location.
 * @param x      Top left corner X coordinate.
 * @param y      Top left corner Y coordinate.
 * @details Draw the car components relative to the anchor point (top, left).
 */
void drawCar(coord_t x, coord_t y)
{
	//Body & Cab
	lcd_fillRect2(BODY_X0, BODY_Y0, BODY_X1, BODY_Y1, CAR_CLR);
	lcd_fillRect2(CAB_X0, CAB_Y0, CAB_X1, CAB_Y1, CAR_CLR);
	//Hood
	lcd_fillTriangle(HOOD_X0, HOOD_Y0, HOOD_X1, HOOD_Y1, HOOD_X2, HOOD_Y2);
	//Windows
	lcd_fillRoundRect2(BACK_WINDOW_X0, BACK_WINDOW_Y0, BACK_WINDOW_X1, BACK_WINDOW_Y1, WINDOW_RADIUS, WINDOW_CLR);
	lcd_fillRoundRect2(FRONT_WINDOW_X0, FRONT_WINDOW_Y0, FRONT_WINDOW_X1, FRONT_WINDOW_Y1, WINDOW_RADIUS, WINDOW_CLR);
	//Tires
	lcd_fillCircle(BACK_WHEEL_X, BACK_WHEEL_Y, TIRE_RADIUS, TIRE_CLR);
	lcd_fillCircle(BACK_WHEEL_X, BACK_WHEEL_Y, HUB_RADIUS, HUB_CLR);
	lcd_fillCircle(FRONT_WHEEL_X, FRONT_WHEEL_Y, TIRE_RADIUS, TIRE_CLR);
	lcd_fillCircle(FRONT_WHEEL_X, FRONT_WHEEL_Y, HUB_RADIUS, HUB_CLR);
}

//----------------------------------------------------------------------------//
// Car Implementation - End
//----------------------------------------------------------------------------//

// Main display constants
#define BACKGROUND_CLR rgb565(0,60,90)
#define TITLE_CLR GREEN
#define STATUS_CLR WHITE
#define STR_BUF_LEN 12 // string buffer length
#define FONT_SIZE 2
#define FONT_W (LCD_CHAR_W*FONT_SIZE)
#define FONT_H (LCD_CHAR_H*FONT_SIZE)
#define STATUS_W (FONT_W*3)

#define WAIT 2000 // milliseconds
#define DELAY_EX3 20 // milliseconds

// Object position and movement
#define OBJ_X 100
#define OBJ_Y 100
#define OBJ_MOVE 3 // pixels


// Application main
void app_main(void)
{
	ESP_LOGI(TAG, "Start up");
	lcd_init();
	lcd_fillScreen(BACKGROUND_CLR);
	lcd_setFontSize(FONT_SIZE);
	lcd_drawString(0, 0, "Hello World! (lcd)", TITLE_CLR);
	printf("Hello World! (terminal)\n");
	DELAY_MS(WAIT);

	// TODO: Exercise 1 - Draw car in one location.

	// TODO: Exercise 2 - Draw moving car (Method 1), one pass across display.
	// Clear the entire display and redraw all objects each iteration.
	// Use a loop and increment x by OBJ_MOVE each iteration.
	// Start x off screen (negative coordinate).

	// TODO: Exercise 3 - Draw moving car (Method 2), one pass across display.
	// Move by erasing car at old position, then redrawing at new position.
	// Objects that don't change or move are drawn once.

	// TODO: Exercise 4 - Draw moving car (Method 3), one pass across display.
	// First, draw all objects into a cleared, off-screen frame buffer.
	// Then, transfer the entire frame buffer to the screen.

	// TODO: Exercise 5 - Draw an animated Pac-Man moving across the display.
	// Use Pac-Man sprites instead of the car object.
	// Cycle through each sprite when moving the Pac-Man character.
}
