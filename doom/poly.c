// kgsws' Doom ACE
#include "engine.h"
#include "utils.h"
#include "main.h"
#include "map.h"
#include "poly.h"

typedef struct polyobj_s
{
	doom_seg_t **segs;
	doom_vertex_t *origin;
	struct polyobj_s *mirror;
	fixed_t position[2];
	uint32_t segcount;
	uint32_t validcount;
	uint32_t busy;
	uint32_t type;
	int32_t box[4];
	doom_degenmobj_t soundorg;
} polyobj_t;

typedef struct
{
	doom_thinker_t thinker;
	polyobj_t *pobj;
	uint32_t delay;
	fixed_t speed[2];
	fixed_t target[2];
	fixed_t revert[2];
	fixed_t thrust;
	int32_t wait;
} poly_move_thinker_t;

polyobj_t poly[MAX_POLYOBJS];

static uint32_t poly_blocked;
static polyobj_t *poly_check;
static fixed_t poly_thrust;

//
// funcs

static uint32_t seg_search(doom_seg_t *start, fixed_t x, fixed_t y, doom_seg_t **dst)
{
	uint32_t segcount = 1;

	if(dst)
	{
		*dst = start;
		dst++;
	}

	while(1)
	{
		doom_seg_t *pick = NULL;

		for(uint32_t i = 0; i < *do_numsegs; i++)
		{
			doom_seg_t *seg = *do_segs + i;

			if(start->v2->x == x && start->v2->y == y)
				return segcount;

			if(seg->v2->x == x && seg->v2->y == y)
			{
				pick = seg;
				break;
			}
		}

		if(!pick)
			break;

		segcount++;

		if(dst)
		{
			*dst = pick;
			dst++;
		}

		x = pick->v1->x;
		y = pick->v1->y;
	}

	doom_I_Error("Unclosed polyobject!");
	return 0;
}

static void remove_from_subsector(doom_seg_t *seg)
{
	// this removes ALL the lines
	// just use propper polyobject container sectors
	for(uint32_t i = 0; i < *do_numsubsectors; i++)
	{
		doom_subsector_t *ss = *do_subsectors + i;
		doom_seg_t *start = *do_segs + ss->firstline;
		doom_seg_t *end = start + ss->numlines;

		if(seg >= start && seg < end)
			ss->numlines = 0;
	}
}

static doom_mobj_t * __attribute__((noinline))
find_anchor(uint32_t idx)
{
	idx++;

	for(doom_thinker_t *th = do_thinkercap->next; th != do_thinkercap; th = th->next)
	{
		if(th->function == ptr_MobjThinker)
		{
			doom_mobj_t *mo = (doom_mobj_t*)th;

			if(	mo->spawnpoint.type == 9300 &&
				mo->spawnpoint.angle == idx
			)
				return mo;
		}
	}

	doom_I_Error("Polyobject %u has no anchor!", idx);
	return NULL;
}

static void update_position(polyobj_t *pobj)
{
	pobj->box[0] = 0x7FFFFFFF;
	pobj->box[1] = 0x7FFFFFFF;
	pobj->box[2] = -0x7FFFFFFF;
	pobj->box[3] = -0x7FFFFFFF;

	// move vertextes
	for(uint32_t i = 0; i < pobj->segcount; i++)
	{
		doom_seg_t *seg = pobj->segs[i];
		doom_vertex_t *origin = pobj->origin + i;

		seg->v1->x = origin->x + pobj->position[0];
		seg->v1->y = origin->y + pobj->position[1];
	}

	// fix linedefs, find bounding box
	for(uint32_t i = 0; i < pobj->segcount; i++)
	{
		doom_seg_t *seg = pobj->segs[i];
		doom_line_t *line = seg->linedef;

		if(seg->v1->x < seg->v2->x)
		{
			line->bbox[BOXLEFT] = seg->v1->x;
			line->bbox[BOXRIGHT] = seg->v2->x;
		} else
		{
			line->bbox[BOXLEFT] = seg->v2->x;
			line->bbox[BOXRIGHT] = seg->v1->x;
		}
		if(seg->v1->y < seg->v2->y)
		{
			line->bbox[BOXBOTTOM] = seg->v1->y;
			line->bbox[BOXTOP] = seg->v2->y;
		} else
		{
			line->bbox[BOXBOTTOM] = seg->v2->y;
			line->bbox[BOXTOP] = seg->v1->y;
		}

		if(seg->v1->x < pobj->box[0])
			pobj->box[0] = seg->v1->x;
		if(seg->v1->y < pobj->box[1])
			pobj->box[1] = seg->v1->y;

		if(seg->v1->x > pobj->box[2])
			pobj->box[2] = seg->v1->x;
		if(seg->v1->y > pobj->box[3])
			pobj->box[3] = seg->v1->y;
	}

	// update sound origin
	pobj->soundorg.x = pobj->box[0] + (pobj->box[2] - pobj->box[0]) / 2;
	pobj->soundorg.y = pobj->box[1] + (pobj->box[3] - pobj->box[1]) / 2;

	// add MAXRADIUS
	pobj->box[0] -= 32 << 16;
	pobj->box[1] -= 32 << 16;
	pobj->box[2] += 32 << 16;
	pobj->box[3] += 32 << 16;

	// convert to BLOCKMAP units
	pobj->box[0] = (pobj->box[0] - *do_bmaporgx) >> 23;
	pobj->box[1] = (pobj->box[1] - *do_bmaporgy) >> 23;
	pobj->box[2] = (pobj->box[2] - *do_bmaporgx) >> 23;
	pobj->box[3] = (pobj->box[3] - *do_bmaporgy) >> 23;
}

