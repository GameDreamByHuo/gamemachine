﻿#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include <set>
#include "foundation/utilities/utilities.h"
#include "gmphysics/physicsworld.h"
#include "gmdatacore/shader.h"

BEGIN_NS

class Character;
GM_PRIVATE_OBJECT(GameWorld)
{
	GameWorldPrivate();
	GamePackage* gamePackage;
	std::set<GameObject*> shapes;
	Character* character;
	LightInfo ambientLight;
	bool start;
};

END_NS
#endif