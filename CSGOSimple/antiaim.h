#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "singleton.hpp"

class CAntiAim : public Singleton<CAntiAim>
{
public:
	void Do(CUserCmd* cmd, bool& bSendPacket);
private:
	bool LBYBreak();
};