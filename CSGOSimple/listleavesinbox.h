#pragma once
#include "valve_sdk/sdk.hpp"

namespace Hooks::Listleavesinbox
{
	int __fastcall hkListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax);
}