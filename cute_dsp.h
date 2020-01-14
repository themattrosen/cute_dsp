/*
	------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

	cute_dsp.h - v1.0

	To create implementation (the function definitions)
		#define CUTE_DSP_IMPLEMENTATION
	in *one* C/CPP file (translation unit) that includes this file
	For more details on integration with cute_sound, read the README.md

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
		1.2		(07/12/2019)	bug fixes from integration testing
		1.3 	(09/02/2019)	added cute_sound plugin interface
*/
#ifndef CUTE_SOUND_H
#	error Please include cute_sound.h before including cute_dsp.h.
#endif

#if !defined(CUTE_DSP_H)

/* BEGIN PLUGIN INTERFACE FUNCTIONS */
/*
	Helper function to create a plugin interface for the lowpass filter
	Called internally, but exposed for use if needed.
*/
cs_plugin_interface_t cd_make_lowpass_plugin();

/*
	Helper function to create a plugin interface for the lowpass filter.
	Called internally, but exposed for use if needed.
*/
cs_plugin_interface_t cd_make_highpass_plugin();
/* END PLUGIN INTERFACE FUNCTIONS */

/* BEGIN FORWARD DECLARATIONS */
/*
	cute_dsp context type
	Manages memory and settings of the cute_dsp context.
*/
struct cd_context_t;
typedef struct cd_context_t cd_context_t;

/*
	cute_dsp lowpass filter type. 
	Implemented using a second order (6dB/Octave roll off) Butterworth filter.
*/
struct cd_lowpass_t;
typedef struct cd_lowpass_t cd_lowpass_t;

/*
	cute_dsp highpass filter type
	Implemented using a second order (6dB/Octave roll off) Butterworth filter
		converted from the second order Butterworth lowpass transfer function.
*/
struct cd_highpass_t;
typedef struct cd_highpass_t cd_highpass_t;

#define CUTE_DSP_MAX_FRAME_LENGTH 4096
/* END FORWARD DECLARATIONS */

/* BEGIN DSP CONTEXT API */

typedef struct cd_context_def_t
{
	unsigned playing_pool_count; // max number of playing sounds
	float sampling_rate;

	// bit field for setting what plugins to use
	struct
	{
		unsigned int use_lowpass : 1;
		unsigned int use_highpass : 1;
	};
} cd_context_def_t;

cd_context_t* cd_make_context(cs_context_t* sound_ctx, cd_context_def_t);

void cd_release_context(cd_context_t** context);
/* END DSP CONTEXT API */

/* BEGIN LOW PASS FILTER API */

/*
	TODO
*/
cs_plugin_interface_t cd_make_lowpass_plugin();

/*
	dynamically allocates a lowpass filter from a lowpass_def
	called via 
	cd_lowpass_def_t def = cd_make_lowpass_def(freq, rate);
	cd_lowpass_t* filter = cd_make_lowpass_filter(context, &def);
*/
cd_lowpass_t* cd_make_lowpass_filter(cd_context_t* context);

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
	Fetches lowpass filter from playing sound and calls cd_set_lowpass_cutoff_frequency.
*/
void cd_set_lowpass_cutoff(cs_playing_sound_t* playing_sound, float cutoff_freq_in_hz);

/*
	Gets the cutoff frequency in a lowpass filter.
	Cutoff frequency parameter in hz.
*/
float cd_get_lowpass_cutoff_frequency(const cd_lowpass_t* filter);

/*
	Gets the cutoff frequency in a lowpass filter.
	Cutoff frequency parameter in hz.
*/
float cd_get_lowpass_cutoff(const cs_playing_sound_t* playing_sound);

/*
	Processes the next sample of the given filter.
	@param input next sample from the signal chain
	@return the input sample processed by the lowpass filter
*/
void cd_sample_lowpass(cd_context_t* context, cd_lowpass_t* filter, const float* input, float** output, unsigned num_samples);
/* END LOWPASS FILTER API */

/* BEGIN HIGHPASS FILTER API */
/*
	dynamically allocates a highpass filter from a lowpass_def
	called via 
	cd_highpass_def_t def = cd_make_highpass_def(freq, rate);
	cd_highpass_t* filter = cd_make_highpass_filter(context, &def);
*/
cd_highpass_t* cd_make_highpass_filter(cd_context_t* context);

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
	Fetches lowpass filter from playing sound and calls cd_set_lowpass_cutoff_frequency.
