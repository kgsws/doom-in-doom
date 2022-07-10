#include "doomtype.h"
#include "d_ticcmd.h"

#ifndef __NET_DEFS_H_
#define __NET_DEFS_H_

#define PROGRAM_PREFIX	""

#define BACKUPTICS	8
#define NET_MAXPLAYERS	4

#define net_client_connected	0
#define drone	0

typedef struct
{
    int ticdup;
    int extratics;
    int deathmatch;
    int episode;
    int nomonsters;
    int fast_monsters;
    int respawn_monsters;
    int map;
    int skill;
    int gameversion;
    int lowres_turn;
    int new_sync;
    int timelimit;
    int loadgame;
    int random;  // [Strife only]

    // These fields are only used by the server when sending a game
    // start message:

    int num_players;
    int consoleplayer;

} net_gamesettings_t;

typedef void net_connect_data_t;

#endif
