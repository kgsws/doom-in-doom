//
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
// DESCRIPTION:
//     Common code to parse command line, identifying WAD files to load.
//

#ifndef W_MAIN_H
#define W_MAIN_H

#include "d_mode.h"

boolean W_ParseCommandLine(void);
void W_CheckCorrectIWAD(GameMission_t mission);

// Autoload all .wad files from the given directory:
void W_AutoLoadWADs(const char *path);

#endif /* #ifndef W_MAIN_H */

