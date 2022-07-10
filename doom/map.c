// kgsws' Doom ACE
#include "engine.h"
#include "utils.h"
#include "main.h"
#include "map.h"
#include "poly.h"
#include "script.h"

#define MTF_EASY	0x0001
#define MTF_MEDIUM	0x0002
#define MTF_HARD	0x0004
#define MTF_AMBUSH	0x0008
#define MTF_INACTIVE	0x0010
#define MTF_CLASS0	0x0020
#define MTF_CLASS1	0x0040
#define MTF_CLASS2	0x0080
#define MTF_SINGLE	0x0100
#define MTF_COOPERATIVE	0x0200
#define MTF_DEATHMATCH	0x0400
#define MTF_SHADOW	0x0800
#define MTF_ALTSHADOW	0x1000
#define MTF_FRIENDLY	0x2000
#define MTF_STANDSTILL	0x4000

typedef struct
{
	uint16_t v1, v2;
	uint16_t flags;
	uint8_t special;
	uint8_t arg[5];
	int16_t sidenum[2];
} __attribute__((packed)) new_maplinedef_t;

typedef struct
{
	uint16_t tid;
	uint16_t x, y, z;
	uint16_t angle;
	int16_t type;
	uint16_t flags;
	uint8_t special;
	uint8_t arg[5];
} __attribute__((packed)) new_mapthing_t;

//

static hook_t map_load_hooks_new[];
static hook_t map_load_hooks_old[];

doom_line_t *map_special_line;
doom_mobj_t *map_special_mobj;
uint_fast8_t map_special_side;

doom_vertex_t **do_vertexes;
doom_line_t **do_lines;
uint32_t *do_numlines;
doom_side_t **do_sides;
doom_sector_t **do_sectors;
uint32_t *do_numsectors;
doom_seg_t **do_segs;
uint32_t *do_numsegs;
doom_subsector_t **do_subsectors;
extra_subsector_t *ex_subsector;
uint32_t *do_numsubsectors;

fixed_t *do_bmaporgx;
fixed_t *do_bmaporgy;

uint32_t *do_totalitems;
uint32_t *do_totalkills;
uint32_t *do_totalsecret;

static const uint8_t skill_flag[] = {MTF_EASY, MTF_EASY, MTF_MEDIUM, MTF_HARD, MTF_HARD};

//
// stuff

doom_mobj_t *map_find_first_mobj(uint32_t tid)
{
	for(doom_thinker_t *th = do_thinkercap->next; th != do_thinkercap; th = th->next)
	{
		if(th->function == ptr_MobjThinker)
		{
			doom_mobj_t *mo = (doom_mobj_t*)th;

			if(mo->spawnpoint.tid == tid)
				return mo;
		}
	}

	return NULL;
}

//
// map loading

static __attribute((regparm(2),no_caller_saved_registers))
void map_LoadLineDefs(int lump)
{
	int nl;
	doom_line_t *ln;
	new_maplinedef_t *ml;
	void *buff;

	nl = do_lumpinfo[lump].size / sizeof(new_maplinedef_t);
	ln = doom_Z_Malloc(nl * sizeof(doom_line_t), 50, NULL);
	buff = doom_W_CacheLumpNum(lump, 1);
	ml = buff;

	*do_numlines = nl;
	*do_lines = ln;

	for(uint32_t i = 0; i < nl; i++, ln++, ml++)
	{
		doom_vertex_t *v1 = *do_vertexes + ml->v1;
		doom_vertex_t *v2 = *do_vertexes + ml->v2;

		ln->v1 = v1;
		ln->v2 = v2;
		ln->dx = v2->x - v1->x;
		ln->dy = v2->y - v1->y;
		ln->flags = ml->flags;
		ln->hexen.special = ml->special;
		ln->hexen.arg[0] = ml->arg[0];
		ln->hexen.arg[1] = ml->arg[1];
		ln->hexen.arg[2] = ml->arg[2];
		ln->aarg[0] = ml->arg[3];
		ln->aarg[1] = ml->arg[4];
		ln->sidenum[0] = ml->sidenum[0];
		ln->sidenum[1] = ml->sidenum[1];
		ln->validcount = 0;

		if(v1->x < v2->x)
		{
			ln->bbox[BOXLEFT] = v1->x;
			ln->bbox[BOXRIGHT] = v2->x;
		} else
		{
			ln->bbox[BOXLEFT] = v2->x;
			ln->bbox[BOXRIGHT] = v1->x;
		}
		if(v1->y < v2->y)
		{
			ln->bbox[BOXBOTTOM] = v1->y;
			ln->bbox[BOXTOP] = v2->y;
		} else
		{
			ln->bbox[BOXBOTTOM] = v2->y;
			ln->bbox[BOXTOP] = v1->y;
		}

		if(!ln->dx)
			ln->slopetype = ST_VERTICAL;
		else
		if(!ln->dy)
			ln->slopetype = ST_HORIZONTAL;
		else
		{
			if(FixedDiv(ln->dy, ln->dx) > 0)
				ln->slopetype = ST_POSITIVE;
			else
				ln->slopetype = ST_NEGATIVE;
		}

		if(ln->sidenum[0] != 0xFFFF)
			ln->frontsector = (*do_sides)[ln->sidenum[0]].sector;
		else
			ln->frontsector = NULL;

		if(ln->sidenum[1] != 0xFFFF)
			ln->backsector = (*do_sides)[ln->sidenum[1]].sector;
		else
			ln->backsector = NULL;
	}

	doom_Z_Free(buff);
}

