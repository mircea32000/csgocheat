#include "hooks.hpp"

#include "createmove.hpp"
#include "endscene.hpp"
#include "reset.h"
#include "framestagenotify.h"
#include "painttraverse.hpp"
#include "emitsound.hpp"
#include "lockcursor.hpp"
#include "drawmodelexecute.hpp"
#include "doposteffects.hpp"
#include "ovverrideview.hpp"
#include "svcheats.hpp"
#include "Filesystemhook.hpp"
#include "listleavesinbox.h"

#include "Listener.hpp"
#include "Hitmarker.h"

#include "features/glow.hpp"
#include "sceneend.h"
namespace Hooks {

	void Initialize()
	{
		vfuncs::hlclient_hook.setup(g_CHLClient);
		vfuncs::direct3d_hook.setup(g_D3DDevice9);
		vfuncs::vguipanel_hook.setup(g_VGuiPanel);
		vfuncs::vguisurf_hook.setup(g_VGuiSurface);
		vfuncs::sound_hook.setup(g_EngineSound);
		vfuncs::mdlrender_hook.setup(g_MdlRender);
		vfuncs::clientmode_hook.setup(g_ClientMode);
		vfuncs::filesystem_hook.setup(g_FileSystem);
		vfuncs::render_view.setup(g_RenderView);
		vfuncs::bsp_query_hook.setup(g_EngineClient->GetBSPTreeQuery());

		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		vfuncs::sv_cheats.setup(sv_cheats_con);

		vfuncs::direct3d_hook.hook_index(index::EndScene, Hooks::Endscene::hkEndScene);
		vfuncs::direct3d_hook.hook_index(index::Reset, Hooks::Reset::hkReset);
		vfuncs::hlclient_hook.hook_index(index::FrameStageNotify, Hooks::FSN::hkFrameStageNotify);
		vfuncs::hlclient_hook.hook_index(index::CreateMove, Hooks::Createmove::hkCreateMove_Proxy);
		vfuncs::vguipanel_hook.hook_index(index::PaintTraverse, Hooks::Painttraverse::hkPaintTraverse);
		vfuncs::sound_hook.hook_index(index::EmitSound1, Hooks::Emitsound::hkEmitSound1);
		vfuncs::render_view.hook_index(9, Hooks::SceneEnd::SceneEnd);
		vfuncs::vguisurf_hook.hook_index(index::LockCursor, Hooks::Lockcursor::hkLockCursor);
		vfuncs::mdlrender_hook.hook_index(index::DrawModelExecute, Hooks::DME::hkDrawModelExecute);
		vfuncs::clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, Hooks::Doposteffects::hkDoPostScreenEffects);
		vfuncs::clientmode_hook.hook_index(index::OverrideView, Hooks::Overrideview::hkOverrideView);
		vfuncs::sv_cheats.hook_index(index::SvCheatsGetBool, Hooks::Svcheats::hkSvCheatsGetBool);
		vfuncs::filesystem_hook.hook_index(index::UnkFunc, Hooks::Filesystem::UnkFileSystemFunc);
		vfuncs::bsp_query_hook.hook_index(index::ListLeavesInBox, Hooks::Listleavesinbox::hkListLeavesInBox);

	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		vfuncs::hlclient_hook.unhook_all();
		vfuncs::direct3d_hook.unhook_all();
		vfuncs::vguipanel_hook.unhook_all();
		vfuncs::viewrender_hook.unhook_all();
		vfuncs::clientmode_hook.unhook_all();
		vfuncs::filesystem_hook.unhook_all();
		vfuncs::vguisurf_hook.unhook_all();
		vfuncs::bsp_query_hook.unhook_all();
		vfuncs::mdlrender_hook.unhook_all();
		vfuncs::clientmode_hook.unhook_all();
		vfuncs::sound_hook.unhook_all();
		vfuncs::sv_cheats.unhook_all();
		vfuncs::render_view.unhook_all();
		Glow::Get().Shutdown();
		CListener::Get().remove();
		g_Hitmarker->Stop();
	}
}
