#include "painttraverse.hpp"
#include "hooks.hpp"
#include "options.hpp"
#include "render.hpp"
#include "helpers/input.hpp"

namespace Hooks::Painttraverse
{
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vfuncs::vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		static uint32_t HudZoomPanel;
		if (!HudZoomPanel && g_Options.esp_removescope)
			if (!strcmp("HudZoom", g_VGuiPanel->GetName(panel)))
				HudZoomPanel = panel;

		if (HudZoomPanel == panel && g_Options.esp_removescope && g_LocalPlayer && g_LocalPlayer->m_hActiveWeapon().Get())
		{
			if (g_LocalPlayer->m_hActiveWeapon().Get()->IsSniper() && g_LocalPlayer->m_bIsScoped())
				return;
		}

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) {
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			if (g_LocalPlayer && InputSys::Get().IsKeyDown(VK_TAB) && g_Options.misc_showranks)
				Utils::RankRevealAll();

			Render::Get().BeginScene();
		}
	}
}