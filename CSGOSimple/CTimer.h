#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"
class CTimer : public Singleton<CTimer>
{
public:
	ULONGLONG dwTime;
	bool bEnable;
	CTimer()
	{
		dwTime = 0;
		bEnable = true;
	}
	bool delay(DWORD dwMsec);
	void reset();
	void stop();
};