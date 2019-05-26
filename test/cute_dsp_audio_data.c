/*
    ------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

    audio_data.c - v1.0

    Summary:
        Implementation of cd_audio_data_t, intended as a test framework for cute_dsp.

    Revision history:
        1.0     (05/25/2019) initial release: implemented reading/writing wav files.
*/

#include "cute_dsp_audio_data.h"

#include <stdio.h>  /* File i/o */
#include <stdlib.h> /* malloc, free */
#include <string.h> /* memcpy, memset */

#define CUTE_DSP_ASSERT_INTERNAL *(int*)0 = 0
#define CUTE_DSP_ASSERT(X) do { if(!(X)) CUTE_DSP_ASSERT_INTERNAL; } while (0)

#define CUTE_DSP_ALLOC(size) malloc(size)
#define CUTE_DSP_FREE(mem)   free(mem)

cd_audio_data_t cd_read_wav_file(const char* filename)
{
    char header[44] = { 0 };
    unsigned size, rate;
    char* data = NULL;
    cd_audio_data_t adata = { 0 };
    FILE* file = fopen(filename, "r");

    CUTE_DSP_ASSERT(file);

    fread(header, sizeof(char), 44, file);

    size = *(unsigned*)(header + 40);
    rate = *(unsigned*)(header + 24);
    adata.sampling_rate = rate;
    adata.bits_per_sample = *(short*)(header + 34);
    adata.size_in_bytes = size;
    adata.num_samples = size / (adata.bits_per_sample / 8.f);
    data = (char *)CUTE_DSP_ALLOC(sizeof(char) * size);
    CUTE_DSP_ASSERT(data);

    fread(data, sizeof(char), size, file);

    if(adata.bits_per_sample == 16)
    {
        unsigned i = 0;
        adata.data = (float*)CUTE_DSP_ALLOC(sizeof(float) * adata.num_samples);
        const float denom = (float)((1 << (adata.bits_per_sample - 1)));
        for(; i < adata.num_samples; ++i)
        {
            short val = *(short*)(data + i * sizeof(short));
            float nextVal = (float)(val) / denom;
            adata.data[i] = nextVal;
        }
    }
    else
    {
        printf("Bits per sample was an unrecognized format: %d\n", adata.bits_per_sample);
        printf("cd_audio_data_t only supports 16 bit audio.\n");
    }

    CUTE_DSP_FREE(data);
    fclose(file);

    return adata;
}

cd_audio_data_t cd_make_audio_data(unsigned num_samples, short bits, float sampling_rate)
{
    cd_audio_data_t data = { 0 };
    data.num_samples = num_samples;
    data.bits_per_sample = bits;
    data.sampling_rate = sampling_rate;
    data.size_in_bytes = bits / 8.f * num_samples;
    data.data = (float *)CUTE_DSP_ALLOC(sizeof(float) * num_samples);

    CUTE_DSP_ASSERT(data.data);

    memset(data.data, 0, sizeof(float) * num_samples);

    return data;
}

void cd_copy_audio_data(cd_audio_data_t* lhs, const cd_audio_data_t* rhs)
{
    CUTE_DSP_ASSERT(lhs != rhs);
    CUTE_DSP_ASSERT(lhs && rhs);
    lhs->sampling_rate = rhs->sampling_rate;
    lhs->size_in_bytes = rhs->size_in_bytes;
    lhs->num_samples = rhs->num_samples;
    lhs->bits_per_sample = rhs->bits_per_sample;
    if(lhs->data)
    {
        CUTE_DSP_FREE(lhs->data);
    }

    lhs->data = (float *)CUTE_DSP_ALLOC(sizeof(float) * rhs->num_samples);
    CUTE_DSP_ASSERT(lhs->data);

    memcpy(lhs->data, rhs->data, sizeof(float) * rhs->num_samples);
}

void cd_release_audio_data(cd_audio_data_t* data)
{
    CUTE_DSP_ASSERT(data);
    if(data->data)
    {
        CUTE_DSP_FREE(data->data);
    }

    memset(data, 0, sizeof(cd_audio_data_t));
}

static void cd_write_header(FILE** output, const cd_audio_data_t* data)
{
    struct
    {
        char riff_chunk[4];
        unsigned chunk_size;
        char wave_fmt[4];
        char fmt_chunk[4];
        unsigned fmt_chunk_size;
        unsigned short audio_format;
        unsigned short number_of_channels;
        unsigned sampling_rate;
        unsigned bytes_per_second;
        unsigned short block_align;
        unsigned short bits_per_sample;
        char data_chunk[4];
        unsigned data_chunk_size;
    }
    header = { {'R', 'I', 'F', 'F'},
                0, /* chunk size = 36 + data->size_in_bytes */
                {'W', 'A', 'V', 'E'},
                {'f', 'm', 't', ' '},
                16, 1, 1, 
                0, /* sampling_rate = data->sampling_rate */
                0, /* bytes per second = sizeof(short) * sampling_rate */
                2, 
                0, /* bits per sample = data->bits_per_sample */
                {'d', 'a', 't', 'a'},
                0  /* data_chunk_size = data->size_in_bytes */
    };

    header.chunk_size = 36 + data->size_in_bytes;
    header.sampling_rate = data->sampling_rate;
    header.bytes_per_second = sizeof(short) * data->sampling_rate;
    header.bits_per_sample = data->bits_per_sample;
    header.data_chunk_size = data->size_in_bytes;
    
    fwrite((char*)(&header), sizeof(char), 44, *output);
}

#define FLOAT_TO_SHORT(s) ((short)(s * (float)((1 << (16 - 1)) - 1)))

void cd_write_wav_file(const char* filename, const cd_audio_data_t* data)
{
    short* outData = NULL;
    unsigned i = 0;
    FILE* file = fopen(filename, "w");
    CUTE_DSP_ASSERT(file);
    CUTE_DSP_ASSERT(data);

    outData = (short*)CUTE_DSP_ALLOC(sizeof(short) * data->num_samples);
    CUTE_DSP_ASSERT(outData);

    for(; i < data->num_samples; ++i)
    {
        float nextF = data->data[i];
        short nextS = FLOAT_TO_SHORT(nextF);
        outData[i] = nextS;
    }

    cd_write_header(&file, data);
    fwrite((char*)(outData), sizeof(char), data->size_in_bytes, file);
    CUTE_DSP_FREE(outData);
    fclose(file);
}

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
