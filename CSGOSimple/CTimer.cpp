#include "CTimer.h"

bool CTimer::delay(DWORD dwMsec)
{
	if (!bEnable)
		return true;

	if (!dwTime)
		dwTime = g_GlobalVars->curtime;

	if (dwTime + dwMsec < g_GlobalVars->curtime)
	{
		//dwTime = 0;
		bEnable = false;
		return true;
	}
	else
	{
		return false;
	}
}

void CTimer::reset()
{
	dwTime = 0;
	bEnable = true;
}

void CTimer::stop()
{
	bEnable = false;
}
