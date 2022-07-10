// kgsws' Doom ACE
//
// This file contains all required engine types and function prototypes.
// This file also contains all required includes.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>

//
// basic

#define SCREENWIDTH	320
#define SCREENHEIGHT	200
#define SCREENSIZE	(SCREENWIDTH*SCREENHEIGHT)

#define NUMPOWERS	6
#define NUMCARDS	6
#define MAXPLAYERS	4
#define NUMWEAPONS	9
#define NUMAMMO	4
#define NUMPSPRITES	2

#define MF_SPECIAL	0x1
#define MF_SOLID	0x2
#define MF_SHOOTABLE	0x4
#define MF_NOSECTOR	0x8
#define MF_NOBLOCKMAP	0x10
#define MF_AMBUSH	0x20
#define MF_JUSTHIT	0x40
#define MF_JUSTATTACKED	0x80
#define MF_SPAWNCEILING	0x100
#define MF_NOGRAVITY	0x200
#define MF_DROPOFF	0x400
#define MF_PICKUP	0x800
#define MF_NOCLIP	0x1000
#define MF_SLIDE	0x2000
#define MF_FLOAT	0x4000
#define MF_TELEPORT	0x8000
#define MF_MISSILE	0x10000
#define MF_DROPPED	0x20000
#define MF_SHADOW	0x40000
#define MF_NOBLOOD	0x80000
#define MF_CORPSE	0x100000
#define MF_INFLOAT	0x200000
#define MF_COUNTKILL	0x400000
#define MF_COUNTITEM	0x800000
#define MF_SKULLFLY	0x1000000
#define MF_NOTDMATCH	0x2000000

#define ANG45	0x20000000
#define ANG90	0x40000000
#define ANG180	0x80000000
#define ANG270	0xC0000000

#define ANGLETOFINESHIFT	19

typedef int32_t fixed_t;
typedef uint32_t angle_t;

enum
{
	ML_LABEL,
	ML_THINGS,
	ML_LINEDEFS,
	ML_SIDEDEFS,
	ML_VERTEXES,
	ML_SEGS,
	ML_SSECTORS,
	ML_NODES,
	ML_SECTORS,
	ML_REJECT,
	ML_BLOCKMAP,
	ML_BEHAVIOR
};

enum
{
	ST_HORIZONTAL,
	ST_VERTICAL,
	ST_POSITIVE,
	ST_NEGATIVE
};

enum
{
	BOXTOP,
	BOXBOTTOM,
	BOXLEFT,
	BOXRIGHT
};

enum
{
	sfx_None,
	sfx_pistol,
	sfx_shotgn,
	sfx_sgcock,
	sfx_dshtgn,
	sfx_dbopn,
	sfx_dbcls,
	sfx_dbload,
	sfx_plasma,
	sfx_bfg,
	sfx_sawup,
	sfx_sawidl,
	sfx_sawful,
	sfx_sawhit,
	sfx_rlaunc,
	sfx_rxplod,
	sfx_firsht,
	sfx_firxpl,
	sfx_pstart,
	sfx_pstop,
	sfx_doropn,
	sfx_dorcls,
	sfx_stnmov,
	sfx_swtchn,
	sfx_swtchx,
	sfx_plpain,
	sfx_dmpain,
	sfx_popain,
	sfx_vipain,
	sfx_mnpain,
	sfx_pepain,
	sfx_slop,
	sfx_itemup,
	sfx_wpnup,
	sfx_oof,
	sfx_telept,
	sfx_posit1,
	sfx_posit2,
	sfx_posit3,
	sfx_bgsit1,
	sfx_bgsit2,
	sfx_sgtsit,
	sfx_cacsit,
	sfx_brssit,
	sfx_cybsit,
	sfx_spisit,
	sfx_bspsit,
	sfx_kntsit,
	sfx_vilsit,
	sfx_mansit,
	sfx_pesit,
	sfx_sklatk,
	sfx_sgtatk,
	sfx_skepch,
	sfx_vilatk,
	sfx_claw,
	sfx_skeswg,
	sfx_pldeth,
	sfx_pdiehi,
	sfx_podth1,
	sfx_podth2,
	sfx_podth3,
	sfx_bgdth1,
	sfx_bgdth2,
	sfx_sgtdth,
	sfx_cacdth,
	sfx_skldth,
	sfx_brsdth,
	sfx_cybdth,
	sfx_spidth,
	sfx_bspdth,
	sfx_vildth,
	sfx_kntdth,
	sfx_pedth,
	sfx_skedth,
	sfx_posact,
	sfx_bgact,
	sfx_dmact,
	sfx_bspact,
	sfx_bspwlk,
	sfx_vilact,
	sfx_noway,
	sfx_barexp,
	sfx_punch,
	sfx_hoof,
	sfx_metal,
	sfx_chgun,
	sfx_tink,
	sfx_bdopn,
	sfx_bdcls,
	sfx_itmbk,
	sfx_flame,
	sfx_flamst,
	sfx_getpow,
	sfx_bospit,
	sfx_boscub,
	sfx_bossit,
	sfx_bospn,
	sfx_bosdth,
	sfx_manatk,
	sfx_mandth,
	sfx_sssit,
	sfx_ssdth,
	sfx_keenpn,
	sfx_keendt,
	sfx_skeact,
	sfx_skesit,
	sfx_skeatk,
	sfx_radio,
	NUMSFX
};

