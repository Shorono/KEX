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
//
// DESCRIPTION: Camera mechanics/behaviors
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "actor.h"
#include "camera.h"

DECLARE_CLASS(kexCamera, kexDisplayObject)

//
// kexCamera::kexCamera
//

kexCamera::kexCamera(void) {
    this->zFar              = -1;
    this->zNear             = 0.1f;
    this->fov               = 45.0f;
    this->bLetterBox        = false;
    this->bFixedFOV         = false;
    this->aspect            = 4.0f/3.0f;
    this->bClampZFarToFog   = false;
    this->sector            = NULL;
}

//
// kexCamera::~kexCamera
//

kexCamera::~kexCamera(void) {
}

//
// kexCamera::UpdateAspect
//

void kexCamera::UpdateAspect(void) {
    aspect = (float)sysMain.VideoWidth() / (float)sysMain.VideoHeight();
}

//
// kexCamera::SetupMatrices
//

void kexCamera::SetupMatrices(void) {
    UpdateAspect();

    // projection
    projMatrix.Identity();
    projMatrix.SetViewProjection(aspect,
        (float)bFixedFOV ? fov : cvarClientFOV.GetFloat(), zNear, zFar);
    
    // scaling the camera object will affect the view projection
    projMatrix.vectors[0].x *= scale.x;
    projMatrix.vectors[1].y *= scale.y;
    
    // model
    modelMatrix.Identity();

    kexQuat yaw(-(angles.yaw + offsetAngle.yaw) + M_PI, kexVec3::vecUp);
    kexQuat pitch(angles.pitch + offsetAngle.pitch, kexVec3::vecRight);
    kexQuat roll(angles.roll + offsetAngle.roll,
        kexVec3(0, kexMath::Sin(angles.pitch), kexMath::Cos(angles.pitch)));

    modelMatrix = kexMatrix((yaw * roll) * pitch);
    rotMatrix = modelMatrix;
    modelMatrix.AddTranslation(-(origin * modelMatrix));

    // frustum
    viewFrustum.MakeClipPlanes(projMatrix, modelMatrix);
    viewFrustum.TransformPoints(origin, angles.ToForwardAxis(), fov, aspect, zNear, zFar);
}

//
// kexCamera::ProjectPoint
//

kexVec3 kexCamera::ProjectPoint(kexVec3 &point, const int offsetX, const int offsetY) {
    return point.ScreenProject(projMatrix,
                               modelMatrix,
                               sysMain.VideoWidth(),
                               sysMain.VideoHeight(),
                               offsetX,
                               offsetY);
}

//
// kexCamera::LocalTick
//

void kexCamera::LocalTick(void) {
    kexDisplayObject *attachedObject;

    attachment.Transform();

    angles.Clamp180();
    rotation = angles.ToQuat();
    sector = NULL;

    attachedObject = attachment.GetAttachedObject();

    if(attachedObject) {
        if(attachedObject->InstanceOf(&kexWorldObject::info)) {
            kexWorldObject *obj = static_cast<kexWorldObject*>(attachedObject);
            sector = obj->Physics()->sector;
        }
    }
}

//
// kexCamera::Tick
//

void kexCamera::Tick(void) {
}

//
// kexCamera::Remove
//

void kexCamera::Remove(void) {
}

//
// kexCamera::InitObject
//

void kexCamera::InitObject(void) {
    kexScriptManager::RegisterRefObjectNoCount<kexCamera>("kCamera");
    kexDisplayObject::RegisterBaseProperties<kexCamera>("kCamera");

#define OBJMETHOD(str, a, b, c)                         \
        scriptManager.Engine()->RegisterObjectMethod(   \
            "kCamera",                                  \
            str,                                        \
            asMETHODPR(kexCamera, a, b, c),             \
            asCALL_THISCALL)

    OBJMETHOD("kAngle &GetOffsetAngle(void)", GetOffsetAngle, (void), kexAngle&);
    OBJMETHOD("void SetOffsetAngle(const kAngle &in)", SetOffsetAngle, (const kexAngle &an), void);
    OBJMETHOD("kVec3 ProjectPoint(kVec3 &in, const int, const int)", ProjectPoint,
              (kexVec3 &point, const int offsetX, const int offsetY), kexVec3);

#define OBJPROPERTY(str, p)                             \
    scriptManager.Engine()->RegisterObjectProperty(     \
        "kCamera",                                      \
        str,                                            \
        asOFFSET(kexCamera, p))

    OBJPROPERTY("float zFar", zFar);
    OBJPROPERTY("float zNear", zNear);
    OBJPROPERTY("float fov", fov);
    OBJPROPERTY("float aspect", aspect);
    OBJPROPERTY("bool bLetterBox", bLetterBox);
    OBJPROPERTY("bool bFixedFOV", bFixedFOV);
    OBJPROPERTY("bool bClampZFarToFog", bClampZFarToFog);

#undef OBJMETHOD
#undef OBJPROPERTY
}
