#include "parser.h"
#include "helpers/utils.hpp"

#include <map>

int GetRarity(CEconItemDefinition* item, CEconPaintKitDefinition* paintkit)
{
	auto v13 = paintkit->GetRarityValue();
	auto v23 = (v13 == 7) + 6;
	auto v14 = item->GetRarityValue() + v13 - 1;
	if (v14 >= 0)
	{
		if (v14 > v23)
			v14 = v23;
	}
	else
		v14 = 0;
	return v14;
}

void CParser::ParseSkins()
{
	auto itemschema = (DWORD*)Utils::GetItemSchema();

	auto GetWeaponIdxByName = [&](const std::string& szWpn) -> short
	{

		for (const auto& e : m_mItems)
		{
			if (e.second.m_szInternalName == szWpn)
			{
				return e.first;
			}
		}

		return -1;
	};

	for (size_t i = 0; i < itemschema[58]; i++)
	{
		auto pItem = *reinterpret_cast<CEconItemDefinition * *>(itemschema[53] + 4 * (3 * i) + 4);

		Item_t Item;

		Item.m_iDefIndex = pItem->GetDefinitionIndex();
		Item.m_szDisplayName = pItem->GetWeaponLocalizeName();
		Item.m_szInternalName = pItem->GetPName();
		Item.m_iEquipPos = pItem->GetEquippedPosition();
		Item.m_iMaxStickerSlots = pItem->GetNumSupportedStickerSlots();
		Item.m_iTeamIdentifier = pItem->GetTeamIdentifier();
		Item.m_szVModelName = "";
		if (char* p = pItem->GetWeaponVModelName())
			Item.m_szVModelName = p;
		Item.m_pItemDef = pItem;
		Item.m_mAttributes = pItem->GetAttributes();
		Item.m_szTypeName = pItem->GetItemTypeName();

		m_mItems[Item.m_iDefIndex] = std::move(Item);

	}


	for (size_t i = 0; i < itemschema[170]; i++)
	{
		auto kit = *reinterpret_cast<CEconPaintKitDefinition * *>(itemschema[164] + 24 * i + 20);
		auto kitname = kit->GetSkinName();

		for (int i = 0; i < itemschema[143]; i++)
		{
			auto result = 88 * i + itemschema[137] + 24;
			auto name = std::string(*reinterpret_cast<const char**>(result));

			if (name[name.length() - 7] != '_')
				continue;

			name = name.substr(23, name.length() - 30);

			auto pos = name.find(kitname);
			if (pos == std::string::npos)
				continue;

			auto weapon_name = name.substr(0, pos - 1);
			if (name.length() - pos != strlen(kitname))
				continue;

			short iWpn = GetWeaponIdxByName(weapon_name);

			if (iWpn == -1)
				continue;

			int iKit = kit->GetPaintKit();

			m_mItems[iWpn].m_vSkins.push_back(iKit);

			auto& KitElem = m_mPaintKits[iKit];
			KitElem.m_SkinRarity = ItemRarity(GetRarity(m_mItems[iWpn].m_pItemDef, kit));
			KitElem.m_szDisplayName = kit->GetSkinLocalizeName();
		}
	}

}

void CParser::ParseStickers()
{
	auto itemschema = (DWORD*)Utils::GetItemSchema();

	for (size_t i = 1; i < itemschema[179]; i++)
	{
		auto pSticker = *(CEconStickerDefinition * *)(itemschema[173] + 24 * i + 0x14);

		std::string szName = std::string(*reinterpret_cast<const char**>((uintptr_t)pSticker + 0x28));
		if (!szName.find("#SprayKit"))
			continue;

		Sticker_t Sticker;

		Sticker.m_szDisplayName = pSticker->GetName();

		m_mStickers[pSticker->GetStickerID()] = std::move(Sticker);
	}
}