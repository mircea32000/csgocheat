#include "CEconPaintKitDefinition.h"
#include <Windows.h>
#include "valve_sdk/sdk.hpp"
int CEconPaintKitDefinition::GetPaintKit()
{
	return *reinterpret_cast<int*>((uintptr_t)this);
}

int CEconPaintKitDefinition::GetRarityValue()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x68);
}
std::string CEconPaintKitDefinition::GetSkinLocalizeName()
{
	static const auto V_UCS2ToUTF8 = reinterpret_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(GetProcAddress(GetModuleHandleW(L"vstdlib.dll"), "V_UCS2ToUTF8"));
	auto wname = *reinterpret_cast<const char**>((uintptr_t)this + 0x24);
	const auto wide_name = g_Localize->Find(wname);
	char name[256];
	V_UCS2ToUTF8(wide_name, name, sizeof(name));
	return std::string(name);
}

const char* CEconPaintKitDefinition::GetSkinName()
{
	return *reinterpret_cast<const char**>((uintptr_t)this + 0x4);
}