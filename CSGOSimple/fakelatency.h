#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "options.hpp"

struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int outstate, int seqnr, float time)
	{
		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int outreliablestate;
	int sequencenr;
	float curtime;
};


class CFakeLatency : public Singleton<CFakeLatency> {
public:
	void AddFakeLag(INetChannel* netchan, float Latency);
	void UpdateIncomingSequences();
	void ClearIncomingSequences();
};

