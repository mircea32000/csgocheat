#include "CEconStickerDefition.h"
#include "valve_sdk/sdk.hpp"
std::string CEconStickerDefinition::GetName()
{
	static const auto V_UCS2ToUTF8 = reinterpret_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(GetProcAddress(GetModuleHandleW(L"vstdlib.dll"), "V_UCS2ToUTF8"));
	auto wname = *reinterpret_cast<const char**>((uintptr_t)this + 0x28);
	const auto wide_name = g_Localize->Find(wname);
	char name[256];
	V_UCS2ToUTF8(wide_name, name, sizeof(name));
	return std::string(name);
}

int  CEconStickerDefinition::GetStickerID()
{
	return *reinterpret_cast<int*>((uintptr_t)this);
}

int  CEconStickerDefinition::GetRarity()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x4);
}

int CEconStickerDefinition::GetEventID()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x68);
}

int CEconStickerDefinition::GetTeamID()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x6C);
}

int CEconStickerDefinition::GetPlayerID()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x70);
}