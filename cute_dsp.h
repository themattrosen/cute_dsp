/*
	------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

	cute_dsp.h - v1.0

	To create implementation (the function definitions)
		#define CUTE_DSP_IMPLEMENTATION
	in *one* C/CPP file (translation unit) that includes this file

	Summary:
		cute_dsp is a C API for various DSP effects suitable for video games and
		meant to interface directly with the cute_sound library created by Randy Gaul.
		The scope of cute_dsp will eventually include:

		-lowpass filter
		-highpass filter
		-white noise injection
		-lowpass filtering with resonances
		-wind noise presets for resonant filters
		-realtime reverb
		-echo filter
		-randomization settings
		-filter presets

	Revision history:
		1.0		(05/25/2019)	initial release: added lowpass and highpass implementation
*/
  
#if !defined(CUTE_DSP_H)

/* BEGIN LOW PASS FILTER API */
/*
	cute_dsp lowpass filter type. 
	Implemented using a second order (6dB/Octave roll off) Butterworth filter
	Implemented as an opaque struct
*/
struct cd_lowpass_t;
typedef struct cd_lowpass_t cd_lowpass_t;

/*
	cute_dsp lowpass struct
	Used to construct a lowpass filter from a cutoff frequency.
	Assumes that the sampling rate of playing sound will remain constant.
*/
typedef struct cd_lowpass_def_t
{
	float freq_cutoff;
	float sampling_rate;
} cd_lowpass_def_t;

/*
	creates a lowpass filter definition.
	cd_lowpass_def_t can also be created on the stack as well.
*/
cd_lowpass_def_t cd_make_lowpass_def(float frequency_cutoff_hz, float sampling_rate);

/*
	dynamically allocates a lowpass filter from a lowpass_def
	called via 
	cd_lowpass_def_t def = cd_make_lowpass_def(freq, rate);
	cd_lowpass_t* filter = cd_make_lowpass_filter(&def);
*/
cd_lowpass_t* cd_make_lowpass_filter(const cd_lowpass_def_t* definition);

/*
	releases memory of the lowpass filter and sets to NULL
*/
void cd_release_lowpass(cd_lowpass_t** filter);

/*
	Sets the cutoff frequency in a lowpass filter, and updates the filter coefficients.
	Cutoff frequency parameter is in samples per second.
	Calls cd_set_lowpass_cutoff_frequency_radians with the converted value
*/
void cd_set_lowpass_cutoff_frequency(cd_lowpass_t* filter, float cutoff_freq_in_hz);

/*
	Gets the cutoff frequency in a lowpass filter.
	Cutoff frequency parameter in hz.
*/
float cd_get_lowpass_cutoff_frequency(const cd_lowpass_t* filter);

/*
	Processes the next sample of the given filter.
	@param input next sample from the signal chain
	@return the input sample processed by the lowpass filter
*/
float cd_sample_lowpass(cd_lowpass_t* filter, float input);
/* END LOWPASS FILTER API */

/* BEGIN HIGHPASS FILTER API */
/*
	cute_dsp highpass filter type
	Implemented using a second order (6dB/Octave roll off) Butterworth filter
		converted from the second order Butterworth lowpass transfer function
	Implemented as an opaque struct
*/
struct cd_highpass_t;
typedef struct cd_highpass_t cd_highpass_t;

typedef struct cd_highpass_def_t
{
	float freq_cutoff;
	float sampling_rate;
};

/*
	creates a highpass filter definition.
	cd_highpass_def_t can also be created on the stack as well.
*/
cd_highpass_def_t cd_make_highpass_def(float frequency_cutoff_hz, float sampling_rate);

/*
	dynamically allocates a highpass filter from a lowpass_def
	called via 
	cd_highpass_def_t def = cd_make_highpass_def(freq, rate);
	cd_highpass_t* filter = cd_make_highpass_filter(&def);
*/
cd_highpass_t* cd_make_highpass_filter(const cd_highpass_def_t* definition);

/*
	releases memory of the highpass filter and sets to NULL
*/
void cd_release_highpass(cd_highpass_t** filter);

/*
	Sets the cutoff frequency in a highpass filter, and updates the filter coefficients.
	Cutoff frequency parameter is in samples per second.
	Calls cd_set_highpass_cutoff_frequency_radians with the converted value
*/
void cd_set_highpass_cutoff_frequency(cd_highpass_t* filter, float cutoff_freq_in_hz);

/*
	Gets the cutoff frequency in a lowpass filter.
	Cutoff frequency parameter in hz.
*/
float cd_get_highpass_cutoff_frequency(const cd_highpass_t* filter);

