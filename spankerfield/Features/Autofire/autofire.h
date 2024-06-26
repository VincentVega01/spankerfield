#pragma once
#include "../../SDK/sdk.h"

using namespace big;

namespace big 
{
	std::string WeaponClassToString(WeaponClass weaponClass);
}

namespace plugins
{
	void autofire();
}