#include "lockcursor.hpp"
#include "hooks.hpp"
#include "menu.hpp"

namespace Hooks::Lockcursor
{
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vfuncs::vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
}