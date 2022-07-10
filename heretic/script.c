// THIS IS NOT ACS IMPLEMENTATION!
#include "engine.h"
#include "utils.h"
#include "main.h"
#include "map.h"
#include "stbar.h"
#include "mlook.h"
#include "poly.h"
#include "script.h"

#include "m_controls.h"
#include "d_event.h"

#define MAX_EXPLOSION_SECTORS	8

static uint32_t message_pos;
static const char *message_text;
static uint32_t message_tick;
static uint32_t message_time;

uint_fast8_t redirect_keys;
uint_fast8_t is_running;
static uint32_t start_sequence;
static uint16_t lightlevel = 140;
static uint32_t light_sequence;
static uint_fast8_t is_unlocked;

static uint_fast8_t is_exploded;
static uint32_t explosion_count;
static doom_sector_t *explosion_sector[MAX_EXPLOSION_SECTORS];
static fixed_t explosion_ceiling[MAX_EXPLOSION_SECTORS];
static fixed_t explosion_floor[MAX_EXPLOSION_SECTORS];

static uint_fast8_t msg_mlook;
static uint_fast8_t msg_stbar;

static uint32_t swtex[2];
static uint32_t ehtex[2];

static doom_line_t fake_line;

//
// helpers

static void set_sector_light(uint16_t tag, uint16_t level)
{
	for(uint32_t i = 0; i < *do_numsectors; i++)
	{
		doom_sector_t *sec = *do_sectors + i;
		if(sec->tag == tag)
			sec->lightlevel = level;
	}
}

//
// message

void center_message(const char *text, uint32_t time)
{
	message_tick = *do_gametick;
	message_pos = SCREENWIDTH / 2 - doom_M_StringWidth(text) / 2;
	message_text = text;
	message_time = time;
}

//
// cinema stuff

static void setup_explosion()
{
	swtex[0] = doom_R_TextureNumForName("SW1GRAY1");
	swtex[1] = doom_R_TextureNumForName("SW2GRAY1");
	ehtex[0] = doom_R_TextureNumForName("ZZZFACE2");
	ehtex[1] = doom_R_TextureNumForName("ZZZFACE1");

	if(is_exploded)
	{
		for(uint32_t i = 0; i < *do_numsectors; i++)
		{
			doom_sector_t *sec = *do_sectors + i;
			if(sec->tag == 2997)
			{
				sec->ceilingheight = 208 << 16;
			}
		}
		return;
	}

	explosion_count = 0;

	for(uint32_t i = 0; i < *do_numsectors; i++)
	{
		doom_sector_t *sec = *do_sectors + i;
		if(sec->tag == 2995)
		{
			doom_mobj_t *mo;
			mo = doom_P_SpawnMobj(sec->soundorg.x, sec->soundorg.y, sec->soundorg.z, 41);
			mo->flags = MF_SHOOTABLE | MF_NOBLOOD | MF_NOSECTOR;
			mo->health = 1;
			mo->tics = -1;
			mo->height = 0;
			mo->radius = 0;
			doom_P_SetThingPosition(mo);
		}
		if(sec->tag == 2996 && explosion_count < MAX_EXPLOSION_SECTORS)
		{
			explosion_sector[explosion_count] = sec;
			explosion_floor[explosion_count] = sec->floorheight;
			explosion_ceiling[explosion_count] = sec->ceilingheight;
			explosion_count++;
			sec->floorheight = 136 << 16;
			sec->ceilingheight = 136 << 16;
		}
	}
}

void __attribute((regparm(2),no_caller_saved_registers))
kg_Explosion0(doom_mobj_t *mo)
{
	fixed_t xx = mo->x - (80 << 16);

	for(uint32_t i = 0; i < 4; i++, xx += 53 << 16)
	{
		doom_mobj_t *mm;
		mm = doom_P_SpawnMobj(xx, mo->y, mo->z + (30 << 16) + ((i & 1) << 22), 33);
		mm->flags &= ~MF_MISSILE;
		mm->tics = 1;
		mm->state = do_states + 127;
	}

	mo->flags |= MF_NOGRAVITY;
	mo->momx = 0;
	mo->momy = 0;
	mo->momz = 0;
	mo->z += 8 << 16;

	mo->tics = 8;

	is_exploded = 1;
	quake_sequence = 15;

	doom_S_StartSound(mo, sfx_barexp);

	for(uint32_t i = 0; i < explosion_count; i++)
	{
		explosion_sector[i]->floorheight = explosion_floor[i];
		explosion_sector[i]->ceilingheight = explosion_ceiling[i];
	}
}

