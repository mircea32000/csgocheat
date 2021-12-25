#pragma once
#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
class PredictionSystem : public Singleton<PredictionSystem>
{

public:
	PredictionSystem() {
		auto client = GetModuleHandleA("client_panorama.dll");
		predictionRandomSeed = *(int**)(Utils::PatternScan(client, "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 2);
		predictionPlayer = *reinterpret_cast<C_BasePlayer * *>(Utils::PatternScan(client, "89 35 ? ? ? ? F3 0F 10 46") + 2);
	}

	void Start(CUserCmd* userCMD, C_BasePlayer* player);
	void End(C_BasePlayer* player);

	inline float GetOldCurTime() { return m_flOldCurTime; }

private:

	float m_flOldCurTime;
	float m_flOldFrametime;

	CMoveData moveData;

	int* predictionRandomSeed;
	C_BasePlayer* predictionPlayer;
};