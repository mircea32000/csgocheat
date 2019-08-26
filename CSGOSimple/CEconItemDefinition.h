
#include <iostream>
#include <map>
#include <vector>
class KeyValues;

class CEconItemDefinition
{
public:

	virtual const char* GetDefinitionIndex__() = 0;
	virtual const char* GetPrefabName() = 0;
	virtual const char* GetItemBaseName() = 0;
	virtual const char* GetItemTypeName() = 0;
	virtual const char* GetItemDesc() = 0;
	virtual const char* GetInventoryImage__() = 0;


	KeyValues* GetRawDefinition();
	const char* GetItemType();
	const char* GetModelName();
	const char* GetWorldModelName();
	std::string GetWeaponLocalizeName();
	const char* GetInventoryImage();
//	const char* GetWeaponName();
	int GetRarityValue();
	char* GetWeaponName();
    char* GetPName();
	int GetEquippedPosition();
	int GetNumSupportedStickerSlots();
	int GetDefinitionIndex();

	std::map<short, int> GetAttributes()
	{
		std::map<short, int> GetAttributes;

		int size = *reinterpret_cast<int*>(this + 0x3C);
		uintptr_t data = *reinterpret_cast<uintptr_t*>(this + 0x30);

		if (data)
		{
			for (int i = 0; i < size; i++)
			{
				int16_t id = *reinterpret_cast<int16_t*>(data + (i * 0xC));
				int32_t value = *reinterpret_cast<int32_t*>(data + (i * 0xC) + 0x4);

				GetAttributes[id] = value;
			}
		}

		return GetAttributes;
	}

	char* GetWeaponVModelName();

	int GetTeamIdentifier();

	std::vector<uint16_t> GetAssociatedItems();
};