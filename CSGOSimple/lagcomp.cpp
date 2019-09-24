#include "lagcomp.h"
#include <minmax.h>
#include "helpers/math.hpp"
#include "options.hpp"
#include <algorithm>
#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) ( g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

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
		ratio = std::clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

	return max(lerp, (ratio / ud_rate));
}

void TimeWarp::UpdateRecords(int i)
{
	C_BasePlayer* pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

	if (!pEntity || i == g_EngineClient->GetLocalPlayer())
	{
		m_Records[i].m_Mutex.lock();
		m_Records[i].m_vecRecords.clear();
		m_Records[i].m_Mutex.unlock();
	}
	else
	{
		LagRecord_Struct record;

		record.m_fSimtime = pEntity->m_flSimulationTime();
		record.m_vecOrigin = pEntity->m_vecOrigin();
		pEntity->SetupBones(record.m_Matrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, record.m_fSimtime); //memory leak? guess we'll find out
         
		auto model = pEntity->GetModel();
		if (!model)
			return;

		auto hdr = g_MdlInfo->GetStudiomodel(model);
		if (!hdr)
			return;

		auto set = hdr->GetHitboxSet(0);
		if (!set)
			return;

		bool error = false;

		for (int i{}; i < HITBOX_MAX; i++)
		{
			auto bbox = set->GetHitbox(i);
			if (!bbox)
			{
				error = true;
				return;
			}
			record.m_arrHitboxes[i].m_iBone = bbox->bone;
			record.m_arrHitboxes[i].m_vecMins = bbox->bbmin;
			record.m_arrHitboxes[i].m_vecMaxs = bbox->bbmax;
			record.m_arrHitboxes[i].m_flRadius = bbox->m_flRadius;
		}

		if (pEntity->IsAlive() && !pEntity->IsDormant())
		{
			m_Records[i].m_Mutex.lock();
			if(!error)
			m_Records[i].m_vecRecords.emplace_back(std::move(record));
			m_Records[i].m_Mutex.unlock();
		}
	}


}

void TimeWarp::DeleteInvalidRecords()
{
	for (int i = 1; i <= g_EngineClient->GetMaxClients(); i++)
	{
		m_Records[i].m_Mutex.lock();

		int iSize = m_Records[i].m_vecRecords.size();

		m_Records[i].m_vecRecords.erase
		(
			std::remove_if
			(
				m_Records[i].m_vecRecords.begin(),
				m_Records[i].m_vecRecords.end(),
				[&](LagRecord_Struct& record) -> bool
				{
					int iDelta = std::addressof(record) - std::addressof(m_Records[i].m_vecRecords[0]) + 1;
					if (iDelta == iSize)
					{
						return false;
					}

					return !IsTimeValid(record.m_fSimtime);
				}
			),
			m_Records[i].m_vecRecords.end()
		 );

		m_Records[i].m_Mutex.unlock();
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

	return (std::abs(deltaTime) <= 0.2f);
}

void TimeWarp::StoreRecords(CUserCmd* cmd)
{
	if (!g_Options.misc_backtrack) return;

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
		if (pEntity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		{
			m_Records[i].m_Mutex.lock();
			m_Records[i].m_vecRecords.clear();
			m_Records[i].m_Mutex.unlock();

			continue;
		}
		Vector HitboxPos = pEntity->GetHitboxPos(0);

		Vector ViewDir;
		Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * 2.f), ViewDir);
		float FOVDistance = Math::DistancePointToLine(HitboxPos, g_LocalPlayer->GetEyePos(), ViewDir);

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
		for (auto& records : m_Records[bestTargetIndex].m_vecRecords)
		{
			auto hitbox = Math::CalculateHitboxFromMatrix(records.m_Matrix,
				records.m_arrHitboxes[HITBOX_HEAD].m_vecMins,
				records.m_arrHitboxes[HITBOX_HEAD].m_vecMaxs,
				records.m_arrHitboxes[HITBOX_HEAD].m_iBone);

			float tempFOVDistance = Math::DistancePointToLine(hitbox, g_LocalPlayer->GetEyePos(), ViewDir);
			if (tempFloat > tempFOVDistance && records.m_fSimtime > g_LocalPlayer->m_flSimulationTime() - 1)
			{
				if (g_LocalPlayer->CanSeePlayer(static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(bestTargetIndex)), hitbox))
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