*/
void cd_set_highpass_cutoff(cs_playing_sound_t* playing_sound, float cutoff_freq_in_hz);

/*
	Gets the cutoff frequency in a highpass filter.
	Cutoff frequency parameter in hz.
*/
float cd_get_highpass_cutoff_frequency(const cd_highpass_t* filter);

/*
	Gets the cutoff frequency in a highpass filter.
	Cutoff frequency parameter in hz.
*/
float cd_get_highpass_cutoff(const cs_playing_sound_t* playing_sound);

/*
	Processes the next sample of the given filter.
	@param input next sample from the signal chain
	@return the input sample processed by the highpass filter
*/
void cd_sample_highpass(cd_context_t* context, cd_highpass_t* filter, const float* input, float** output, unsigned num_samples);

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
	#include <string.h> // memcpy
	#define CUTE_DSP_ALLOC(size, mem_ctx) malloc(size)
	#define CUTE_DSP_FREE(mem, mem_ctx)   free(mem)
#endif

#define CUTE_DSP_ASSERT_INTERNAL *(int*)0 = 0
#define CUTE_DSP_ASSERT(X) do { if(!(X)) CUTE_DSP_ASSERT_INTERNAL; } while (0)

#if !defined(CUTE_DSP_MATH)
	#include <math.h> // sinf, cosf, absf, atan2f
	#define CUTE_DSP_MATH
	#define CUTE_DSP_SIN(angle_rad) (sinf(angle_rad))
	#define CUTE_DSP_COS(angle_rad) (cosf(angle_rad))
	#define CUTE_DSP_ABS(val)       (fabsf(val))
	#define CUTE_DSP_PI				(3.1415926f)
	#define CUTE_DSP_SQRT_2 		(1.4142136f)
	#define CUTE_DSP_CLAMP(val, least, most) ( (val) < (least) ? (least) : ( (val) > (most) ? (most) : (val) ) )

	/* more here */
#endif

#if !defined(CUTE_DSP_DEFINES)
	#define CUTE_DSP_DEFINES
	#define CUTE_DSP_MONO (1)
	#define CUTE_DSP_STEREO (2)
	#define CUTE_DSP_MAX_FRAME_LENGTH (4096)
	#define CUTE_DSP_INVALID_PLUGIN_ID (-1)

	// default cutoff frequencies for LPF and HPF chosen 
	// such that their effect is inaudible (at the edges of human hearing)
	// unless changed
	#define CUTE_DSP_DEFAULT_HIGHPASS_CUTOFF (20.0f)
	#define CUTE_DSP_DEFAULT_LOWPASS_CUTOFF  (20000.0f)
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
	cd_memory_pool_t lowpass_filters;
	cd_memory_pool_t highpass_filters;
	unsigned pool_size;
	cs_plugin_id_t lowpass_id;
	cs_plugin_id_t highpass_id;
	float* current_output;
	float output1[CUTE_DSP_MAX_FRAME_LENGTH * CUTE_DSP_STEREO]; // number of samples is frames * channels
	float output2[CUTE_DSP_MAX_FRAME_LENGTH * CUTE_DSP_STEREO];
} cd_context_t;

static cd_context_t* g_dsp_context = 0;

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

/* BEGIN PLUGIN INTERFACE IMPLEMENTATION */
static cd_context_t* cd_get_context(void);

static void cd_lowpass_on_make_playing_sound(cs_context_t* cs_ctx, void* plugin_instance, void** playing_sound_udata, const cs_playing_sound_t* sound)
{
	cd_context_t* ctx = (cd_context_t*)plugin_instance;
	switch(sound->loaded_sound->channel_count)
	{
		case 1:
		{
			cd_lowpass_t* lowpass = cd_make_lowpass_filter(ctx);
			*playing_sound_udata = lowpass;
			break;
		}
		case 2:
		{
			cd_lowpass_t* lowpass = cd_make_lowpass_filter(ctx);
			*playing_sound_udata = lowpass;
			lowpass->next = cd_make_lowpass_filter(ctx);
			break;
		}
	}

	// unused parameters
	(void)cs_ctx;
	(void)sound;
}

