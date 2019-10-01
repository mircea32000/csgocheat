#include "legitbot.h"
#include "helpers/math.hpp"
#include "options.hpp"
#include <minmax.h>
#include "CTimer.h"
#include "lagcomp.h"

QAngle m_vecAimAngle;
Vector m_vecLagCompAngle;
QAngle m_vecLocalAngle;
int bestHitbox = -1;
int bestPlayer = -1;
std::map<int, LegitBotConfig> m_mapConfig;

Vector LagCompHitbox(C_BasePlayer* ent, int option, C_BaseCombatWeapon* weapon)
{
	float LagCompHitboxFOV = FLT_MAX;
	float hitboxfov = FLT_MAX;

	Vector hitboxCopy;

	auto& settings = g_Options.m_mapAim[weapon->m_Item().m_iItemDefinitionIndex()];

	float fov = -1;

	for (auto& records : TimeWarp::Get().m_Records[ent->EntIndex()].m_vecRecords)
	{
		static std::vector<int> m_intHitboxes{ HITBOX_HEAD, HITBOX_CHEST };

		if (option == 0)
		{
			auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
				records.m_arrHitboxes[HITBOX_HEAD].m_vecMins,
				records.m_arrHitboxes[HITBOX_HEAD].m_vecMaxs,
				records.m_arrHitboxes[HITBOX_HEAD].m_iBone);

			float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitbox);


			if (fov != -1 && fov > settings.m_iFOV)
				continue;

			if (fov != -1 && fov < hitboxfov)
			{
				hitboxfov = fov;
				hitboxCopy = hitbox;
			}
		}

		if (option == 1)
		{
			auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
				records.m_arrHitboxes[HITBOX_NECK].m_vecMins,
				records.m_arrHitboxes[HITBOX_NECK].m_vecMaxs,
				records.m_arrHitboxes[HITBOX_NECK].m_iBone);

			float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitbox);

			if (fov != -1 && fov > settings.m_iFOV)
				continue;

			if (fov != -1 && fov < hitboxfov)
			{
				hitboxfov = fov;
				hitboxCopy = hitbox;
			}
		}

		if (option == 2)
		{
			auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
				records.m_arrHitboxes[HITBOX_PELVIS].m_vecMins,
				records.m_arrHitboxes[HITBOX_PELVIS].m_vecMaxs,
				records.m_arrHitboxes[HITBOX_PELVIS].m_iBone);

			float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitbox);

			if (fov != -1 && fov > settings.m_iFOV)
				continue;

			if (fov != -1 && fov < hitboxfov)
			{
				hitboxfov = fov;
				hitboxCopy = hitbox;
			}
		}

		if (option == 3)
		{
			auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
				records.m_arrHitboxes[HITBOX_STOMACH].m_vecMins,
				records.m_arrHitboxes[HITBOX_STOMACH].m_vecMaxs,
				records.m_arrHitboxes[HITBOX_STOMACH].m_iBone);

			float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitbox);

			if (fov != -1 && fov > settings.m_iFOV)
				continue;

			if (fov != -1 && fov < hitboxfov)
			{
				hitboxfov = fov;
				hitboxCopy = hitbox;
			}
		}
		if (option == 4)
		{
			auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
				records.m_arrHitboxes[HITBOX_CHEST].m_vecMins,
				records.m_arrHitboxes[HITBOX_CHEST].m_vecMaxs,
				records.m_arrHitboxes[HITBOX_CHEST].m_iBone);

			float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitbox);

			if (fov != -1 && fov > settings.m_iFOV)
				continue;

			if (fov != -1 && fov < hitboxfov)
			{
				hitboxfov = fov;
				hitboxCopy = hitbox;
			}
		}

		if (option == 5)
		{
			for (auto& hitboxes : m_intHitboxes)
			{
				auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
					records.m_arrHitboxes[hitboxes].m_vecMins,
					records.m_arrHitboxes[hitboxes].m_vecMaxs,
					records.m_arrHitboxes[hitboxes].m_iBone);

				if (hitbox.IsZero())
					continue;

				float fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), hitbox);

				if (fov > settings.m_iFOV)
					continue;

				if (fov < LagCompHitboxFOV)
				{
					LagCompHitboxFOV = fov;
					hitboxCopy = hitbox;
				}
			}
		}
	}

	return hitboxCopy;

}

Vector ClosestRecords(C_BasePlayer* ent, C_BaseCombatWeapon* weapon)
{
	float closestFOV = FLT_MAX;

	auto& settings = g_Options.m_mapAim[weapon->m_Item().m_iItemDefinitionIndex()];

	float fov = -1;
	if (LagCompHitbox(ent, settings.m_iHitbox, weapon).IsValid())
	{
		fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), LagCompHitbox(ent, settings.m_iHitbox, weapon));
	}

	if (fov != -1 && fov < settings.m_iFOV)
	{
		if (fov != -1 && fov < closestFOV)
		{
			closestFOV = fov;
			m_vecLagCompAngle = LagCompHitbox(ent, settings.m_iHitbox, weapon);
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

void PickUserHitbox(C_BasePlayer* ent, int option)
{
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

	auto& settings = g_Options.m_mapAim[itemdef];

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

		PickUserHitbox(ent, settings.m_iHitbox);

		if (!g_LocalPlayer->CanSeePlayer(ent, ent->GetHitboxPos(bestHitbox)))
			continue;

		float fov = -1;
		Vector punch = g_LocalPlayer->m_aimPunchAngle().ToVector();
		if (!settings.m_bTargetBacktrack)
			fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), ent->GetHitboxPos(bestHitbox));
		if (settings.m_bTargetBacktrack && ClosestRecords(ent, weapon).IsValid() && m_vecLagCompAngle.IsValid())
			fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), ClosestRecords(ent, weapon) + punch);

		if (fov != -1 && fov > settings.m_iFOV)
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

		float fov = -1;
		Vector punch = g_LocalPlayer->m_aimPunchAngle().ToVector();

		if(!settings.m_bTargetBacktrack)
			fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), ent->GetHitboxPos(bestHitbox));
		if(settings.m_bTargetBacktrack && ClosestRecords(ent, weapon).IsValid() && m_vecLagCompAngle.IsValid())
			fov = Math::get_fov(m_vecLocalAngle + g_LocalPlayer->m_aimPunchAngle(), g_LocalPlayer->GetEyePos(), ClosestRecords(ent, weapon) + punch);

		if ((settings.m_fFlashTolerance * 2.55f) < g_LocalPlayer->FlashDuration())
			return;

		if (fov != -1 && fov > settings.m_iFOV)
			return;

		Vector vecTargetHitboxPos = ent->GetHitboxPos(bestHitbox);
		if (!settings.m_bIgnoreSmoke && g_LocalPlayer->IsBehindSmoke(vecTargetHitboxPos))
			return;

		if (settings.m_bIgnoreJumping && !(ent->m_fFlags() & FL_ONGROUND))
			return;

		if(!settings.m_bTargetBacktrack)
		cmd->viewangles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), ent->GetHitboxPos(bestHitbox)); 
		if(settings.m_bTargetBacktrack && ClosestRecords(ent, weapon).IsValid() && m_vecLagCompAngle.IsValid())
			cmd->viewangles = Math::CalcAngle(g_LocalPlayer->GetEyePos(), ClosestRecords(ent, weapon) + punch);

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