static __attribute((regparm(2),no_caller_saved_registers))
void map_LoadThings(int lump)
{
	doom_mapthing_t ot;
	new_mapthing_t *mt;
	void *buff;
	uint32_t count, idx;
	uint16_t cflags = MTF_CLASS0 | MTF_CLASS1 | MTF_CLASS2;

	buff = doom_W_CacheLumpNum(lump, 1);
	count = do_lumpinfo[lump].size / sizeof(new_mapthing_t);
	mt = buff;

	for(uint32_t i = 0; i < count; i++, mt++)
	{
		ot.x = mt->x;
		ot.y = mt->y;
		ot.angle = mt->angle;
		ot.type = mt->type;
		ot.tid = mt->tid;

		if(mt->type < 1)
			continue;

		if(mt->type == 11)
			// TODO: deathmatch start
			continue;

		if(mt->type <= 4)
		{
			if(mt->type > 0)
			{
				// TODO: playerstarts
				if(!*do_deathmatch)
					doom_P_SpawnPlayer(&ot);
			}
		}

		// check for polyobjects
		if(mt->type >= 9300 && mt->type <= 9303)
		{
			doom_mobj_t *mo = doom_P_SpawnMobj(mt->x << 16, mt->y << 16, mt->z << 16, 41);
			mo->spawnpoint = ot;
			mo->tics = 1;
			continue;
		}

		// skill checks
		if(*do_gameskill < sizeof(skill_flag) && !(mt->flags & skill_flag[*do_gameskill]))
			continue;

		// flag checks
		if(*do_netgame)
		{
			if(*do_deathmatch)
			{
				if(!(mt->flags & MTF_DEATHMATCH))
					continue;
			} else
			{
				if(!(mt->flags & MTF_COOPERATIVE))
					continue;
			}
		} else
		{
			if(!(mt->flags & MTF_SINGLE))
				continue;
			// class check
			if(!(mt->flags & cflags))
				continue;
		}

		// find this thing
		for(idx = 136; idx; idx--)
			if(do_mobjinfo[idx].doomednum == mt->type)
				break;

		if(!idx)
			// maybe spawn something else instead ?
			continue;

		// check spawn flags
		if(*do_deathmatch && do_mobjinfo[idx].flags & MF_NOTDMATCH)
			continue;

		// TODO: no monsters ?

		// special position
		if(do_mobjinfo[idx].flags & MF_SPAWNCEILING)
			mt->z = 0x7FFF;

		// spawn the thing
		doom_mobj_t *mo = doom_P_SpawnMobj(mt->x << 16, mt->y << 16, mt->z << 16, idx);
		mo->spawnpoint = ot;

		// change Z
		if(!(do_mobjinfo[idx].flags & MF_SPAWNCEILING) && mo->subsector)
			mo->z += mo->subsector->sector->floorheight;

		// set angle
		mo->angle = ANG45 * (mt->angle / 45);

		// counters
		if(mo->flags & MF_COUNTKILL)
			*do_totalkills = *do_totalkills + 1;
		if(mo->flags & MF_COUNTITEM)
			*do_totalitems = *do_totalitems + 1;

		// more flags
		if(mt->flags & MTF_AMBUSH)
			mo->flags |= MF_AMBUSH;
		if(mt->flags & MTF_SHADOW)
			mo->flags |= MF_SHADOW;

		// cool new stuff
		if(mt->flags & MTF_INACTIVE)
		{
			// TODO
			if(mo->flags & MF_SHOOTABLE)
			{
				mo->flags |= MF_NOBLOOD; // this is a hack to disable blood
				mo->health = -mo->health; // this is a hack to disable damage, crusher breaks this
			}
			mo->tics = -1;
		} else
		{
			// random tick
			if(mo->tics > 0)
				mo->tics = 1 + (doom_P_Random() % mo->tics);
		}

		// hide TID in unused space
		mo->spawnpoint.options = mt->tid;
	}

	doom_Z_Free(buff);
}

//
// specials

