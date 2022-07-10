//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

//#include "SDL_mixer.h"

#include "config.h"
#include "doomtype.h"

//#include "gusconf.h"
#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"

// Sound sample rate to use for digital output (Hz)

int snd_samplerate = 44100;

// Maximum number of bytes to dedicate to allocated sound effects.
// (Default: 64MB)

int snd_cachesize = 64 * 1024 * 1024;

// Config variable that controls the sound buffer size.
// We default to 28ms (1000 / 35fps = 1 buffer per tic).

int snd_maxslicetime_ms = 28;

// External command to invoke to play back music.

char *snd_musiccmd = "";

// Whether to vary the pitch of sound effects
// Each game will set the default differently

int snd_pitchshift = -1;

int snd_musicdevice = SNDDEVICE_SB;
int snd_sfxdevice = SNDDEVICE_SB;

// Low-level sound and music modules we are using
static sound_module_t *sound_module;
static music_module_t *music_module;

// If true, the music pack module was successfully initialized.
static boolean music_packs_active = false;

// This is either equal to music_module or &music_pack_module,
// depending on whether the current track is substituted.
static music_module_t *active_music_module;

// Sound modules

extern void I_InitTimidityConfig(void);
extern sound_module_t sound_sdl_module;
extern sound_module_t sound_pcsound_module;
extern music_module_t music_sdl_module;
extern music_module_t music_opl_module;
extern music_module_t music_pack_module;

// For OPL module:

extern opl_driver_ver_t opl_drv_ver;
extern int opl_io_port;

// For native music module:

extern char *music_pack_path;
extern char *fluidsynth_sf_path;
extern char *timidity_cfg_path;

// DOS-specific options: These are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq = 0;
static int snd_sbdma = 0;
static int snd_mport = 0;

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void I_InitSound(boolean use_sfx_prefix)
{
}

void I_ShutdownSound(void)
{
}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
	return 0;
}

void I_UpdateSound(void)
{
}

void I_UpdateSoundParams(int channel, int vol, int sep)
{
}

int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch)
{
	return 0;
}

void I_StopSound(int channel)
{
}

boolean I_SoundIsPlaying(int channel)
{
	return false;
}

void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{
}

void I_SetMusicVolume(int volume)
{
}

void I_PauseSong(void)
{
}

void I_ResumeSong(void)
{
}

void *I_RegisterSong(void *data, int len)
{
	return NULL;
}

void I_UnRegisterSong(void *handle)
{
}

void I_PlaySong(void *handle, boolean looping)
{
}

void I_StopSong(void)
{
}

boolean I_MusicIsPlaying(void)
{
	return false;
}

void I_BindSoundVariables(void)
{
    M_BindIntVariable("snd_musicdevice",         &snd_musicdevice);
    M_BindIntVariable("snd_sfxdevice",           &snd_sfxdevice);
    M_BindIntVariable("snd_sbport",              &snd_sbport);
    M_BindIntVariable("snd_sbirq",               &snd_sbirq);
    M_BindIntVariable("snd_sbdma",               &snd_sbdma);
    M_BindIntVariable("snd_mport",               &snd_mport);
    M_BindIntVariable("snd_maxslicetime_ms",     &snd_maxslicetime_ms);
    M_BindStringVariable("snd_musiccmd",         &snd_musiccmd);
//    M_BindStringVariable("snd_dmxoption",        &snd_dmxoption);
    M_BindIntVariable("snd_samplerate",          &snd_samplerate);
    M_BindIntVariable("snd_cachesize",           &snd_cachesize);
//    M_BindIntVariable("opl_io_port",             &opl_io_port);
    M_BindIntVariable("snd_pitchshift",          &snd_pitchshift);

//    M_BindStringVariable("music_pack_path",      &music_pack_path);
//    M_BindStringVariable("fluidsynth_sf_path",   &fluidsynth_sf_path);
//    M_BindStringVariable("timidity_cfg_path",    &timidity_cfg_path);
//    M_BindStringVariable("gus_patch_path",       &gus_patch_path);
//    M_BindIntVariable("gus_ram_kb",              &gus_ram_kb);

//    M_BindIntVariable("use_libsamplerate",       &use_libsamplerate);
//    M_BindFloatVariable("libsamplerate_scale",   &libsamplerate_scale);
}

