#include "C_CSPlayer.h"

bool _fastcall Animations::ShouldInterpolate(uintptr_t ecx, uintptr_t edx)
{
	return g_EngineClient->GetLocalPlayer() == reinterpret_cast<C_BasePlayer*>(ecx)->EntIndex();
}