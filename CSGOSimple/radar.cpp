#include "radar.h"
#include <minmax.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "options.hpp"
#include "render.hpp"
ImVec2 m_MapCenter = ImVec2(512, 512);
float m_fZoom = 3.f;

#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h

#define M_PI_F		((float)(M_PI))	// Shouldn't collide with anything.

#ifndef DEG2RAD
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#endif

#ifdef NDEBUG
#define Assert( _exp ) ((void)0)
#else
#define Assert(x)
#endif

void SinCos(float a, float* s, float* c)
{
	*s = sin(a);
	*c = cos(a);
}


void AngleVectors(const Vector& angles, Vector* forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);

	float	sp, sy, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

void CRadar::VectorYawRotate(const ImVec2& in, float flYaw, ImVec2& out)
{
	if (&in == &out)
	{
		ImVec2 tmp;
		tmp = in;
		VectorYawRotate(tmp, flYaw, out);
		return;
	}
	float sy, cy;
	SinCos(DEG2RAD(flYaw), &sy, &cy);
	out.x = in.x * cy - in.y * sy;
	out.y = in.x * sy + in.y * cy;
}

void CRadar::VectorYawRotate2(ImVec2 center, float angle, ImVec2* p)
{
	float flSin = sin(angle);
	float flCos = cos(angle);

	p->x -= center.x;
	p->y -= center.y;

	float xnew = p->x * flCos - p->y * flSin;
	float ynew = p->x * flSin + p->y * flCos;

	p->x = xnew + center.x;
	p->y = ynew + center.y;
}

void CRadar::GetSize(int& x, int& y)
{
	ImVec2 WindowSize = ImGui::GetWindowSize();
	x = WindowSize.x;
	y = WindowSize.y;
}

void CRadar::GetPos(int& x, int& y)
{
	ImVec2 WindowPos = ImGui::GetWindowPos();
	x = WindowPos.x;
	y = WindowPos.y;
}

ImVec2 CRadar::WorldToMap(const Vector& worldpos)
{
	ImVec2 offset(worldpos.x - m_flPosX, worldpos.y - m_flPosY);

	if (offset.x == 0.f)
		offset.x += FLT_EPSILON;
	if (offset.y == 0.f)
		offset.y += FLT_EPSILON;

	offset.x /= (m_flScale * 2.f);
	offset.y /= -(m_flScale * 2.f);

	return offset;
}

QAngle viewAngle;

ImVec2 CRadar::MapToRadar(const ImVec2& mappos)
{

	if (!g_LocalPlayer)
		return ImVec2(0, 0);

	int pwidth, pheight;
	int px, py;
	ImVec2 panelpos;

	GetSize(pwidth, pheight);
	GetPos(px, py);

	ImVec2 OriginOffset = WorldToMap(g_LocalPlayer->m_vecOrigin());

	ImVec2 offset;
	offset.x = (mappos.x - m_MapCenter.x);
	offset.y = (mappos.y - m_MapCenter.y);

	offset.x += OriginOffset.x;
	offset.y += OriginOffset.y;

	VectorYawRotate(offset, viewAngle.yaw + 90, offset);

	float fScale = m_fZoom / 1024;

	offset.x *= fScale;
	offset.y *= fScale;

	panelpos.x = px + (pwidth * 0.5f) + (pheight * offset.x);
	panelpos.y = py + (pheight * 0.5f) + (pheight * offset.y);

	return panelpos;
}

