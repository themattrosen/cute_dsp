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

// allocate the context
cd_context_t* dsp_context = cd_make_context(dsp_context_definition);

// attach the dsp context to the sound context
cs_set_dsp_context(sound_context, dsp_context);

//...

// at the end of the application, after shutting down the sound context, release the dsp context
cs_shutdown_context(sound_context);
cd_release_context(&dsp_context);
```

### cd_mixer_t
To create a DSP mixer to attach to a playing sound do the following:
```cpp
// somewhere in your init code, you need to set the proper callbacks in cute_sound 
// for playing sounds to create mixers properly
cs_make_dsp_mixer = cd_make_mixer_callback;
cs_release_dsp_mixer = cd_release_mixer_callback;

// you can statically create the mixer def
cd_mixer_def_t mixer_def;

// specify the number of audio channels from cs_loaded_sound_t.
mixer_def.channel_count = loaded_sound.channel_count; 

// specify which filters this mixer will use
mixer_def.has_highpass = 0; 
mixer_def.has_lowpass = 1;

// set the definitions for filters you want
mixer_def.lowpass_def = lowpass_def;

// add the mixer to your play_sound_def;
cs_play_sound_def_t play_sound_def = cs_make_def(&music1);
play_sound_def.dsp_mixer_def = &mixer_def;

// when you call cs_play_sound, a mixer will be created via the callback
// from the provided mixer def
cs_playing_sound_t* playing_sound = cs_play_sound(sound_ctx, play_sound_def);
cd_mixer_t* dsp_mixer = playing_sound->dsp_mixer;

// from this mixer, you can change the values for your filters

```

### cd_lowpass_t/cd_highpass_t
To create a lowpass or highpass filter attached to a DSP mixer, you must first create a def for it:
```cpp
cd_lowpass_def_t lowpass_def = cd_make_lowpass_def(cutoff_frequency, (float)sampling_rate);
cd_highpass_def_t highpass_def = cd_make_highpass_def(cutoff_frequency, (float)sampling_rate);
```
Then you can set this as the low/highpass def when creating a cd_mixer_def_t, as shown above
  
In order to modify the cutoff frequencies of the lowpass/highpass filters, you can either call a setter directly from the mixer, or it can be done on individual filters manually. To set the cutoff frequencies, any of the following functions can be called:
```cpp
void cd_set_lowpass_filter_cutoffs(cd_mixer_t* mixer, float cutoff);
void cd_set_lowpass_cutoff_frequency(cd_lowpass_t* filter, float cutoff_freq_in_hz);
float cd_get_lowpass_cutoff_frequency(const cd_lowpass_t* filter);

// corresponding versions of these exist for highpass filters as well
```
