/*
    ------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

    cute_dsp_test.c - v1.0

    To compile (windows only):

        cl cute_dsp_test.c /EHsc User32.lib

    To run:

        ./cute_dsp_test <option>
		<option> can be either of the following:
			-u = unit test: runs offline unit tests
			-i = integration test: runs realtime integration test. actually runs cute_sound
		if <option> isn't provided, unit tests are run first, followed by the integration test.

    Summary:
        Meant as a test framework for cute_dsp.h for integration with cute_sound.h

    Revision history:
        1.0     (06/30/2019) initial release: 
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#define CUTE_DSP_IMPLEMENTATION
#include "../cute_dsp.h"

#define CUTE_SOUND_IMPLEMENTATION
#include "../../cute_headers/cute_sound.h"

#define CUTE_DSP_ASSERT_INTERNAL *(int*)0 = 0
#define CUTE_DSP_ASSERT(X) do { if(!(X)) CUTE_DSP_ASSERT_INTERNAL; } while (0)

/* BEGIN INTEGRATION TEST */
#define PLAYING_POOL_SIZE 10

// data on certain keys current and last frame status
// each takes one bit, 1 for pressed, 0 for not
struct
{
	unsigned q0 : 1;
	unsigned q1 : 1;
	unsigned w0 : 1;
	unsigned w1 : 1;
	unsigned e0 : 1;
	unsigned e1 : 1;
	unsigned r0 : 1;
	unsigned r1 : 1;
	unsigned t0 : 1;
	unsigned t1 : 1;
	unsigned a0 : 1;
	unsigned a1 : 1;
	unsigned s0 : 1;
	unsigned s1 : 1;
	unsigned d0 : 1;
	unsigned d1 : 1;
	unsigned f0 : 1;
	unsigned f1 : 1;
	unsigned g0 : 1;
	unsigned g1 : 1;
} g_inputs;

// called each frame of loop to check key presses
// checks hardcoded keys and sets appropriate values in g_inputs
void handle_input()
{
	// T
	g_inputs.t1 = g_inputs.t0;
	if(GetAsyncKeyState ('T'))
		g_inputs.t0 = 1;
	else
		g_inputs.t0 = 0;

	// R
	g_inputs.r1 = g_inputs.r0;
	if(GetAsyncKeyState ('R'))
		g_inputs.r0 = 1;
	else
		g_inputs.r0 = 0;
	
	// E
	g_inputs.e1 = g_inputs.e0;
	if(GetAsyncKeyState ('E'))
		g_inputs.e0 = 1;
	else
		g_inputs.e0 = 0;
	
	// W
	g_inputs.w1 = g_inputs.w0;
	if(GetAsyncKeyState ('W'))
		g_inputs.w0 = 1;
	else
		g_inputs.w0 = 0;
	
	// Q
	g_inputs.q1 = g_inputs.q0;
	if(GetAsyncKeyState ('Q'))
		g_inputs.q0 = 1;
	else
		g_inputs.q0 = 0;

	// G
	g_inputs.g1 = g_inputs.g0;
	if(GetAsyncKeyState ('G'))
		g_inputs.g0 = 1;
	else
		g_inputs.g0 = 0;

	// F
	g_inputs.f1 = g_inputs.f0;
	if(GetAsyncKeyState ('F'))
		g_inputs.f0 = 1;
	else
		g_inputs.f0 = 0;
	
	// D
	g_inputs.d1 = g_inputs.d0;
	if(GetAsyncKeyState ('D'))
		g_inputs.d0 = 1;
	else
		g_inputs.d0 = 0;
	
	// S
	g_inputs.s1 = g_inputs.s0;
	if(GetAsyncKeyState ('S'))
		g_inputs.s0 = 1;
	else
		g_inputs.s0 = 0;
	
	// A
	g_inputs.a1 = g_inputs.a0;
	if(GetAsyncKeyState ('A'))
		g_inputs.a0 = 1;
	else
		g_inputs.a0 = 0;
}
#define BUTTON_IS_RELEASED(b0, b1) !g_inputs.b0 && g_inputs.b0 != g_inputs.b1