void CRadar::Render()
{
	auto cfg = g_Options.esp_radar;

	if (!cfg)
		return;
	if (!g_LocalPlayer)
		return;
	if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
		return;

	g_EngineClient->GetViewAngles(viewAngle);
	m_fZoom = g_Options.iZoom * 0.001f;
	int iAlpha = g_Options.iAlpha * 2.55f;

	ImGui::SetNextWindowBgAlpha((std::max)(g_Options.iAlpha / 100.f - FLT_EPSILON, 0.f));

	ImGuiWindowFlags iFlags = 0;
	if (g_Options.bHideHeader)
		iFlags |= ImGuiWindowFlags_NoTitleBar;

	if (ImGui::Begin("Radar", true, cfg, iFlags))
	{
		ImVec2 points[4] =
		{
			MapToRadar(ImVec2(0, 0)),
			MapToRadar(ImVec2(512, 0)),
			MapToRadar(ImVec2(512, 512)),
			MapToRadar(ImVec2(0, 512))
		};

		VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &points[0]);
		VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &points[1]);
		VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &points[2]);
		VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &points[3]);

		ImGuiWindow* pWnd = ImGui::GetCurrentWindow();
		ImDrawList* pDrawList = pWnd->DrawList;

		if (CRadar::Get().m_bCanDraw && m_pMapTexture)
		{
			pDrawList->AddImageQuad
			(
				m_pMapTexture,
				points[0], points[1],
				points[2], points[3],
				ImVec2(0, 0), ImVec2(1, 0),
				ImVec2(1, 1), ImVec2(0, 1),
				Color(255, 255, 255, iAlpha).ToU32()
			);
		}

		ImVec2 vWinPos = ImGui::GetWindowPos();
		ImVec2 Offset(2, 2);

		int iLocal = g_EngineClient->GetLocalPlayer();
		Vector& vLocalOrigin = g_LocalPlayer->m_vecOrigin();

		for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
			Color color;
			auto entity = C_BaseEntity::GetEntityByIndex(i);

			if (!entity)
				continue;

			if ((entity->IsC4() && !entity->IsDormant() && entity->m_hOwnerEntity().ToInt() == -1) || entity->IsPlantedC4())
			{
				color = Color(25,255,0,255);// nice shade of green

				ImVec2 vMapPos = MapToRadar(WorldToMap(entity->m_vecOrigin()));
				VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &vMapPos);
				auto sz = g_pWeaponIcons24->CalcTextSizeA(16.f, FLT_MAX, 0.0f, "M");
				ImGui::PushFont(g_pWeaponIcons24);
				pDrawList->AddText(ImVec2(vMapPos.x - sz.x / 2 + 1, vMapPos.y - sz.y / 2 + 1), Color::Black.ToU32(), "M");
				pDrawList->AddText(ImVec2(vMapPos.x - sz.x / 2 - 1, vMapPos.y - sz.y / 2 - 1), Color::Black.ToU32(), "M");//lets just hope theres not any memory leak here
				pDrawList->AddText(ImVec2(vMapPos.x - sz.x / 2 + 1, vMapPos.y - sz.y / 2 - 1), Color::Black.ToU32(), "M");
				pDrawList->AddText(ImVec2(vMapPos.x - sz.x / 2 - 1, vMapPos.y - sz.y / 2 + 1), Color::Black.ToU32(), "M");
				pDrawList->AddText(ImVec2(vMapPos.x - sz.x / 2, vMapPos.y - sz.y / 2), color.ToU32(), "M");

				ImGui::PopFont();
			}

		}

		for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++)
		{

			C_BasePlayer* ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

			if (!ent)
				continue;

			auto class_id = ent->GetClientClass()->m_ClassID;

			Vector& vOrigin = ent->m_vecOrigin();
			const Vector& vAngles = ent->m_vecAngles();

			Color color;

			if (i == g_LocalPlayer->EntIndex())
			{
				color = Color::White;
			}
			else
			{
				if (vOrigin.IsZero() || !ent->IsAlive() || ent->m_iTeamNum() == 1 /*spectator*/ || ent->IsDormant())
					continue;
				//@TODO: color picker
				if (ent->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())//teammate
				{
					color = Color(0,0,255,255);
				}
				else //enemy of the states
				{
					color = Color(255, 0, 0, 255);
				}

			}

			ImVec2 vMidPoint = MapToRadar(WorldToMap(vOrigin));
			VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &vMidPoint);

			Vector vForward, vLeft, vRight;

			Vector vAngNoPitch = vAngles;
			vAngNoPitch.x = 0.f;

			Vector vForwardAng = vAngNoPitch;
			AngleVectors(vForwardAng, &vForward);

			Vector vLeftAng = vAngNoPitch;
			vLeftAng.y -= 90.f;
			AngleVectors(vLeftAng, &vLeft);

			Vector vRightAng = vAngNoPitch;
			vRightAng.y += 90.f;
			AngleVectors(vRightAng, &vRight);

			float flFactor = 100.f;

			Vector vDelta = (vForward * flFactor);

			ImVec2 vPlayerPos = MapToRadar(WorldToMap(vOrigin));
			VectorYawRotate2(MapToRadar(ImVec2(256, 256)), DEG2RAD(180), &vPlayerPos);

			pDrawList->AddRectFilled
			(
				{ vPlayerPos.x - 3.f, vPlayerPos.y - 3.f },
				{ vPlayerPos.x + 3.f, vPlayerPos.y + 3.f },
				color.ToU32(),
				12.f
			);

		}
	}

	ImGui::End();
}