/*
	Processes the next sample of the given filter.
	@param input next sample from the signal chain
	@return the input sample processed by the highpass filter
*/
float cd_sample_highpass(cd_highpass_t* filter, float input);

/* END HIGHPASS FILTER API */

#define CUTE_DSP_H
#endif
/* END HEADER SECTION */

/* BEGIN IMPLEMENTATION SECTION */
#ifdef CUTE_DSP_IMPLEMENTATION
#ifndef CUTE_DSP_IMPLEMENTATION_ONCE
#define CUTE_DSP_IMPLEMENTATION_ONCE

/* HELPER MACROS */
#if !defined(CUTE_DSP_ALLOC)
	#include <stdlib.h> // malloc, free
	#define CUTE_DSP_ALLOC(size) malloc(size)
	#define CUTE_DSP_FREE(mem)   free(mem)
#endif

#define CUTE_DSP_ASSERT_INTERNAL *(int*)0 = 0
#define CUTE_DSP_ASSERT(X) do { if(!(X)) CUTE_DSP_ASSERT_INTERNAL; } while (0)

#if !defined(CUTE_DSP_MATH)
	#include <math.h> // sinf, cosf, absf, atan2f
	#define CUTE_DSP_SIN(angle_rad) sinf(angle_rad)
	#define CUTE_DSP_COS(angle_rad) cosf(angle_rad)
	#define CUTE_DSP_ABS(val)       fabsf(val)
	#define CUTE_DSP_PI				3.1415926f
	#define CUTE_DSP_SQRT_2 		1.4142136f
	/* more here */
#endif
/* END HELPER MACROS */

/* BEGIN OPAQUE STRUCT IMPLEMENTATION */
typedef struct cd_lowpass_t
{
	float freq_cutoff;
	float sampling_rate;
	float y1;
	float y2;
	float x_coeff;
	float y1_coeff;
	float y2_coeff;
} cd_lowpass_t;

typedef struct cd_highpass_t
{
	float freq_cutoff;
	float sampling_rate;
	float y1;
	float y2;
	float x1;
	float x2;
	float x_coeff;
	float x1_coeff;
	float y1_coeff;
	float y2_coeff;
} cd_highpass_t;

/* END OPAQUE STRUCT IMPLEMENTATION */
/* BEGIN FUNCTION IMPLEMENTATION */

/* BEGIN LOWPASS IMPLEMENTATION */
cd_lowpass_def_t cd_make_lowpass_def(float frequency_cutoff_hz, float sampling_rate)
{
	cd_lowpass_def_t def = { 0.f, 0.f };
	def.freq_cutoff = frequency_cutoff_hz;
	def.sampling_rate = sampling_rate;
	return def;
}

cd_lowpass_t* cd_make_lowpass_filter(const cd_lowpass_def_t* definition)
{
	cd_lowpass_t* filter = NULL;
	CUTE_DSP_ASSERT(definition);

	filter = (cd_lowpass_t*)CUTE_DSP_ALLOC(sizeof(cd_lowpass_t));
	CUTE_DSP_ASSERT(filter);

	filter->sampling_rate = definition->sampling_rate;
	cd_set_lowpass_cutoff_frequency(filter, definition->freq_cutoff);

	return filter;
}

void cd_release_lowpass(cd_lowpass_t** filter)
{
	CUTE_DSP_ASSERT(filter && *filter);
	CUTE_DSP_FREE(*filter);
	*filter = NULL;
}

static void cd_set_lowpass_cutoff_frequency_radians(cd_lowpass_t* filter, float cutoff_freq_in_rad)
{
	float T = cutoff_freq_in_rad / filter->sampling_rate;
	float Y = 1.f / (1.f + CUTE_DSP_SQRT_2 * T + T * T);
	filter->x_coeff  = T * T * Y;
	filter->y1_coeff = (2.f + CUTE_DSP_SQRT_2 * T) * Y;
	filter->y2_coeff = -1.f * Y;
}

void cd_set_lowpass_cutoff_frequency(cd_lowpass_t* filter, float cutoff_freq_in_hz)
{
	CUTE_DSP_ASSERT(filter);
	filter->freq_cutoff = cutoff_freq_in_hz;
	float freq_in_rad = 2.f * CUTE_DSP_PI * cutoff_freq_in_hz;

	cd_set_lowpass_cutoff_frequency_radians(filter, freq_in_rad);
}

float cd_get_lowpass_cutoff_frequency(const cd_lowpass_t* filter)
{
	CUTE_DSP_ASSERT(filter);
	return filter->freq_cutoff;
}

