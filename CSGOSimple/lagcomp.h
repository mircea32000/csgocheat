#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"
#include <deque>

class TimeWarp : public Singleton<TimeWarp>
{
public:
	void StoreRecords(CUserCmd* cmd);
	void DoBackTrack(CUserCmd* cmd);
	bool IsTimeValid(float flTime);
};
