// kgsws' Doom ACE
#include "engine.h"
#include "utils.h"
#include "mlook.h"
#include "m_fixed.h"

#define MLOOK_TOP	(180 << FRACBITS)
#define MLOOK_BOT	(-180 << FRACBITS)


static int32_t *do_mousey;

static uint32_t *do_viewheight;
static uint32_t *do_viewwidth;
static uint32_t *do_detailshift;

static fixed_t *do_centery;
static fixed_t *do_centeryfrac;
static fixed_t *do_yslope;

static fixed_t cy_weapon;
static fixed_t cy_look;

static fixed_t mlookpos;

uint32_t mlook_enable;
fixed_t mlook_pitch = 0x7FFFFFFF;

//
// hooks

static __attribute((regparm(2),no_caller_saved_registers))
void custom_BuildTiccmd(doom_ticcmd_t *cmd)
{
	if(mlook_enable)
	{
		// use current mouse Y
		mlookpos += *do_mousey << (FRACBITS - 3);
		if(mlookpos > MLOOK_TOP)
			mlookpos = MLOOK_TOP;
		if(mlookpos < MLOOK_BOT)
			mlookpos = MLOOK_BOT;
	} else
		mlookpos = 0;

	// disable mouse Y for original function
	*do_mousey = 0;

	doom_G_BuildTiccmd(cmd);
}

static __attribute((regparm(2),no_caller_saved_registers))
void custom_SetupFrame(doom_player_t *pl)
{
	fixed_t pn = mlookpos >> FRACBITS;

	if(mlook_pitch != pn)
	{
		fixed_t cy = *do_viewheight / 2;
		mlook_pitch = pn;

		if(pn > 0)
			// allow weapon offset when looking up
			// but not full range
			cy_weapon = cy + (pn >> 4);
		else
			// and not at all when looking down
			cy_weapon = cy;

		cy += pn;
		cy_look = cy;
		*do_centery = cy;
		*do_centeryfrac = cy << FRACBITS;

		// tables for planes
		for(int i = 0; i < *do_viewheight; i++)
		{
			int dy = ((i - cy) << FRACBITS) + FRACUNIT/2;
			dy = abs(dy);
			do_yslope[i] = FixedDiv((*do_viewwidth << *do_detailshift) / 2*FRACUNIT, dy);
		}
	}

	// and now run the original
	doom_R_SetupFrame(pl);
}

static __attribute((regparm(2),no_caller_saved_registers))
void custom_DrawPlayerSprites()
{
	*do_centery = cy_weapon;
	*do_centeryfrac = cy_weapon << FRACBITS;

	doom_R_DrawPlayerSprites();

	*do_centery = cy_look;
	*do_centeryfrac = cy_look << FRACBITS;
}

//
// hooks
static const hook_t hooks[] __attribute__((used,section(".hooks"),aligned(4))) =
{
	// custom R_SetupFrame
	{0x00035FB1, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_SetupFrame},
	// custom R_DrawPlayerSprites
	{0x0003864D, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_DrawPlayerSprites},
	// custom G_BuildTiccmd
	{0x0001d5b3, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_BuildTiccmd},
	{0x0001F221, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_BuildTiccmd},
	// required variables
	{0x0003952C, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_centeryfrac},
	{0x00039538, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_centery},
	{0x0004F91C, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_yslope},
	{0x0002B33C, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_mousey},
	{0x00032304, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_viewheight},
	{0x0003230C, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_viewwidth},
	{0x00038FF8, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_detailshift},
};

