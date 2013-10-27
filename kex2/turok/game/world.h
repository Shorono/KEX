// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2013 Samuel Villarreal
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

#ifndef __WORLD_H__
#define __WORLD_H__

#include "common.h"
#include "array.h"
#include "linkedlist.h"
#include "camera.h"

typedef struct gridBound_s {
    kexBBox                             box;
    kexLinklist<kexWorldActor>          staticActors;
    kexLinklist<kexWorldActor>          linkedActors;
    kexPtrArray<struct gridBound_s*>    linkedBounds;
    bool                                bTraced;
} gridBound_t;

class kexWorld {
public:
                                        kexWorld(void);
                                        ~kexWorld(void);

    void                                Init(void);
    void                                Tick(void);
    void                                LocalTick(void);
    void                                Load(const char *mapFile);
    void                                Unload(void);
    const char                          *GetMapFileFromID(const int id);
    kexWorldActor                       *ConstructActor(const char *className);
    void                                AddActor(kexWorldActor *actor);
    kexWorldActor                       *SpawnActor(const char *className, const char *component,
                                            const kexVec3 &origin, const kexAngle &angles);
    kexWorldActor                       *SpawnActor(kexStr &className, kexStr &component,
                                            kexVec3 &origin, kexAngle &angles);
    void                                RemoveActor(kexWorldActor *actor);
    void                                SpawnLocalPlayer(void);
    void                                Trace(traceInfo_t *trace);

    bool                                IsLoaded(void) const { return bLoaded; }
    float                               DeltaTime(void) { return deltaTime; }
    kexCamera                           *Camera(void) { return &camera; };
    kexVec3                             &GetGravity(void) { return gravity; }
    void                                SetGravity(const kexVec3 &in) { gravity = in; }
    const int                           GetTicks(void) const { return ticks; }

    kexLinklist<kexWorldActor>          actors;
    kexPtrArray<gridBound_t*>           gridBounds;

    kexWorldActor                       *actorRover;
    // TEMP
    kexVec4                             worldLightOrigin;
    kexVec4                             worldLightColor;
    kexVec4                             worldLightAmbience;
    kexVec4                             worldLightModelAmbience;
    //

    static void                         InitObject(void);

private:
    void                                ParseGridBound(kexLexer *lexer);
    void                                LinkGridBounds(void);

    bool                                bLoaded;
    bool                                bReadyUnload;
    int                                 mapID;
    int                                 nextMapID;
    kexStr                              title;
    kexCamera                           camera;
    kexVec3                             gravity;
    int                                 ticks;
    float                               time;
    float                               deltaTime;
};

extern kexWorld localWorld;

#endif
