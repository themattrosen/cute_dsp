/*
    ------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

    cute_dsp_test.c - v1.0

    To compile (windows only):

        cl cute_dsp_test.c /EHsc User32.lib

    To run:

        ./cute_dsp_test

		More testing options will be added later as new features and tools become available.
		
    Summary:
        Meant as a test file for cute_dsp.h for integration with cute_sound.h
*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>

#define CUTE_SOUND_IMPLEMENTATION
#include "../../cute_headers/cute_sound.h"

#define CUTE_DSP_IMPLEMENTATION
#include "../cute_dsp.h"

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

	// create the sound context
	cs_context_t* sound_ctx = cs_make_context(GetConsoleWindow(), frequency, /*latency_in_Hz,*/ buffered_seconds, num_elements_in_playing_pool, 0);

	// set mix thread running
	cs_spawn_mix_thread(sound_ctx);
	cs_thread_sleep_delay(sound_ctx, 10);
	
	// create the dsp contexts
	cd_context_def_t context_definition;
	context_definition.playing_pool_count = num_elements_in_playing_pool;
	context_definition.sampling_rate = (float)frequency;
	context_definition.use_highpass = 1;
	context_definition.use_lowpass = 1;
	cd_context_t* dsp_ctx = cd_make_context(sound_ctx, context_definition);
	CUTE_DSP_ASSERT(dsp_ctx);

	// load audio files
	cs_loaded_sound_t music1 = cs_load_wav("music2.wav");
	cs_loaded_sound_t stinger1 = cs_load_wav("stinger1.wav");
	cs_loaded_sound_t stinger2 = cs_load_wav("stinger2.wav");
	CUTE_DSP_ASSERT(music1.channel_count);
	CUTE_DSP_ASSERT(stinger1.channel_count);
	CUTE_DSP_ASSERT(stinger2.channel_count);

	// play sound defs to start playing audio
	cs_play_sound_def_t def0 = cs_make_def(&music1);
	cs_play_sound_def_t def1 = cs_make_def(&stinger1);
	cs_play_sound_def_t def2 = cs_make_def(&stinger2);

	def0.looped = 1;

	// start playing music
	cs_playing_sound_t* music_playing = cs_play_sound(sound_ctx, def0);

	float mlp_cutoff = cd_get_lowpass_cutoff(music_playing);
	float mhp_cutoff = cd_get_highpass_cutoff(music_playing);
	cs_playing_sound_t* lose_stinger = 0;
	float lose_lp = 2000.f;
	cs_playing_sound_t* win_stinger = 0;
	float win_hp = 500.f;

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
			cd_set_lowpass_cutoff(music_playing, mlp_cutoff);
			printf("T PRESSED, music lpf cutoff: %.4f\n", mlp_cutoff);
		}
		else if(BUTTON_IS_RELEASED(r0, r1))
		{
			mlp_cutoff -= 100.f;
			cd_set_lowpass_cutoff(music_playing, mlp_cutoff);
			printf("R PRESSED, music lpf cutoff: %.4f\n", mlp_cutoff);
		}

		// music hpf
		if(BUTTON_IS_RELEASED(g0, g1))
		{
			mhp_cutoff += 100.f;
			cd_set_highpass_cutoff(music_playing, mhp_cutoff);
			printf("G PRESSED, music hpf cutoff: %.4f\n", mhp_cutoff);
		}
		else if(BUTTON_IS_RELEASED(f0, f1))
		{
			mhp_cutoff -= 100.f;
			cd_set_highpass_cutoff(music_playing, mhp_cutoff);
			printf("F PRESSED, music hpf cutoff: %.4f\n", mhp_cutoff);
		}

		if(BUTTON_IS_RELEASED(q0, q1))
		{
			printf("Q PRESSED, playing stinger1\n");
			lose_stinger = cs_play_sound(sound_ctx, def1);
			cd_set_lowpass_cutoff(lose_stinger, lose_lp);
		}

		// stinger1 lpf
		if(BUTTON_IS_RELEASED(e0, e1) && lose_stinger && lose_stinger->active)
		{
			lose_lp += 100.f;
			printf("E PRESSED, stinger1 lpf cutoff: %.4f\n", lose_lp);
			cd_set_lowpass_cutoff(lose_stinger, lose_lp);
		}
		else if(BUTTON_IS_RELEASED(w0, w1) && lose_stinger && lose_stinger->active)
		{
			lose_lp -= 100.f;
			printf("W PRESSED, stinger1 lpf cutoff: %.4f\n", lose_lp);
			cd_set_lowpass_cutoff(lose_stinger, lose_lp);
		}

		if(BUTTON_IS_RELEASED(a0, a1))
		{
			printf("A PRESSED, playing stinger2\n");
			win_stinger = cs_play_sound(sound_ctx, def2);
			cd_set_highpass_cutoff(win_stinger, win_hp);
		}

		// stinger2 hpf
		if(BUTTON_IS_RELEASED(d0, d1) && win_stinger && win_stinger->active)
		{
			win_hp += 100.f;
			printf("D PRESSED, stinger2 hpf cutoff: %.4f\n", win_hp);
			cd_set_highpass_cutoff(win_stinger, win_hp);
		}
		else if(BUTTON_IS_RELEASED(s0, s1) && win_stinger && win_stinger->active)
		{
			win_hp -= 100.f;
			printf("S PRESSED, stinger2 hpf cutoff: %.4f\n", win_hp);
			cd_set_highpass_cutoff(win_stinger, win_hp);
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
