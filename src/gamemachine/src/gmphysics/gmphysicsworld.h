﻿#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include <gmcommon.h>
#include "gmphysicsobject.h"
BEGIN_NS

class GMGameWorld;
class GMPhysicsObject;

GM_PRIVATE_CLASS(GMPhysicsWorld);
class GM_EXPORT GMPhysicsWorld : public GMObject
{
	GM_DECLARE_PRIVATE(GMPhysicsWorld)
	GM_DECLARE_PROPERTY(bool, Enabled)
	GM_FRIEND_CLASS(GMGameWorld)

public:
	GMPhysicsWorld(GMGameWorld* world);
	virtual ~GMPhysicsWorld();

public:
	virtual void update(GMDuration dt, GMGameObject* obj) = 0;
	virtual void applyMove(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) {}
	virtual void applyJump(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) {}
};

END_NS
#endif