void __attribute((regparm(2),no_caller_saved_registers))
kg_Explosion1(doom_mobj_t *mo)
{
	fixed_t xx = mo->x - (90 << 16);

	for(uint32_t i = 0; i < 4; i++, xx += 60 << 16)
	{
		doom_mobj_t *mm;
		mm = doom_P_SpawnMobj(xx, mo->y, mo->z + (30 << 16) + (((i & 1) ^ 1) << 22), 33);
		mm->flags &= ~MF_MISSILE;
		mm->tics = 1;
		mm->state = do_states + 127;
	}

	doom_S_StartSound(mo, sfx_barexp);

	setup_explosion();
}

//
// API

void script_draw()
{
	if(message_text)
	{
		if(*do_gametick - message_tick < message_time)
			doom_M_WriteText(message_pos, SCREENHEIGHT / 2 - 10, message_text);
		else
			message_text = NULL;
	}
}

void script_tick()
{
	// quake
	if(quake_sequence)
		quake_sequence--;

	// banner light-up
	if(light_sequence)
	{
		light_sequence--;
		switch(light_sequence)
		{
			case 35 * 2:
				set_sector_light(2992, 255);
				doom_S_StartSound(NULL, sfx_swtchx);
			break;
			case 35 * 1:
				set_sector_light(2993, 255);
				doom_S_StartSound(NULL, sfx_swtchx);
			break;
			case 0:
				set_sector_light(2994, 255);
				doom_S_StartSound(NULL, sfx_swtchx);
			break;
		}
	}

	// show countdown
	if(start_sequence)
	{
		start_sequence--;
		if(start_sequence == 35 * 2)
		{
			lightlevel = 140;
			set_sector_light(2991, lightlevel);
			doom_S_StartSound(NULL, sfx_swtchx);
		}
		if(!start_sequence)
		{
			is_running = 1;
			// close main doors
			fake_line.hexen.special = 4;
			fake_line.hexen.arg[0] = 1;
			fake_line.hexen.arg[1] = 24;
			fake_line.hexen.arg[2] = 128;
			fake_line.aarg[0] = 44;
			poly_MoveXY(&fake_line, 0);
		}
	}
}

void script_spawn()
{
	redirect_keys = 0;
	start_sequence = 0;
	light_sequence = 0;
	quake_sequence = 0;

	if(*do_gameepisode > 1)
	{
		// explosion stuff
		setup_explosion();

		// check linedefs
		for(uint32_t i = 0; i < *do_numlines; i++)
		{
			doom_line_t *ln = *do_lines + i;
			doom_side_t *side = *do_sides + ln->sidenum[0];

			// mouse look
			if(ln->doom.special == 0x1E50)
				side->bottomtexture = ehtex[mlook_enable];

			// status bar
			if(ln->doom.special == 0x2050)
				side->bottomtexture = ehtex[stbar_enable];

			// lock
			if(ln->doom.special == 0x3250 && is_unlocked)
				side->midtexture = swtex[1];
		}

		// reveal unlock switch
		if(is_unlocked)
		{
			for(uint32_t i = 0; i < *do_numsectors; i++)
			{
				doom_sector_t *sec = *do_sectors + i;
				if(sec->tag == 2998)
					sec->ceilingheight = 40 << 16;
			}
		}

		// cinema status
		if(is_running)
		{
			// already started
			is_running = 1;

			// update lights
			set_sector_light(2991, lightlevel);
			set_sector_light(2992, 255);
			set_sector_light(2993, 255);
			set_sector_light(2994, 255);

			// close main doors
			fake_line.hexen.special = 4;
			fake_line.hexen.arg[0] = 1;
			fake_line.hexen.arg[1] = 24;
			fake_line.hexen.arg[2] = 128;
			fake_line.aarg[0] = 44;
			poly_MoveXY(&fake_line, 0);
		} else
			// intro lights
			light_sequence = 35 * 2 + 20;
	} else
	{
		// not a cinema map; stop the game
		if(is_running)
			is_running = 2;
	}
}

