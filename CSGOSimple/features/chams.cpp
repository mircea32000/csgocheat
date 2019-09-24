#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../lagcomp.h"
#include "../drawmodelexecute.hpp"
Chams::Chams()
{
	std::ofstream("csgo\\materials\\simple_regular.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\simple_ignorez.vmt") << R"#("VertexLitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
	std::ofstream("csgo\\materials\\simple_flat.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	std::ofstream("csgo\\materials\\simple_ignorez_reflective.vmt") << R"#("VertexLitGeneric"
{
 "$basetexture" "vgui/white_additive"
 "$ignorez"      "1"
 "$envmap"       "env_cubemap"
 "$normalmapalphaenvmapmask"  "1"
 "$envmapcontrast"             "1"
 "$nofog"        "1"
 "$model"        "1"
 "$nocull"       "0"
 "$selfillum"    "1"
 "$halflambert"  "1"
 "$znearer"      "0"
 "$flat"         "1"
}
)#";

	std::ofstream("csgo\\materials\\simple_regular_reflective.vmt") << R"#("VertexLitGeneric"
{
 "$basetexture" "vgui/white_additive"
 "$ignorez"      "0"
 "$envmap"       "env_cubemap"
 "$normalmapalphaenvmapmask"  "1"
 "$envmapcontrast"             "1"
 "$nofog"        "1"
 "$model"        "1"
 "$nocull"       "0"
 "$selfillum"    "1"
 "$halflambert"  "1"
 "$znearer"      "0"
 "$flat"         "1"
}
)#";

	std::ofstream("csgo\\materials\\simple_flat_ignorez.vmt") << R"#("UnlitGeneric"
{
  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

	materialRegular = g_MatSystem->FindMaterial("simple_regular", TEXTURE_GROUP_MODEL);
	materialRegularIgnoreZ = g_MatSystem->FindMaterial("simple_ignorez", TEXTURE_GROUP_MODEL);
	materialFlatIgnoreZ = g_MatSystem->FindMaterial("simple_flat_ignorez", TEXTURE_GROUP_MODEL);
	materialFlat = g_MatSystem->FindMaterial("simple_flat", TEXTURE_GROUP_MODEL);
	materialMetall = g_MatSystem->FindMaterial("simple_ignorez_reflective", TEXTURE_GROUP_MODEL);
	materialMetallnZ = g_MatSystem->FindMaterial("simple_regular_reflective", TEXTURE_GROUP_MODEL);
}

Chams::~Chams()
{
	std::remove("csgo\\materials\\simple_regular.vmt");
	std::remove("csgo\\materials\\simple_ignorez.vmt");
	std::remove("csgo\\materials\\simple_flat.vmt");
	std::remove("csgo\\materials\\simple_flat_ignorez.vmt");
	std::remove("csgo\\materials\\simple_ignorez_reflective.vmt");
	std::remove("csgo\\materials\\simple_regular_reflective.vmt");
}


void Chams::OverrideMaterial(bool ignoreZ, bool flat, bool wireframe, bool glass, bool metallic, const Color& rgba)
{
	IMaterial* material = nullptr;

	if (flat) {
		if (ignoreZ)
			material = materialFlatIgnoreZ;
		else
			material = materialFlat;
	}
	else {
		if (ignoreZ)
			material = materialRegularIgnoreZ;
		else
			material = materialRegular;
	}

	if (metallic)
	{
		if (ignoreZ)
			material = materialMetall;
		else
			material = materialMetallnZ;
	}
	else
	{
		if (ignoreZ)
			material = materialRegularIgnoreZ;
		else
			material = materialRegular;
	}

	if (glass) {
		material = materialFlat;
		material->AlphaModulate(0.45f);
	}
	else {
		material->AlphaModulate(
			rgba.a() / 255.0f);
	}

	material->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, wireframe);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);

	if (g_Options.chams_gloss && g_Options.chams_player_metalic) {

		Color col = g_Options.color_chams_gloss;

		bool bFound = false;
		auto pVar = materialMetallnZ->FindVar("$envmaptint", &bFound);
		if (bFound)
			(*(void(__thiscall * *)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, col.r() / 255.f, col.g() / 255.f, col.b() / 255.f); //tfw no IMaterialVar class
		g_MdlRender->ForcedMaterialOverride(materialMetallnZ);
	}

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
	//bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

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
				OverrideMaterial(
					true,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,	
					false,
					g_Options.chams_player_metalic,
					clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					false,
					g_Options.chams_player_metalic,
					clr_front);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_player_flat,
					g_Options.chams_player_wireframe,
					g_Options.chams_player_glass,
					g_Options.chams_player_metalic,
					clr_front);
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
				OverrideMaterial(
					true,
					g_Options.chams_arms_flat,				
					g_Options.chams_arms_wireframe,
					false,
					g_Options.chams_player_metalic,
					g_Options.color_chams_arms_occluded);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					false,
					g_Options.chams_player_metalic,
					g_Options.color_chams_arms_visible);
			}
			else {
				OverrideMaterial(
					false,
					g_Options.chams_arms_flat,
					g_Options.chams_arms_wireframe,
					g_Options.chams_arms_glass,
					g_Options.chams_player_metalic,
					g_Options.color_chams_arms_visible);
			}
		}
	}
}