//
// blocking logic

static void thrust_and_damage(doom_mobj_t *mo, doom_seg_t *seg)
{
	uint32_t angle;
	fixed_t mx, my;

	if(!(mo->flags & MF_SHOOTABLE) && !mo->player)
		return;

	angle = (seg->angle - ANG90) >> ANGLETOFINESHIFT;

	mx = FixedMul(poly_thrust, do_finecosine[angle]);
	my = FixedMul(poly_thrust, do_finesine[angle]);

	mo->momx += mx;
	mo->momy += my;

	if(poly_check->type > 0)
	{
		if(poly_check->type == 2 || !doom_P_CheckPosition(mo, mo->x + mo->momx, mo->y + mo->momy))
			doom_P_DamageMobj(mo, NULL, NULL, 3);
	}
}

static uint32_t __attribute((regparm(2),no_caller_saved_registers))
check_blocking(doom_mobj_t *mo)
{
	fixed_t box[4];

	if(!(mo->flags & MF_SOLID) && !mo->player)
		return 1;

	box[BOXTOP] = mo->y + mo->radius;
	box[BOXBOTTOM] = mo->y - mo->radius;
	box[BOXLEFT] = mo->x - mo->radius;
	box[BOXRIGHT] = mo->x + mo->radius;

	for(uint32_t i = 0; i < poly_check->segcount; i++)
	{
		doom_seg_t *seg = poly_check->segs[i];
		doom_line_t *line = seg->linedef;

		if(	box[BOXRIGHT] <= line->bbox[BOXLEFT] ||
			box[BOXLEFT] >= line->bbox[BOXRIGHT] ||
			box[BOXTOP] <= line->bbox[BOXBOTTOM] ||
			box[BOXBOTTOM] >= line->bbox[BOXTOP]
		)
			continue;

		if(doom_P_BoxOnLineSide(box, line) != -1)
			continue;

		thrust_and_damage(mo, seg);
		poly_blocked = 1;
	}

	return 1;
}

//
// thinkers

