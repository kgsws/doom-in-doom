// kgsws' Doom ACE
#include "engine.h"
#include "utils.h"
#include "main.h"
#include "mlook.h"
#include "stbar.h"
#include "map.h"
#include "script.h"

#include "m_argv.h"
#include "d_event.h"

// variables
uint32_t do_numlumps;
doom_lumpinfo_t *do_lumpinfo;

int32_t *do_myargc;
void ***do_myargv;

uint8_t gfx_print_buffer[128];
static uint8_t *screen_text;
static uint32_t text_line;

static event_t event;

uint32_t *do_paused;
uint32_t *do_menuactive;
uint32_t *do_netgame;
uint32_t *do_deathmatch;
uint32_t *do_gametick;
uint32_t *do_leveltime;
uint32_t *do_gamemap;
uint32_t *do_gameepisode;
uint32_t *do_gameskill;
uint32_t *do_validcount;

uint32_t *do_setsizeneeded;
uint32_t *do_consoleplayer;

fixed_t *do_finesine;
fixed_t *do_finecosine;

doom_thinker_t *do_thinkercap;

void *ptr_MobjThinker;

static doom_patch_t *p_interpic;
static doom_patch_t **p_stcfn;
static doom_patch_t *p_cinema;
static doom_patch_t ***do_lnames;
static uint32_t *do_gamekeydown;

doom_player_t *do_players;

doom_state_t *do_states;
doom_mobjinfo_t *do_mobjinfo;
doom_weaponinfo_t *do_weaponinfo;

extern pixel_t *I_VideoBuffer;
static void ***do_texturecomposite;
static int16_t ***do_texturecolumnlump;
static uint16_t ***do_texturecolumnofs;

void G_ClearInput();

static char *argv[] =
{
	"doom2.exe",
#if 1
	"-playdemo",
	"playdemo",
#endif
	NULL
};

void D_DoomMain();

//
void TryRunTics();
void game_frame_step();

//
// stuff

uint32_t inet_addr(const char *cp)
{
	return 0;
}

char *inet_ntoa(void *in)
{
	return "0.0.0.0";
}

int socket(int net, int proto, int subproto)
{
	return -1;
}

int bind(int fd, void *addr, size_t addrlen)
{
	return -1;
}

int sendto(int fd, void *buffer, size_t length, uint32_t flags, void *addr, size_t addrlen)
{
	return -1;
}

int recvfrom(int fd, void *buffer, size_t length, uint32_t flags, void *addr, size_t addrlen)
{
	return -1;
}

void *__errno_location()
{
	static uint32_t temp;
	return &temp;
}

size_t __attribute((regparm(2)))
doom_fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
	// this function does not seem to be present in EXE file
	// so do this the slow way
	uint32_t tmp = size * count;
	uint32_t i;

	for(i = 0; i < tmp; i++, ptr++)
	{
		uint8_t cc = *((uint8_t*)ptr);
		if(doom_fputc(cc, stream) != cc)
			break;
	}
	return i / size;
}

//
// loading

void __attribute((regparm(2),no_caller_saved_registers))
update_gfx_text()
{
	uint8_t *src = gfx_print_buffer;
	uint8_t *dst = screen_text + text_line * 64;

	if(text_line >= 22)
		return;

	while(*dst)
		dst++;

	while(*src)
	{
		if(text_line >= 22)
			break;
		if(dst - (screen_text + text_line * 64) >= 39)
		{
			text_line++;
			break;
		}
		if(*src == '\n')
		{
			text_line++;
			dst = screen_text + text_line * 64;
		} else
			*dst++ = *src;
		src++;
	}

	doom_V_DrawPatchDirect(0, 0, 0, p_interpic);
	for(uint32_t i = 0; i < 22; i++)
	{
		uint8_t *src = screen_text + i * 64;
		uint32_t x = 1;

		while(*src)
		{
			uint8_t tmp = *src;

			if(tmp >= 'a' && tmp <= 'z')
				tmp &= 0xDF;

			if(tmp >= '!' && tmp <= '_' && p_stcfn[tmp - '!'])
			{
				doom_patch_t *pp = p_stcfn[tmp - '!'];
				int32_t xx = x;

				if(pp->width < 7)
					xx += 4 - pp->width / 2;

				doom_V_DrawPatchDirect(xx, 1 + i * 9, 0, pp);
			}

			x += 8;
			src++;
		}
	}

	doom_I_FinishUpdate();
}

void exit_I_Error()
{
	doom_I_Error("%s", gfx_print_buffer);
}

//
// hooks

