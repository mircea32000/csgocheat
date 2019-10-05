#pragma once

#include "valve_sdk/csgostructs.hpp"
#include "helpers/vfunc_hook.hpp"
#include <d3d9.h>



namespace index
{
	constexpr auto ListLeavesInBox          = 6;
	constexpr auto EmitSound1               = 5;
	constexpr auto EmitSound2               = 6;
    constexpr auto EndScene                 = 42;
    constexpr auto Reset                    = 16;
    constexpr auto PaintTraverse            = 41;
    constexpr auto CreateMove               = 22;
    constexpr auto PlaySound                = 82;
    constexpr auto FrameStageNotify         = 37;
    constexpr auto DrawModelExecute         = 21;
    constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto SvCheatsGetBool          = 13;
	constexpr auto UnkFunc                  = 89;
	constexpr auto OverrideView             = 18;
	constexpr auto LockCursor               = 67;
	constexpr auto SendDatagram             = 46;
}

namespace Hooks
{
    void Initialize();
    void Shutdown();

	namespace vfuncs
	{
		inline vfunc_hook hlclient_hook;
		inline vfunc_hook direct3d_hook;
		inline vfunc_hook vguipanel_hook;
		inline vfunc_hook vguisurf_hook;
		inline vfunc_hook mdlrender_hook;
		inline vfunc_hook senddatagram_hook;
		inline vfunc_hook viewrender_hook;
		inline vfunc_hook sound_hook;
		inline vfunc_hook clientmode_hook;
		inline vfunc_hook filesystem_hook;
		inline vfunc_hook bsp_query_hook;
		inline vfunc_hook sv_cheats;
		inline vfunc_hook render_view;
	}
}
