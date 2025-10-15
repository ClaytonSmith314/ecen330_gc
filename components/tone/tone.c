#include "sound.h"
#include "tone.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define BIAS 0x80U
#define MAX_VAL 0xFFU
#define MAX_VALF 255.0F
#define PI 3.14159

#define QUARTER_FREQ 4 
#define THREE_X_PERIOD 3

#define CLAMP(x,max,min) (((x)<(max))? (((x)>(min))? (x):(min)) : (max))
#define BIAS_AND_SCALE(x) ((uint8_t)CLAMP(BIAS*((x)+1.0F),MAX_VALF, 0.0F))

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
    // make sure frequency is larger than niquist frequency for all samples
    if (sample_hz > (2 * LOWEST_FREQ)) {
        //define buffer
        sound_buffer_size = sample_hz/LOWEST_FREQ+1;
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

    // loop over each index of sound sample to fill in
    for(int i=0; i<num_samples; i++) {
        float t = 1.0F*i/sample_freq;
        // switch depending on mode of sound production
        switch(tone) {
            // sine wave
            case SINE_T: {
                sound_buffer[i] = BIAS_AND_SCALE(sinf(2*PI*freq*t));
            } break;
            // square wave
            case SQUARE_T: {
                if(i < (num_samples/2)) {
                    sound_buffer[i] = MAX_VAL;
                } else {
                    sound_buffer[i] = 0;
                }
            } break;
            // triangle wave
            case TRIANGLE_T: {
                if (i < (num_samples/QUARTER_FREQ)) {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*QUARTER_FREQ);
                } else if ((num_samples/QUARTER_FREQ<=i) && (i<num_samples*THREE_X_PERIOD/QUARTER_FREQ)) {
                    sound_buffer[i] = BIAS_AND_SCALE(2-t*freq*QUARTER_FREQ);
                } else {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*QUARTER_FREQ-QUARTER_FREQ);
                }
            } break;
            // sawtooth wave
            case SAW_T: {
                if (i < (num_samples/2)) {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*2);
                } else {
                    sound_buffer[i] = BIAS_AND_SCALE(t*freq*2-2);
                }
            } break;
            // Nothing
            case LAST_T: {
            } break;
            default: break;
        }
    }

    sound_cyclic(sound_buffer, num_samples);
}