void __attribute((regparm(2),no_caller_saved_registers))
poly_move_thinker(poly_move_thinker_t *move)
{
	uint32_t done = 0;
	fixed_t target[2];
	fixed_t oldpos[2];

	// sleeping
	if(move->wait > 0)
	{
		move->wait--;
		if(!move->wait)
		{
			move->wait = -1;
		}
		return;
	}

	poly_check = move->pobj;

	// movement sound
	if(!(*do_leveltime & 7))
		doom_S_StartSound((doom_mobj_t*)&poly_check->soundorg, sfx_stnmov);

	// choose destination
	if(move->wait == -1)
	{
		// closing
		target[0] = move->revert[0];
		target[1] = move->revert[1];
	} else
	{
		// opening
		target[0] = move->target[0];
		target[1] = move->target[1];
	}

	// save position
	oldpos[0] = poly_check->position[0];
	oldpos[1] = poly_check->position[1];

	// X movement
	if(target[0] > poly_check->position[0])
	{
		poly_check->position[0] += move->speed[0];
		if(poly_check->position[0] >= target[0])
		{
			poly_check->position[0] = target[0];
			done |= 1;
		}
	} else
	{
		poly_check->position[0] -= move->speed[0];
		if(poly_check->position[0] <= target[0])
		{
			poly_check->position[0] = target[0];
			done |= 1;
		}
	}

	// Y movement
	if(target[1] > poly_check->position[1])
	{
		poly_check->position[1] += move->speed[1];
		if(poly_check->position[1] >= target[1])
		{
			poly_check->position[1] = target[1];
			done |= 2;
		}
	} else
	{
		poly_check->position[1] -= move->speed[1];
		if(poly_check->position[1] <= target[1])
		{
			poly_check->position[1] = target[1];
			done |= 2;
		}
	}

	// change location
	update_position(poly_check);

	// check for blocking
	poly_blocked = 0;
	poly_thrust = move->thrust;
	for(uint32_t y = poly_check->box[1]; y <= poly_check->box[3] && !poly_blocked; y++)
		for(uint32_t x = poly_check->box[0]; x <= poly_check->box[2] && !poly_blocked; x++)
			doom_P_BlockThingsIterator(x, y, check_blocking);

	// fix blocking
	if(poly_blocked)
	{
		// revert back
		poly_check->position[0] = oldpos[0];
		poly_check->position[1] = oldpos[1];
		update_position(poly_check);

		if(!poly_check->type && move->wait == -1)
		{
			// reverse direction
			move->wait = 0;
			doom_S_StartSound((doom_mobj_t*)&poly_check->soundorg, sfx_pstop);
		}

		return;
	}

	// destination
	if(done == 3)
	{
		if(move->wait < 0)
		{
			// finished
			poly_check->busy = 0;
			doom_P_RemoveThinker(&move->thinker);
			doom_S_StartSound((doom_mobj_t*)&poly_check->soundorg, sfx_pstop);
		} else
		{
			// reverse
			if(move->delay)
			{
				move->wait = move->delay;
				doom_S_StartSound((doom_mobj_t*)&poly_check->soundorg, sfx_pstop);
			} else
				move->wait = -1;
		}
	}
}

//
// action

void poly_MoveXY(doom_line_t *ln, uint32_t is_mirror)
{
	uint32_t idx = ln->hexen.arg[0]; // polyobject ID
	poly_move_thinker_t *move;
	polyobj_t *pobj;
	fixed_t vector[2];
	uint32_t angle;
	fixed_t dist;

	if(!idx)
		return;
	idx--;

	if(idx >= MAX_POLYOBJS)
		return;

	pobj = poly + idx;

	if(is_mirror)
		pobj = pobj->mirror;

	if(pobj->busy == 1)
		return;
	pobj->busy = 1;

	move = doom_Z_Malloc(sizeof(poly_move_thinker_t), 51, NULL);
	doom_P_AddThinker(&move->thinker);
	move->thinker.function = poly_move_thinker;

	move->pobj = pobj;
	move->delay = ln->aarg[1]; // delay

	angle = ln->hexen.arg[2]; // angle
	if(is_mirror)
		angle = (angle + 128) & 255;

	switch(angle)
	{
		// handle grid aligned directions without tables
		case 0:
			vector[0] = 1 << 16;
			vector[1] = 0;
		break;
		case 64:
			vector[0] = 0;
			vector[1] = 1 << 16;
		break;
		case 128:
			vector[0] = -(1 << 16);
			vector[1] = 0;
		break;
		case 192:
			vector[0] = 0;
			vector[1] = -(1 << 16);
		break;
		default:
		{
			angle <<= 5;
			vector[0] = do_finecosine[angle];
			vector[1] = do_finesine[angle];
		}
		break;
	}

	move->speed[0] = (vector[0] * ln->hexen.arg[1]) / 8;
	move->speed[1] = (vector[1] * ln->hexen.arg[1]) / 8;
	if(move->speed[0] < 0)
		move->speed[0] = -move->speed[0];
	if(move->speed[1] < 0)
		move->speed[1] = -move->speed[1];

	dist = ln->aarg[0];
	if(ln->hexen.special == 6)
		dist *= 8;

	move->revert[0] = pobj->position[0];
	move->revert[1] = pobj->position[1];

	move->target[0] = pobj->position[0] + vector[0] * dist;
	move->target[1] = pobj->position[1] + vector[1] * dist;

	if(ln->hexen.special == 8)
		move->wait = 0;
	else
		move->wait = -2;

	// thrust calculation
	move->thrust = ln->hexen.arg[1] << 10;
	if(move->thrust < (1 << 16))
		move->thrust = 1 << 16;
	else
	if(move->thrust > (4 << 16))
		move->thrust = 4 << 16;

	// mirror
	if(!is_mirror && pobj->mirror)
		poly_MoveXY(ln, 1);
}

//
// API

