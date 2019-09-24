#include "ovverrideview.hpp"
#include "hooks.hpp"
#include "features/visuals.hpp"

namespace Hooks::Overrideview
{
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = vfuncs::clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView)
			Visuals::Get().ThirdPerson();

	    Overrideview::Components::currentFOV = vsView->fov; //for fov changers later

		ofunc(g_ClientMode, edx, vsView);
	}
}