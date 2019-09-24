#include "framestagenotify.h"
#include "hooks.hpp"
#include "skinchanger.h"
#include "lagcomp.h"

namespace Hooks::FSN
{
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = vfuncs::hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);

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
}