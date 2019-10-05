#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../drawmodelexecute.hpp"
#include "../lagcomp.h"
#include <string>

Chams::Chams() {
	std::ofstream("csgo\\materials\\antion_reflective.vmt") << R"#("VertexLitGeneric" {
            "$basemapalphaphongmask"    "1"
            "$basetexture"    "vgui/white"
            "$halflambert"    "1"
            "$nocull"    "1"
            "$phong"    "1"
            "$forcephong"    "1"
            "$phongexponent"    "20"
            "$phongboost"    "1.0"
            "$rimlight"    "1"
            "$rimlightboost"    "1"
            "$rimlightexponent"    "10"
            "$envmap"    "env_cubemap"
            "$envmaptint"    "[0 0 0]"
            "$normalmapalphaenvmask"    "1"
		})#";
	std::ofstream("csgo\\materials\\antion_glow.vmt") << R"#("VertexLitGeneric" {
            "$additive" "1"
            "$envmap" "models/effects/cube_white"
            "$envmaptint" "[1 1 1]"
            "$envmapfresnel" "1"
            "$envmapfresnelminmaxexp" "[0 1 2]"
            "$alpha" "0.8"
		})#";
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube", TEXTURE_GROUP_MODEL);
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat", TEXTURE_GROUP_MODEL);
	materialGlass = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_MODEL);
	materialReflective = g_MatSystem->FindMaterial("antion_reflective", TEXTURE_GROUP_MODEL);
	materialGlow = g_MatSystem->FindMaterial("antion_glow", TEXTURE_GROUP_MODEL);
}

Chams::~Chams() {
	std::remove("csgo\\materials\\antion_reflective.vmt");
	std::remove("csgo\\materials\\antion_glow.vmt");
}


void Chams::OverrideMaterial(bool ignoreZ, int option, const Color& rgba) {
	IMaterial* material = nullptr;
	bool wireframe = false;

	switch (option)
	{
	case 0:
		material = materialRegular;
		break;
	case 1:
		material = materialFlat;
		break;
	case 2:
		material = materialGlass;
		break;
	case 3:
		material = materialReflective;
		break;
	case 4:
		if (option == 4)
		{
			material = materialRegular;
			wireframe = true;
		}
		else
			wireframe = false;
		break;
	}
	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	g_MdlRender->ForcedMaterialOverride(material);
}


