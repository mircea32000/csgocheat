#include "legitbot.h"
#include "helpers/math.hpp"
#include "options.hpp"
#define ClampAngle(x) Math::ClampAngles(x)

void Legit::Aimbot::Do(CUserCmd* cmd)
{
	if (!g_LocalPlayer)
		return;

	if (!g_EngineClient->IsInGame() && !g_EngineClient->IsConnected())
		return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (!weapon->IsGun()) //@TODO : taser check
		return;

	QAngle m_vecLocalAngle;

	float bestFOV = FLT_MAX;
	int bestPlayer = -1;
	int oldbestplayer = 0;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		C_BasePlayer* ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!ent)
			continue;

		if (ent->IsDormant() || !ent->IsAlive())
			continue;

		if (ent->EntIndex() == g_LocalPlayer->EntIndex())
			continue;

		if (ent->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) //@TODO : friendlies checkbox
			continue;

		if (!g_LocalPlayer->CanSeePlayer(ent, ent->GetEyePos()))
			continue;

		g_EngineClient->GetViewAngles(m_vecLocalAngle);//gets localplayer viewangles
		Vector m_vecLocalEyes = g_LocalPlayer->GetEyePos();//gets localplayer eye angles
		Vector m_vecPlayerEyes = ent->GetEyePos(); //gets nigger eye angles

		QAngle m_vecPlayerAngle = Math::CalcAngle(m_vecLocalEyes, ent->GetHitboxPos(0)); //calculates the angle between local eyes and player eyes

		//float fov = (m_vecPlayerAngle - m_vecLocalAngle).Length();//substract the angle difference from the localplayer viewangles

		float fov = Math::get_fov(m_vecLocalAngle, m_vecLocalEyes, ent->GetHitboxPos(0));

		if (fov > g_Options.legit_fov)
			continue;

		bestFOV = g_Options.legit_fov;

		if (fov < bestFOV)//if the fov smaller than flt_max
		{
			bestFOV = fov; //our best fov is the current fov
			bestPlayer = i; //save the player so we can aim at him later
		}

	}
	if (bestPlayer > -1)
	{
    	C_BasePlayer* ent = (C_BasePlayer*)g_EntityList->GetClientEntity(bestPlayer);

		QAngle m_vecAimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), ent->GetHitboxPos(0));
		if (GetAsyncKeyState(VK_LBUTTON))
		{
			m_vecAimAngle = (m_vecLocalAngle - (m_vecLocalAngle - m_vecAimAngle) / 10.f);
			Math::correct_angles(m_vecAimAngle);
			g_EngineClient->SetViewAngles(m_vecAimAngle);
		}
		oldbestplayer = bestPlayer;

	}
}