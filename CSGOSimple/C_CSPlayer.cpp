#include "C_CSPlayer.h"
#include "hooks.hpp"
void CCSPlayer::Initialise()
{
	if (!g_EngineClient->IsInGame())
		return;

	for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++)//i mean im not saving vtable for every player
	{
		C_BasePlayer* ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!ent)
			continue;
	
		Hooks::vfuncs::player_hook.setup(ent);
		Hooks::vfuncs::player_hook.hook_index(197, Animations::DoExtraBoneProcessing);
		Hooks::vfuncs::player_hook.hook_index(178, Animations::ShouldInterpolate);
		Hooks::vfuncs::player_hook.bInitFinished = true;

		Hooks::vfuncs::iclientrenderable_hook.setup(ent->GetClientRenderable());
		//Hooks::vfuncs::iclientrenderable_hook.hook_index(13, Animations::SetupBones);
		Hooks::vfuncs::iclientrenderable_hook.bInitFinished = true;
	}

}

void CCSPlayer::Shutdown()
{
	Hooks::vfuncs::player_hook.unhook_all();
}