float cd_sample_lowpass(cd_lowpass_t* filter, float input)
{
	CUTE_DSP_ASSERT(filter);
	float output = 	filter->x_coeff * input + 
					filter->y1_coeff * filter->y1 + 
					filter->y2_coeff * filter->y2;

	filter->y2 = filter->y1;
	filter->y1 = output;

	return output;
}
/* END LOWPASS IMPLEMENTATION */

/* BEGIN HIGHPASS IMPLEMENTATION */
cd_highpass_def_t cd_make_highpass_def(float frequency_cutoff_hz, float sampling_rate)
{
	cd_highpass_def_t def = { 0.f, 0.f };
	def.freq_cutoff = frequency_cutoff_hz;
	def.sampling_rate = sampling_rate;
	return def;
}

cd_highpass_t* cd_make_highpass_filter(const cd_highpass_def_t* definition)
{
	cd_highpass_t* filter = NULL;
	CUTE_DSP_ASSERT(definition);

	filter = (cd_highpass_t*)CUTE_DSP_ALLOC(sizeof(cd_highpass_t));
	CUTE_DSP_ASSERT(filter);

	filter->sampling_rate = definition->sampling_rate;
	cd_set_highpass_cutoff_frequency(filter, definition->freq_cutoff);

	return filter;
}

void cd_release_highpass(cd_highpass_t** filter)
{
	CUTE_DSP_ASSERT(filter && *filter);
	CUTE_DSP_FREE(*filter);
	*filter = NULL;
}

static void cd_set_highpass_cutoff_frequency_radians(cd_highpass_t* filter, float cutoff_freq_in_rad)
{
	float T = cutoff_freq_in_rad / filter->sampling_rate;
	float Y = 1.f / (1 + CUTE_DSP_SQRT_2 * T + T * T);
	filter->x_coeff = Y;
	filter->x1_coeff = -2.f * Y;
	filter->y1_coeff = (2.f + CUTE_DSP_SQRT_2 * T) * Y;
	filter->y2_coeff = -Y;
}

void cd_set_highpass_cutoff_frequency(cd_highpass_t* filter, float cutoff_freq_in_hz)
{
	CUTE_DSP_ASSERT(filter);
	filter->freq_cutoff = cutoff_freq_in_hz;
	float freq_in_rad = 2.f * CUTE_DSP_PI * cutoff_freq_in_hz;

	cd_set_highpass_cutoff_frequency_radians(filter, freq_in_rad);
}

float cd_get_highpass_cutoff_frequency(const cd_highpass_t* filter)
{
	CUTE_DSP_ASSERT(filter);
	return filter->freq_cutoff;
}

float cd_sample_highpass(cd_highpass_t* filter, float input)
{
	CUTE_DSP_ASSERT(filter);
	float output = 	filter->x_coeff  * input +
					filter->x1_coeff * filter->x1 +
					filter->x_coeff  * filter->x2 +
					filter->y1_coeff * filter->y1 +
					filter->y2_coeff * filter->y2;
	filter->x2 = filter->x1;
	filter->x1 = input;
	filter->y2 = filter->y1;
	filter->y1 = output;

	return output;
}
/* END HIGHPASS IMPLEMENTATION */

/* END FUNCTION IMPLEMENTATION */

#endif /* CUTE_DSP_IMPLEMENTATION_ONCE */
#endif /* CUTE_DSP_IMPLEMENTATION */
/* END IMPLEMENTATION SECTION */

/*
	------------------------------------------------------------------------------
	This software is available under 2 licenses - you may choose the one you like.
	------------------------------------------------------------------------------
	ALTERNATIVE A - zlib license
	Copyright (c) 2019 Matthew Rosen
	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from
	the use of this software.
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	  1. The origin of this software must not be misrepresented; you must not
		 claim that you wrote the original software. If you use this software
		 in a product, an acknowledgment in the product documentation would be
		 appreciated but is not required.
	  2. Altered source versions must be plainly marked as such, and must not
		 be misrepresented as being the original software.
	  3. This notice may not be removed or altered from any source distribution.
	------------------------------------------------------------------------------
	ALTERNATIVE B - Public Domain (www.unlicense.org)
	This is free and unencumbered software released into the public domain.
	Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
	software, either in source code form or as a compiled binary, for any purpose,
	commercial or non-commercial, and by any means.
	In jurisdictions that recognize copyright laws, the author or authors of this
	software dedicate any and all copyright interest in the software to the public
	domain. We make this dedication for the benefit of the public at large and to
	the detriment of our heirs and successors. We intend this dedication to be an
	overt act of relinquishment in perpetuity of all present and future rights to
	this software under copyright law.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
	ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	------------------------------------------------------------------------------
*/
