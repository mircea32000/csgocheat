#include "lagcomp.h"
#include <minmax.h>
#include "helpers/math.hpp"
#include "options.hpp"

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) ( g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )


template<class T, class U>
T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;

	if (in >= high)
		return high;

	return in;
}

float GetLerpTime()
{
	int ud_rate = g_CVar->FindVar("cl_updaterate")->GetInt();
	ConVar* min_ud_rate = g_CVar->FindVar("sv_minupdaterate");
	ConVar* max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");

	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetInt();

	float ratio = g_CVar->FindVar("cl_interp_ratio")->GetFloat();

	if (ratio == 0)
		ratio = 1.0f;

	float lerp = g_CVar->FindVar("cl_interp")->GetFloat();
	ConVar* c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	ConVar* c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, (ratio / ud_rate));
}

void TimeWarp::UpdateRecords(int i)
{
	C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
	LagRecord_Struct record;

	record.m_fSimtime = pEntity->m_flSimulationTime();
	record.m_vecHitboxPos = pEntity->GetHitboxPos(0);
	record.m_vecHeadPos = pEntity->GetBonePos(8);
	record.m_vecOrigin = pEntity->m_vecOrigin();
	pEntity->SetupBones(record.m_Matrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_GlobalVars->curtime); //memory leak? guess we'll find out

	m_vecRecords.push_back(std::move(record));
}

void TimeWarp::DeleteInvalidRecords()
{
	for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++)
	{
		int iSize = m_vecRecords.size();

		m_vecRecords.erase
		(
			std::remove_if
			(
				m_vecRecords.begin(),
				m_vecRecords.end(),
				[&](LagRecord_Struct& record) -> bool
				{
					return !IsTimeValid(record.m_fSimtime);
				}
			),
			m_vecRecords.end()
		 );
	}
}


bool TimeWarp::IsTimeValid(float flTime)
{
	float correct = 0.f;

	correct += g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
	correct += TICKS_TO_TIME(TIME_TO_TICKS(GetLerpTime()));

	static ConVar* sv_maxunlag = g_CVar->FindVar("sv_maxunlag");
	correct = std::clamp(correct, 0.f, sv_maxunlag->GetFloat());

	float deltaTime = correct - (g_GlobalVars->curtime - flTime);

	return (std::abs(deltaTime));
}

void TimeWarp::StoreRecords(CUserCmd* cmd)
{
	if (!g_Options.misc_backtrack) return;

	int bestTargetIndex = -1;
	float bestFov = FLT_MAX;

	if (!g_LocalPlayer->IsAlive())
		return;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto pEntity = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
		if (!pEntity || !g_LocalPlayer) continue;
		if (!pEntity->IsPlayer()) continue;
		if (pEntity == g_LocalPlayer) continue;
		if (pEntity->IsDormant()) continue;
		if (!pEntity->IsAlive()) continue;
		if (pEntity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) continue;
		Vector HitboxPos = pEntity->GetHitboxPos(0);

		UpdateRecords(i);

		Vector ViewDir;
		Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * 2.f), ViewDir);
		float FOVDistance = Math::DistancePointToLine(HitboxPos, g_LocalPlayer->GetEyePos(), ViewDir); //yes i can use the hitbox pos from the records but i cant be fucked rn

		if (bestFov > FOVDistance)
		{
			bestFov = FOVDistance;
			bestTargetIndex = i;
		}
	}
}

void TimeWarp::DoBackTrack(CUserCmd* cmd)
{
	if (!g_Options.misc_backtrack) return;

	float bestFov = FLT_MAX;

	if (!g_LocalPlayer->IsAlive())
		return;

	float bestTargetSimTime = -1;
	if (bestTargetIndex != -1)
	{
		float tempFloat = FLT_MAX;
		Vector ViewDir;
		Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * 2.f), ViewDir);
		for (const auto& records : m_vecRecords)
		{
			float tempFOVDistance = Math::DistancePointToLine(records.m_vecHitboxPos, g_LocalPlayer->GetEyePos(), ViewDir);
			if (tempFloat > tempFOVDistance && records.m_fSimtime > g_LocalPlayer->m_flSimulationTime() - 1)
			{
				if (g_LocalPlayer->CanSeePlayer(static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(bestTargetIndex)), records.m_vecHitboxPos))
				{
					tempFloat = tempFOVDistance;
					bestTargetSimTime = records.m_fSimtime;
				}
			}
		}

		if (IsTimeValid(bestTargetSimTime) && cmd->buttons & IN_ATTACK)
			cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
	}
}
