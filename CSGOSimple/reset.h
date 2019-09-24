#pragma once
#include "menu.hpp"
#include "render.hpp"

namespace Hooks::Reset
{
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
}