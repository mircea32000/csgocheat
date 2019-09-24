#pragma once
#include "valve_sdk/sdk.hpp"

namespace Hooks::Painttraverse
{
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce);
}