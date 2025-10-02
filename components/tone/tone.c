#include "sound.h"
#include "tone.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BIAS 0x80U
#define MAX_VAL 0xFFU
#define MAX_VALF 255.0F
#define PI 3.14159

#define FOUR 4 
#define THREE 3

#define CLAMP(x,max,min) (((x)<(max))? (((x)>(min))? (x):(min)) : (max))
#define BIAS_AND_SCALE(x) ((uint8_t)CLAMP(BIAS*((x)+1.0F),MAX_VALF, 0.0F))
//#define BIAS_AND_SCALE(x) ((uint8_t)((BIAS)*((x)+1.0F)))

// This component is a thin layer around the sound component.
// One cycle of a waveform is generated and then given to the
// sound component to play out cyclically until told to stop.
// Macros are provided for tone functions that are aliases
// of sound functions.

uint32_t sample_freq;

uint8_t* sound_buffer;
size_t sound_buffer_size;

// Initialize the tone driver. Must be called before using.
// May be called again to change sample rate.
// sample_hz: sample rate in Hz to playback tone.
// Return zero if successful, or non-zero otherwise.
int32_t tone_init(uint32_t sample_hz) {
    sample_freq = sample_hz;
    if (sample_hz > (2 * LOWEST_FREQ)) {
        //define buffer
        sound_buffer_size = sample_hz/LOWEST_FREQ;
        sound_buffer = (uint8_t*)malloc(sizeof(uint8_t)*sound_buffer_size);
        return sound_init(sample_hz);
    } else {
        return 1;
    }
}

// Free resources used for tone generation (DAC, etc.).
// Return zero if successful, or non-zero otherwise.
int32_t tone_deinit(void) {
    free(sound_buffer);
    return sound_deinit();
}

// Start playing the specified tone.
// tone: one of the enumerated tone types.
// freq: frequency of the tone in Hz.
void tone_start(tone_t tone, uint32_t freq) {
    if(freq < LOWEST_FREQ) return;

    uint32_t num_samples = sample_freq/freq;
    printf("here! %ld\n", num_samples);

    for(int i=0; i<num_samples; i++) {
        float t = 1.0F*i/sample_freq;

        switch(tone) {
            case SINE_T: {
                sound_buffer[i] = BIAS_AND_SCALE(sinf(2*PI*freq*t));
            } break;
            case SQUARE_T: {
                if(i < (num_samples/2)) {
                    sound_buffer[i] = MAX_VAL;
                } else {
                    sound_buffer[i] = 0;
                }
            } break;
            case TRIANGLE_T: {
                if (i < (num_samples/FOUR)) {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*FOUR);
                } else if ((num_samples/FOUR<=i) && (i<num_samples*THREE/FOUR)) {
                    sound_buffer[i] = BIAS_AND_SCALE(2-t*freq*FOUR);
                } else {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*FOUR-FOUR);
                }
            } break;
            case SAW_T: {
                if (i < (num_samples/2)) {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*2);
                } else {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*2.0-2.0);
                }
            } break;
            case LAST_T: {
                // sound_buffer[i] = 0;
            } break;
            default: break;
        }
        // printf("%x.", sound_buffer[i]);
    }

    sound_cyclic(sound_buffer, num_samples-1);
}
