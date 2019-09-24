#pragma once
#include "valve_sdk/sdk.hpp"

namespace Hooks::Senddatagram
{
	extern int __fastcall SendDatagram(INetChannel* netchan, void*, bf_write* datagram);

	using SendDatagramFn = int(__thiscall*)(INetChannel*, bf_write*);
	inline SendDatagramFn oSendDatagram;

}