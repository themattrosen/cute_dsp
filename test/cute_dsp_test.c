/*
    ------------------------------------------------------------------------------
		Licensing information can be found at the end of the file.
	------------------------------------------------------------------------------

    cute_dsp_test.c - v1.5

    To compile (windows only):

        cl cute_dsp_test.c /EHsc User32.lib

    To run:

        ./cute_dsp_test <test_num>

		<test_num> = 0 for lowpass, 1 for highpass, 2 for echo, 3 for noise

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

/* BEGIN INPUT MANAGEMENT */

struct Input
{
	char keyboard[256];
	char prevState[256];
} g_input;

void input_init()
{
	memset(g_input.prevState, 0, sizeof(char) * 256);
	memset(g_input.keyboard, 0, sizeof(char) * 256);
	GetKeyboardState(g_input.keyboard);
}

void input_update()
{
	memcpy(g_input.prevState, g_input.keyboard, sizeof(char) * 256);
	for (int i = 0; i < 256; ++i)
		g_input.keyboard[i] = (char)GetAsyncKeyState(i);
}

int input_get_key_released(int key)
{
	return !g_input.keyboard[key] && g_input.prevState[key];
}

int input_get_key_triggered(int key)
{
	return g_input.keyboard[key] && !g_input.prevState[key];
}

/* END INPUT MANAGEMENT */

/* BEGIN INTEGRATION TEST */
#define PLAYING_POOL_SIZE 10