static void __attribute((regparm(2),no_caller_saved_registers))
custom_Ticker()
{
	// run this game - single tick
	if(is_running & 1)
		TryRunTics();

	// scripting
	if(!*do_paused && !*do_menuactive)
		script_tick();

	// run the original game
	doom_G_Ticker();
}

static void __attribute((regparm(2),no_caller_saved_registers))
custom_Display()
{
	// draw this game
	if(is_running & 1)
		game_frame_step();

	// mouselook reset
	if(*do_setsizeneeded)
		mlook_pitch = 0x7FFFFFFF;

	// draw the original game
	doom_D_Display();

	// draw center message
	script_draw();

	// finish
	doom_I_FinishUpdate();
}

uint32_t __attribute((regparm(2),no_caller_saved_registers))
custom_Input(doom_event_t *ev)
{
	if(is_running < 2 && ev->type == ev_keydown && ev->data1 == 216)
	{
		// F12 pressed
		if(is_running)
		{
			doom_S_StartSound(NULL, sfx_swtchn);
			redirect_keys = !redirect_keys;
			if(redirect_keys)
			{
				center_message("You control the game now.", 35 * 1);
				memset(do_gamekeydown, 0, 0x400);
				ev->type = ev_mouse;
				ev->data1 = 0;
				ev->data2 = 0;
				ev->data3 = 0;
				return doom_M_Responder(ev);
			} else
			{
				center_message("You control yourself now.", 35 * 1);
				G_ClearInput();
			}
		} else
			center_message("The show hasn't started yet.", 35 * 2);

		return 1;
	}

	if(redirect_keys)
	{
		event.type = ev->type;
		event.data1 = ev->data1;
		if(event.type == ev_keydown)
			event.data2 = ev->data1; // for cheats
		else
			event.data2 = ev->data2;
		event.data3 = ev->data3;
		D_PostEvent(&event);
		return 1; // 'menu' ate the event
	}

	return doom_M_Responder(ev);
}

void __attribute((regparm(2),no_caller_saved_registers))
custom_WI_Start(doom_wbstartstruct_t *wb)
{
	if(*do_gameepisode > 1)
	{
		wb->epsd = 0;
		wb->next = 0;
		wb->last = 30;
		if(!wb->maxkills)
			wb->plyr[*do_consoleplayer].skills = 1;
	}

	doom_WI_Start(wb);

	if(*do_gameepisode > 1)
	{
		*do_gameepisode = 1;
		(*do_lnames)[30] = p_cinema;
	}
}

//
// update texture

static void place_framebuffer_l(uint32_t index, uint32_t offset)
{
	int16_t *lump = (*do_texturecolumnlump)[index];
	uint16_t *offs = (*do_texturecolumnofs)[index];

	(*do_texturecomposite)[index] = I_VideoBuffer;

	// compensate for rounding error
	lump[255] = -1;
	offs[255] = offset;

	// generate column range
	for(uint32_t i = 0; i < 160; i++)
	{
		offs[i] = offset;
		lump[i] = -1;
		offset += SCREENHEIGHT;
	}

	// compensate for rounding error
	lump[160] = -1;
	offs[160] = offset - SCREENHEIGHT;
}

static void place_framebuffer_r(uint32_t index, uint32_t offset)
{
	int16_t *lump = (*do_texturecolumnlump)[index];
	uint16_t *offs = (*do_texturecolumnofs)[index];

	(*do_texturecomposite)[index] = I_VideoBuffer;

	// compensate for rounding error
	lump[95] = -1;
	offs[95] = offset;

	// generate column range
	for(uint32_t i = 0; i < 160; i++)
	{
		offs[96 + i] = offset;
		lump[96 + i] = -1;
		offset += SCREENHEIGHT;
	}

	// compensate for rounding error
	lump[0] = -1;
	offs[0] = offset - SCREENHEIGHT;
}

//
// MAIN

