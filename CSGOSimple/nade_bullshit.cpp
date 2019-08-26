#include "nade_bullshit.h"
#include "helpers/math.hpp"
#include "render.hpp"
#include "options.hpp"
#define PI 3.14159265358979323846f

void CNadePred::predict(CUserCmd* ucmd) {
	//	readability
	constexpr float restitution = 0.45f;
	constexpr float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	constexpr float velocity = 403.0f * 0.9f;

	float step, gravity, new_velocity, unk01;
	int index{}, grenade_act{ 1 };
	Vector pos, thrown_direction, start, eye_origin;
	QAngle angles, thrown;

	//	first time setup
	static auto sv_gravity = g_CVar->FindVar("sv_gravity");

	//	calculate step and actual gravity value
	gravity = sv_gravity->GetFloat() / 8.0f;
	step = g_GlobalVars->interval_per_tick;

	//	get local view and eye origin
	eye_origin = g_LocalPlayer->GetEyePos();
	angles = ucmd->viewangles;

	//	copy current angles and normalise pitch
	thrown = angles;

	if (thrown.pitch < 0) {
		thrown.pitch = -10 + thrown.pitch * ((90 - 10) / 90.0f);
	}
	else {
		thrown.pitch = -10 + thrown.pitch * ((90 + 10) / 90.0f);
	}

	//	find out how we're throwing the grenade
	auto primary_attack = ucmd->buttons & IN_ATTACK;
	auto secondary_attack = ucmd->buttons & IN_ATTACK2;

	if (primary_attack && secondary_attack) {
		grenade_act = ACT_LOB;
	}
	else if (secondary_attack) {
		grenade_act = ACT_DROP;
	}

	//	apply 'magic' and modulate by velocity
	unk01 = power[grenade_act];

	unk01 = unk01 * 0.7f;
	unk01 = unk01 + 0.3f;

	new_velocity = velocity * unk01;

	//	here's where the fun begins
	Math::AngleVectors(thrown, thrown_direction);

	start = eye_origin + thrown_direction * 16.0f;
	thrown_direction = (thrown_direction * new_velocity) + g_LocalPlayer->m_vecVelocity();

	//	let's go ahead and predict
	for (auto time = 0.0f; index < 500; time += step) {
		pos = start + thrown_direction * step;

		//	setup trace
		trace_t trace;
		CTraceFilterSkipEntity filter(g_LocalPlayer);

		g_EngineTrace->TraceRay(Ray_t(start, pos), MASK_SHOT_BRUSHONLY, &filter, &trace);
		//	modify path if we have hit something
		if (trace.fraction != 1.0f) {
			thrown_direction = trace.plane.normal * -2.0f * thrown_direction.Dot(trace.plane.normal) + thrown_direction;

			thrown_direction *= restitution;

			pos = start + thrown_direction * trace.fraction * step;

			time += (step * (1.0f - trace.fraction));
		}

		//	check for detonation
		auto detonate = detonated(g_LocalPlayer->m_hActiveWeapon(), time, trace);

		//	emplace nade point
		_points.at(index++) = c_nadepoint(start, pos, trace.fraction != 1.0f, true, trace.plane.normal, detonate);
		start = pos;

		//	apply gravity modifier
		thrown_direction.z -= gravity * trace.fraction * step;

		if (detonate) {
			break;
		}
	}

	//	invalidate all empty points and finish prediction
	for (auto n = index; n < 500; ++n) {
		_points.at(n).m_valid = false;
	}

	_predicted = true;
}

bool CNadePred::detonated(C_BaseCombatWeapon* weapon, float time, trace_t& trace) {
	if (!weapon) {
		return true;
	}

	//	get weapon item index
	const auto index = weapon->m_Item().m_iItemDefinitionIndex();

	switch (index) {
		//	flash and HE grenades only live up to 2.5s after thrown
	case 43:
	case 44:
		if (time > 2.5f) {
			return true;
		}
		break;

		//	fire grenades detonate on ground hit, or 3.5s after thrown
	case WEAPON_MOLOTOV:
	case 48:
		if (trace.fraction != 1.0f && trace.plane.normal.z > 0.7f || time > 3.5f) {
			return true;
		}
		break;

		//	decoy and smoke grenades were buggy in prediction, so i used this ghetto work-around
	case WEAPON_DECOY:
	case 45:
		if (time > 2.5f) {
			return true;
		}
		break;
	}

	return false;
}

void CNadePred::trace(CUserCmd* ucmd) {
	if (!g_Options.esp_grenade_prediction)
		return;

	if (!(ucmd->buttons & IN_ATTACK) && !(ucmd->buttons & IN_ATTACK2)) {
		_predicted = false;
		return;
	}

	const static std::vector< int > nades{
		WEAPON_FLASHBANG,
		WEAPON_SMOKEGRENADE,
		WEAPON_HEGRENADE,
		WEAPON_MOLOTOV,
		WEAPON_DECOY,
		WEAPON_INCGRENADE
	};

	//	grab local weapon
	auto weapon = g_LocalPlayer->m_hActiveWeapon();

	if (!weapon) {
		return;
	}

	if (std::find(nades.begin(), nades.end(), weapon->m_Item().m_iItemDefinitionIndex()) != nades.end()) {
		return predict(ucmd);
	}

	_predicted = false;
}

void CNadePred::draw() {
	if (!g_Options.esp_grenade_prediction)
		return;

	if (!g_EngineClient->IsInGame() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	auto draw_3d_dotted_circle = [](Vector position, float points, float radius) {
		float step = PI * 2.0f / points;
		Vector prev_2d;

		for (float a = 0; a < PI * 2.0f; a += step) {
			Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);

			Vector start2d;
			if (Math::WorldToScreen(start, start2d))
			{
				if (prev_2d.IsZero())
				{
					prev_2d = start2d;
				}
				Render::Get().RenderLine(prev_2d.x, prev_2d.y, start2d.x, start2d.y, Color::White);
				prev_2d = start2d;
			}
		}
	};

	Vector start, end;

	//	draw nade path
	if (_predicted) {
		for (auto& p : _points) {
			if (!p.m_valid) {
				break;
			}

			if (Math::WorldToScreen(p.m_start, start) && Math::WorldToScreen(p.m_end, end)) {
				//	draw line
				//g_renderer.line(OSHColor::FromRGB(0, 125, 255), start.x, start.y, end.x, end.y);
				Render::Get().RenderLine(start.x, start.y, end.x, end.y, Color(0,125,255));
				//	draw small box if detonated or hit a wall
				if (p.m_detonate || p.m_plane) {
					Render::Get().RenderBoxFilled(start.x - 2, start.y - 2, (start.x - 2) + 5, (start.y - 2) + 5, p.m_detonate ? Color(255, 0, 0) : Color::White);
				
				}

				if (p.m_detonate)
					draw_3d_dotted_circle(p.m_end, 700, 150);
			}
		}
	}
}