void poly_spawn()
{
	// reset all polyobjects
	memset(poly, 0, sizeof(poly));

	// find all polyobjects
	for(uint32_t i = 0; i < *do_numsegs; i++)
	{
		doom_seg_t *seg = *do_segs + i;

		if(seg->linedef->hexen.special == 1) // Polyobj_StartLine
		{
			uint32_t idx = seg->linedef->hexen.arg[0];
			uint32_t sc;
			polyobj_t *pobj;

			if(!idx)
				doom_I_Error("Polyobject ZERO is banned!");
			idx--;

			if(idx >= MAX_POLYOBJS)
				doom_I_Error("Polyobject ID too big!");

			pobj = poly + idx;

			seg->linedef->doom.special = 0; // this also clears ARG[0]

			if(pobj->segs)
				doom_I_Error("Second polyobject %u found!", idx);

			// count segs
			sc = seg_search(seg, seg->v1->x, seg->v1->y, NULL);

			// get segs
			pobj->segcount = sc;
			pobj->segs = doom_Z_Malloc(sc * sizeof(doom_seg_t*), 50, NULL);
			pobj->origin = doom_Z_Malloc(sc * sizeof(doom_vertex_t), 50, NULL);

			seg_search(seg, seg->v1->x, seg->v1->y, pobj->segs);

			// mirror
			if(seg->linedef->hexen.arg[1])
				pobj->mirror = poly + (seg->linedef->hexen.arg[1] - 1);

			// find polyobject destination
			for(doom_thinker_t *th = do_thinkercap->next; th != do_thinkercap; th = th->next)
			{
				if(th->function == ptr_MobjThinker)
				{
					doom_mobj_t *mo = (doom_mobj_t*)th;

					if(	mo->spawnpoint.type >= 9301 &&
						mo->spawnpoint.type <= 9303 &&
						mo->spawnpoint.angle == idx + 1
					){
						doom_mobj_t *anchor;

						// save type
						pobj->type = mo->spawnpoint.type - 9301;

						// save position
						pobj->position[0] = mo->x;
						pobj->position[1] = mo->y;

						// find anchor
						anchor = find_anchor(idx);

						// calculate origin, fix front sector
						for(uint32_t i = 0; i < sc; i++)
						{
							doom_seg_t *s = pobj->segs[i];
							doom_vertex_t *v = pobj->origin + i;

							v->x = s->v1->x - anchor->x;
							v->y = s->v1->y - anchor->y;

							remove_from_subsector(s);
						}

						// reset position
						update_position(poly + idx);

						// add this polyobject
						ex_subsector[mo->subsector - *do_subsectors].poly = poly + idx;
						break;
					}
				}
			}
		}
	}
}

//
// hooks

void __attribute((regparm(2),no_caller_saved_registers))
poly_render(uint32_t idx)
{
	polyobj_t *pobj = ex_subsector[idx].poly;

	if(!pobj)
		return;

	for(uint32_t i = 0; i < pobj->segcount; i++)
		doom_R_AddLine(pobj->segs[i]);
}

uint32_t __attribute((regparm(2),no_caller_saved_registers))
poly_BlockLinesIterator(int32_t x, int32_t y, uint32_t (*func)(doom_line_t*))
{
	for(uint32_t i = 0; i < MAX_POLYOBJS; i++)
	{
		polyobj_t *pobj = poly + i;

		if(!pobj->segs)
			continue;

		if(pobj->validcount == *do_validcount)
			continue;

		if(x >= pobj->box[0] && x <= pobj->box[2] && y >= pobj->box[1] && y <= pobj->box[3])
		{
			pobj->validcount = *do_validcount;

			for(uint32_t i = 0; i < pobj->segcount; i++)
			{
				doom_seg_t *seg = pobj->segs[i];
				doom_line_t *line = seg->linedef;

				if(line->validcount == *do_validcount)
					continue;

				line->validcount = *do_validcount;

				if(!func(line))
					return 0;
			}
		}
	}

	return doom_P_BlockLinesIterator(x, y, func);
}

//
// hooks

static const hook_t hooks[] __attribute__((used,section(".hooks"),aligned(4))) =
{
	// hook 'R_AddSprites'
	{0x00033A9A, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_polyobj},
	// hook 'P_BlockLinesIterator'
	{0x0002B1DC, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_BlockLinesIterator},
	{0x0002CA90, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_BlockLinesIterator},
	// make 'R_Subsector' store subsector index on the stack
	{0x00033AD0, CODE_HOOK | HOOK_RELADDR_DOOM, 0x000339CF},
	{0x00033ADC, CODE_HOOK | HOOK_RELADDR_DOOM, 0x000339CF},
	{0x00033AA5, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)hook_subsector_ret},
	{0x000339CF, CODE_HOOK | HOOK_UINT8, 0x50},
};

