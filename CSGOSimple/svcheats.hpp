#pragma once
#include "svcheats.hpp"

namespace Hooks::Svcheats
{
	bool __fastcall hkSvCheatsGetBool(void* pConVar, void* edx);
}