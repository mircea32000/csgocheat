#include "lagcomp.h"
#include <minmax.h>
#include "helpers/math.hpp"
#include "options.hpp"
#include <algorithm>
#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) ( g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

void debug_draw_record( LagRecord_Struct* rec)
{

	static ConVar* sv_showlagcompensation_duration = g_CVar->FindVar("sv_showlagcompensation_duration");
	static ConVar* sv_showlagcompensation = g_CVar->FindVar("sv_showlagcompensation");
	if (sv_showlagcompensation->GetInt())
	{
		float duration = sv_showlagcompensation_duration->GetFloat();

		if (duration <= 0)
		{
			duration = -1.f;
		}
		for (auto& e : rec->m_arrHitboxes)
		{
			if (e.m_flRadius == -1.0f)
			{
				Vector pos;
				QAngle angle;
				Math::MatrixAngles(rec->m_Matrix[e.m_iBone], angle, pos);

				g_DebugOverlay->AddBoxBullshit(pos, e.m_vecMins, e.m_vecMaxs, angle, 255, 0, 0, 192, duration);
			}
			else
			{
				Vector 
					min{ Math::VectorTransform(e.m_vecMins, rec->m_Matrix[e.m_iBone]) },
			      	max { Math::VectorTransform(e.m_vecMaxs, rec->m_Matrix[e.m_iBone]) };

				g_DebugOverlay->DrawPillBullshit(min, max, e.m_flRadius, 255, 0, 0, 192, duration, 1, 1);
			}
		}
	}
}

float GetLerpTime()
{
	static ConVar* cl_interp = g_CVar->FindVar("cl_interp");
	static ConVar* cl_updaterate = g_CVar->FindVar("cl_updaterate");
	static ConVar* cl_interp_ratio = g_CVar->FindVar("cl_interp_ratio");
	static ConVar* sv_maxupdaterate = g_CVar->FindVar("sv_maxupdaterate");
	static ConVar* sv_minupdaterate = g_CVar->FindVar("sv_minupdaterate");
	static ConVar* sv_client_min_interp_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	static ConVar* sv_client_max_interp_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");
	float Interp = cl_interp->GetFloat();
	float UpdateRate = cl_updaterate->GetFloat();
	int InterpRatio = static_cast<float>(cl_interp_ratio->GetInt());
	int MaxUpdateRate = static_cast<float>(sv_maxupdaterate->GetInt());
	int MinUpdateRate = static_cast<float>(sv_minupdaterate->GetInt());
	float ClientMinInterpRatio = sv_client_min_interp_ratio->GetFloat();
	float ClientMaxInterpRatio = sv_client_max_interp_ratio->GetFloat();

	if (ClientMinInterpRatio > InterpRatio)
		InterpRatio = ClientMinInterpRatio;

	if (InterpRatio > ClientMaxInterpRatio)
		InterpRatio = ClientMaxInterpRatio;

	if (MaxUpdateRate <= UpdateRate)
		UpdateRate = MaxUpdateRate;

	if (MinUpdateRate > UpdateRate)
		UpdateRate = MinUpdateRate;

	float v20 = InterpRatio / UpdateRate;
	if (v20 <= Interp)
		return Interp;
	else
		return v20;
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
		auto record = m_Records[pEntity->EntIndex()].m_vecRecords.back();

		auto hitbox = Math::CalculateHitboxFromMatrix(record.m_Matrix,
			record.m_arrHitboxes[HITBOX_HEAD].m_vecMins,
			record.m_arrHitboxes[HITBOX_HEAD].m_vecMaxs,
			record.m_arrHitboxes[HITBOX_HEAD].m_iBone);

		Vector ViewDir;
		Math::AngleVectors(cmd->viewangles + (g_LocalPlayer->m_aimPunchAngle() * 2.f), ViewDir);
		float FOVDistance = Math::DistancePointToLine(hitbox, g_LocalPlayer->GetEyePos(), ViewDir);

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
	LagRecord_Struct* shit;
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
			if (tempFloat > tempFOVDistance)// && records.m_fSimtime > g_LocalPlayer->m_flSimulationTime() - 1)
			{
				if (g_LocalPlayer->CanSeePlayer(static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(bestTargetIndex)), hitbox))
				{
					tempFloat = tempFOVDistance;
					bestTargetSimTime = records.m_fSimtime;	
					shit = &records;
				}
			}
		}

		if (IsTimeValid(bestTargetSimTime) && cmd->buttons & IN_ATTACK)
		{
			cmd->tick_count = TIME_TO_TICKS(bestTargetSimTime) + TIME_TO_TICKS(GetLerpTime());
			if (shit)
			{
				if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
					return;

				if (g_LocalPlayer->m_flNextAttack() > g_GlobalVars->curtime)
					return;

				debug_draw_record(shit);

			}
		}
			
	}
}
