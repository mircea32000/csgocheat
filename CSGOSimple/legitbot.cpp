#include "legitbot.h"
#include "helpers/math.hpp"
#include "options.hpp"
#include <minmax.h>
#include "CTimer.h"
#include "lagcomp.h"

QAngle m_vecAimAngle;
Vector m_vecLagCompAngle;
QAngle m_vecLocalAngle;
Vector bestHitboxLagComp;
int bestHitbox = -1;
int bestPlayer = -1;
std::map<int, LegitBotConfig> m_mapConfig;

Vector ClosestRecords(C_BasePlayer* ent)
{
	float closestFOV = FLT_MAX;

	float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), bestHitboxLagComp);

	for (auto& records : TimeWarp::Get().m_Records[ent->EntIndex()].m_vecRecords)
	{
		if (fov < closestFOV)
		{
			closestFOV = fov;
			m_vecLagCompAngle = records.m_vecHitboxPos;
		}
	}
	return m_vecLagCompAngle;
}

void PickUserSmothing(int type, CUserCmd* cmd, C_BaseCombatWeapon* weapon)
{
	auto& settings = g_Options.m_mapAim[weapon->m_Item().m_iItemDefinitionIndex()];

	if (type == 0)
	{
		float smooth = 1 - settings.m_fSmooth;
		QAngle m_vecSmoothClamp = (m_vecLocalAngle - cmd->viewangles);
		Math::correct_angles(m_vecSmoothClamp);

		cmd->viewangles = (m_vecLocalAngle - m_vecSmoothClamp * smooth);
		Math::correct_angles(cmd->viewangles);
	}
	else if (type == 1 /*constant*/ || type == 2 /*algorithmically faster*/)
	{

		QAngle delta = (cmd->viewangles - m_vecLocalAngle);
		Math::correct_angles(delta);
		QAngle toChange = QAngle();

		float smooth = powf(settings.m_fSmooth, 0.4f);

		smooth = min(0.99f, smooth);

		float coeff = (1.0f - smooth) / delta.Length() * 4.f;

		if (type == 2 /*algo fast yes*/)
			coeff = powf(coeff, 2.f) * 10.f;

		coeff = min(1.f, coeff);
		toChange = delta * coeff;

		cmd->viewangles = m_vecLocalAngle + toChange;
	}
}

void PickUserHitboxLagComp(C_BasePlayer* ent, int option, C_BaseCombatWeapon* weapon)
{
	auto& settings = g_Options.m_mapAim[weapon->m_Item().m_iItemDefinitionIndex()];

	float lagcompbestfov = FLT_MAX;
	Vector gaycopy;

	for (auto& records : TimeWarp::Get().m_Records[ent->EntIndex()].m_vecRecords)
	{

		std::vector<Vector> lagcomphitboxes =
		{
			records.m_vecHitboxPos,
			records.m_vecHitboxPosStomach
		};

		switch (option)
		{
		case 0:
			bestHitboxLagComp = records.m_vecHitboxPos;
			break;
		case 1:
			bestHitboxLagComp = records.m_vecHitboxPosNeck;
			break;
		case 2:
			bestHitboxLagComp = records.m_vecHitboxPosPelvis;
			break;
		case 3:
			bestHitboxLagComp = records.m_vecHitboxPosStomach;
			break;
		case 4:
			bestHitboxLagComp = records.m_vecHitboxPosChest;
			break;
		case 5:
			for (auto& hitboxes : lagcomphitboxes)
			{
				if (hitboxes.IsZero())
					continue;

				float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitboxes);
				if (fov < lagcompbestfov)
				{
					lagcompbestfov = fov;
					gaycopy = hitboxes;
				}
				bestHitboxLagComp = gaycopy;
			}

			break;
		}
	}
}

void PickUserHitbox(C_BasePlayer* ent, int option, C_BaseCombatWeapon* weapon)
{
	auto& settings = g_Options.m_mapAim[weapon->m_Item().m_iItemDefinitionIndex()];

	std::vector<int> hitboxes = {
	HITBOX_HEAD,
	HITBOX_STOMACH,
	};

	Vector m_vecLocalEyes = g_LocalPlayer->GetEyePos();

	auto& records = TimeWarp::Get().m_Records[ent->EntIndex()].m_vecRecords;

	float niggabestfov = FLT_MAX;

	switch (option)
	{
	case 0:
		bestHitbox = HITBOX_HEAD;
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

			if (temp.IsZero())
				continue;

			float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), m_vecLocalEyes, temp);
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

