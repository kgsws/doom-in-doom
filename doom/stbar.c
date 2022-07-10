// kgsws' Doom ACE
#include "engine.h"
#include "utils.h"
#include "main.h"
#include "stbar.h"

uint32_t stbar_enable;
uint32_t quake_sequence;

static uint32_t *do_screenblocks;

static doom_patch_t **do_tallnum;
static doom_patch_t **do_tallpercent;
static doom_patch_t **do_keygfx;

static uint32_t tallnum_height;
static uint32_t tallnum_width;

static uint32_t stbar_y;
static uint32_t stbar_hp_x;
static uint32_t stbar_ar_x;

//
// draw

static void stbar_big_number_r(int x, int y, int value, int digits)
{
	if(digits < 0)
		// negative digit count means also show zero
		digits = -digits;
	else
		if(!value)
			return;

	if(value < 0) // TODO: minus
		value = -value;

	while(digits--)
	{
		x -= tallnum_width;
		doom_V_DrawPatchDirect(x, y, 0, do_tallnum[value % 10]);
		value /= 10;
		if(!value)
			return;
	}
}

//
// init

void stbar_init()
{
	// initialize variables

	tallnum_height = do_tallnum[0]->height;
	tallnum_width = do_tallnum[0]->width;

	stbar_y = STBAR_Y - tallnum_height;
	stbar_hp_x = 4 + tallnum_width * 3;
	stbar_ar_x = stbar_hp_x * 2 + tallnum_width + 4;
}

//
// hooks

static __attribute((regparm(2),no_caller_saved_registers))
void custom_RenderPlayerView(doom_player_t *pl)
{
	uint32_t tmp;
	fixed_t z;
	uint32_t a;

	// quake hook
	if(quake_sequence)
	{
		z = pl->viewz;
		a = pl->mo->angle;

		pl->viewz -= 1 << 16;
		pl->viewz += (quake_sequence & 1) * (2 << 16);

		pl->mo->angle -= 0x00400000;
		pl->mo->angle += (quake_sequence & 2) * 0x00400000;
	}

	// actually render 3D view
	doom_R_RenderPlayerView(pl);

	// quake hook
	if(quake_sequence)
	{
		pl->viewz = z;
		pl->mo->angle = a;
	}

	// status bar on top
	if(!stbar_enable)
		return;
	if(*do_screenblocks < 11)
		return;
	if(pl->playerstate)
		return;

	// health
	stbar_big_number_r(stbar_hp_x, stbar_y, pl->health, 3);
	doom_V_DrawPatchDirect(stbar_hp_x, stbar_y, 0, *do_tallpercent);

	// armor
	if(pl->armorpoints)
	{
		stbar_big_number_r(stbar_ar_x, stbar_y, pl->armorpoints, 3);
		doom_V_DrawPatchDirect(stbar_ar_x, stbar_y, 0, *do_tallpercent);
	}

	// AMMO
	if(do_weaponinfo[pl->readyweapon].ammo < NUMAMMO)
		stbar_big_number_r(SCREENWIDTH - 4, stbar_y, pl->ammo[do_weaponinfo[pl->readyweapon].ammo], -4);

	// keys
	tmp = 1;
	for(uint32_t i = 0; i < 3; i++)
	{
		if(pl->cards[i] || pl->cards[i+3])
		{
			doom_V_DrawPatchDirect(SCREENWIDTH - 8, tmp, 0, do_keygfx[i]);
			tmp += 8;
		}
	}
}

//
// hooks

static const hook_t hooks[] __attribute__((used,section(".hooks"),aligned(4))) =
{
	{0x0001d362, CODE_HOOK | HOOK_RELADDR_ACE, (uint32_t)custom_RenderPlayerView},
	// some variables
	{0x0002B698, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_screenblocks},
	{0x000752f0, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_tallnum},
	{0x00075458, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_tallpercent},
	{0x00075458, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_tallpercent},
	{0x00075208, DATA_HOOK | HOOK_IMPORT, (uint32_t)&do_keygfx},
};

