#include "CEconItemDefinition.h"
#include "valve_sdk/csgostructs.hpp"
#include "valve_sdk/sdk.hpp"
std::vector<uint16_t> CEconItemDefinition::GetAssociatedItems()
{
	std::vector<uint16_t> items;

	auto size = *reinterpret_cast<int*>((uintptr_t)this + 0x18);
	auto data = *reinterpret_cast<uintptr_t*>((uintptr_t)this + 0xC);

	if (data)
	{
		for (int i = 0; i < size; i++)
			items.push_back(*reinterpret_cast<uint16_t*>(data + (i * sizeof(uint16_t))));
	}

	return items;
}

int CEconItemDefinition::GetNumSupportedStickerSlots()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x108);
}

int CEconItemDefinition::GetRarityValue()
{
	return *reinterpret_cast<char*>((uintptr_t)this + 0x2A);
}

int CEconItemDefinition::GetEquippedPosition()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x24C);
}

std::string CEconItemDefinition::GetWeaponLocalizeName()
{
	static const auto V_UCS2ToUTF8 = reinterpret_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(GetProcAddress(GetModuleHandleW(L"vstdlib.dll"), "V_UCS2ToUTF8")); /* nSkinz namazso*/
	auto wname = *reinterpret_cast<const char**>((uintptr_t)this + 0x4C);
	const auto wide_name = g_Localize->Find(wname);
	char name[256];
	V_UCS2ToUTF8(wide_name, name, sizeof(name));
	return std::string(name);
}

char* CEconItemDefinition::GetWeaponName()
{
	return *reinterpret_cast<char**>(uintptr_t(this) + 0x1B4);
}

char* CEconItemDefinition::GetPName()
{
	return *reinterpret_cast<char**>(uintptr_t(this) + 0x1C0);
}

KeyValues* CEconItemDefinition::GetRawDefinition()
{
	typedef KeyValues* (__thiscall * oOriginal)(PVOID);
	return CallVFunction< oOriginal >(this, 10)(this);
}

const char* CEconItemDefinition::GetItemType()
{
	return *reinterpret_cast<const char**>((uintptr_t)this + 0x54);
}

const char* CEconItemDefinition::GetModelName()
{
	return *reinterpret_cast<const char**>((uintptr_t)this + 0x94);
}

const char* CEconItemDefinition::GetWorldModelName()
{
	return *reinterpret_cast<const char**>((uintptr_t)this + 0x9C);
}

char* CEconItemDefinition::GetWeaponVModelName()
{
	return *reinterpret_cast<char**>(uintptr_t(this) + 0x94);
}

int CEconItemDefinition::GetTeamIdentifier()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x26C); // 0xC -> both, 0x8 -> CT, 0x4 -> T
}


const char* CEconItemDefinition::GetInventoryImage()
{
	return (const char*)(*(int(__thiscall * *)(int))(*(DWORD*)(DWORD)this + 0x14))((DWORD)this);
}

int CEconItemDefinition::GetDefinitionIndex()
{
	return *reinterpret_cast<int*>((uintptr_t)this + 0x8);
}
CEconItemDefinition* C_EconItemView::GetGetStaticData()
{
	static auto fnGetStaticData
		= reinterpret_cast<CEconItemDefinition * (__thiscall*)(void*)>(
			Utils::PatternScan("client_panorama.dll", "55 8B EC 51 53 8B D9 8B 0D ? ? ? ? 56 57 8B B9")
			);
	return fnGetStaticData(this);
}