//
//

typedef struct
{
	uint16_t width, height;
	int16_t x, y;
	uint32_t offs[];
} doom_patch_t;

typedef struct
{
	uint32_t type;
	uint32_t data1;
	uint32_t data2;
	uint32_t data3;
} doom_event_t;

typedef struct
{
	int16_t x;
	int16_t y;
	int16_t angle;
	int16_t type;
	union
	{
		int16_t options;
		uint16_t tid;
	};
} __attribute__((packed)) doom_mapthing_t;

typedef struct
{
	uint32_t sprite;
	uint32_t frame;
	int32_t tics;
	void *action;
	uint32_t nextstate;
	uint32_t misc1;
	uint32_t misc2;
} doom_state_t;

typedef struct
{
	int32_t doomednum;
	int32_t spawnstate;
	int32_t spawnhealth;
	int32_t seestate;
	int32_t seesound;
	int32_t reactiontime;
	int32_t attacksound;
	int32_t painstate;
	int32_t painchance;
	int32_t painsound;
	int32_t meleestate;
	int32_t missilestate;
	int32_t deathstate;
	int32_t xdeathstate;
	int32_t deathsound;
	int32_t speed;
	int32_t radius;
	int32_t height;
	int32_t mass;
	int32_t damage;
	int32_t activesound;
	int32_t flags;
	int32_t raisestate;
} doom_mobjinfo_t;

//
// thinker

typedef struct thinker_s
{
	struct thinker_s *prev;
	struct thinker_s *next;
	void *function;
} doom_thinker_t;

//
// mobj

typedef struct
{
	doom_thinker_t unused;
	fixed_t x, y, z;
} doom_degenmobj_t;

typedef struct doom_mobj_s
{
	doom_thinker_t thinker;
	fixed_t x;
	fixed_t y;
	fixed_t z;
	struct doom_mobj_s *snext;
	struct doom_mobj_s *sprev;
	angle_t angle;
	uint32_t sprite;
	uint32_t frame;
	struct doom_mobj_s *bnext;
	struct doom_mobj_s *bprev;
	struct doom_subsector_s *subsector;
	fixed_t floorz;
	fixed_t ceilingz;
	fixed_t radius;
	fixed_t height;
	fixed_t momx;
	fixed_t momy;
	fixed_t momz;
	uint32_t validcount;
	uint32_t type;
	doom_mobjinfo_t *info;
	int32_t tics;
	doom_state_t *state;
	int32_t flags;
	int32_t health;
	int32_t movedir;
	int32_t movecount;
	struct doom_mobj_s *target;
	int32_t reactiontime;
	int32_t threshold;
	struct doom_player_s *player;
	int32_t lastlook;
	doom_mapthing_t spawnpoint;
	struct doom_mobj_s *tracer;
} __attribute__((packed)) doom_mobj_t;

//
// map (loaded)

typedef struct
{
	fixed_t x, y;
} doom_vertex_t;

typedef struct
{
	fixed_t floorheight;
	fixed_t ceilingheight;
	uint16_t floorpic;
	uint16_t ceilingpic;
	uint16_t lightlevel;
	uint16_t special;
	uint16_t tag;
	int32_t sndtraversed;
	doom_mobj_t *soundtarget;
	int32_t blockbox[4];
	doom_degenmobj_t soundorg;
	uint32_t validcount;
	doom_mobj_t *thinglist;
	void *specialdata;
	uint32_t linecount;
	struct line_s **lines;
} __attribute__((packed)) doom_sector_t;

