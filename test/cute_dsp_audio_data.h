/*
    ------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

    audio_data.h - v1.0

    Summary:
        Header for cd_audio_data_t, intended as a test framework for cute_dsp.

    Revision history:
        1.0     (05/25/2019) initial release: prototypes for audio_data.
*/
#if !defined(CUTE_DSP_AUDIO_DATA_H)

/*
    Encapsulates a set of audio data.
    Can read in a .wav file, or can be used to manually generate audio data.
    Can also output as a .wav file.
*/
typedef struct cd_audio_data_t
{
    float sampling_rate;
    unsigned size_in_bytes;
    unsigned num_samples;
    float* data;    /* dynamically allocated */
    short bits_per_sample;

} cd_audio_data_t;

/*
    Reads in a simple .wav file and returns the corresponding audio data.
*/
cd_audio_data_t cd_read_wav_file(const char* filename);

/*
    Basic constructs audio data from the given parameters.
    Dynamically allocates the array of data, and initializes it to zero.
*/
cd_audio_data_t cd_make_audio_data(unsigned num_samples, short bits, float sampling_rate);

/*
    Copies audio data from rhs to lhs
    like operator= overload
*/
void cd_copy_audio_data(cd_audio_data_t* lhs, const cd_audio_data_t* rhs);

/*
    Releases the resources inside audio data.
    Assumes that the struct itself wasn't dynamically allocated
*/
void cd_release_audio_data(cd_audio_data_t* data);

/*
    Writes the audio data to a file.
*/
void cd_write_wav_file(const char* filename, const cd_audio_data_t* data);

#define CUTE_DSP_AUDIO_DATA_H
#endif

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
