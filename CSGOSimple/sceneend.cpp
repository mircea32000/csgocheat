#include "sceneend.h"
#include "hooks.hpp"
#include "features/chams.hpp"
using scene_end_fn = void(__thiscall*)(void*);

namespace Hooks::SceneEnd
{
	void __stdcall SceneEnd()
	{
		static auto ofunc = vfuncs::render_view.get_original<scene_end_fn>(9);


		ofunc(g_RenderView);
	}
}