static void cd_lowpass_on_free_playing_sound(cs_context_t* cs_ctx, void* plugin_instance, void* playing_sound_udata, const cs_playing_sound_t* sound)
{
	cd_context_t* ctx = (cd_context_t*)plugin_instance;
	cd_lowpass_t* filter = (cd_lowpass_t*)playing_sound_udata;
	if(filter->next)
	{
		cd_release_lowpass(ctx, &(filter->next));
	}
	cd_release_lowpass(ctx, &filter);

	// unused parameters
	(void)cs_ctx;
	(void)sound;
}

static void cd_lowpass_on_mix(cs_context_t* cs_ctx, void* plugin_instance, int channel_index, const float* samples_in, int sample_count, float** samples_out, void* playing_sound_udata, const cs_playing_sound_t* sound)
{
	cd_lowpass_t* filter = 0;
	cd_context_t* context = (cd_context_t*)plugin_instance;
	switch(channel_index)
	{
	case 0:
		filter = (cd_lowpass_t*)playing_sound_udata;
		break;
	case 1:
		filter = (cd_lowpass_t*)playing_sound_udata;
		filter = filter->next;
		break;
	}
	cd_sample_lowpass(context, filter, samples_in, samples_out, (unsigned)sample_count);

	// unused parameters
	(void)cs_ctx;
	(void)sound;
}

cs_plugin_interface_t cd_make_lowpass_plugin(void)
{
	cs_plugin_interface_t plugin;
	plugin.plugin_instance = cd_get_context();
	plugin.on_make_playing_sound_fn = cd_lowpass_on_make_playing_sound;
	plugin.on_free_playing_sound_fn = cd_lowpass_on_free_playing_sound;
	plugin.on_mix_fn = cd_lowpass_on_mix;

	return plugin;
}

static void cd_highpass_on_make_playing_sound(cs_context_t* cs_ctx, void* plugin_instance, void** playing_sound_udata, const cs_playing_sound_t* sound)
{
	cd_context_t* ctx = (cd_context_t*)plugin_instance;
	switch(sound->loaded_sound->channel_count)
	{
		case 1:
		{
			cd_highpass_t* filter = cd_make_highpass_filter(ctx);
			*playing_sound_udata = filter;
			break;
		}
		case 2:
		{
			cd_highpass_t* filter = cd_make_highpass_filter(ctx);
			*playing_sound_udata = filter;
			filter->next = cd_make_highpass_filter(ctx);
			break;
		}
	}
}

static void cd_highpass_on_free_playing_sound(cs_context_t* cs_ctx, void* plugin_instance, void* playing_sound_udata, const cs_playing_sound_t* sound)
{
	cd_context_t* ctx = (cd_context_t*)plugin_instance;
	cd_highpass_t* filter = (cd_highpass_t*)playing_sound_udata;
	if(filter->next)
	{
		cd_release_highpass(ctx, &(filter->next));
	}
	cd_release_highpass(ctx, &filter);
}

static void cd_highpass_on_mix(cs_context_t* cs_ctx, void* plugin_instance, int channel_index, const float* samples_in, int sample_count, float** samples_out, void* playing_sound_udata, const cs_playing_sound_t* sound)
{
	cd_highpass_t* filter = 0;
	cd_context_t* context = (cd_context_t*)plugin_instance;

	switch(channel_index)
	{
	case 0:
		filter = (cd_highpass_t*)playing_sound_udata;
		break;
	case 1:
		filter = (cd_highpass_t*)playing_sound_udata;
		filter = filter->next;
		break;
	}
	cd_sample_highpass(context, filter, samples_in, samples_out, (unsigned)sample_count);
}

cs_plugin_interface_t cd_make_highpass_plugin()
{
	cs_plugin_interface_t plugin;
	plugin.plugin_instance = cd_get_context();
	plugin.on_make_playing_sound_fn = cd_highpass_on_make_playing_sound;
	plugin.on_free_playing_sound_fn = cd_highpass_on_free_playing_sound;
	plugin.on_mix_fn = cd_highpass_on_mix;

	return plugin;
}
/* END PLUGIN INTERFACE IMPLEMENTATION */

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
		cd_memory_pool_object_t* obj = (cd_memory_pool_object_t *)(mem_pool->pool + i * def.size_per_object);
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
static cd_context_t* cd_get_context(void)
{
	return g_dsp_context;
}