uint32_t script_execute(uint8_t idx, uint8_t map, uint8_t arg0, uint8_t arg1, uint8_t arg2)
{
	static event_t event;
	doom_side_t *side;

	switch(idx)
	{
		case 1:
			if(!is_running)
			{
				start_sequence = 35 * 5;
				center_message("The show will begin soon.", 35 * 2 + 17);
			} else
			if(is_exploded)
				center_message("You broke it!", 35 * 2);
			else
				center_message("You are back, huh?", 35 * 2);
		break;
		case 2:
			center_message("You can't go there!", 35 * 1);
			doom_S_StartSound(NULL, sfx_pdiehi);
		break;
		case 10:
			if(!is_running)
				break;
			// change window size (-)
			event.type = ev_keydown;
			event.data1 = key_menu_decscreen;
			event.data2 = 0;
			event.data3 = 0;
			D_PostEvent(&event);
			event.type = ev_keyup;
			D_PostEvent(&event);

			// start sound
			doom_S_StartSound(NULL, sfx_stnmov);
		break;
		case 11:
			if(!is_running)
				break;
			// change window size (+)
			event.type = ev_keydown;
			event.data1 = key_menu_incscreen;
			event.data2 = 0;
			event.data3 = 0;
			D_PostEvent(&event);
			event.type = ev_keyup;
			D_PostEvent(&event);

			// start sound
			doom_S_StartSound(NULL, sfx_stnmov);
		break;
		case 12:
			if(!is_running)
				break;

			// change light
			lightlevel -= 32;
			if(lightlevel < 100)
				lightlevel = 200;
			set_sector_light(2991, lightlevel);

			// start sound
			doom_S_StartSound(NULL, sfx_swtchx);
		break;
		case 30:
			side = *do_sides + map_special_line->sidenum[0];

			// toggle mouselook
			mlook_enable = !mlook_enable;

			// change texture
			side->bottomtexture = ehtex[mlook_enable];

			// start sound
			doom_S_StartSound(NULL, sfx_tink);

			// extra message
			if(!msg_mlook)
			{
				msg_mlook = 1;
				center_message("This is not a mouse-aim!", 35 * 2);
			}
		break;
		case 31:
			side = *do_sides + map_special_line->sidenum[0];

			if(do_players[*do_consoleplayer].psprites[0].state)
			{
				// disable weapon
				do_players[*do_consoleplayer].psprites[0].state = NULL;
				// change texture
				side->bottomtexture = ehtex[1];
			} else
			{
				// enable weapon
				do_players[*do_consoleplayer].psprites[0].state = do_states + do_weaponinfo[do_players[*do_consoleplayer].readyweapon].readystate;
				// change texture
				side->bottomtexture = ehtex[0];
			}

			// start sound
			doom_S_StartSound(NULL, sfx_tink);
		break;
		case 32:
			side = *do_sides + map_special_line->sidenum[0];

			// toggle status bar
			stbar_enable = !stbar_enable;

			// change texture
			side->bottomtexture = ehtex[stbar_enable];

			// extra message
			if(!msg_stbar)
			{
				msg_stbar = 1;
				center_message("Useful in other maps.", 35 * 3);
			}

			// start sound
			doom_S_StartSound(NULL, sfx_tink);
		break;
		case 40:
			if(!is_running)
				break;
			fake_line.hexen.special = 8;
			fake_line.hexen.arg[0] = 1;
			fake_line.hexen.arg[1] = 24;
			fake_line.hexen.arg[2] = 0;
			fake_line.aarg[0] = 44;
			fake_line.aarg[1] = 70;
			poly_MoveXY(&fake_line, 0);
		break;
		case 41:
			if(!is_unlocked)
			{
				center_message("It's locked!", 35 * 1);
				doom_S_StartSound(map_special_mobj, sfx_flamst);
				break;
			}
			fake_line.hexen.special = 8;
			fake_line.hexen.arg[0] = 5;
			fake_line.hexen.arg[1] = 20;
			fake_line.hexen.arg[2] = 0;
			fake_line.aarg[0] = 44;
			fake_line.aarg[1] = 70;
			poly_MoveXY(&fake_line, 0);
		break;
		case 42:
			if(!is_unlocked)
			{
				center_message("It's locked!", 35 * 1);
				doom_S_StartSound(map_special_mobj, sfx_flamst);
				// reveal unlock switch
				for(uint32_t i = 0; i < *do_numsectors; i++)
				{
					doom_sector_t *sec = *do_sectors + i;
					if(sec->tag == 2998)
						sec->ceilingheight = 40 << 16;
				}
				break;
			}
			fake_line.hexen.special = 8;
			fake_line.hexen.arg[0] = 7;
			fake_line.hexen.arg[1] = 12;
			fake_line.hexen.arg[2] = 64;
			fake_line.aarg[0] = 72;
			fake_line.aarg[1] = 128;
			poly_MoveXY(&fake_line, 0);
		break;
		case 50:
			if(is_unlocked)
				break;

			is_unlocked = 1;
			side = *do_sides + map_special_line->sidenum[0];
			side->midtexture = swtex[1];

			if(map_special_line->frontsector->ceilingheight == map_special_line->frontsector->floorheight)
			{
				center_message("Hmm ...", 35 * 1);
				map_special_line->frontsector->ceilingheight = 40 << 16;
				doom_S_StartSound(NULL, sfx_vilact);
			} else
			{
				center_message("Unlocked.", 35 * 1);
				doom_S_StartSound(NULL, sfx_swtchn);
			}
		break;
	}

	return 0;
}

