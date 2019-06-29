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
		1.1		(06/29/2019)	added internal memory pool, dsp context and mixers to be used by cute_sound
*/

#if !defined(CUTE_DSP_H)
/* BEGIN FORWARD DECLARATIONS */
/*
	cute_dsp context type
	Manages memory and settings of the cute_dsp context.
*/
struct cd_context_t;
typedef struct cd_context_t cd_context_t;

/*
	cute_dsp mixer type
	Holds ptrs to filters. Should be stored one per playing sound.
*/
struct cd_mixer_t;
typedef struct cd_mixer_t cd_mixer_t;

/*
	cute_dsp lowpass filter type. 
	Implemented using a second order (6dB/Octave roll off) Butterworth filter.
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
	cute_dsp highpass filter type
	Implemented using a second order (6dB/Octave roll off) Butterworth filter
		converted from the second order Butterworth lowpass transfer function.
*/
struct cd_highpass_t;
typedef struct cd_highpass_t cd_highpass_t;

/*
	cute_dsp highpass struct
	Used to construct a highpass filter from a cutoff frequency.
	Assumes that the sampling rate of playing sound will remain constant.
*/
typedef struct cd_highpass_def_t
{
	float freq_cutoff;
	float sampling_rate;
} cd_highpass_def_t;

/* END FORWARD DECLARATIONS */

/* BEGIN DSP CONTEXT API */

typedef struct cd_context_def_t
{
	unsigned playing_pool_count; // max number of playing sounds
	float sampling_rate;
} cd_context_def_t;

cd_context_t* cd_make_context(cd_context_def_t);

void cd_release_context(cd_context_t** context);
/* END DSP CONTEXT API */

/* BEGIN DSP MIXER API */
typedef struct cd_mixer_t
{
	cd_lowpass_t** lowpass;		// linkedlist of lowpass filters, size = channel_count
	cd_highpass_t** highpass;	// linkedlist of highpass filters, size = channel_count
	unsigned int channel_count;	// number of audio channels (mono or stereo audio)
} cd_mixer_t;

typedef struct cd_mixer_def_t
{
	cd_lowpass_def_t lowpass_def;
	cd_highpass_def_t highpass_def;

	// bool creation flags and miscellaneous data
	struct
	{
		unsigned int channel_count: 2; // can be 1 or 2, needs only 2 bits
		unsigned int has_lowpass: 1;
		unsigned int has_highpass: 1;
	};
} cd_mixer_def_t;

/*
	creates a dsp mixer to be attached to playing sounds
*/
cd_mixer_t* cd_make_mixer(cd_context_t* context, cd_mixer_def_t def);

/*
	called from the mix thread
	gets the next frame of samples from the dsp mixer for a given channel
*/
float* cd_sample_mixer(cd_mixer_t* mixer, float* input, unsigned channel_num, unsigned sample_count);

/*
	releases a mixer object from a context
*/
void cd_release_mixer(cd_context_t* context, cd_mixer_t** mixer);

/* END DSP MIXER API */

/* BEGIN LOW PASS FILTER API */

/*
	creates a lowpass filter definition.
	cd_lowpass_def_t can also be created on the stack as well.
*/
cd_lowpass_def_t cd_make_lowpass_def(float frequency_cutoff_hz, float sampling_rate);

/*
	dynamically allocates a lowpass filter from a lowpass_def
	called via 
	cd_lowpass_def_t def = cd_make_lowpass_def(freq, rate);
	cd_lowpass_t* filter = cd_make_lowpass_filter(context, &def);
*/
cd_lowpass_t* cd_make_lowpass_filter(cd_context_t* context, const cd_lowpass_def_t* definition);

/*
	releases memory of the lowpass filter and sets to NULL
*/
void cd_release_lowpass(cd_context_t* context, cd_lowpass_t** filter);

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
	creates a highpass filter definition.
	cd_highpass_def_t can also be created on the stack as well.
*/
cd_highpass_def_t cd_make_highpass_def(float frequency_cutoff_hz, float sampling_rate);

/*
	dynamically allocates a highpass filter from a lowpass_def
	called via 
	cd_highpass_def_t def = cd_make_highpass_def(freq, rate);
	cd_highpass_t* filter = cd_make_highpass_filter(context, &def);
*/
cd_highpass_t* cd_make_highpass_filter(cd_context_t* context, const cd_highpass_def_t* definition);

