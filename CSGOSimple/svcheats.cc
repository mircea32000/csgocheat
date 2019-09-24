#include "svcheats.hpp"
#include "valve_sdk/sdk.hpp"
#include "hooks.hpp"
#include "helpers/utils.hpp"
#include <intrin.h>  
#pragma intrinsic(_ReturnAddress)  

namespace Hooks::Svcheats
{
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "85 C0 75 30 38 86");
		static auto ofunc = vfuncs::sv_cheats.get_original<bool(__thiscall*)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
}