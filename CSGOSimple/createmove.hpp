#pragma once
#include "hooks.hpp"

namespace Hooks
{
	namespace Createmove
	{
		void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket);
		void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
	}
}