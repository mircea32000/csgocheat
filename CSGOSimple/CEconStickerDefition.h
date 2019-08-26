#pragma once
#include <string>

class CEconStickerDefinition
{
public:
	std::string GetName();
	int GetStickerID();
	int GetRarity();
	int GetEventID();
	int GetTeamID();
	int GetPlayerID();
};