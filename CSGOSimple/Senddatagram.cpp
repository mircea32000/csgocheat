#include "Senddatagram.h"
#include "hooks.hpp"
#include "options.hpp"
#include "fakelatency.h"

namespace Hooks::Senddatagram
{
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
}