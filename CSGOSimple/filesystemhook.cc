#include "Filesystemhook.hpp"
#include "radar.h"
#include "hooks.hpp"
#include <intrin.h>  
#pragma intrinsic(_ReturnAddress)  

#define DDS_HEADER 542327876

struct CMapOverview
{
	char pad[220];
	float m_flPosX;
	float m_flPosY;
	char pad1[12];
	float m_flScale;
	char pad_0x00F0[0x1061B + 64];
};
using UnkFileSystemFunc_t = void(__thiscall*)(void*, void*);
using SetMap_t = int(__thiscall*)(CMapOverview*, const char*);
CMapOverview MapOverview;

namespace Hooks::Filesystem
{
	void __fastcall UnkFileSystemFunc(void* pThis, void*, void* pImage)
	{
		static UnkFileSystemFunc_t oUnkFileSystemFunc = vfuncs::filesystem_hook.get_original<UnkFileSystemFunc_t>(index::UnkFunc);

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
}