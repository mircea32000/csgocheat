#pragma once
#include "valve_sdk/sdk.hpp"

namespace Hooks::DME
{
	void __fastcall hkDrawModelExecute(void* _this, int, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
}