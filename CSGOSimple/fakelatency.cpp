#include "fakelatency.h"
#include <deque>
std::deque<CIncomingSequence> sequences;
int32_t lastincomingsequencenumber;

void CFakeLatency::AddFakeLag(INetChannel* netchan, float Latency)
{
	for (auto& seq : sequences)
	{
		if (g_GlobalVars->realtime - seq.curtime >= Latency)
		{
			netchan->m_nInReliableState = seq.inreliablestate;
			netchan->m_nInSequenceNr = seq.sequencenr;
			break;
		}
	}
}

void CFakeLatency::ClearIncomingSequences()
{
	sequences.clear();
	lastincomingsequencenumber = 0;
}

void CFakeLatency::UpdateIncomingSequences()
{
	if (!g_ClientState)
		return;

	INetChannel* netchan = reinterpret_cast<INetChannel*>(g_ClientState->m_NetChannel);
	if (netchan)
	{
		if (netchan->m_nInSequenceNr > lastincomingsequencenumber)
		{
			lastincomingsequencenumber = netchan->m_nInSequenceNr;

			sequences.push_front(CIncomingSequence(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, g_GlobalVars->realtime));
		}
		if (sequences.size() > 2048)
			sequences.pop_back();
	}
}
