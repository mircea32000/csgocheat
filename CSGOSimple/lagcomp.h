#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"
#include <deque>
#include <map>
#include <mutex>
#include <array>

struct HitboxRecord_Struct
{
	Vector		m_vecMins;
	Vector		m_vecMaxs;
	int			m_iBone;
	float		m_flRadius;
};

struct LagRecord_Struct
{
	std::array <HitboxRecord_Struct, HITBOX_MAX> m_arrHitboxes;
    matrix3x4_t	    m_Matrix[128];
	Vector			m_vecOrigin;
	float           m_fSimtime;
};

struct EntityRecord_Struct
{
	std::vector<LagRecord_Struct> m_vecRecords;
	std::mutex m_Mutex;
};

class TimeWarp : public Singleton<TimeWarp>
{
public:
	void StoreRecords(CUserCmd* cmd);
	void DoBackTrack(CUserCmd* cmd);
	bool IsTimeValid(float flTime);
	void DeleteInvalidRecords();
	std::array<EntityRecord_Struct, 129> m_Records;
	void UpdateRecords(int i);

private:
	int bestTargetIndex = -1;
};
