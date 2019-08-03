/*
    ------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

    cute_dsp_wav_demo.c - v1.1

    To compile:

        gcc -o cute_dsp_wav_demo cute_dsp_wav_demo.c cute_dsp_audio_data.c

    To run:

        ./cute_dsp_wav_demo <input-file>.wav <options>

        <input-file> is a simple 16 bit, 44.1kHZ .wav file to process
        <options> can be any of the following:
            -l = process the input with a lowpass filter, output-l.wav
            -h = process the input with a highpass filter, output-h.wav

    Summary:
        Meant as a test framework for cute_dsp.h to output .wav files that will 
        demonstrate the features in the project.

    Revision history:
        1.0     (05/25/2019) initial release: 
        1.1     (06/29/2019) Updated functions to use the new dsp context to manage memory.
*/

#include "cute_dsp_audio_data.h"

#define CUTE_DSP_IMPLEMENTATION
#include "../cute_dsp.h"

#include <stdio.h>

/* BEGIN TEST FUNCTIONS */
static void process_low_pass(cd_context_t* context, const cd_audio_data_t* in)
{
    cd_audio_data_t out = cd_make_audio_data(in->num_samples, in->bits_per_sample, in->sampling_rate);

    unsigned num_samples = in->num_samples;
    unsigned change_index = num_samples / 4;
    unsigned i = 0;
    float cutoff = 4000.f;

    cd_lowpass_def_t def = cd_make_lowpass_def(cutoff, in->sampling_rate);
    cd_lowpass_t* lpf = cd_make_lowpass_filter(context, &def);

    for(; i < num_samples; ++i)
    {
        out.data[i] = cd_sample_lowpass(lpf, in->data[i]);
        if(i && i % change_index == 0)
        {
            cutoff /= 2.f;
            cd_set_lowpass_cutoff_frequency(lpf, cutoff);
        }
    }

    cd_write_wav_file("output-l.wav", &out);

    cd_release_lowpass(context, &lpf);
    cd_release_audio_data(&out);
}

static void process_high_pass(cd_context_t* context, const cd_audio_data_t* in)
{
    cd_audio_data_t out = cd_make_audio_data(in->num_samples, in->bits_per_sample, in->sampling_rate);

    unsigned num_samples = out.num_samples;
    unsigned change_index = num_samples / 4;
    unsigned i = 0;
    float cutoff = 400.f;

    cd_highpass_def_t def = cd_make_highpass_def(cutoff, in->sampling_rate);
    cd_highpass_t* hpf = cd_make_highpass_filter(context, &def);

    for(; i < num_samples; ++i)
    {
        out.data[i] = cd_sample_highpass(hpf, in->data[i]);
        if(i && i % change_index == 0)
        {
            cutoff *= 2.f;
            cd_set_highpass_cutoff_frequency(hpf, cutoff);
        }
    }

    cd_write_wav_file("output-h.wav", &out);

    cd_release_highpass(context, &hpf);
    cd_release_audio_data(&out);
}
/* END TEST FUNCTIONS */

/* BEGIN OPTIONS DEFINITIONS */
typedef void (*process_func)(cd_context_t*, const cd_audio_data_t* );

typedef struct process_option
{
    char option;
    process_func func;
} process_option;

static const unsigned NUM_OPS = 2;
static const process_option OPS[] =
{
    {'l', process_low_pass},
    {'h', process_high_pass}
};
/* END OPTIONS DEFINITIONS */

/* BEGIN MAIN */
int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("NO WAVE FILE SPECIFIED!\n");
        return 1;
    }
    const char* filename = argv[1];
    unsigned i = 2;
    unsigned length = argc;
    cd_audio_data_t in_data = cd_read_wav_file(filename);
    cd_context_t* context;
    cd_context_def_t context_def;
    context_def.playing_pool_count = 10;
    context_def.sampling_rate = in_data.sampling_rate;
    context = cd_make_context(context_def);

    /* if no options provided, do all tests */
    if(length == 2)
    {
        for(i = 0; i < NUM_OPS; ++i)
        {
            OPS[i].func(context, &in_data);
        }
    }
    else
    {
        /* process each option provided */
        for(; i < length; ++i)
        {
            const char* next_arg = argv[i];
            char next_option = next_arg[1];
            unsigned j = 0;

            /* find the option and call the process func */
            for(; j < NUM_OPS; ++j)
            {
                if(OPS[j].option == next_option)
                {
                    OPS[j].func(context, &in_data);
                    break;
                }
            }
        }
    }

    cd_release_context(&context);
    cd_release_audio_data(&in_data);
    return 0;
}
/* END MAIN */

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
