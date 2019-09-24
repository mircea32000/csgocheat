#pragma once
#include "valve_sdk/sdk.hpp"

namespace Hooks::Overrideview
{
	void __fastcall hkOverrideView(void* _this, int, CViewSetup* vsView);

	namespace Components
	{
		inline float currentFOV = 0;
	}
}