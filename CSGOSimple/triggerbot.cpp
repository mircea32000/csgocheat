#include "legitbot.h"
#include "lagcomp.h"
#include "helpers/math.hpp"
#include "options.hpp"
#include <optional>
static std::optional<Vector> GetIntersectionPoint(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, float radius)
{
	auto sphereRayIntersection = [start, end, radius](auto&& center) -> std::optional<Vector>
	{
		auto direction = (end - start).Normalized();

		auto q = center - start;
		auto v = q.Dot(direction); 
		auto d = radius * radius - (q.LengthSqr() - v * v);

		if (d < FLT_EPSILON)
			return {};

		return start + direction * (v - std::sqrt(d));
	};

	auto delta = (maxs - mins).Normalized();
	for (size_t i{}; i < std::floor(mins.DistTo(maxs)); ++i)
	{
		if (auto intersection = sphereRayIntersection(mins + delta * float(i)); intersection)
			return intersection;
	}

	if (auto intersection = sphereRayIntersection(maxs); intersection)
		return intersection;

	return {};
}

static std::optional<LagRecord_Struct> GetTargetRecord(C_BasePlayer* entity, const Vector& start, const Vector& end)
{
	auto hdr = g_MdlInfo->GetStudiomodel(entity->GetModel());
	if (!hdr)
		return {};

	auto set = hdr->GetHitboxSet(0);
	if (!set)
		return {};

	for (auto& record : TimeWarp::Get().m_Records[entity->EntIndex()].m_vecRecords)
	{
		for (size_t i{}; i < set->numhitboxes; ++i)
		{
			auto hitbox = set->GetHitbox(i);
			if (!hitbox || hitbox->m_flRadius == -1.f)
				continue;

			Vector mins, maxs;
			Math::VectorTransform(hitbox->bbmin, record.m_Matrix[hitbox->bone], mins);
			Math::VectorTransform(hitbox->bbmax, record.m_Matrix[hitbox->bone], maxs);

			if (auto intersection = GetIntersectionPoint(start, end, mins, maxs, hitbox->m_flRadius); intersection)
			{

					return record;
			}
		}
	}

	return {};
}

void Legit::TriggerbotBacktrack::Do(CUserCmd* cmd)
{
	auto local = g_LocalPlayer;
	auto weapon = local->m_hActiveWeapon();
	auto data = weapon->GetCSWeaponData();

	QAngle viewangles;
	Vector start = local->GetEyePos();
	Vector end;

	g_EngineClient->GetViewAngles(viewangles);

	Math::AngleVectors(viewangles + local->m_aimPunchAngle(), end);
	end = start + (end * data->flRange);

	for (size_t i{}; i <= g_GlobalVars->maxClients; ++i)
	{
		auto entity = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (!entity || !(entity->m_iTeamNum() != g_LocalPlayer->m_iTeamNum()) || !entity->IsAlive() || entity->IsDormant() || entity->m_bGunGameImmunity())
			continue;

		if (auto record = GetTargetRecord(entity, start, end); record)
		{
			if (!data->bFullAuto)
			{
				if (cmd->tick_count % 2)	cmd->buttons |= IN_ATTACK;
				else						cmd->buttons &= ~IN_ATTACK;
			}
			else if(GetAsyncKeyState(VK_MENU)) cmd->buttons |= IN_ATTACK;

			//backtrack your target entity

			break;
		}
	}
}

void Legit::Triggerbot::Do(CUserCmd* cmd)
{
	if (!g_EngineClient->IsConnected() && !g_EngineClient->IsInGame())
		return;

	if (!g_LocalPlayer)
		return;

	if (!g_LocalPlayer->IsAlive())
		return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon)
		return;

	if (weapon->IsGrenade() || weapon->IsKnife())
		return;

	if (weapon->m_iClip1() <= 0)
		return;

	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
		return;

	int itemdef = weapon->m_Item().m_iItemDefinitionIndex();

	auto& settings = g_Options.m_mapAim[itemdef];

	Vector src, dst, forward;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	QAngle viewangle = cmd->viewangles;

	viewangle += g_LocalPlayer->m_aimPunchAngle() * 2.f;

	Math::AngleVectors(viewangle, forward);
	forward *= weapon->GetCSWeaponData()->flRange;
	filter.pSkip = g_LocalPlayer;
	src = g_LocalPlayer->GetEyePos();
	dst = src + forward;

	ray.Init(src, dst);

	g_EngineTrace->TraceRay(ray, 0x46004003, &filter, &tr);

	if (!tr.hit_entity)
		return;

	int hitgroup = tr.hitgroup;
	bool didHit = false;

	if (settings.m_bAttackHead && hitgroup == HITGROUP_HEAD)
		didHit = true;
	if (settings.m_bAttackChest && hitgroup == HITGROUP_CHEST)
		didHit = true;
	if (settings.m_bAttackStomach && hitgroup == HITGROUP_STOMACH)
		didHit = true;
	if (settings.m_bAttackArms && (hitgroup == HITGROUP_LEFTARM || hitgroup == HITGROUP_RIGHTARM))
		didHit = true;
	if (settings.m_bAttackLegs && (hitgroup == HITGROUP_LEFTLEG || hitgroup == HITGROUP_RIGHTLEG))
		didHit = true;

	if (!settings.m_bAttackEnemies && ((C_BasePlayer*)tr.hit_entity)->m_iTeamNum() != g_LocalPlayer->m_iTeamNum())
		return;

	if (!settings.m_bAttackFriendlies && ((C_BasePlayer*)tr.hit_entity)->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		return;

	if ( ((C_BasePlayer*)tr.hit_entity)->m_iHealth() < 0)//no sir we do not want to triggerbot corpses
	    return;

	if (((C_BasePlayer*)tr.hit_entity)->m_bGunGameImmunity())
		return;

	if (GetAsyncKeyState(VK_MENU))
	{
		float hitchance = 75.f + (settings.m_fTriggerHitchance / 4);
		if (didHit && (!settings.m_bTriggerHitchanceToggle || (1.0f - weapon->m_fAccuracyPenalty()) * 100.f >= hitchance))
			cmd->buttons |= IN_ATTACK;
	}
}