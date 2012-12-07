// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------

#ifndef _PRED_H_
#define _PRED_H_

#include "shared.h"

typedef enum
{
    PMF_JUMP        = 0x1,
    PMF_SUBMERGED   = 0x2,
    PMF_ONGROUND    = 0x4
} pmflags_t;

typedef struct
{
    vec3_t      origin;
    vec3_t      velocity;
    float       angles[3];
    float       radius;
    float       height;
    float       centerheight;
    float       viewheight;
    pmflags_t   flags;
    int         terraintype;
    int         plane;
} pmove_t;

typedef struct
{
    pmove_t     pmove;
    ticcmd_t    cmd;
} pred_t;

void Pred_ClientMovement(void);
void Pred_ServerMovement(void);

#endif