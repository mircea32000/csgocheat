#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"
#include <deque>
#include <map>
#include <mutex>
struct LagRecord_Struct
{
	Vector			m_vecOrigin;
	Vector          m_vecHitboxPos;
	Vector          m_vecHitboxPosNeck;
	Vector          m_vecHitboxPosPelvis;
	Vector          m_vecHitboxPosStomach;
    matrix3x4_t	m_Matrix[128];
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
private:
	void UpdateRecords(int i);
	int bestTargetIndex = -1;
};