typedef struct
{
	fixed_t textureoffset;
	fixed_t rowoffset;
	uint16_t toptexture;
	uint16_t bottomtexture;
	uint16_t midtexture;
	doom_sector_t *sector;
} __attribute__((packed)) doom_side_t;

typedef struct
{
	doom_vertex_t *v1;
	doom_vertex_t *v2;
	fixed_t	dx;
	fixed_t	dy;
	uint16_t flags;
	union
	{
		struct
		{
			uint16_t special;
			uint16_t tag;
		} doom;
		struct
		{
			uint8_t special;
			uint8_t arg[3];
		} hexen;
	};
	uint16_t sidenum[2];
	fixed_t	bbox[4];
	uint32_t slopetype;
	doom_sector_t *frontsector;
	doom_sector_t *backsector;
	uint32_t validcount;
	union
	{
		void *specialdata;
		struct
		{
			uint8_t aarg[2];
			uint8_t id;
		};
	};
} __attribute__((packed)) doom_line_t;

typedef struct doom_subsector_s
{
	doom_sector_t *sector;
	int16_t numlines;
	int16_t firstline;
} doom_subsector_t;

typedef struct
{
	doom_vertex_t *v1;
	doom_vertex_t *v2;
	fixed_t	offset;
	angle_t	angle;
	doom_side_t *sidedef;
	doom_line_t *linedef;
	doom_sector_t *frontsector;
	doom_sector_t *backsector;
} doom_seg_t;

//
// player

typedef struct
{
	int8_t forwardmove;
	int8_t sidemove;
	int16_t angleturn;
	int16_t consistancy;
	uint8_t chatchar;
	uint8_t buttons;
} doom_ticcmd_t;

typedef struct
{
	doom_state_t *state;
	int tics;
	fixed_t	sx;
	fixed_t	sy;
} doom_pspdef_t;

typedef struct doom_player_s
{
	doom_mobj_t *mo;
	uint32_t playerstate;
	doom_ticcmd_t cmd;
	fixed_t viewz;
	fixed_t viewheight;
	fixed_t deltaviewheight;
	fixed_t bob;
	int32_t health;
	int32_t armorpoints;
	int32_t armortype;
	int32_t powers[NUMPOWERS];
	uint32_t cards[NUMCARDS];
	uint32_t backpack;
	int32_t frags[MAXPLAYERS];
	uint32_t readyweapon;
	uint32_t pendingweapon;
	uint32_t weaponowned[NUMWEAPONS];
	int32_t ammo[NUMAMMO];
	int32_t maxammo[NUMAMMO];
	uint32_t attackdown;
	uint32_t usedown;
	uint32_t cheats;
	uint32_t refire;
	int32_t killcount;
	int32_t itemcount;
	int32_t secretcount;
	char *message;
	int32_t damagecount;
	int32_t bonuscount;
	doom_mobj_t *attacker;
	int32_t extralight;
	int32_t fixedcolormap;
	uint32_t colormap;
	doom_pspdef_t psprites[NUMPSPRITES];
	uint32_t didsecret;
} __attribute__((packed)) doom_player_t;

typedef struct
{
	uint32_t ammo;
	uint32_t upstate;
	uint32_t downstate;
	uint32_t readystate;
	uint32_t atkstate;
	uint32_t flashstate;
} doom_weaponinfo_t;

typedef struct
{
	uint32_t in;
	int32_t skills;
	int32_t sitems;
	int32_t ssecret;
	int32_t stime; 
	int32_t frags[4];
	int32_t score;
} doom_wbplayerstruct_t;

typedef struct
{
	int32_t epsd;
	uint32_t didsecret;
	int32_t last;
	int32_t next;
	int32_t maxkills;
	int32_t maxitems;
	int32_t maxsecret;
	int32_t maxfrags;
	int32_t partime;
	int32_t pnum;
	doom_wbplayerstruct_t plyr[MAXPLAYERS];
} doom_wbstartstruct_t;

typedef struct
{
	union
	{
		char name[8];
		uint64_t wame;
		uint32_t same[2];
	};
	int32_t handle;
	uint32_t position;
	uint32_t size;
} doom_lumpinfo_t;

//
// hooks

void hook_CrossSpecialLine();
void hook_UseSpecialLine();
void hook_BlockLinesIterator();
void hook_polyobj();
void hook_subsector_ret();

//
// Doom Engine Functions
// Since Doom uses different calling conventions, most functions have to use special GCC attribute.
// Even then functions with more than two arguments need another workaround. This is done in 'asm.S'.

#define FixedMul(a,b)	((int32_t)(((int64_t)(a) * (int64_t)(b)) >> 16))

