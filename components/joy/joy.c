
#include <stdlib.h>

#include "joy.h"

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"


#define X_JOY_CHAN  ADC_CHANNEL_6
#define Y_JOY_CHAN  ADC_CHANNEL_7

#define JOY_CALIB_SAMPLES 10

adc_oneshot_unit_handle_t adc1_handle;

int_fast32_t x_joy_center_offs;
int_fast32_t y_joy_center_offs;


// Initialize the joystick driver. Must be called before use.
// May be called multiple times. Return if already initialized.
// Return zero if successful, or non-zero otherwise.
int32_t joy_init(void) {
    //configure ACD one-shot unit
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //configure ADC1 channels 6 & 7
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, X_JOY_CHAN, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, Y_JOY_CHAN, &config));

    // Calculate center position
    int_fast32_t adc_in;
    int_fast32_t sample_total=0;
    for(int i=0; i<JOY_CALIB_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, X_JOY_CHAN, &adc_in));
        sample_total += adc_in;
    }
    x_joy_center_offs = sample_total/JOY_CALIB_SAMPLES;
    sample_total=0;
    for(int i=0; i<JOY_CALIB_SAMPLES; i++) {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, Y_JOY_CHAN, &adc_in));
        sample_total += adc_in;
    }
    y_joy_center_offs = sample_total/JOY_CALIB_SAMPLES;

    return 0;
}

// Free resources used by the joystick (ADC unit).
// Return zero if successful, or non-zero otherwise.
int32_t joy_deinit(void) {
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
    return 0;
}

// Get the joystick displacement from center position.
// Displacement values range from 0 to +/- JOY_MAX_DISP.
// This function is not safe to call from an ISR context.
// Therefore, it must be called from a software task context.
// *dcx: pointer to displacement in x.
// *dcy: pointer to displacement in y.
void joy_get_displacement(int32_t *dcx, int32_t *dcy) {
    int_fast32_t raw_x;
    int_fast32_t raw_y;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, X_JOY_CHAN, &raw_x));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, Y_JOY_CHAN, &raw_y));
    *dcx = raw_x - x_joy_center_offs;
    *dcy = raw_y - y_joy_center_offs;
}