void Chams::OnDrawModelExecute(
	IMatRenderContext* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::vfuncs::mdlrender_hook.get_original<decltype(&Hooks::DME::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	std::string model_name = mdl->szName;
	bool is_weapon = (model_name.find("weapons/v") != std::string::npos && model_name.find("arms") == std::string::npos);

	if (is_player && g_Options.chams_player_enabled) {
		// 
		// Draw player Chams.
		// 
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && ent->IsAlive() && ent->IsPlayer() && !ent->IsDormant() && g_Options.misc_backtrack && g_Options.esp_bt_chams) {
			for (auto& records : TimeWarp::Get().m_Records[ent->EntIndex()].m_vecRecords) //RANGE BASED LOOPS FTW!!!!!!!!!!!
			{
				float record_time = std::abs(g_GlobalVars->curtime - records.m_fSimtime) * 5.f;
				if (records.m_fSimtime && records.m_fSimtime + 1 > g_LocalPlayer->m_flSimulationTime())
				{

					Color color;

					if (g_Options.chams_backtrack_rainbow)
					{
						color.blend(Color(255, 0, 0, 255), Color(255, 255, 255, 255), record_time);
					}
					else
					{
						color = Color::White;
					}

					static IMaterial* mat = materialRegular;
					mat->ColorModulate(color.r() / 255.f, color.g() / 255.f, color.b() / 255.f);
					mat->AlphaModulate(1.f);
					g_MdlRender->ForcedMaterialOverride(mat);
					fnDME(g_MdlRender, 0, ctx, state, info, records.m_Matrix);
					g_MdlRender->ForcedMaterialOverride(nullptr);
				}
			}
		}

		if (ent && g_LocalPlayer && ent->IsAlive()) {
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			if (!enemy && g_Options.chams_player_enemies_only)
				return;

			const auto clr_front = enemy ? g_Options.color_chams_player_enemy_visible : g_Options.color_chams_player_ally_visible;
			const auto clr_back = enemy ? g_Options.color_chams_player_enemy_occluded : g_Options.color_chams_player_ally_occluded;

			if (g_Options.chams_player_ignorez) {
				OverrideMaterial(true, g_Options.chams_type, clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(false, g_Options.chams_type, clr_front);
			}
			else {
				OverrideMaterial(false, g_Options.chams_type, clr_front);
				if (g_Options.chams_type == 3) {

					Color col = g_Options.color_chams_gloss;

					bool bFound = false;
					if (auto pVar = materialReflective->FindVar("$envmaptint", &bFound); bFound)
					{
						(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
					}
					g_MdlRender->ForcedMaterialOverride(materialReflective);
				}
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				if (g_Options.chams_glowoverlay)
				{
					Color col = g_Options.color_chams_glowoverlay;
					IMaterial* mat = nullptr;
					if (mat == nullptr)
						mat = materialGlow;

					bool bFound = false;
					if (auto pVar = mat->FindVar("$envmaptint", &bFound); bFound)
					{
						(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
					}
					g_MdlRender->ForcedMaterialOverride(mat);
				}
			}
		}
	}
	else if (is_sleeve && g_Options.chams_arms_enabled) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Remove sleeves when drawing Chams.
		// 
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		g_MdlRender->ForcedMaterialOverride(material);
	}
	else if (is_arm) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.misc_no_hands) {
			// 
			// No hands.
			// 
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			if (g_Options.chams_arms_ignorez) {
				OverrideMaterial(true, g_Options.chams_type_arms, g_Options.color_chams_arms_occluded);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(false, g_Options.chams_type_arms, g_Options.color_chams_arms_visible);
			}
			else {
				OverrideMaterial(false, g_Options.chams_type_arms, g_Options.color_chams_arms_visible);
				if (g_Options.chams_type_arms == 3) {

					Color col = g_Options.color_chams_arms_gloss;

					bool bFound = false;
					if (auto pVar = materialReflective->FindVar("$envmaptint", &bFound); bFound)
					{
						(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
					}
					g_MdlRender->ForcedMaterialOverride(materialReflective);
				}
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				if (g_Options.chams_glowoverlayarms)
				{
					Color col = g_Options.color_chams_arms_glowoverlay;
					IMaterial* mat = nullptr;
					if (mat == nullptr)
						mat = materialGlow;

					bool bFound = false;
					if (auto pVar = mat->FindVar("$envmaptint", &bFound); bFound)
					{
						(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
					}
					g_MdlRender->ForcedMaterialOverride(mat);
				}

			}
		}
	}
	else if (is_weapon && g_Options.chams_weapon_enabled)
	{
		OverrideMaterial(false, g_Options.chams_type_weapon, g_Options.color_chams_weapon_visible);
		if (g_Options.chams_type_weapon == 3) {

			Color col = g_Options.color_chams_weapon_gloss;

			bool bFound = false;
			if (auto pVar = materialReflective->FindVar("$envmaptint", &bFound); bFound)
			{
				(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
			}
			g_MdlRender->ForcedMaterialOverride(materialReflective);
		}
		fnDME(g_MdlRender, 0, ctx, state, info, matrix);
		if (g_Options.chams_glowoverlayweapon)
		{
			Color col = g_Options.color_chams_weapon_glowoverlay;
			IMaterial* mat = nullptr;
			if (mat == nullptr)
				mat = materialGlow;

			bool bFound = false;
			if (auto pVar = mat->FindVar("$envmaptint", &bFound); bFound)
			{
				(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
			}
			g_MdlRender->ForcedMaterialOverride(mat);
		}
	}
}