void dos_exit() __attribute((regparm(2)));
// Variadic functions require no attributes.
void doom_I_Error(uint8_t*, ...);
uint32_t doom_sprintf(uint8_t*, const uint8_t*, ...);
uint32_t doom_printf(const uint8_t*, ...);

// SDK
int32_t doom_write(int32_t,void*,uint32_t) __attribute((regparm(2)));
int32_t doom_read(int32_t,void*,uint32_t) __attribute((regparm(2)));
int32_t doom_lseek(int32_t,int32_t,uint32_t) __attribute((regparm(2)));
int32_t doom_fstat(int32_t,void*) __attribute((regparm(2)));
void *doom_malloc(uint32_t) __attribute((regparm(2)));
void *doom_calloc(uint32_t,uint32_t) __attribute((regparm(2)));
void *doom_realloc(void*,uint32_t) __attribute((regparm(2)));
void doom_free(void*) __attribute((regparm(2)));

// math
fixed_t FixedDiv (fixed_t a, fixed_t b) __attribute((regparm(2)));

// stuff
uint32_t doom_I_GetTime();
void *doom_Z_Malloc(uint32_t size, uint32_t tag, void *ptr) __attribute((regparm(2)));
void doom_Z_Free(void*) __attribute((regparm(2)));
void *doom_W_CacheLumpName(const char *name, uint32_t tag) __attribute((regparm(2)));
void *doom_W_CacheLumpNum(uint32_t num, uint32_t tag) __attribute((regparm(2)));
void doom_V_DrawPatchDirect(int32_t x, int32_t y, uint32_t zero, doom_patch_t *patch) __attribute((regparm(2)));
void doom_I_FinishUpdate();
int32_t doom_R_TextureNumForName(const char *) __attribute((regparm(2)));
void doom_W_ReadLump(uint32_t, void*) __attribute((regparm(2)));
int32_t doom_W_GetNumForName(const char *) __attribute((regparm(2)));
uint32_t doom_M_Responder(doom_event_t*) __attribute((regparm(2)));
void doom_G_Ticker();
void doom_D_Display();
void doom_S_StartSound(void *origin, uint32_t id) __attribute((regparm(2)));

uint32_t doom_M_StringWidth(const char*) __attribute((regparm(2)));
void doom_M_WriteText(int32_t, int32_t, const char *) __attribute((regparm(2)));

void doom_G_InitNew(uint32_t, uint32_t, uint32_t) __attribute((regparm(2)));
void doom_G_BuildTiccmd(doom_ticcmd_t*) __attribute((regparm(2)));
void doom_R_SetupFrame(doom_player_t*) __attribute((regparm(2)));
void doom_R_DrawPlayerSprites();

void doom_R_RenderPlayerView(doom_player_t*) __attribute((regparm(2)));

doom_mobj_t *doom_P_SpawnMobj(fixed_t, fixed_t, fixed_t, uint32_t) __attribute((regparm(2)));
void doom_P_SetThingPosition(doom_mobj_t*) __attribute((regparm(2)));

void doom_WI_Start(doom_wbstartstruct_t *wb) __attribute((regparm(2)));

void doom_P_SpawnSpecials() __attribute((regparm(2)));
void doom_P_SpawnPlayer(doom_mapthing_t*) __attribute((regparm(2)));

uint8_t doom_P_Random() __attribute((regparm(2)));

void doom_R_AddLine(doom_seg_t*) __attribute((regparm(2)));
uint32_t doom_P_BlockLinesIterator(int32_t x, int32_t y, void *func) __attribute((regparm(2)));
uint32_t doom_P_BlockThingsIterator(int32_t x, int32_t y, void *func) __attribute((regparm(2)));

void doom_P_AddThinker(doom_thinker_t*) __attribute((regparm(2)));
void doom_P_RemoveThinker(doom_thinker_t*) __attribute((regparm(2)));

int32_t doom_P_BoxOnLineSide(fixed_t*, doom_line_t*) __attribute((regparm(2)));

uint32_t doom_P_CheckPosition(doom_mobj_t*, fixed_t, fixed_t) __attribute((regparm(2)));
void doom_P_DamageMobj(doom_mobj_t*, doom_mobj_t*, doom_mobj_t*, int32_t) __attribute((regparm(2)));

uint32_t doom_P_TeleportMove(doom_mobj_t*, fixed_t, fixed_t) __attribute((regparm(2)));
void doom_G_ExitLevel() __attribute((regparm(2)));