void __attribute((regparm(2),no_caller_saved_registers))
old_SpawnSpecials()
{
	ex_subsector = doom_Z_Malloc(*do_numsubsectors * sizeof(extra_subsector_t), 50, NULL);
	memset(ex_subsector, 0, *do_numsubsectors * sizeof(extra_subsector_t));

	// scripting
	script_spawn();

	// doom original
	doom_P_SpawnSpecials();
}

void __attribute((regparm(2),no_caller_saved_registers))
map_SpawnSpecials()
{
	// allocate extra storage
	ex_subsector = doom_Z_Malloc(*do_numsubsectors * sizeof(extra_subsector_t), 50, NULL);
	memset(ex_subsector, 0, *do_numsubsectors * sizeof(extra_subsector_t));

	// process lines
	for(uint32_t i = 0; i < *do_numlines; i++)
	{
		doom_line_t *line = *do_lines + i;

		if(line->hexen.special == 121) // Line_SetIdentification
		{
			line->id = line->hexen.arg[0];
			line->doom.special = 0;
		}
	}

	// process sectors
	for(uint32_t i = 0; i < *do_numsectors; i++)
	{
		doom_sector_t *sec = *do_sectors + i;
		if(sec->special & 1024)
			*do_totalsecret = *do_totalsecret + 1;
	}

	// polyobjects
	poly_spawn();

	// scripting
	script_spawn();
}

static __attribute((regparm(2),no_caller_saved_registers))
void map_PlayerInSpecialSector(doom_player_t *pl)
{
	if(pl->mo->subsector->sector->special & 1024)
	{
		// new secret message
		pl->mo->subsector->sector->special &= ~1024;
		pl->message = "SECRET!";
		pl->secretcount++;
		doom_S_StartSound(pl->mo, sfx_radio);
	}
}

//
// line specials

static void activate_special(doom_line_t *ln, doom_mobj_t *mo, uint32_t side)
{
	map_special_line = ln;
	map_special_mobj = mo;
	map_special_side = side;

	switch(ln->hexen.special)
	{
		case 4: // Polyobj_Move
		case 6: // Polyobj_MoveTimes8
		case 8: // Polyobj_DoorSlide
			poly_MoveXY(ln, 0);
		break;
		case 70: // Teleport
		{
			doom_mobj_t *dest, *fog;

			// this is not a correct implementation
			if(side)
				return;

			// find destination
			dest = map_find_first_mobj(ln->hexen.arg[0]);
			if(dest)
			{
				fixed_t ox, oy, oz;

				ox = mo->x;
				oy = mo->y;
				oz = mo->z;

				if(doom_P_TeleportMove(mo, dest->x, dest->y))
				{
					uint32_t angle;

					mo->momx = 0;
					mo->momy = 0;
					mo->momz = 0;

					mo->z = dest->floorz;
					mo->angle = dest->angle;
					angle = dest->angle >> ANGLETOFINESHIFT;

					if(mo->player)
					{

						mo->player->viewz = mo->z + mo->player->viewheight;
						mo->reactiontime = 18;
					}

					fog = doom_P_SpawnMobj(ox, oy, oz, 39);
					doom_S_StartSound(fog, sfx_telept);

					fog = doom_P_SpawnMobj(mo->x + 20 * do_finecosine[angle], mo->y + 20 * do_finesine[angle], mo->z, 39);
					doom_S_StartSound(fog, sfx_telept);
				}
			}
		}
		break;
		case 80: // ACS_Execute
			script_execute(ln->hexen.arg[0], ln->hexen.arg[1], ln->hexen.arg[2], ln->aarg[0], ln->aarg[1]);
		break;
		case 243: // Exit_Normal
			doom_G_ExitLevel();
		break;
#if 0
		default:
			doom_printf_xx("TODO: activate line special %u\n", ln->hexen.special);
		break;
#endif
	}

	if(!(ln->flags & 0x0200))
		// this also clears ARG[0]
		ln->doom.special = 0;
}

uint32_t __attribute((regparm(2),no_caller_saved_registers))
map_UseSpecialLine(doom_mobj_t *mo, doom_line_t *ln, uint32_t side)
{
	if(side)
		// wrong side
		return 1;

	if((ln->flags & 0x1C00) != 0x0400 && (ln->flags & 0x1C00) != 0x1800)
		// not a 'use' special
		return 1;

	if(!mo->player && !(ln->flags & 0x2000))
		// monsters can't use this
		return 0;

	activate_special(ln, mo, side);

	// TODO: switch texture / button

	if(mo->player)
		// passtrough for player
		return (ln->flags & 0x1C00) == 0x1800;

	// continue checking for monster
	return 1;
}