void ace_main()
{
	int32_t i;

	// install hooks
	utils_init();

	// init loading screen
	screen_text = doom_Z_Malloc(22 * 64, 1, 0);
	p_interpic = doom_W_CacheLumpName(*do_gameskill > 3 ? "BOSSBACK" : "INTERPIC", 1);
	memset(screen_text, 0, 22 * 64);

	// intro
	doom_printf("       -= Doom-in-Doom by kgsws =-\n\n");

	// allocate video memory
	I_VideoBuffer = doom_malloc(SCREENWIDTH * SCREENHEIGHT);
	memset(I_VideoBuffer, 0, SCREENWIDTH * SCREENHEIGHT);

	// DOOMHAX
	myargc = (sizeof(argv) / sizeof(void*)) - 1;
	myargv = argv;
	D_DoomMain();

	// free stuff
	doom_Z_Free(p_interpic);
	doom_Z_Free(screen_text);

	// patch texture lookup - expose framebuffer
	i = doom_R_TextureNumForName("ZZZFACE8");
	place_framebuffer_l(i, 0);
	i = doom_R_TextureNumForName("ZZZFACE9");
	place_framebuffer_l(i, 72);
	i = doom_R_TextureNumForName("ZZZFACE6");
	place_framebuffer_r(i, 160 * SCREENHEIGHT);
	i = doom_R_TextureNumForName("ZZZFACE7");
	place_framebuffer_r(i, 72 + 160 * SCREENHEIGHT);

	// load logo
	doom_W_ReadLump(doom_W_GetNumForName("KGSWS"), I_VideoBuffer);

	// init substuff
	stbar_init();

	// map title
	p_cinema = doom_W_CacheLumpName("M_CINEMA", 1);

	// explosion stuff
	do_states[892].action = kg_Explosion0;
	do_states[892].nextstate = 893;
	do_states[893].action = kg_Explosion1;

	// load map CINEMA, that is episode 2
	doom_G_InitNew(*do_gameskill, 2, 1);

	// continue running original game
}

//
// hooks

static const hook_t hooks[] __attribute__((used,section(".hooks"),aligned(4))) =
{
	// invert 'run key' function (auto run)
	{0x0001FBC5, CODE_HOOK | HOOK_UINT8, 0x01},
	// disable 'wipe', skip 'I_FinishUpdate'
	{0x0001D4A4, CODE_HOOK | HOOK_SET_NOPS, 7},
	// patch 'printf' in 'I_ZoneBase'
	{0x0001AC83, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)doom_printf},
	{0x0001ACB1, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)doom_printf},
	// fix blazing door double close sound
	{0x00026912, CODE_HOOK | HOOK_SET_NOPS, 5},
	// some variables
	{0x00005A84, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_finesine},
	{0x00013584, DATA_HOOK | HOOK_READ32, (uint32_t)&do_finecosine},
	{0x0002B408, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_paused},
	{0x0002B6CC, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_menuactive},
	{0x00074FA0, DATA_HOOK | HOOK_READ32, (uint32_t)&do_numlumps},
	{0x00074FA4, DATA_HOOK | HOOK_READ32, (uint32_t)&do_lumpinfo},
	{0x0002B6F0, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_myargc},
	{0x0002B6F4, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_myargv},
	{0x000758C8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&p_stcfn},
	{0x00075C00, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_lnames},
	{0x0002B3FC, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_deathmatch},
	{0x0002B400, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_netgame},
	{0x0002B3BC, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_gametick},
	{0x0002CF80, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_leveltime},
	{0x0002B3E8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_gamemap},
	{0x0002B3F8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_gameepisode},
	{0x0002B3E0, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_gameskill},
	{0x00013580, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_validcount},
	{0x0002A880, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_gamekeydown},
	{0x00038FE8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_setsizeneeded},
	{0x0002B3DC, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_consoleplayer},
	{0x0002AE78, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_players},
	{0x0002CF74, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_thinkercap},
	// function pointers
	{0x00031490, CODE_HOOK | HOOK_IMPORT, (uint32_t)&ptr_MobjThinker},
	// info
	{0x00012D90, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_weaponinfo},
	{0x00015A28, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_states},
	{0x0001C3EC, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_mobjinfo},
	// texture updates
	{0x000300E8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_texturecomposite},
	{0x000300F4, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_texturecolumnlump},
	{0x000300D0, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_texturecolumnofs},
	// hook 'G_Ticker'
	{0x0001FA7F, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_Ticker},
	// hook 'D_Display'
	{0x0001D604, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_Display},
	// hook 'M_Responder'
	{0x0001D13A, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_Input},
	// add 'death state' to 'MT_TELEPORTMAN'
	{0x0001D2D8, DATA_HOOK | HOOK_UINT32, 892},
	// change MAP01 name
	{0x00013CBC, DATA_HOOK | HOOK_UINT32, (uint32_t)"-= Doom-in-Doom by kgsws =-"},
	// prepare episode menu
	{0x000122E4, DATA_HOOK | HOOK_UINT32, 2},
	{0x00022734, CODE_HOOK | HOOK_UINT8, 0xEB},
	// hook 'WI_Start'
	{0x0002107E, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_WI_Start},
};

