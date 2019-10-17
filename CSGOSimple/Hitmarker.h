#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"
struct ImpactInfo
{
	float x, y, z;
	long long time;
};
struct Hitmarkerinfo
{
	ImpactInfo impact;
	int alpha;
};
class CHitmarker : IGameEventListener2
{
public:
	void Init();
	void Stop();
	void Paint();
private:
	void FireGameEvent(IGameEvent *event) override;
	int GetEventDebugID(void) override;
	void PlayerHurt(IGameEvent* event);
	void BulletImpact(IGameEvent* event);
	C_BasePlayer* GetPlayerFromId(int uid);
	std::vector<ImpactInfo> m_vecImpacts;
	std::vector<Hitmarkerinfo> m_vecHitmarkers;
};

extern CHitmarker* g_Hitmarker;