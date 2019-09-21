#include "hooks.hpp"
#include <intrin.h>  
#include "OverrideView.hpp"
#include "render.hpp"
#include "menu.hpp"
#include "EnginePrediction.h"
#include "options.hpp"
#include "radar.h"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "legitbot.h"
#include "skinchanger.h"
#include "features/visuals.hpp"
#include "parser.h"
#include "RenderableInfo.h"
#include "nade_bullshit.h"
#include "lagcomp.h"
#include "features/glow.hpp"
#include "fakelatency.h"
#pragma intrinsic(_ReturnAddress)  

CMapOverview MapOverview;
#define DDS_HEADER 542327876
#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )

namespace Hooks {

	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		filesystem_hook.setup(g_FileSystem);
		bsp_query_hook.setup(g_EngineClient->GetBSPTreeQuery());

		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		filesystem_hook.hook_index(index::UnkFunc, UnkFileSystemFunc);
		bsp_query_hook.hook_index(index::ListLeavesInBox, hkListLeavesInBox);

	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto crosshair_cvar = g_CVar->FindVar("crosshair");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Options.viewmodel_fov);
		mat_ambient_light_r->SetValue(g_Options.mat_ambient_light_r);
		mat_ambient_light_g->SetValue(g_Options.mat_ambient_light_g);
		mat_ambient_light_b->SetValue(g_Options.mat_ambient_light_b);
		
		crosshair_cvar->SetValue(!(g_Options.esp_enabled && g_Options.esp_crosshair));

		DWORD colorwrite, srgbwrite;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		ImDrawList* esp_drawlist = nullptr;

		esp_drawlist = Render::Get().RenderScene();

		CRadar::Get().Render();

		Menu::Get().Render();

		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;
		
		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		if (g_Options.misc_bhop)
			BunnyHop::OnCreateMove(cmd);

		TimeWarp::Get().DeleteInvalidRecords();
		TimeWarp::Get().StoreRecords(cmd);
		TimeWarp::Get().DoBackTrack(cmd);

		PredictionSystem::Get().Start(cmd, g_LocalPlayer);
		{
		Legit::Aimbot::Do(cmd);
		}
		PredictionSystem::Get().End(g_LocalPlayer);

		CNadePred::Get().trace(cmd);

		static uintptr_t pSavedNetChannel = NULL;
		uintptr_t pNetChannel = uintptr_t(g_ClientState->m_NetChannel);

		bool rehook = (pNetChannel != pSavedNetChannel);

		if (rehook)
		{
			CFakeLatency::Get().ClearIncomingSequences();
			senddatagram_hook.setup((void*)g_ClientState->m_NetChannel);
			senddatagram_hook.hook_index(index::SendDatagram, SendDatagram);
			oSendDatagram = senddatagram_hook.get_original<SendDatagramFn>(index::SendDatagram);
			
			pSavedNetChannel = pNetChannel;
		}
	
		if (g_EngineClient->IsConnected() && g_EngineClient->IsInGame()) {
			if (g_LocalPlayer) {
				CFakeLatency::Get().UpdateIncomingSequences();
			}
		}

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	int __fastcall SendDatagram(INetChannel* netchan, void*, bf_write* datagram)
	{

		if (datagram)
		{
			return oSendDatagram(netchan, datagram);
		}

		int instate = netchan->m_nInReliableState;
		int insequencenr = netchan->m_nInSequenceNr;

		float ping = 0;

		INetChannelInfo* nci = g_EngineClient->GetNetChannelInfo();
		if (nci)
		{
			ping = nci->GetLatency(FLOW_OUTGOING);
		}

		double fakeLatVal = 0.2f - ping;

		if (g_Options.fake_latency) {
			if (g_EngineClient->IsConnected() && g_EngineClient->IsInGame() && g_LocalPlayer) {
				CFakeLatency::Get().AddFakeLag(netchan, fakeLatVal);
			}
		}

		int ret = oSendDatagram(netchan, datagram);

		netchan->m_nInReliableState = instate;
		netchan->m_nInSequenceNr = insequencenr;
		return ret;
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

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
	void __fastcall UnkFileSystemFunc(void* pThis, void*, void* pImage)
	{
		static UnkFileSystemFunc_t oUnkFileSystemFunc = filesystem_hook.get_original<UnkFileSystemFunc_t>(index::UnkFunc);
		
		static auto pUnkFuncReturnAddress =
			Utils::PatternScan
			(
				GetModuleHandleW(L"panorama.dll"),
				("C7 43 ? ? ? ? ? 83 7C 24 ? ? 7C 25 8B 54 24 48")
			);

		DWORD dwThisPointer; __asm mov dwThisPointer, ebx;

		if (pImage && *(DWORD*)pImage == DDS_HEADER && _ReturnAddress() == pUnkFuncReturnAddress)
		{
			auto unkClass = *(DWORD*)(dwThisPointer + 0x48);

			if (unkClass && *(DWORD*)(unkClass + 0x2C))
			{
				const char* fileName = *(const char**)(unkClass + 0x20);

				static SetMap_t SetMap = (SetMap_t)Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 81 EC ? ? ? ? 53 8B 5D 08 8B D3");

				std::string szLevelName(g_ClientState->m_szLevelNameShort);

				if (!szLevelName.size())
				{
					//ex: resource/overviews/de_dust2_radar.dds -> de_dust2
					szLevelName = fileName;
					szLevelName.erase(szLevelName.begin(), szLevelName.begin() + 19);
					szLevelName.erase(szLevelName.end() - 10, szLevelName.end());
				}

				SetMap(&MapOverview, szLevelName.c_str());
				

				CRadar::Get().m_flPosX = MapOverview.m_flPosX;
				CRadar::Get().m_flPosY = MapOverview.m_flPosY;
				CRadar::Get().m_flScale = MapOverview.m_flScale;
			}

			auto dwSize = *(DWORD*)(dwThisPointer + 0x50);

			CRadar::Get().m_bCanDraw = SUCCEEDED(D3DXCreateTextureFromFileInMemory(g_D3DDevice9, pImage, dwSize, &CRadar::Get().m_pMapTexture));
		}

		return oUnkFileSystemFunc(pThis, pImage);
	}
	int __fastcall hkListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax) {
		typedef int(__thiscall * ListLeavesInBox)(void*, const Vector&, const Vector&, unsigned short*, int);
		static auto ofunc = bsp_query_hook.get_original< ListLeavesInBox >(index::ListLeavesInBox);

		// occulusion getting updated on player movement/angle change,
		// in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
		// check for return in CClientLeafSystem::InsertIntoTree
		if (!g_Options.chams_player_enabled || !g_Options.chams_disable_occlusion || *(uint32_t*)_ReturnAddress() != 0x14244489) // 89 44 24 14 ( 0x14244489 ) - new / 8B 7D 08 8B ( 0x8B087D8B ) - old
			return ofunc(bsp, mins, maxs, pList, listMax);

		// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
		auto info = *(RenderableInfo_t * *)((uintptr_t)_AddressOfReturnAddress() + 0x14);
		if (!info || !info->m_pRenderable)
			return ofunc(bsp, mins, maxs, pList, listMax);

		// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
		auto base_entity = info->m_pRenderable->GetIClientUnknown()->GetBaseEntity();
		if (!base_entity || !base_entity->IsPlayer())
			return ofunc(bsp, mins, maxs, pList, listMax);

		// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
		// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
		info->m_Flags &= ~0x100;
		info->m_Flags2 |= 0xC0;

		// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
		static const Vector map_min = Vector(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
		static const Vector map_max = Vector(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
		auto count = ofunc(bsp, map_min, map_max, pList, listMax);
		return count;
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);

		if (stage == ClientFrameStage_t::FRAME_RENDER_START)
		{
#ifdef NDEBUG
			static bool bOnce = false;
			if (!bOnce)
			{
				//CParser::Get().GetItemScema();
				CParser::Get().ParseSkins();
				CParser::Get().ParseStickers();
				bOnce = true;
			}
#else
			// debug code
#endif
			
		}
		if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			CSkinChanger::Get().Do();
		}
		if (stage == FRAME_NET_UPDATE_END)
		{
			for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++)
			{
				if (!g_LocalPlayer)
				{
					TimeWarp::Get().m_Records[i].m_Mutex.lock();
					TimeWarp::Get().m_Records[i].m_vecRecords.clear();
					TimeWarp::Get().m_Records[i].m_Mutex.unlock();
				}
				else
					TimeWarp::Get().UpdateRecords(i);

			}

		}
		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView)
			Visuals::Get().ThirdPerson();

		OverrideView::Get().currentFOV = vsView->fov; //for fov changers later

		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	
	
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
}