#define cd_context_swap_buffers(context)	\
{	\
	if (context->current_output == context->output1)	\
	{	\
		context->current_output = context->output2;	\
	}	\
	else	\
	{	\
		context->current_output = context->output1;	\
	}	\
}

cd_context_t* cd_make_context(cs_context_t* sound_ctx, cd_context_def_t def)
{
	cd_memory_pool_def_t lowpass_def;
	cd_memory_pool_def_t highpass_def;

	cd_context_t* context = (cd_context_t *)CUTE_DSP_ALLOC(sizeof(cd_context_t), 0);
	CUTE_DSP_ASSERT(context && def.playing_pool_count);
	g_dsp_context = context;

	context->sampling_rate = def.sampling_rate;
	context->pool_size = def.playing_pool_count * 2;
	context->channel_count = CUTE_DSP_STEREO;
	memset(context->output1, 0, sizeof(float) * CUTE_DSP_MAX_FRAME_LENGTH);
	memset(context->output2, 0, sizeof(float) * CUTE_DSP_MAX_FRAME_LENGTH);
	context->current_output = context->output1;

	if(def.use_lowpass)
	{
		lowpass_def.max_objects = context->pool_size;
		lowpass_def.size_per_object = sizeof(cd_lowpass_t);
		cd_make_memory_pool(&context->lowpass_filters, lowpass_def);

		cs_plugin_interface_t lowpass_interface = cd_make_lowpass_plugin();
		context->lowpass_id = cs_add_plugin(sound_ctx, &lowpass_interface);
	}
	else
	{
		memset(&context->lowpass_filters, 0, sizeof(cd_memory_pool_t));
		context->lowpass_id = CUTE_DSP_INVALID_PLUGIN_ID;
	}

	if(def.use_highpass)
	{
		highpass_def.max_objects = context->pool_size;
		highpass_def.size_per_object = sizeof(cd_highpass_t);
		cd_make_memory_pool(&context->highpass_filters, highpass_def);

		cs_plugin_interface_t highpass_interface = cd_make_highpass_plugin();
		context->highpass_id = cs_add_plugin(sound_ctx, &highpass_interface);
	}
	else
	{
		memset(&context->highpass_filters, 0, sizeof(cd_memory_pool_t));
		context->highpass_id = CUTE_DSP_INVALID_PLUGIN_ID;
	}

	return context;
}

void cd_release_context(cd_context_t** context)
{
	CUTE_DSP_ASSERT(context && *context);

	cd_release_memory_pool(&(*context)->lowpass_filters);
	cd_release_memory_pool(&(*context)->highpass_filters);

	CUTE_DSP_FREE(*context, 0);
	*context = 0;
	g_dsp_context = 0;
}

/* END CONTEXT IMPLEMENTATION */

/* BEGIN LOWPASS IMPLEMENTATION */
cd_lowpass_t* cd_make_lowpass_filter(cd_context_t* context)
{
	cd_lowpass_t* filter = NULL;
	CUTE_DSP_ASSERT(context);

	filter = (cd_lowpass_t*)cd_memory_pool_alloc(&context->lowpass_filters);
	CUTE_DSP_ASSERT(filter);

	filter->sampling_rate = context->sampling_rate;
	cd_set_lowpass_cutoff_frequency(filter, CUTE_DSP_DEFAULT_LOWPASS_CUTOFF);
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
	filter->freq_cutoff = cutoff_freq_in_hz;
	float freq_in_rad = 2.f * CUTE_DSP_PI * cutoff_freq_in_hz;

	cd_set_lowpass_cutoff_frequency_radians(filter, freq_in_rad);
	if(filter->next)
	{
		cd_set_lowpass_cutoff_frequency_radians(filter->next, freq_in_rad);
	}
}

void cd_set_lowpass_cutoff(cs_playing_sound_t* playing_sound, float cutoff_freq_in_hz)
{
	CUTE_DSP_ASSERT(playing_sound);
	cd_context_t* ctx = cd_get_context();
	cd_lowpass_t* filter = (cd_lowpass_t*)playing_sound->plugin_udata[ctx->lowpass_id];
	cd_set_lowpass_cutoff_frequency(filter, cutoff_freq_in_hz);
}

