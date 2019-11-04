#include "createmove.hpp"
#include "lagcomp.h"
#include "nade_bullshit.h"
#include "EnginePrediction.h"
#include "features/bhop.hpp"
#include "options.hpp"
#include "legitbot.h"
#include "menu.hpp"
#include "fakelatency.h"
#include "Senddatagram.h"
#include "ragebot.h"
#include "antiaim.h"
#include "fixmovement.h"
#include "C_CSPlayer.h"
namespace Hooks
{
	namespace Createmove
	{
		void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
		{
			CCSPlayer::Get().Initialise();

			static auto oCreateMove = vfuncs::hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

			oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

			auto cmd = g_Input->GetUserCmd(sequence_number);
			auto verified = g_Input->GetVerifiedCmd(sequence_number);

			if (!cmd || !cmd->command_number)
				return;

			if (Menu::Get().IsVisible())
				cmd->buttons &= ~IN_ATTACK;

			if (g_Options.misc_bhop)
				BunnyHop::OnCreateMove(cmd);

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

			TimeWarp::Get().DeleteInvalidRecords();
			TimeWarp::Get().StoreRecords(cmd);
			TimeWarp::Get().DoBackTrack(cmd);

			PredictionSystem::Get().Start(cmd, g_LocalPlayer);
			{
				Legit::Aimbot::Do(cmd);
				Legit::Triggerbot::Do(cmd);		
			}
			PredictionSystem::Get().End(g_LocalPlayer);

			CNadePred::Get().trace(cmd);

			static uintptr_t pSavedNetChannel = NULL;
			uintptr_t pNetChannel = uintptr_t(g_ClientState->m_NetChannel);

			bool rehook = (pNetChannel != pSavedNetChannel);

			if (rehook)
			{
				CFakeLatency::Get().ClearIncomingSequences();
				vfuncs::senddatagram_hook.setup((void*)g_ClientState->m_NetChannel);
				vfuncs::senddatagram_hook.hook_index(index::SendDatagram, Hooks::Senddatagram::SendDatagram);
				Hooks::Senddatagram::oSendDatagram = vfuncs::senddatagram_hook.get_original<Hooks::Senddatagram::SendDatagramFn>(index::SendDatagram);

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
				call Hooks::Createmove::hkCreateMove
				pop  ebx
				pop  ebp
				retn 0Ch
			}
		}
	}
}