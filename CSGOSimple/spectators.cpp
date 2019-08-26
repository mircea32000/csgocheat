
#include "spectators.h"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "render.hpp"
#include "config.hpp"
#include "helpers/utils.hpp"
#include <wchar.h>

void DrawPlayerName(int x, int y, Color color, DWORD alignment, const char* msg, ...)
{
	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	auto textsize = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, (const char*)wbuf);

	if (alignment & FONT_RIGHT)
		x -= textsize.x;
	if (alignment & FONT_CENTER)
		x -= textsize.x / 2;

    std::wstring ws(wbuf);
	std::string str (ws.begin(), ws.end());
	Render::Get().RenderText(str, x, (int)(y - textsize.y / 2), 14, color);
}

void CSpectators::RenderSpectators()
{
	RECT scrn = Utils::GetViewport();
	int kapi = 0;

	if (g_LocalPlayer)// && (g_EngineClient->IsConnected() || g_EngineClient->IsInGame()))
	{
		for (int i = 0; i < g_EntityList->GetHighestEntityIndex(); i++)
		{
			// Get the entity
			C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
			player_info_t pinfo;
			if (pEntity && pEntity != g_LocalPlayer)
			{
				if (g_EngineClient->GetPlayerInfo(i, &pinfo) && !pEntity->IsAlive() && !pEntity->IsDormant())
				{
					auto obs = pEntity->m_hObserverTarget();
					if (obs)
					{
						C_BasePlayer* pTarget = (C_BasePlayer*)g_EntityList->GetClientEntityFromHandle(obs);
						player_info_t pinfo2;
						if (pTarget && pTarget->EntIndex() == g_LocalPlayer->EntIndex())
						{
							if (g_EngineClient->GetPlayerInfo(pTarget->EntIndex(), &pinfo2))
							{

								DrawPlayerName(scrn.right - 98, (scrn.top) + (16 * kapi) + 20, Color(255, 0, 0, 255), FONT_LEFT, "%s", pinfo.szName);
								kapi++;
							}
						}
					}
				}
			}
		}
	}
	DrawPlayerName(scrn.right - 98, (scrn.top) + 10, Color(255, 255, 255, 255), FONT_LEFT, "Spectating you:");

}