float cd_get_lowpass_cutoff_frequency(const cd_lowpass_t* filter)
{
	return filter->freq_cutoff;
}

float cd_get_lowpass_cutoff(const cs_playing_sound_t* playing_sound)
{
	CUTE_DSP_ASSERT(playing_sound);
	cd_context_t* ctx = cd_get_context();
	const cd_lowpass_t* filter = (const cd_lowpass_t*)playing_sound->plugin_udata[ctx->lowpass_id];
	return cd_get_lowpass_cutoff_frequency(filter);
}

void cd_sample_lowpass(cd_context_t* context, cd_lowpass_t* filter, const float* input, float** output, unsigned num_samples)
{
	unsigned i = 0; 
	float* samples = context->current_output;
	//memset(samples, 0, sizeof(float) * CUTE_DSP_MAX_FRAME_LENGTH);

	for(; i < num_samples; ++i)
	{
		*samples = filter->x_coeff * *input++ + 
					filter->y1_coeff * filter->y1 + 
					filter->y2_coeff * filter->y2;
		filter->y2 = filter->y1;
		filter->y1 = *samples++;
	}

	*output = context->current_output;
	cd_context_swap_buffers(context);
}
/* END LOWPASS IMPLEMENTATION */

/* BEGIN HIGHPASS IMPLEMENTATION */
cd_highpass_t* cd_make_highpass_filter(cd_context_t* context)
{
	cd_highpass_t* filter = NULL;
	CUTE_DSP_ASSERT(context);

	filter = (cd_highpass_t*)cd_memory_pool_alloc(&context->highpass_filters);
	CUTE_DSP_ASSERT(filter);

	filter->sampling_rate = context->sampling_rate;
	cd_set_highpass_cutoff_frequency(filter, CUTE_DSP_DEFAULT_HIGHPASS_CUTOFF);
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
	filter->freq_cutoff = cutoff_freq_in_hz;
	float freq_in_rad = 2.f * CUTE_DSP_PI * cutoff_freq_in_hz;

	cd_set_highpass_cutoff_frequency_radians(filter, freq_in_rad);
	if(filter->next)
	{
		cd_set_highpass_cutoff_frequency_radians(filter->next, freq_in_rad);
	}
}

void cd_set_highpass_cutoff(cs_playing_sound_t* playing_sound, float cutoff_freq_in_hz)
{
	CUTE_DSP_ASSERT(playing_sound);
	cd_context_t* ctx = cd_get_context();
	cd_highpass_t* filter = (cd_highpass_t*)playing_sound->plugin_udata[ctx->highpass_id];
	cd_set_highpass_cutoff_frequency(filter, cutoff_freq_in_hz);
}

float cd_get_highpass_cutoff_frequency(const cd_highpass_t* filter)
{
	return filter->freq_cutoff;
}

float cd_get_highpass_cutoff(const cs_playing_sound_t* playing_sound)
{
	CUTE_DSP_ASSERT(playing_sound);
	cd_context_t* ctx = cd_get_context();
	const cd_highpass_t* filter = (const cd_highpass_t*)playing_sound->plugin_udata[ctx->highpass_id];
	return cd_get_highpass_cutoff_frequency(filter);
}

void cd_sample_highpass(cd_context_t* context, cd_highpass_t* filter, const float* input, float** output, unsigned num_samples)
{
	unsigned i = 0;
	float* samples = context->current_output;
	//memset(samples, 0, sizeof(float) * CUTE_DSP_MAX_FRAME_LENGTH);

	for(; i < num_samples; ++i)
	{
		*samples = filter->x_coeff  * *input +
					filter->x1_coeff * filter->x1 +
					filter->x_coeff  * filter->x2 +
					filter->y1_coeff * filter->y1 +
					filter->y2_coeff * filter->y2;
		filter->x2 = filter->x1;
		filter->x1 = *input++;
		filter->y2 = filter->y1;
		filter->y1 = *samples++;
	}

	*output = context->current_output;
	cd_context_swap_buffers(context);
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
