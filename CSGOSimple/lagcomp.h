#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"
#include <deque>
#include <map>
#include <mutex>
struct LagRecord_Struct
{
	Vector			m_vecOrigin;
	Vector          m_vecHeadPos;
	Vector          m_vecHitboxPos;
	matrix3x4_t		m_Matrix[256];
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
private:
	std::map<int, EntityRecord_Struct> m_RecordMap;
	void UpdateRecords(int i);
	int bestTargetIndex = -1;
};
