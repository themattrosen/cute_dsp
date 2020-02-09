# cute_dsp

cute_dsp is a C API for various DSP effects suitable for video games and
meant to interface directly with the cute_sound library created by Randy Gaul.
The scope of cute_dsp will eventually include:

* lowpass filter
* highpass filter
* white noise injection
* lowpass filtering with resonances
* wind noise presets for resonant filters
* realtime reverb
* echo filter
* randomization settings
* filter presets

## Implemented Features

### Lowpass Filtering
Uses a second order Butterworth filter with a 6dB per octave rolloff. 

### Highpass Filtering
Uses a second order Butterworth filter with a 6dB per octave rolloff, converted from the lowpass filter equation.

## Usage
cute_dsp must be used concurrently with cute_sound. 
  
To set up cute_dsp, `#include "cute_dsp.h"` somewhere above your include for cute_sound.h.
Above the include to cute_dsp, there must be one place where you `#define CUTE_DSP_IMPLEMENTATION`.
  
### cd_context_t
Before or after creating your `cs_context_t` cute sound context, you will need to create a cute_dsp context.

```cpp
cs_context_t* sound_context = cs_make_context(...);

cd_context_def_t dsp_context_definition; // statically create dsp context

// set the max number of mixers
dsp_context_definition.playing_pool_count = num_elements_in_playing_pool;

// set dsp sampling rate
dsp_context_definition.sampling_rate = (float)frequency;

// enable filters that you want enable cute_dsp to use
dsp_context_definition.use_highpass = 0;
dsp_context_definition.use_lowpass = 1;

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