static void lowpass_test(void)
{
	unsigned frequency = 44100; // a good standard frequency for playing commonly saved OGG + wav files
	int buffered_samples = 8192; // number of samples the buffer will hold in memory. want this long enough in case of frame-delays
	int num_elements_in_playing_pool = PLAYING_POOL_SIZE; // pooled memory array size for playing sounds

	// create the sound context
	cs_context_t* sound_ctx = cs_make_context(GetConsoleWindow(), frequency, buffered_samples, num_elements_in_playing_pool, 0);

	// set mix thread running
	cs_spawn_mix_thread(sound_ctx);
	cs_thread_sleep_delay(sound_ctx, 10);

	// create the dsp contexts
	cd_context_def_t context_definition;
	context_definition.playing_pool_count = num_elements_in_playing_pool;
	context_definition.sampling_rate = (float)frequency;
	context_definition.use_highpass = 0;
	context_definition.use_lowpass = 1;
	context_definition.use_echo = 0;
	context_definition.use_noise = 1;
	context_definition.echo_max_delay_s = 0.f;
	context_definition.rand_seed = 2;
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

	printf("Lowpass Filter Test\n");
	printf("--------------------\n");
	printf("-To change filter coefficients on background music press\n");
	printf(" 'W' to increase the cutoff frequency and \n");
	printf(" 'S' to decrease the cutoff frequency.\n");
	printf(" 'E' to increase background noise amplitude DB and \n");
	printf(" 'Q' to decrease background noise amplitude DB.\n");
	printf(" 'R' to increase the resonance and \n");
	printf(" 'F' to decrease the resonance.\n");
	printf("-Pressing 'A' will prompt you to pick a cutoff frequency\n");
	printf(" for stinger1, which will then play with that coefficient.\n");
	printf("-Pressing 'D' will prompt you to pick a cutoff frequency\n");
	printf(" for stinger2, which will then play with that coefficient.\n");
	printf("\n-To quit, press ESCAPE\n");
	printf("--------------------\n\n");

	cs_playing_sound_t* music_sound = cs_play_sound(sound_ctx, def0);
	float music_cutoff = cd_get_lowpass_cutoff(music_sound);
	float music_resonance = cd_get_lowpass_resonance(music_sound);
	float music_noise = cd_get_noise_amplitude_db(music_sound);
	
	for (;;)
	{
		input_update();

		// if user ever presses escape, break out
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			printf("QUITTING INTEGRATION_TEST\n");
			break;
		}

		// increase cuttoff freq of music
		if (input_get_key_released('W'))
		{
			music_cutoff += 50.f;
			printf("New music cutoff frequency: %f\n", music_cutoff);
			cd_set_lowpass_cutoff(music_sound, music_cutoff);
		}

		// decrease cutoff freq of music
		if (input_get_key_released('S'))
		{
			music_cutoff -= 50.f;
			printf("New music cutoff frequency: %f\n", music_cutoff);
			cd_set_lowpass_cutoff(music_sound, music_cutoff);
		}

		// increase resonance of music
		if (input_get_key_released('R'))
		{
			music_resonance += 0.01f;
			printf("New music resonance: %f\n", music_resonance);
			cd_set_lowpass_resonance(music_sound, music_resonance);
		}

		// decrease resonance of music
		if (input_get_key_released('F'))
		{
			music_resonance -= 0.01f;
			printf("New music resonance: %f\n", music_resonance);
			cd_set_lowpass_resonance(music_sound, music_resonance);
		}

		// increase noise amplitude of music
		if (input_get_key_released('E'))
		{
			music_noise += 1.f;
			printf("New music noise amplitude DB: %f\n", music_noise);
			cd_set_noise_amplitude_db(music_sound, music_noise);
		}

		// decrease noise amplitude of music
		if (input_get_key_released('Q'))
		{
			music_noise -= 1.f;
			printf("New music noise amplitude DB: %f\n", music_noise);
			cd_set_noise_amplitude_db(music_sound, music_noise);
		}

		// stinger 1
		if (input_get_key_released('A'))
		{
			printf("Set Stinger1 cutoff frequency: ");
			float freq = 20000.f;
			scanf("%f", &freq);
			printf("Set Stinger1 resonance: ");
			float res = 0.f;
			scanf("%f", &res);
			printf("Set Stinger1 noise amplitude DB: ");
			float db = -96.f;
			scanf("%f", &db);
			printf("Playing Stinger1\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def1);
			cd_set_lowpass_cutoff(stinger_sound, freq);
			cd_set_lowpass_resonance(stinger_sound, res);
			cd_set_noise_amplitude_db(stinger_sound, db);
		}

		// stinger 2
		if (input_get_key_released('D'))
		{
			printf("Set Stinger2 cutoff frequency: ");
			float freq = 20000.f;
			scanf("%f", &freq);
			printf("Set Stinger2 resonance: ");
			float res = 0.f;
			scanf("%f", &res);
			printf("Set Stinger2 noise amplitude DB: ");
			float db = -96.f;
			scanf("%f", &db);
			printf("Playing Stinger2\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def2);
			cd_set_lowpass_cutoff(stinger_sound, freq);
			cd_set_lowpass_resonance(stinger_sound, res);
			cd_set_noise_amplitude_db(stinger_sound, db);
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

static void highpass_test(void)
{
	int frequency = 44100; // a good standard frequency for playing commonly saved OGG + wav files
	int buffered_samples = 8192; // number of seconds the buffer will hold in memory. want this long enough in case of frame-delays
	int num_elements_in_playing_pool = PLAYING_POOL_SIZE; // pooled memory array size for playing sounds

	// create the sound context
	cs_context_t* sound_ctx = cs_make_context(GetConsoleWindow(), frequency, /*latency_in_Hz,*/ buffered_samples, num_elements_in_playing_pool, 0);

	// set mix thread running
	cs_spawn_mix_thread(sound_ctx);
	cs_thread_sleep_delay(sound_ctx, 10);

	// create the dsp contexts
	cd_context_def_t context_definition;
	context_definition.playing_pool_count = num_elements_in_playing_pool;
	context_definition.sampling_rate = (float)frequency;
	context_definition.use_highpass = 1;
	context_definition.use_lowpass = 0;
	context_definition.use_echo = 0;
	context_definition.use_noise = 1;
	context_definition.echo_max_delay_s = 0.f;
	context_definition.rand_seed = 2;
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

	printf("Highpass Filter Test\n");
	printf("--------------------\n");
	printf("-To change filter coefficients on background music press\n");
	printf(" 'W' to increase the cutoff frequency and \n");
	printf(" 'S' to decrease the cutoff frequency.\n");
	printf(" 'E' to increase background noise amplitude DB and \n");
	printf(" 'Q' to decrease background noise amplitude DB.\n");
	printf("-Pressing 'A' will prompt you to pick a cutoff frequency\n");
	printf(" for stinger1, which will then play with that coefficient.\n");
	printf("-Pressing 'D' will prompt you to pick a cutoff frequency\n");
	printf(" for stinger2, which will then play with that coefficient.\n");
	printf("\n-To quit, press ESCAPE\n");
	printf("--------------------\n\n");

	cs_playing_sound_t* music_sound = cs_play_sound(sound_ctx, def0);
	float music_cutoff = cd_get_highpass_cutoff(music_sound);
	float music_noise = cd_get_noise_amplitude_db(music_sound);

	for (;;)
	{
		input_update();

		// if user ever presses escape, break out
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			printf("QUITTING INTEGRATION_TEST\n");
			break;
		}

		// increase cuttoff freq of music
		if (input_get_key_released('W'))
		{
			music_cutoff += 50.f;
			printf("New music cutoff frequency: %f\n", music_cutoff);
			cd_set_highpass_cutoff(music_sound, music_cutoff);
		}

		// decrease cutoff freq of music
		if (input_get_key_released('S'))
		{
			music_cutoff -= 50.f;
			printf("New music cutoff frequency: %f\n", music_cutoff);
			cd_set_highpass_cutoff(music_sound, music_cutoff);
		}

		// increase noise amplitude of music
		if (input_get_key_released('E'))
		{
			music_noise += 1.f;
			printf("New music noise amplitude DB: %f\n", music_noise);
			cd_set_noise_amplitude_db(music_sound, music_noise);
		}

		// decrease noise amplitude of music
		if (input_get_key_released('Q'))
		{
			music_noise -= 1.f;
			printf("New music noise amplitude DB: %f\n", music_noise);
			cd_set_noise_amplitude_db(music_sound, music_noise);
		}

		// stinger 1
		if (input_get_key_released('A'))
		{
			printf("Set Stinger1 cutoff frequency: ");
			float freq = 20000.f;
			scanf("%f", &freq);
			printf("Set Stinger1 noise amplitude DB: ");
			float db = -96.f;
			scanf("%f", &db);
			printf("Playing Stinger1\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def1);
			cd_set_highpass_cutoff(stinger_sound, freq);
			cd_set_noise_amplitude_db(stinger_sound, db);
		}

		// stinger 2
		if (input_get_key_released('D'))
		{
			printf("Set Stinger2 cutoff frequency: ");
			float freq = 20000.f;
			scanf("%f", &freq);
			printf("Set Stinger2 noise amplitude DB: ");
			float db = -96.f;
			scanf("%f", &db);
			printf("Playing Stinger2\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def2);
			cd_set_highpass_cutoff(stinger_sound, freq);
			cd_set_noise_amplitude_db(stinger_sound, db);
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

static void echo_test(void)
{
	int frequency = 44100; // a good standard frequency for playing commonly saved OGG + wav files
	int buffered_samples = 8192; // number of seconds the buffer will hold in memory. want this long enough in case of frame-delays
	int num_elements_in_playing_pool = PLAYING_POOL_SIZE; // pooled memory array size for playing sounds

	// create the sound context
	cs_context_t* sound_ctx = cs_make_context(GetConsoleWindow(), frequency, buffered_samples, num_elements_in_playing_pool, 0);

	// set mix thread running
	cs_spawn_mix_thread(sound_ctx);
	cs_thread_sleep_delay(sound_ctx, 10);

	// create the dsp contexts
	cd_context_def_t context_definition;
	context_definition.playing_pool_count = num_elements_in_playing_pool;
	context_definition.sampling_rate = (float)frequency;
	context_definition.use_highpass = 0;
	context_definition.use_lowpass = 0;
	context_definition.use_echo = 1;
	context_definition.use_noise = 1;
	context_definition.echo_max_delay_s = 0.f;
	context_definition.rand_seed = 2;
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

	printf("Echo Filter Test\n");
	printf("--------------------\n");
	printf("-To change filter coefficients on background music press\n");
	printf(" '1' to increase the echo delay and \n");
	printf(" '2' to decrease the echo delay.\n");
	printf(" '3' to increase the echo mix and \n");
	printf(" '4' to decrease the echo mix.\n");
	printf(" '5' to increase the echo feedback and \n");
	printf(" '6' to decrease the echo feedback.\n");
	printf(" 'E' to increase background noise amplitude DB and \n");
	printf(" 'Q' to decrease background noise amplitude DB.\n");
	printf("-Pressing 'A' will prompt you to pick parameters\n");
	printf(" for stinger1, which will then play with those coefficients.\n");
	printf("-Pressing 'D' will prompt you to pick parameters\n");
	printf(" for stinger2, which will then play with those coefficients.\n");
	printf("\n-To quit, press ESCAPE\n");
	printf("--------------------\n\n");

	cs_playing_sound_t* music_sound = cs_play_sound(sound_ctx, def0);
	float music_mix = cd_get_echo_mix(music_sound);
	float music_feedback = cd_get_echo_feedback(music_sound);
	float music_delay = cd_get_echo_delay(music_sound);
	float music_noise = cd_get_noise_amplitude_db(music_sound);

	for (;;)
	{
		input_update();

		// if user ever presses escape, break out
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			printf("QUITTING INTEGRATION_TEST\n");
			break;
		}

		if (input_get_key_released('1'))
		{
			music_delay += 0.02f;
			printf("New music delay: %f\n", music_delay);
			cd_set_echo_delay(music_sound, music_delay);
		}

		if (input_get_key_released('2'))
		{
			music_delay -= 0.02f;
			printf("New music delay: %f\n", music_delay);
			cd_set_echo_delay(music_sound, music_delay);
		}

		if (input_get_key_released('3'))
		{
			music_mix += 0.02f;
			printf("New music mix: %f\n", music_mix);
			cd_set_echo_mix(music_sound, music_mix);
		}

		if (input_get_key_released('4'))
		{
			music_mix -= 0.02f;
			printf("New music mix: %f\n", music_mix);
			cd_set_echo_mix(music_sound, music_mix);
		}

		if (input_get_key_released('5'))
		{
			music_feedback += 0.02f;
			printf("New music feedback: %f\n", music_feedback);
			cd_set_echo_feedback(music_sound, music_feedback);
		}

		if (input_get_key_released('6'))
		{
			music_feedback -= 0.02f;
			printf("New music feedback: %f\n", music_feedback);
			cd_set_echo_feedback(music_sound, music_feedback);
		}

		// increase noise amplitude of music
		if (input_get_key_released('E'))
		{
			music_noise += 1.f;
			printf("New music noise amplitude DB: %f\n", music_noise);
			cd_set_noise_amplitude_db(music_sound, music_noise);
		}

		// decrease noise amplitude of music
		if (input_get_key_released('Q'))
		{
			music_noise -= 1.f;
			printf("New music noise amplitude DB: %f\n", music_noise);
			cd_set_noise_amplitude_db(music_sound, music_noise);
		}

		if (input_get_key_released('A'))
		{
			float delay = 0.f;
			float mix = 0.f;
			float feedback = 0.f;
			printf("Set Stinger1 delay time: ");
			scanf("%f", &delay);
			printf("Set Stinger1 mix factor: ");
			scanf("%f", &mix);
			printf("Set Stinger1 feedback: ");
			scanf("%f", &feedback);
			printf("Set Stinger1 noise amplitude DB: ");
			float db = -96.f;
			scanf("%f", &db);
			printf("Playing Stinger1\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def1);
			cd_set_echo_delay(stinger_sound, delay);
			cd_set_echo_feedback(stinger_sound, feedback);
			cd_set_echo_mix(stinger_sound, mix);
			cd_set_noise_amplitude_db(stinger_sound, db);
		}

		if (input_get_key_released('D'))
		{
			float delay = 0.f;
			float mix = 0.f;
			float feedback = 0.f;
			printf("Set Stinger2 delay time: ");
			scanf("%f", &delay);
			printf("Set Stinger2 mix factor: ");
			scanf("%f", &mix);
			printf("Set Stinger2 feedback: ");
			scanf("%f", &feedback);
			printf("Set Stinger2 noise amplitude DB: ");
			float db = -96.f;
			scanf("%f", &db);
			printf("Playing Stinger2\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def2);
			cd_set_echo_delay(stinger_sound, delay);
			cd_set_echo_feedback(stinger_sound, feedback);
			cd_set_echo_mix(stinger_sound, mix);
			cd_set_noise_amplitude_db(stinger_sound, db);
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

static void noise_test(void)
{
	int frequency = 44100; // a good standard frequency for playing commonly saved OGG + wav files
	int buffered_samples = 8192; // number of seconds the buffer will hold in memory. want this long enough in case of frame-delays
	int num_elements_in_playing_pool = PLAYING_POOL_SIZE; // pooled memory array size for playing sounds

	// create the sound context
	cs_context_t* sound_ctx = cs_make_context(GetConsoleWindow(), frequency, buffered_samples, num_elements_in_playing_pool, 0);

	// set mix thread running
	cs_spawn_mix_thread(sound_ctx);
	cs_thread_sleep_delay(sound_ctx, 10);

	// create the dsp contexts
	cd_context_def_t context_definition;
	context_definition.playing_pool_count = num_elements_in_playing_pool;
	context_definition.sampling_rate = (float)frequency;
	context_definition.use_highpass = 0;
	context_definition.use_lowpass = 0;
	context_definition.use_echo = 0;
	context_definition.use_noise = 1;
	context_definition.echo_max_delay_s = 0.f;
	context_definition.rand_seed = 2;
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

	printf("Noise Generator Test\n");
	printf("--------------------\n");
	printf("-To change noise coefficients on background music press\n");
	printf(" '1' to increase the noise amplitude DB \n");
	printf(" '2' to decrease the noise amplitude DB \n");
	printf(" '3' to increase the noise amplitude Gain \n");
	printf(" '4' to decrease the noise amplitude Gain \n");
	printf("-Pressing 'A' will prompt you to pick parameters\n");
	printf(" for stinger1, which will then play with those coefficients.\n");
	printf("-Pressing 'D' will prompt you to pick parameters\n");
	printf(" for stinger2, which will then play with those coefficients.\n");
	printf("\n-To quit, press ESCAPE\n");
	printf("--------------------\n\n");

	cs_playing_sound_t* music_sound = cs_play_sound(sound_ctx, def0);
	float music_amp_db = cd_get_noise_amplitude_db(music_sound);
	float music_amp_gain = cd_get_noise_amplitude_gain(music_sound);

	for (;;)
	{
		input_update();

		// if user ever presses escape, break out
		if (GetAsyncKeyState(VK_ESCAPE))
		{
			printf("QUITTING INTEGRATION_TEST\n");
			break;
		}

		if (input_get_key_released('1'))
		{
			music_amp_db += 1.f;
			printf("New music amplitude db: %f\n", music_amp_db);
			cd_set_noise_amplitude_db(music_sound, music_amp_db);
		}

		if (input_get_key_released('2'))
		{
			music_amp_db -= 1.f;
			printf("New music amplitude db: %f\n", music_amp_db);
			cd_set_noise_amplitude_db(music_sound, music_amp_db);
		}

		if (input_get_key_released('3'))
		{
			music_amp_gain += 0.01f;
			printf("New music amplitude gain: %f\n", music_amp_gain);
			cd_set_noise_amplitude_gain(music_sound, music_amp_gain);
		}

		if (input_get_key_released('4'))
		{
			music_amp_gain -= 0.01f;
			printf("new music amplitude gain: %f\n", music_amp_gain);
			cd_set_noise_amplitude_gain(music_sound, music_amp_gain);
		}

		if (input_get_key_released('A'))
		{
			float db = 0.f;
			printf("Set Stinger1 amplitude DB: ");
			scanf("%f", &db);
			printf("Playing Stinger1\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def1);
			cd_set_noise_amplitude_db(stinger_sound, db);
		}

		if (input_get_key_released('D'))
		{
			float db = 0.f;
			printf("Set Stinger2 amplitude DB: ");
			scanf("%f", &db);
			printf("Playing Stinger2\n");
			cs_playing_sound_t* stinger_sound = cs_play_sound(sound_ctx, def2);
			cd_set_noise_amplitude_db(stinger_sound, db);
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

typedef void(*test_func)(void);
test_func tests[] = {
	lowpass_test,
	highpass_test,
	echo_test,
	noise_test
};

/* END INTEGRATION TESTS */

/* BEGIN MAIN */
int main(int argc, char** argv)
{
	// usage
	if (argc != 2)
	{
		printf("Invalid number of arguments!\n");
		printf("Usage: \n");
		printf("    ./cute_dsp_test <test_num>\n");
		printf("    <test_num> = 0 for lowpass test,\n");
		printf("                 1 for highpass test,\n");
		printf("                 2 for echo test\n");
		printf("                 3 for noise test\n\n");
		return 1;
	}

	int test_num = atoi(argv[1]);
	input_init();
	tests[test_num]();
	getchar();
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
