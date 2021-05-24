# cute_dsp

cute_dsp is a C API for various DSP effects suitable for video games and
meant to interface directly with the cute_sound library created by Randy Gaul.
The scope of cute_dsp will eventually include:

* Lowpass filter
* Highpass filter
* White noise injection
* Lowpass filtering with resonances
* Wind noise presets for resonant filters
* Realtime reverb
* Echo filter
* Randomization settings
* Filter presets

## Implemented Features

### Lowpass Filtering
Uses a second order Butterworth filter with a 6dB per octave rolloff. 

### Highpass Filtering
Uses a second order Butterworth filter with a 6dB per octave rolloff, converted from the lowpass filter equation.

### Echo Filtering
Uses two ring buffers for delay of input and output samples. There are three designable parameters:
 
* Delay time
* Mix factor (echo loudness)
* Feedback factor (amount that echoes feedback into themselves)

### Noise Generator
Generates white noise and adds to a signal. The signal path of cute_dsp allows this white noise to be fed into other filters in the signal chain. The white noise is generated using a xorshift128 PRNG. 

## Usage
cute_dsp must be used concurrently with cute_sound. 
  
To set up cute_dsp, `#include "cute_dsp.h"` somewhere below your include for cute_sound.h.
Above the include to cute_dsp, there must be one place where you `#define CUTE_DSP_IMPLEMENTATION`.
  
### cd_context_t
After creating your `cs_context_t` cute sound context, you will need to create a cute_dsp context.

```cpp
cs_context_t* sound_context = cs_make_context(...);

cd_context_def_t dsp_context_definition; // statically create dsp context

// set the max number of mixers
dsp_context_definition.playing_pool_count = num_elements_in_playing_pool;

// set dsp sampling rate
dsp_context_definition.sampling_rate = (float)frequency;

// set which filters you would like to use and some optional parameters
dsp_context_definition.use_highpass = 1;
dsp_context_definition.use_lowpass = 1;
dsp_context_definition.use_echo = 0;
dsp_context_definition.use_noise = 1;
dsp_context_definition.echo_max_delay_s = 0.f;
dsp_context_definition.rand_seed = 2;

// allocate the context
cd_context_t* dsp_context = cd_make_context(dsp_context_definition);

//...

// at the end of the application, after shutting down the sound context, release the dsp context
cs_shutdown_context(sound_context);
cd_release_context(&dsp_context);
```
For each sound played using cute_sound, there will be a copy of each filter you've enabled added as a cute_sound plugin. However, the filter will by default set its internal values such that the effect of the filter is not audible. I.e. if you've enabled lowpass filters in `cd_context_def_t`, then each sound played using cute_sound will have an instance of a lowpass filter. That lowpass filter will have its cutoff frequency at 20kHz, therefore negating its effect.
  
### cd_lowpass_t/cd_highpass_t
To modify the cutoff frequencies of the lowpass/highpass filters:
```cpp
void cd_set_lowpass_cutoff(cs_playing_sound_t* playing_sound, float cutoff_in_hz);
void cd_set_highpass_cutoff(cs_playing_sound_t* playing_sound, float cutoff_in_hz);

float cd_get_lowpass_cutoff(const cs_playing_sound_t* playing_sound);
float cd_get_highpass_cutoff(const cs_playing_sound_t* playing_sound);
```

### cd_echo_t
To modify the parameters of the echo filter:
```cpp
void cd_set_echo_delay(cs_playing_sound_t* playing_sound, float t);
void cd_set_echo_mix(cs_playing_sound_t* playing_sound, float a);
void cd_set_echo_feedback(cs_playing_sound_t* playing_sound, float b);

float cd_get_echo_delay(const cs_playing_sound_t* playing_sound);
float cd_get_echo_mix(const cs_playing_sound_t* playing_sound);
float cd_get_echo_feedback(const cs_playing_sound_t* playing_sound);
float cd_get_echo_max_delay(const cs_playing_sound_t* playing_sound);
```

### cd_noise_t
To modify the parameters of the noise generator:
```cpp
void cd_set_noise_amplitude_db(cs_playing_sound_t* playing_sound, float db);
void cd_set_noise_amplitude_gain(cs_playing_sound_t* playing_sound, float gain);

float cd_get_noise_amplitude_db(const cs_playing_sound_t* playing_sound);
float cd_get_noise_amplitude_gain(const cs_playing_sound_t* playing_sound);
```
