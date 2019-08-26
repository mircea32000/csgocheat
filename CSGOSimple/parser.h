#pragma once
#include "singleton.hpp"
#include <vector>
#include <map>
#include "CEconItemDefinition.h"
#include "CEconPaintKitDefinition.h"
#include "CEconStickerDefition.h"


enum ItemRarity : int
{
	ITEM_RARITY_DEFAULT,
	ITEM_RARITY_COMMON,
	ITEM_RARITY_UNCOMMON,
	ITEM_RARITY_RARE,
	ITEM_RARITY_MYTHICAL,
	ITEM_RARITY_LEGENDARY,
	ITEM_RARITY_ANCIENT,
	ITEM_RARITY_IMMORTAL
};

struct Sticker_t
{
	std::string m_szDisplayName;
};

struct PaintKit_t
{
	std::string m_szDisplayName = "";
	ItemRarity m_SkinRarity;
};

struct Item_t
{
	short m_iDefIndex;
	std::string m_szDisplayName;
	std::string m_szInternalName;
	int m_iEquipPos;
	int m_iMaxStickerSlots;
	int m_iTeamIdentifier;
	std::string m_szVModelName;
	CEconItemDefinition* m_pItemDef;
	std::map<short, int> m_mAttributes;
	std::string m_szTypeName;

	bool m_bIsPurchasable = false;
	bool m_bIsAimable = false;
	std::vector<int> m_vSkins;
};

class CParser : public Singleton<CParser>
{
public:
	//void GetItemScema();
	void ParseSkins();
	void ParseStickers();

	std::map<short, Item_t> m_mItems;
	std::map<int, Sticker_t> m_mStickers;
	std::map<int, PaintKit_t> m_mPaintKits;
	std::map<std::string, std::string> m_mIconOverride;
	//CEconItemDefinition* GetWeaponIdxByName(const std::string& szWpn);
};
//extern std::map<CEconItemDefinition*, std::vector<CEconPaintKitDefinition*>> m_mItems;
