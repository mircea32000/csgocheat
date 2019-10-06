#include "Hitmarker.h"
#include <chrono>
#include "helpers/math.hpp"
#include "render.hpp"
#include "options.hpp"

#pragma comment(lib, "Winmm")

void CHitmarker::Init()
{
	g_GameEvents->AddListener(this, "player_hurt", false);
	g_GameEvents->AddListener(this, "bullet_impact", false);
}

C_BasePlayer* CHitmarker::GetPlayerFromId(int uid)
{
	int index = g_EngineClient->GetPlayerForUserID(uid);
	return (C_BasePlayer*)g_EntityList->GetClientEntity(index);
}

int CHitmarker::GetEventDebugID()
{
	return 0x2A;
}
void CHitmarker::Paint()
{
	if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() || !g_LocalPlayer)
	{
		if (!m_vecImpacts.empty()) m_vecImpacts.clear();
		if (!m_vecHitmarkers.empty()) m_vecHitmarkers.clear();
		return;
	}

	unsigned int EpochMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	long long time = EpochMS;

	std::vector<Hitmarkerinfo>::iterator iter;
	for (iter = m_vecHitmarkers.begin(); iter != m_vecHitmarkers.end();)
	{
		bool expired = time > iter->impact.time + 2000;
		static int alphaint = 255 / 50;
		if (expired)
			iter->alpha -= alphaint;
		if (expired && iter->alpha <= 0)
		{
			iter = m_vecHitmarkers.erase(iter);
			continue;
		}
		
		Vector pos3D = Vector(iter->impact.x, iter->impact.y, iter->impact.z); 
		Vector pos2D;
		if (!Math::WorldToScreen(pos3D, pos2D))
		{
			++iter;
			continue;
		}
		Color c = Color(255,255,255, iter->alpha);
		int lineSize = g_Options.esp_hitmarker_size;

		Render::Get().RenderLine(pos2D.x - lineSize, pos2D.y - lineSize, pos2D.x - (lineSize / 4), pos2D.y - (lineSize / 4), c);
		Render::Get().RenderLine(pos2D.x - lineSize, pos2D.y + lineSize, pos2D.x - (lineSize / 4), pos2D.y + (lineSize / 4), c);
		Render::Get().RenderLine(pos2D.x + lineSize, pos2D.y - lineSize, pos2D.x + (lineSize / 4), pos2D.y - (lineSize / 4), c);
		Render::Get().RenderLine(pos2D.x + lineSize, pos2D.y + lineSize, pos2D.x + (lineSize / 4), pos2D.y + (lineSize / 4), c);

		++iter;
	}
}

void CHitmarker::FireGameEvent(IGameEvent* event)
{
	if (!event || !g_LocalPlayer)
		return;

	if (!strcmp(event->GetName(), "player_hurt"))
		PlayerHurt(event);
    

	if (!strcmp(event->GetName(), "bullet_impact"))
		BulletImpact(event);
}

void CHitmarker::PlayerHurt(IGameEvent* event)
{
	C_BasePlayer* attacker = GetPlayerFromId(event->GetInt("attacker"));
	C_BasePlayer* victim = GetPlayerFromId(event->GetInt("userid"));

	if (!attacker || !victim || attacker != g_LocalPlayer)
		return;

	Vector origin = victim->m_vecOrigin();
	ImpactInfo bestImpact;
	float bestImpactDist = -1.f;
	unsigned int EpochMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	long long time = EpochMS;
	std::vector<ImpactInfo>::iterator iter;
	for (iter = m_vecImpacts.begin(); iter != m_vecImpacts.end();)
	{
		if (time > iter->time + 25)
		{
			iter = m_vecImpacts.erase(iter);
			continue;
		}

		Vector impactPos = Vector(iter->x, iter->y, iter->z);
		float distance = impactPos.DistTo(origin);
		if (distance < bestImpactDist || bestImpactDist == -1.f)
		{
			bestImpactDist = distance;
			bestImpact = *iter;
		}
		++iter;
	}

	if (bestImpactDist == -1.f)
		return;

	Hitmarkerinfo info;
	info.impact = bestImpact;
	info.alpha = 255;
	m_vecHitmarkers.push_back(info);
}
void CHitmarker::BulletImpact(IGameEvent* event)
{
	C_BasePlayer* ent = GetPlayerFromId(event->GetInt("userid"));

	if (!ent || ent != g_LocalPlayer)
		return;

	ImpactInfo info;
	info.x = event->GetFloat("x");
	info.y = event->GetFloat("y");
	info.z = event->GetFloat("z");
	unsigned int EpochMS = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	info.time = EpochMS;
	m_vecImpacts.push_back(info);
}
CHitmarker* g_Hitmarker = new CHitmarker();