void RCS(QAngle& angle, CUserCmd* cmd)
{
	auto weap = g_LocalPlayer->m_hActiveWeapon();
	if (!weap->IsWeapon())
		return;

	QAngle m_oldangle;
	QAngle aimpunch;
	if (cmd->buttons & IN_ATTACK)
	{
		aimpunch.pitch = g_LocalPlayer->m_aimPunchAngle().pitch * g_Options.m_mapAim[weap->m_Item().m_iItemDefinitionIndex()].m_fRCSX;
		aimpunch.yaw = g_LocalPlayer->m_aimPunchAngle().yaw * g_Options.m_mapAim[weap->m_Item().m_iItemDefinitionIndex()].m_fRCSY;
		aimpunch.roll = 0;

		cmd->viewangles += (m_oldangle - aimpunch);

		m_oldangle = aimpunch;
	}
	else
	{
		m_oldangle.pitch = m_oldangle.yaw = m_oldangle.roll = 0;
	}
}

void Legit::Aimbot::Do(CUserCmd* cmd)
{
	if (!g_LocalPlayer)
		return;

	if (!g_EngineClient->IsInGame() && !g_EngineClient->IsConnected())
		return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return;

	if (!weapon->IsWeapon())
		return;

	int itemdef = weapon->m_Item().m_iItemDefinitionIndex();

	int itemdefDEBUG = itemdef;

	auto& settings = g_Options.m_mapAim[itemdefDEBUG];

	float bestFOV = FLT_MAX;

	if (!weapon->IsGun()) //@TODO : taser check
		return;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		C_BasePlayer* ent = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

		if (!ent)
			continue;

		if (ent->IsDormant() || !ent->IsAlive())
			continue;

		if (ent->EntIndex() == g_LocalPlayer->EntIndex())
			continue;

		g_EngineClient->GetViewAngles(m_vecLocalAngle);
		Vector m_vecLocalEyes = g_LocalPlayer->GetEyePos();
		Vector m_vecPlayerEyes = ent->GetEyePos(); 

		PickUserHitbox(ent, settings.m_iHitbox, weapon);

		if (!g_LocalPlayer->CanSeePlayer(ent, ent->GetHitboxPos(bestHitbox)))
			continue;

		float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), m_vecLocalEyes, settings.m_bTargetBacktrack ? m_vecLagCompAngle : ent->GetHitboxPos(bestHitbox));

		if (fov > settings.m_iFOV)
			continue;

		if (!settings.m_bAttackEnemies && g_LocalPlayer->m_iTeamNum() != ent->m_iTeamNum())
			continue;

		if (!settings.m_bAttackFriendlies && g_LocalPlayer->m_iTeamNum() == ent->m_iTeamNum())
			continue;

		bestFOV = settings.m_iFOV;

		if (fov < bestFOV)
		{
			bestFOV = fov; 
			bestPlayer = i; 
		}

	}
	if (bestPlayer > -1)
	{
		C_BasePlayer* ent = (C_BasePlayer*)g_EntityList->GetClientEntity(bestPlayer);

		float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), settings.m_bTargetBacktrack ? m_vecLagCompAngle : ent->GetHitboxPos(bestHitbox));

		if ((settings.m_fFlashTolerance * 2.55f) < g_LocalPlayer->FlashDuration())
			return;

		if (fov > settings.m_iFOV)
			return;

		Vector vecTargetHitboxPos = ent->GetHitboxPos(bestHitbox);
		if (!settings.m_bIgnoreSmoke && g_LocalPlayer->IsBehindSmoke(vecTargetHitboxPos))
			return;

		if (settings.m_bIgnoreJumping && !(ent->m_fFlags() & FL_ONGROUND))
			return;

		if(!settings.m_bTargetBacktrack)
		cmd->viewangles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), ent->GetHitboxPos(bestHitbox)); //CHANGE THIS TO BACKTRACC SHIT
		else
			cmd->viewangles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), ClosestRecords(ent)); //CHANGE THIS TO BACKTRACC SHIT

		if (settings.m_bRCS)
		{
			RCS(m_vecAimAngle, cmd);
		}

		PickUserSmothing(settings.m_iSmoothingMethod, cmd, weapon);

		Math::correct_angles(cmd->viewangles);
		if (GetAsyncKeyState(VK_LBUTTON))
		{

			g_EngineClient->SetViewAngles(cmd->viewangles);
#ifdef NDEBUG
#else
			std::string str, str1, str2, str3;
			str = "X: " + std::to_string(cmd->viewangles.pitch);
			str1 = " | Y: " + std::to_string(cmd->viewangles.yaw);
			str2 = " | Z: " + std::to_string(cmd->viewangles.roll);
			str3 = str + str1 + str2 + "\n";

			Utils::ConsolePrint(str3.c_str());
			Utils::ConsolePrint("X: %d", m_vecAimAngle.pitch, +" | Y: %d", m_vecAimAngle.yaw, +" | Z: %d", m_vecAimAngle.roll, "\n");
#endif

			if (!CTimer::Get().delay(settings.m_fDelay) &&
				cmd->buttons & IN_ATTACK &&
				g_LocalPlayer->m_iShotsFired() == 0 &&
				weapon->m_Item().m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			{
				cmd->buttons &= ~IN_ATTACK;
			}
			else
			{
				CTimer::Get().reset();
			}
		}
	}
	else
	{
		CTimer::Get().reset();
	}
}