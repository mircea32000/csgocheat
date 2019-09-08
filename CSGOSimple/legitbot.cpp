#include "legitbot.h"
#include "helpers/math.hpp"
#include "options.hpp"

QAngle m_vecLocalAngle;
int bestHitbox = -1;

void PickUserHitbox(C_BasePlayer* ent, int option)
{
	std::vector<int> hitboxes = {
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM,
	};

	Vector m_vecLocalEyes = g_LocalPlayer->GetEyePos();

	float niggabestfov = FLT_MAX;

	switch (option)
	{
	case 0:
		bestHitbox = HITBOX_HEAD;
		break;
	case 1:
		bestHitbox = HITBOX_NECK;
		break;
	case 2:
		bestHitbox = HITBOX_PELVIS;
		break;
	case 3:
		bestHitbox = HITBOX_STOMACH;
		break;
	case 4:
		bestHitbox = HITBOX_CHEST; //5
		break;
	case 5:
	{
		for (auto hitbox : hitboxes)
		{
			Vector temp;
			if (!ent->GetHitboxPos(hitbox, temp))
				continue;

			float fov = Math::get_fov(m_vecLocalAngle, m_vecLocalEyes, temp);
			if (fov < niggabestfov)
			{
				niggabestfov = fov;
				bestHitbox = hitbox;
			}
		}
	}
	break;

	}
}

void Legit::Aimbot::Do(CUserCmd* cmd)
{
	if (!g_LocalPlayer)
		return;

	if (!g_EngineClient->IsInGame() && !g_EngineClient->IsConnected())
		return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	float bestFOV = FLT_MAX;

	if (!weapon->IsGun()) //@TODO : taser check
		return;

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

		g_EngineClient->GetViewAngles(m_vecLocalAngle);//gets localplayer viewangles
		Vector m_vecLocalEyes = g_LocalPlayer->GetEyePos();//gets localplayer eye angles
		Vector m_vecPlayerEyes = ent->GetEyePos(); //gets nigger eye angles

		PickUserHitbox(ent, g_Options.legit_hitbox);


		if (!g_LocalPlayer->CanSeePlayer(ent, ent->GetHitboxPos(bestHitbox)))
			continue;

		float fov = Math::get_fov(m_vecLocalAngle, m_vecLocalEyes, ent->GetHitboxPos(bestHitbox));

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

		QAngle m_vecAimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), ent->GetHitboxPos(bestHitbox));
		if (GetAsyncKeyState(VK_LBUTTON))
		{
			QAngle m_vecSmoothClamp = (m_vecLocalAngle - m_vecAimAngle);
			Math::correct_angles(m_vecSmoothClamp);
		
			m_vecAimAngle = (m_vecLocalAngle - m_vecSmoothClamp / g_Options.legit_smooth);
			Math::correct_angles(m_vecAimAngle);

			m_vecAimAngle -= g_LocalPlayer->m_aimPunchAngle() * 2.f;
			Math::correct_angles(m_vecAimAngle);

			g_EngineClient->SetViewAngles(m_vecAimAngle);
		}
		oldbestplayer = bestPlayer;

	}
}