/*
	releases memory of the highpass filter and sets to NULL
*/
void cd_release_highpass(cd_context_t* context, cd_highpass_t** filter);

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
/**********************************************************************************************/

/**********************************************************************************************/
/* BEGIN IMPLEMENTATION SECTION */
#ifdef CUTE_DSP_IMPLEMENTATION
#ifndef CUTE_DSP_IMPLEMENTATION_ONCE
#define CUTE_DSP_IMPLEMENTATION_ONCE

/* HELPER MACROS */
#if !defined(CUTE_DSP_ALLOC)
	#include <stdlib.h> // malloc, free
	#define CUTE_DSP_ALLOC(size, mem_ctx) malloc(size)
	#define CUTE_DSP_FREE(mem, mem_ctx)   free(mem)
#endif

#define CUTE_DSP_ASSERT_INTERNAL *(int*)0 = 0
#define CUTE_DSP_ASSERT(X) do { if(!(X)) CUTE_DSP_ASSERT_INTERNAL; } while (0)

#if !defined(CUTE_DSP_MATH)
	#include <math.h> // sinf, cosf, absf, atan2f
	#define CUTE_DSP_MATH
	#define CUTE_DSP_SIN(angle_rad) sinf(angle_rad)
	#define CUTE_DSP_COS(angle_rad) cosf(angle_rad)
	#define CUTE_DSP_ABS(val)       fabsf(val)
	#define CUTE_DSP_PI				3.1415926f
	#define CUTE_DSP_SQRT_2 		1.4142136f
	/* more here */
#endif

#if !defined(CUTE_DSP_DEFINES)
	#define CUTE_DSP_DEFINES
	#define CUTE_DSP_MONO 1
	#define CUTE_DSP_STEREO 2
	#define CUTE_DSP_MAX_FRAME_LENGTH 4096
#endif
/* END HELPER MACROS */

/* BEGIN OPAQUE STRUCT IMPLEMENTATION */
typedef struct cd_memory_pool_object_t
{
	struct cd_memory_pool_object_t* next;
} cd_memory_pool_object_t;

typedef struct cd_memory_pool_t
{
	unsigned size_per_object;
	unsigned max_objects;
	unsigned pool_size;
	unsigned num_objects;
	char* pool;
	cd_memory_pool_object_t* free_list;
} cd_memory_pool_t;

typedef struct cd_memory_pool_def_t
{
	unsigned size_per_object;
	unsigned max_objects;
} cd_memory_pool_def_t;

typedef struct cd_context_t
{
	float sampling_rate;
	unsigned channel_count;
	cd_memory_pool_t mixer_pool;
	cd_memory_pool_t lowpass_filters;
	cd_memory_pool_t highpass_filters;
	unsigned lowpass_pool_size;
	unsigned highpass_pool_size;
	unsigned playing_pool_size;
} cd_context_t;

