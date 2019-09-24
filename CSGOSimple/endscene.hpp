#pragma once
#include "menu.hpp"

namespace Hooks::Endscene
{
	long __stdcall hkEndScene(IDirect3DDevice9* device);
}