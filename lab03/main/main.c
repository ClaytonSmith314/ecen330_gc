#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/gptimer.h"
#include "hw.h"
#include "lcd.h"
#include "pin.h"
#include "watch.h"

#include "esp_timer.h"


#define CLOCK_RESOLUTION 1000000
#define ALARM_COUNT 10000
#define TICKS_PER_FIVE_SEC 500

static const char *TAG = "lab03";

volatile int64_t isr_max = 0;
volatile uint64_t isr_count = 0;
volatile uint64_t timer_ticks = 0;
volatile bool running=false;

// ISR timer callback. Incriments timer_ticks if the stopwatch is running
static bool main_timer_callback(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    int64_t start, finish;
    start = esp_timer_get_time();
    if(!pin_get_level(HW_BTN_A)) 
    {
        running = true;
    }
    if(!pin_get_level(HW_BTN_B))
    {
        running = false;
    }
    if(!pin_get_level(HW_BTN_START))
    {
        running = false;
        timer_ticks = 0;
    }
    if(running) {
        timer_ticks++;
    }
    finish = esp_timer_get_time();

    isr_max = (isr_max>finish - start)? isr_max : finish - start;
    isr_count++;
    
    return false;
}


// Main application
void app_main(void)
{
	ESP_LOGI(TAG, "Starting");

    int64_t start, finish;


    // Set pins for buttons A, B, & Start as inputs
    start = esp_timer_get_time();
    pin_reset(HW_BTN_B);
    pin_reset(HW_BTN_A);
    pin_reset(HW_BTN_START);
    pin_input(HW_BTN_A, true);
    pin_input(HW_BTN_B, true);
    pin_input(HW_BTN_START, true);
    finish = esp_timer_get_time();
    printf("Pin set time:%lld microseconds\n", finish-start);

    // Configure timer
    start = esp_timer_get_time();
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
        .direction = GPTIMER_COUNT_UP,      // Counting direction is up
        .resolution_hz = CLOCK_RESOLUTION,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
    };
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_alarm_config_t alarm_config = {
        .reload_count = 0,      // When the alarm event occurs, the timer will automatically reload to 0
        .alarm_count = ALARM_COUNT, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s
        .flags.auto_reload_on_alarm = true, // Enable auto-reload function
    };
    // Set the timer's alarm action
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    gptimer_event_callbacks_t cbs = {
        .on_alarm = main_timer_callback, // Call the user callback function when the alarm event occurs
    };
    // Register timer event callback functions, allowing user context to be carried
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    finish = esp_timer_get_time();
    printf("Timer config time:%lld microseconds\n", finish-start);

    start = esp_timer_get_time();
    ESP_LOGI(TAG, "Stopwatch update");
    finish = esp_timer_get_time();
    printf("ESP_LOGI time:%lld microseconds\n", finish-start);
    lcd_init(); // Initialize LCD display
    watch_init(); // Initialize stopwatch face
    for (;;) { // forever update loop
        watch_update(timer_ticks);
        if(isr_count >= TICKS_PER_FIVE_SEC) {
            printf("Max ISR time:%lld microseconds\n", isr_max);
            isr_count = 0;
            isr_max = 0;
        }
    }

}