#pragma once
#include "valve_sdk/sdk.hpp"

namespace Hooks::FSN
{
	void __fastcall hkFrameStageNotify(void* _this, int, ClientFrameStage_t stage);
}