typedef struct cd_lowpass_t
{
	struct cd_lowpass_t* next;
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
	struct cd_highpass_t* next;
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

/* BEGIN MEMORY POOL IMPLEMENTATION */

void cd_make_memory_pool(cd_memory_pool_t* mem_pool, cd_memory_pool_def_t def)
{
	char* pool = NULL;
	unsigned i = 0;

	CUTE_DSP_ASSERT(mem_pool);
	mem_pool->size_per_object = def.size_per_object;
	mem_pool->max_objects = def.max_objects;
	mem_pool->pool_size = def.max_objects * def.size_per_object;
	mem_pool->num_objects = 0;
	
	pool = CUTE_DSP_ALLOC(mem_pool->pool_size, 0);
	mem_pool->pool = pool;
	CUTE_DSP_ASSERT(pool);

	// initialize free list
	for(i = 0; i < def.max_objects; ++i)
	{
		cd_memory_pool_object_t* obj = (cd_memory_pool_object_t *)(pool + i * def.size_per_object);
		obj->next = mem_pool->free_list;
		mem_pool->free_list = obj;
	}
}

void cd_release_memory_pool(cd_memory_pool_t* mem_pool)
{
	CUTE_DSP_ASSERT(mem_pool);
	if(mem_pool->pool)
	{
		CUTE_DSP_FREE(mem_pool->pool, 0);
		mem_pool->pool = NULL;
	}
}

void* cd_memory_pool_alloc(cd_memory_pool_t* mem_pool)
{
	CUTE_DSP_ASSERT(mem_pool);

	// case the pool is empty
	if(mem_pool->free_list == NULL)
	{
		// allocate from the heap normally
		void* object = CUTE_DSP_ALLOC(mem_pool->size_per_object, 0);
		CUTE_DSP_ASSERT(object);
		++mem_pool->num_objects;
		return object;
	}
	else
	{
		// pull another object from the free list
		cd_memory_pool_object_t* object = mem_pool->free_list;
		mem_pool->free_list = mem_pool->free_list->next;
		++mem_pool->num_objects;
		return object;
	}
}

void cd_memory_pool_free(cd_memory_pool_t* mem_pool, void* object)
{
	char* obj = (char *)object;
	unsigned obj_distance, pool_distance;
	CUTE_DSP_ASSERT(object && mem_pool);

	obj_distance = obj - mem_pool->pool;
	pool_distance = (mem_pool->pool + mem_pool->pool_size) - mem_pool->pool;

	// case object isn't in the pool
	if(pool_distance < obj_distance && obj_distance >= 0)
	{
		CUTE_DSP_FREE(object, 0);
	}
	// case object is in the pool
	else
	{
		// put it back on the free list
		cd_memory_pool_object_t* mem_obj = (cd_memory_pool_object_t*)(object);
		mem_obj->next = mem_pool->free_list;
		mem_pool->free_list = mem_obj;
	}
	
	--mem_pool->num_objects;
}
/* END MEMORY POOL IMPLEMENTATION */

/* BEGIN CONTEXT IMPLEMENTATION */
cd_context_t* cd_make_context(cd_context_def_t def)
{
	cd_memory_pool_def_t mixer_def;
	cd_memory_pool_def_t lowpass_def;
	cd_memory_pool_def_t highpass_def;

	cd_context_t* context = (cd_context_t *)CUTE_DSP_ALLOC(sizeof(cd_context_t), 0);
	CUTE_DSP_ASSERT(context && def.playing_pool_count);
	context->sampling_rate = def.sampling_rate;

	mixer_def.max_objects = def.playing_pool_count;
	mixer_def.size_per_object = sizeof(cd_mixer_t);
	cd_make_memory_pool(&context->mixer_pool, mixer_def);
	context->playing_pool_size = def.playing_pool_count;

	lowpass_def.max_objects = def.playing_pool_count;
	lowpass_def.size_per_object = sizeof(cd_lowpass_t);
	cd_make_memory_pool(&context->lowpass_filters, lowpass_def);
	context->lowpass_pool_size = def.playing_pool_count * 2;

	highpass_def.max_objects = def.playing_pool_count;
	highpass_def.size_per_object = sizeof(cd_highpass_t);
	cd_make_memory_pool(&context->highpass_filters, highpass_def);
	context->highpass_pool_size = def.playing_pool_count * 2;

	return context;
}

void cd_release_context(cd_context_t** context)
{
	CUTE_DSP_ASSERT(context && *context);

	cd_release_memory_pool(&(*context)->mixer_pool);
	cd_release_memory_pool(&(*context)->lowpass_filters);
	cd_release_memory_pool(&(*context)->highpass_filters);

	CUTE_DSP_FREE(*context, 0);
	*context = 0;
}

/* END CONTEXT IMPLEMENTATION */

/* BEGIN MIXER IMPLEMENTATION */

cd_mixer_t* cd_make_mixer(cd_context_t* context, cd_mixer_def_t def)
{
	CUTE_DSP_ASSERT(context);
	CUTE_DSP_ASSERT(def.channel_count == CUTE_DSP_MONO || def.channel_count == CUTE_DSP_STEREO);
	cd_mixer_t* mixer = (cd_mixer_t *)cd_memory_pool_alloc(&context->mixer_pool);

	// add low pass
	if(def.has_lowpass)
	{
		cd_lowpass_t* lowpass = cd_make_lowpass_filter(context, &(def.lowpass_def));
		mixer->lowpass = &lowpass;
		if(def.channel_count == CUTE_DSP_STEREO)
		{
			cd_lowpass_t* lowpass1 = cd_make_lowpass_filter(context, &(def.lowpass_def));
			lowpass->next = lowpass1;
		}
	}
	else
	{
		mixer->lowpass = 0;
	}
	

	// add high pass
	if(def.has_highpass)
	{
		cd_highpass_t* highpass = cd_make_highpass_filter(context, &(def.highpass_def));
		mixer->highpass = &highpass;
		if(def.channel_count == CUTE_DSP_STEREO)
		{
			cd_highpass_t* highpass1 = cd_make_highpass_filter(context, &(def.highpass_def));
			highpass->next = highpass1;
		}
	}
	else
	{
		mixer->highpass = 0;
	}
	

	return mixer;
}

float* cd_sample_mixer(cd_mixer_t* mixer, float* input_ptr, unsigned channel_num, unsigned sample_count)
{
	cd_lowpass_t* lowpass = 0;
	cd_highpass_t* highpass = 0;
	unsigned i = 0;
	float* output = input_ptr;

	// assign the set of filters to be used for the channel
	switch(channel_num)
	{
		case 0:
			lowpass = *mixer->lowpass;
			highpass = *mixer->highpass;
			break;
		case 1:
			lowpass = (*mixer->lowpass)->next;
			highpass = (*mixer->highpass)->next;
			break;
	}

	//TODO: to optimize in the future:
	// make sample functions into macros to process everything inline

	// process the lowpass filter
	for(i = 0; i < sample_count; ++i)
	{
		*output = cd_sample_lowpass(lowpass, *output);
		++output;
	}

	// reset the output_ptr
	output = input_ptr;

	// process the highpass filter
	for(i = 0; i < sample_count; ++i)
	{
		*output = cd_sample_highpass(highpass, *output);
		++output;
	}

	return input_ptr;
}

void cd_release_mixer(cd_context_t* context, cd_mixer_t** mixer_ptr)
{
	CUTE_DSP_ASSERT(context && mixer_ptr && *mixer_ptr);
	cd_mixer_t* mixer = *mixer_ptr;
	
	// handle both channels of lowpass filters
	if(mixer->lowpass)
	{
		cd_lowpass_t* next = (*mixer->lowpass)->next;
		cd_release_lowpass(context, mixer->lowpass);
		if(next)
		{
			cd_release_lowpass(context, &next);
		}
		*mixer->lowpass = 0;
	}

	// handle both channels of highpass filters
	if(mixer->highpass)
	{
		cd_highpass_t* next = (*mixer->highpass)->next;
		cd_release_highpass(context, mixer->highpass);
		if(next)
		{
			cd_release_highpass(context, &next);
		}
		*mixer->highpass = 0;
	}

	cd_memory_pool_free(&context->mixer_pool, mixer);
	*mixer_ptr = 0;
}

/* END MIXER IMPLEMENTATION */

/* BEGIN LOWPASS IMPLEMENTATION */
cd_lowpass_def_t cd_make_lowpass_def(float frequency_cutoff_hz, float sampling_rate)
{
	cd_lowpass_def_t def = { 0.f, 0.f };
	def.freq_cutoff = frequency_cutoff_hz;
	def.sampling_rate = sampling_rate;
	return def;
}

cd_lowpass_t* cd_make_lowpass_filter(cd_context_t* context, const cd_lowpass_def_t* definition)
{
	cd_lowpass_t* filter = NULL;
	CUTE_DSP_ASSERT(definition);

	filter = (cd_lowpass_t*)cd_memory_pool_alloc(&context->lowpass_filters);
	CUTE_DSP_ASSERT(filter);

	filter->sampling_rate = definition->sampling_rate;
	cd_set_lowpass_cutoff_frequency(filter, definition->freq_cutoff);
	filter->y1 = filter->y2 = 0.f;
	filter->next = 0;
	return filter;
}

void cd_release_lowpass(cd_context_t* context, cd_lowpass_t** filter)
{
	CUTE_DSP_ASSERT(filter && *filter);
	cd_memory_pool_free(&context->lowpass_filters, *filter);
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

cd_highpass_t* cd_make_highpass_filter(cd_context_t* context, const cd_highpass_def_t* definition)
{
	cd_highpass_t* filter = NULL;
	CUTE_DSP_ASSERT(context && definition);

	filter = (cd_highpass_t*)cd_memory_pool_alloc(&context->highpass_filters);
	CUTE_DSP_ASSERT(filter);

	filter->sampling_rate = definition->sampling_rate;
	cd_set_highpass_cutoff_frequency(filter, definition->freq_cutoff);
	filter->x1 = filter->x2 = filter->y1 = filter->y2 = 0;
	filter->next = 0;
	return filter;
}

void cd_release_highpass(cd_context_t* context, cd_highpass_t** filter)
{
	CUTE_DSP_ASSERT(filter && *filter);
	cd_memory_pool_free(&context->highpass_filters, *filter);
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