static void test_integration()
{
	int frequency = 44000; // a good standard frequency for playing commonly saved OGG + wav files
	int latency_in_Hz = 15; // a good latency, too high will cause artifacts, too low will create noticeable delays
	int buffered_seconds = 5; // number of seconds the buffer will hold in memory. want this long enough in case of frame-delays
	int use_playing_pool = 1; // non-zero uses high-level API, 0 uses low-level API
	int num_elements_in_playing_pool = PLAYING_POOL_SIZE; // pooled memory array size for playing sounds

	// create the dsp contexts
	cd_context_def_t context_definition;
	context_definition.playing_pool_count = num_elements_in_playing_pool;
	context_definition.sampling_rate = (float)frequency;
	cd_context_t* dsp_ctx = cd_make_context(context_definition);
	cs_make_dsp_mixer = cd_make_mixer_callback;
	cs_release_dsp_mixer = cd_release_mixer_callback;
	CUTE_DSP_ASSERT(dsp_ctx);

	// create the sound context
	cs_context_t* sound_ctx = cs_make_context(GetConsoleWindow(), frequency, latency_in_Hz, buffered_seconds, num_elements_in_playing_pool);
	cs_set_dsp_context(sound_ctx, dsp_ctx);

	// set mix thread running
	cs_spawn_mix_thread(sound_ctx);
	cs_thread_sleep_delay(sound_ctx, 10);

	// load audio files
	cs_loaded_sound_t music1 = cs_load_wav("music1.wav");
	cs_loaded_sound_t stinger1 = cs_load_wav("stinger1.wav");
	cs_loaded_sound_t stinger2 = cs_load_wav("stinger2.wav");

	// defs for highpass and lowpass
	cd_lowpass_def_t lpdef = cd_make_lowpass_def(2000.f, (float)music1.sample_rate);
	cd_highpass_def_t hpdef = cd_make_highpass_def(500.f, (float)music1.sample_rate);

	// mixer for music track
	cd_mixer_def_t mdef0;
	mdef0.channel_count = music1.channel_count;
	mdef0.has_highpass = 1;
	mdef0.has_lowpass = 1;
	mdef0.lowpass_def = lpdef;
	mdef0.highpass_def = hpdef;

	// mixer for stinger 1
	cd_mixer_def_t mdef1;
	mdef1.channel_count = stinger1.channel_count;
	mdef1.has_highpass = 0;
	mdef1.has_lowpass = 1;
	lpdef.sampling_rate = (float)stinger1.sample_rate;
	mdef1.lowpass_def = lpdef;

	// mixer for stinger 2
	cd_mixer_def_t mdef2;
	mdef2.channel_count = stinger2.channel_count;
	mdef2.has_highpass = 1;
	mdef2.has_lowpass = 0;
	hpdef.sampling_rate = (float)stinger2.sample_rate;
	mdef2.highpass_def = hpdef;

	// play sound defs to start playing audio
	cs_play_sound_def_t def0 = cs_make_def(&music1);
	cs_play_sound_def_t def1 = cs_make_def(&stinger1);
	cs_play_sound_def_t def2 = cs_make_def(&stinger2);

	// set mixers on play defs
	def0.dsp_mixer_def = &mdef0;
	def0.looped = 1;
	def1.dsp_mixer_def = &mdef1;
	def2.dsp_mixer_def = &mdef2;

	// start playing music
	cs_playing_sound_t* music_playing = cs_play_sound(sound_ctx, def0);

	cd_mixer_t* mixer0 = music_playing->dsp_mixer;
	cd_mixer_t* mixer1 = 0;
	cd_mixer_t* mixer2 = 0;

	// retrieve copies of the filters and cutoffs
	cd_lowpass_t* mlp = mixer0->lowpass;
	float mlp_cutoff = cd_get_lowpass_cutoff_frequency(mlp);
	cd_highpass_t* mhp = mixer0->highpass;
	float mhp_cutoff = cd_get_highpass_cutoff_frequency(mhp);
	cd_lowpass_t* slp = 0; // = mixer1->lowpass;
	float slp_cutoff = 2000.f; // = cd_get_lowpass_cutoff_frequency(slp);
	cd_highpass_t* shp = 0; // = mixer2->highpass;
	float shp_cutoff = 500.f; // = cd_get_highpass_cutoff_frequency(shp);

	// loop until triggered otherwise
	for(;;)
	{
		// handle key up/down
		handle_input();

		// if user ever presses escape, break out
		if(GetAsyncKeyState(VK_ESCAPE))
		{
			printf("QUITTING INTEGRATION_TEST\n");
			break;
		}

		// music lpf
		if(BUTTON_IS_RELEASED(t0, t1))
		{
			mlp_cutoff += 100.f;
			cd_set_lowpass_filter_cutoffs(mixer0, mlp_cutoff);
			printf("T PRESSED, music lpf cutoff: %.4f\n", mlp_cutoff);
		}
		else if(BUTTON_IS_RELEASED(r0, r1))
		{
			mlp_cutoff -= 100.f;
			cd_set_lowpass_filter_cutoffs(mixer0, mlp_cutoff);
			printf("R PRESSED, music lpf cutoff: %.4f\n", mlp_cutoff);
		}

		// music hpf
		if(BUTTON_IS_RELEASED(g0, g1))
		{
			mhp_cutoff += 100.f;
			cd_set_highpass_filter_cutoffs(mixer0, mhp_cutoff);
			printf("G PRESSED, music hpf cutoff: %.4f\n", mhp_cutoff);
		}
		else if(BUTTON_IS_RELEASED(f0, f1))
		{
			mhp_cutoff -= 100.f;
			cd_set_highpass_filter_cutoffs(mixer0, mhp_cutoff);
			printf("F PRESSED, music hpf cutoff: %.4f\n", mhp_cutoff);
		}

		if(BUTTON_IS_RELEASED(q0, q1))
		{
			printf("Q PRESSED, playing stinger1\n");
			cs_playing_sound_t* stinger = cs_play_sound(sound_ctx, def1);
			mixer1 = stinger->dsp_mixer;
			slp = mixer1->lowpass;
			slp_cutoff = cd_get_lowpass_cutoff_frequency(slp);
		}

		// stinger1 lpf
		if(BUTTON_IS_RELEASED(e0, e1) && mixer1)
		{
			slp_cutoff += 100.f;
			printf("E PRESSED, stinger1 lpf cutoff: %.4f\n", slp_cutoff);
			cd_set_lowpass_filter_cutoffs(mixer1, slp_cutoff);
		}
		else if(BUTTON_IS_RELEASED(w0, w1) && mixer1)
		{
			slp_cutoff -= 100.f;
			printf("W PRESSED, stinger1 lpf cutoff: %.4f\n", slp_cutoff);
			cd_set_lowpass_filter_cutoffs(mixer1, slp_cutoff);
		}

		if(BUTTON_IS_RELEASED(a0, a1))
		{
			printf("A PRESSED, playing stinger2\n");
			cs_playing_sound_t* stinger = cs_play_sound(sound_ctx, def2);
			mixer2 = stinger->dsp_mixer;
			shp = mixer2->highpass;
			shp_cutoff = cd_get_highpass_cutoff_frequency(shp);
		}

		// stinger2 hpf
		if(BUTTON_IS_RELEASED(d0, d1) && mixer2)
		{
			shp_cutoff += 100.f;
			printf("D PRESSED, stinger2 hpf cutoff: %.4f\n", shp_cutoff);
			cd_set_highpass_filter_cutoffs(mixer2, shp_cutoff);
		}
		else if(BUTTON_IS_RELEASED(s0, s1) && mixer2)
		{
			shp_cutoff -= 100.f;
			printf("S PRESSED, stinger2 hpf cutoff: %.4f\n", shp_cutoff);
			cd_set_highpass_filter_cutoffs(mixer2, shp_cutoff);
		}
	}

	// release contexts
	cs_shutdown_context(sound_ctx);
	cd_release_context(&dsp_ctx);

	// free sounds
	cs_free_sound(&music1);
	cs_free_sound(&stinger1);
	cs_free_sound(&stinger2);
}

/* END INTEGRATION TEST */

/* BEGIN MAIN */
int main(int argc, char** argv)
{
	printf("Beginning Integration Test\n");
	printf("**************************\n\n");
	printf("To stop the test, press ESC\n\n");
	printf("To play stinger1, press Q\n");
	printf("To increase cutoff frequency of stinger1 lowpass filter, press E\n");
	printf("To decrease cutoff frequency of stinger1 lowpass filter, press W\n\n");
	printf("To play stinger2, press A\n");
	printf("To increase cutoff frequency of stinger2 highpass filter, press D\n");
	printf("To decrease cutoff frequency of stinger2 highpass filter, press S\n\n");
	printf("To decrease/increase cutoff frequency of music1 lowpass filter, press R/T\n");
	printf("To decrease/increase cutoff frequency of music1 highpass filter, press F/G\n\n");
	printf("Note: Size of playing pool is set to %d, to increase/decrease, need to set PLAYING_POOL_SIZE and recompile.\n", PLAYING_POOL_SIZE);
	printf("**************************\n\n");

	test_integration();

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