void __attribute((regparm(2),no_caller_saved_registers))
map_CrossSpecialLine(int lnum, int side, doom_mobj_t *mo)
{
	doom_line_t *ln = &(*do_lines)[lnum];

	if(mo->player)
	{
		if(ln->flags & 0x1C00)
			// not a 'player cross' special
			return;
	} else
	if(mo->flags & MF_COUNTKILL) // TODO: lost souls?
	{
		if((ln->flags & 0x1C00) != 0x0800)
			// not a 'monster cross' special
			return;
	} else
	if(mo->flags & MF_MISSILE)
	{
		if((ln->flags & 0x1C00) != 0x1400)
			// not a 'projectile cross' special
			return;
	} else
		// this thing can't activate stuff
		return;

	activate_special(ln, mo, side);
}

static __attribute((regparm(2),no_caller_saved_registers))
void map_ShootSpecialLine(doom_mobj_t *mo, doom_line_t *ln)
{
	if((ln->flags & 0x1C00) != 0x0C00)
		// not a 'shoot' special
		return;

	activate_special(ln, mo, 0);
}

//
// hooks

__attribute((regparm(2),no_caller_saved_registers))
int32_t get_map_lump(char *name)
{
	int32_t lump;

	if(*do_gameepisode > 1)
		name = "CINEMA";

	lump = doom_W_GetNumForName(name);

	// check for hexen format
	if(lump + ML_BEHAVIOR < do_numlumps && do_lumpinfo[lump + ML_BEHAVIOR].wame == 0x524f495641484542)
		// new map format
		utils_install_hooks(map_load_hooks_new, 0);
	else
		// old map format
		utils_install_hooks(map_load_hooks_old, 0);

	return lump;
}

//
// hooks

static hook_t map_load_hooks_new[] =
{
	// replace call to map format specific lump loading
	{0x0002e8f3, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)map_LoadLineDefs},
	{0x0002e93b, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)map_LoadThings},
	// replace call to specials initialization
	{0x0002e982, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)map_SpawnSpecials},
	// replace call to player sector function
	{0x000333e1, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)map_PlayerInSpecialSector},
	// replace call to 'use line' function
	{0x0002bcff, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_UseSpecialLine}, // by player
	{0x00027287, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_UseSpecialLine}, // by monster
	{0x0002bd03, CODE_HOOK | HOOK_UINT16, 0x9090}, // allow for 'pass trough' for player use
	// replace call to 'cross line' function
	{0x0002b341, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_CrossSpecialLine},
	// replace call to 'shoot line' function
	{0x0002b907, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)map_ShootSpecialLine},
	// change exit switch sound line special
	{0x00030368, CODE_HOOK | HOOK_UINT8, 243},
	// terminator
	{0}
};

static hook_t map_load_hooks_old[] =
{
	// restore call to map format specific lump loading
	{0x0002e8f3, CODE_HOOK | HOOK_RELADDR_DOOM, 0x0002E220},
	{0x0002e93b, CODE_HOOK | HOOK_RELADDR_DOOM, 0x0002E180},
	// restore call to specials initialization
	{0x0002e982, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)old_SpawnSpecials},
	// restore call to player sector function
	{0x000333e1, CODE_HOOK | HOOK_RELADDR_DOOM, 0x0002FB20},
	// restore call to 'use line' function
	{0x0002bcff, CODE_HOOK | HOOK_RELADDR_DOOM, 0x00030710},
	{0x00027287, CODE_HOOK | HOOK_RELADDR_DOOM, 0x00030710},
	{0x0002bd03, CODE_HOOK | HOOK_UINT16, 0xC031},
	// restore call to 'cross line' function
	{0x0002b341, CODE_HOOK | HOOK_RELADDR_DOOM, 0x0002F500},
	// restore call to 'shoot line' function
	{0x0002b907, CODE_HOOK | HOOK_RELADDR_DOOM, 0x0002FA70},
	// restore exit switch sound line special
	{0x00030368, CODE_HOOK | HOOK_UINT8, 11},
	// terminator
	{0}
};

static const hook_t hooks[] __attribute__((used,section(".hooks"),aligned(4))) =
{
	// replace call to W_GetNumForName in P_SetupLevel
	{0x0002E8C1, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)get_map_lump},
	// ignore unknown things in (Doom) map
	{0x00031A0A, CODE_HOOK | HOOK_UINT16, 0x3FEB},
	// variables
	{0x0002B3D0, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_totalitems},
	{0x0002B3D4, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_totalkills},
	{0x0002B3C8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_totalsecret},
	{0x0002C138, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_vertexes},
	{0x0002C120, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_lines},
	{0x0002C134, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_numlines},
	{0x0002C118, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_sides},
	{0x0002C148, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_sectors},
	{0x0002C14C, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_numsectors},
	{0x0002C12C, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_segs},
	{0x0002C130, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_numsegs},
	{0x0002C140, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_subsectors},
	{0x0002C144, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_numsubsectors},
	{0x0002C104, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_bmaporgx},
	{0x0002C108, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_bmaporgy},
};

