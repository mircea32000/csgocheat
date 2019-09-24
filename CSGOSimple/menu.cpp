#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include "render.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"
#include "parser.h"

// =========================================================
// 
// These are the tabs on the sidebar
// 
// =========================================================
static char* sidebar_tabs[] = {
    "ESP",
    "AIM",
    "MISC",
    "CONFIG"
};

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  50.0f; }

enum {
	TAB_ESP,
	TAB_AIMBOT,
	TAB_MISC,
	TAB_CONFIG
};

namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
    {
        auto clr = ImVec4{
            v->r() / 255.0f,
            v->g() / 255.0f,
            v->b() / 255.0f,
            v->a() / 255.0f
        };

        if(ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
            v->SetColor(clr.x, clr.y, clr.z, clr.w);
            return true;
        }
        return false;
    }
    inline bool ColorEdit3(const char* label, Color* v)
    {
        return ColorEdit4(label, v, false);
    }
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
	auto& style = ImGui::GetStyle();
	bool values[N] = { false };
	values[activetab] = true;
	for (auto i = 0; i < N; ++i) {

		if (ImGui::ButtonGradient(names[i], ImVec2{ w, h })) {
			activetab = i;
		}
		if (sameline && i < N - 1)
			ImGui::SameLine();
	}
	
}

ImVec2 get_sidebar_size()
{
    constexpr float padding = 10.0f;
    constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
    constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();

    return ImVec2{ size_w, ImMax(325.0f, size_h) };
}

int get_fps()
{
    using namespace std::chrono;
    static int count = 0;
    static auto last = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    static int fps = 0;

    count++;

    if(duration_cast<milliseconds>(now - last).count() > 1000) {
        fps = count;
        count = 0;
        last = now;
    }

    return fps;
}

void RenderEspTab()
{
    static char* esp_tab_names[] = { "ESP", "GLOW", "CHAMS" };
	const char* esp_box_types[] = {  "Full", "Corners", "3D" };
	const char* weaponname_types[] = { "Name", "Icons" };

    static int   active_esp_tab = 0;

    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
   // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
   // {
   //     render_tabs(esp_tab_names, active_esp_tab, group_w / _countof(esp_tab_names), 25.0f, true);
   // }
   // ImGui::PopStyleVar();

	int tabs = _countof(esp_tab_names);
	float flW = (ImGui::GetContentRegionAvailWidth() / tabs) - (ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().ItemSpacing.x / tabs);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	if (ImGui::ButtonCustom("Esp", ImVec2(flW + 6.25f, 25), 0, active_esp_tab == 0 ? true : false)) active_esp_tab = 0; ImGui::SameLine();
	if (ImGui::ButtonCustom("Glow", ImVec2(flW + 6.25f, 25), 0, active_esp_tab == 1 ? true : false)) active_esp_tab = 1; ImGui::SameLine();
	if (ImGui::ButtonCustom("Chams", ImVec2(flW + 6.25f, 25), 0, active_esp_tab == 2 ? true : false)) active_esp_tab = 2;

	ImGui::PopStyleVar();


    ImGui::BeginGroupBox("##body_content");
    {
        if(active_esp_tab == 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", g_Options.esp_enabled);
            ImGui::Checkbox("Team check", g_Options.esp_enemies_only);
            ImGui::Checkbox("Boxes", g_Options.esp_player_boxes);
			ImGui::Combo("Type##boxtype", g_Options.esp_box_type, esp_box_types, IM_ARRAYSIZE(esp_box_types));

            ImGui::Checkbox("Names", g_Options.esp_player_names);
            ImGui::Checkbox("Health", g_Options.esp_player_health);
            ImGui::Checkbox("Flags", g_Options.esp_player_flags);
			//ImGui::Checkbox("Render FOV##aimbot", g_Options.esp_draw_fov);
			//ImGui::Checkbox("Render FOV fill##aimbot", g_Options.esp_fov_filled);
            ImGui::Checkbox("Weapon", g_Options.esp_player_weapons);
			ImGui::Combo("Type##weapoin", g_Options.esp_weaponname_type, weaponname_types, IM_ARRAYSIZE(weaponname_types));
			ImGui::Checkbox("Skeleton", g_Options.esp_spooky_shit);

            ImGui::NextColumn();

            ImGui::Checkbox("Crosshair", g_Options.esp_crosshair);
            ImGui::Checkbox("Dropped Weapons", g_Options.esp_dropped_weapons);
            ImGui::Checkbox("Defuse Kit", g_Options.esp_defuse_kit);
            ImGui::Checkbox("Planted C4", g_Options.esp_planted_c4);
			ImGui::Checkbox("Item Esp", g_Options.esp_items);
			ImGui::Checkbox("Snaplines", g_Options.esp_player_snaplines);
			ImGui::Checkbox("Grenade prediction", g_Options.esp_grenade_prediction);
			ImGui::Checkbox("Spectator list", g_Options.esp_spectator_list);
			ImGui::Checkbox("Radar", g_Options.esp_radar);
			ImGui::Checkbox("Remove scope", g_Options.esp_removescope);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit3("Allies Visible", g_Options.color_esp_ally_visible);
            ImGuiEx::ColorEdit3("Enemies Visible", g_Options.color_esp_enemy_visible);
            ImGuiEx::ColorEdit3("Allies Occluded", g_Options.color_esp_ally_occluded);
            ImGuiEx::ColorEdit3("Enemies Occluded", g_Options.color_esp_enemy_occluded);
            ImGuiEx::ColorEdit3("Crosshair", g_Options.color_esp_crosshair);
            ImGuiEx::ColorEdit3("Dropped Weapons", g_Options.color_esp_weapons);
            ImGuiEx::ColorEdit3("Defuse Kit", g_Options.color_esp_defuse);
            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_esp_c4);
			ImGuiEx::ColorEdit3("Skeleton##color", g_Options.color_esp_skeleton);
			ImGuiEx::ColorEdit3("Item Esp", g_Options.color_esp_item);
			ImGuiEx::ColorEdit3("FOV fill", g_Options.color_fov_filled);
			ImGuiEx::ColorEdit3("FOV outline", g_Options.color_fov_outline);
            ImGui::PopItemWidth();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 1) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", g_Options.glow_enabled);
            ImGui::Checkbox("Team check", g_Options.glow_enemies_only);
            ImGui::Checkbox("Players", g_Options.glow_players);
            ImGui::Checkbox("Chickens", g_Options.glow_chickens);
            ImGui::Checkbox("C4 Carrier", g_Options.glow_c4_carrier);
            ImGui::Checkbox("Planted C4", g_Options.glow_planted_c4);
            ImGui::Checkbox("Defuse Kits", g_Options.glow_defuse_kits);
            ImGui::Checkbox("Weapons", g_Options.glow_weapons);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit3("Ally", g_Options.color_glow_ally);
            ImGuiEx::ColorEdit3("Enemy", g_Options.color_glow_enemy);
            ImGuiEx::ColorEdit3("Chickens", g_Options.color_glow_chickens);
            ImGuiEx::ColorEdit3("C4 Carrier", g_Options.color_glow_c4_carrier);
            ImGuiEx::ColorEdit3("Planted C4", g_Options.color_glow_planted_c4);
            ImGuiEx::ColorEdit3("Defuse Kits", g_Options.color_glow_defuse);
            ImGuiEx::ColorEdit3("Weapons", g_Options.color_glow_weapons);
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 2) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 2.9f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::BeginGroupBox("Players");
            {
                ImGui::Checkbox("Enabled", g_Options.chams_player_enabled); ImGui::SameLine();
                ImGui::Checkbox("Team Check", g_Options.chams_player_enemies_only);
				ImGui::Checkbox("Disable Occlusion", g_Options.chams_disable_occlusion); 

                ImGui::Checkbox("Wireframe", g_Options.chams_player_wireframe);
                ImGui::Checkbox("Flat", g_Options.chams_player_flat);
                ImGui::Checkbox("Ignore-Z", g_Options.chams_player_ignorez); ImGui::SameLine();
                ImGui::Checkbox("Glass", g_Options.chams_player_glass);
				ImGui::Checkbox("Metallic", g_Options.chams_player_metalic);
				ImGui::Checkbox("Gloss", g_Options.chams_gloss);
                ImGui::PushItemWidth(110);
                ImGuiEx::ColorEdit4("Ally (Visible)", g_Options.color_chams_player_ally_visible);
				ImGuiEx::ColorEdit4("Gloss", g_Options.color_chams_gloss);
                ImGuiEx::ColorEdit4("Ally (Occluded)", g_Options.color_chams_player_ally_occluded);
                ImGuiEx::ColorEdit4("Enemy (Visible)", g_Options.color_chams_player_enemy_visible);
                ImGuiEx::ColorEdit4("Enemy (Occluded)", g_Options.color_chams_player_enemy_occluded);
                ImGui::PopItemWidth();
            }
            ImGui::EndGroupBox();

            ImGui::NextColumn();

            ImGui::BeginGroupBox("Arms");
            {
                ImGui::Checkbox("Enabled", g_Options.chams_arms_enabled);
                ImGui::Checkbox("Wireframe", g_Options.chams_arms_wireframe);
                ImGui::Checkbox("Flat", g_Options.chams_arms_flat);
                ImGui::Checkbox("Ignore-Z", g_Options.chams_arms_ignorez);
                ImGui::Checkbox("Glass", g_Options.chams_arms_glass);
                ImGui::PushItemWidth(110);
                ImGuiEx::ColorEdit4("Color (Visible)", g_Options.color_chams_arms_visible);
                ImGuiEx::ColorEdit4("Color (Occluded)", g_Options.color_chams_arms_occluded);
                ImGui::PopItemWidth();
            }
            ImGui::EndGroupBox();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        }
    }
    ImGui::EndGroupBox();
}

void RenderMiscTab()
{
	bool placeholder_true = true;

	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::ToggleButton("MISC", &placeholder_true, ImVec2{ group_w, 25.0f });
	ImGui::PopStyleVar();

	ImGui::BeginGroupBox("##body_content");
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
		ImGui::Columns(3, nullptr, false);
		ImGui::SetColumnOffset(1, group_w / 3.0f);
		ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
		ImGui::SetColumnOffset(3, group_w);

		ImGui::Checkbox("Bunny hop", g_Options.misc_bhop);

		ImGui::Checkbox("Third Person", g_Options.misc_thirdperson);
		if (g_Options.misc_thirdperson)
			ImGui::SliderFloat("Distance", g_Options.misc_thirdperson_dist, 0.f, 150.f);
		ImGui::Checkbox("No hands", g_Options.misc_no_hands);
		ImGui::Checkbox("Rank reveal", g_Options.misc_showranks);
		ImGui::Checkbox("Watermark##hc", g_Options.misc_watermark);
		//ImGui::PushItemWidth(-1.0f);
		ImGui::NextColumn();
		ImGui::SliderInt("viewmodel_fov:", g_Options.viewmodel_fov, 68, 120);
		ImGui::SliderFloat("Red", g_Options.mat_ambient_light_r, 0, 1);
		ImGui::SliderFloat("Green", g_Options.mat_ambient_light_g, 0, 1);
		ImGui::SliderFloat("Blue", g_Options.mat_ambient_light_b, 0, 1);


	//	ImGui::BeginGroupBox("##shutyomouf");
		//{
			if (ImGui::ButtonGradient("Save cfg")) {
				Config::Get().Save();
			}
			if (ImGui::ButtonGradient("Load cfg")) {
				Config::Get().Load();
			}
	//	}
		//ImGui::EndGroupBox();

		ImGui::Columns(1, nullptr, false);
		ImGui::PopStyleVar();
	}
	ImGui::EndGroupBox();
}

static std::map<int, const char*> k_weapon_names =
{
{ 7, "AK-47" },
{ 8, "AUG" },
{ 9, "AWP" },
{ 63, "CZ75 Auto" },
{ 1, "Desert Eagle" },
{ 2, "Dual Berettas" },
{ 10, "FAMAS" },
{ 3, "Five-SeveN" },
{ 11, "G3SG1" },
{ 13, "Galil AR" },
{ 4, "Glock-18" },
{ 14, "M249" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 60, "M4A1-S" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 16, "M4A4" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 17, "MAC-10" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 27, "MAG-7" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 33, "MP7" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 33, "MP5" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 34, "MP9" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 28, "Negev" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 35, "Nova" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 32, "P2000" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 36, "P250" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 19, "P90" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 23, "MP5" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 26, "PP-Bizon" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 64, "R8 Revolver" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 29, "Sawed-Off" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 38, "SCAR-20" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 40, "SSG 08" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 39, "SG 553" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 30, "Tec-9" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 24, "UMP-45" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 61, "USP-S" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
{ 25, "XM1014" },//////////////////////////////////////////////////////REMOVE AFTER TESTING
};//////////////////////////////////////////////////////REMOVE AFTER TESTING

void RenderEmptyTab()
{
	static int weapon_index = 7;
	static int selected_aimbot_window = 0;

	ImGui::BeginGroupBox("Weapons", ImVec2(175, 400));
	{
		for (auto weapons : k_weapon_names)
		{
			if (ImGui::Selectable(weapons.second, weapon_index == weapons.first))
				weapon_index = weapons.first;
		}
	}
	ImGui::EndGroupBox();

	ImGui::SameLine();

	float width = (ImGui::GetContentRegionAvailWidth()) - (ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().ItemSpacing.x);
	auto& settings = g_Options.m_mapAim[weapon_index];

	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			if (ImGui::ButtonCustom("Settings##aimbot", ImVec2(275 / 2, 20), 0, selected_aimbot_window == 0)) selected_aimbot_window = 0;
			ImGui::SameLine();
			if (ImGui::ButtonCustom("Triggerbot##aimbot", ImVec2(275 / 2, 20), 0, selected_aimbot_window == 1)) selected_aimbot_window = 1;
		}
		ImGui::EndGroup();

		if (selected_aimbot_window == 0)
		{
			ImGui::BeginGroup();
			{
				ImGui::BeginGroupBox("Aimbot", ImVec2(275, 320));
				{
					const char* aim_hitbox[] = { "Head", "Neck", "Pelvis", "Stomach", "Chest", "Closest" };
					const char* aim_smoothing[] = { "Slow at end", "Constant", "Fast at end" };

					ImGui::Checkbox("Enabled", &settings.m_bEnabled);
					ImGui::Combo("Hitbox", &settings.m_iHitbox, aim_hitbox, IM_ARRAYSIZE(aim_hitbox));
					ImGui::Combo("Smooth type", &settings.m_iSmoothingMethod, aim_smoothing, IM_ARRAYSIZE(aim_smoothing));
					ImGui::SliderInt("FOV:", &settings.m_iFOV, 0, 30);
					ImGui::SliderFloat("Smooth:", &settings.m_fSmooth, 0, 1, "%.3f");
					ImGui::Checkbox("RCS", &settings.m_bRCS);

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.369f, 0.369f, 0.369f, 1.f));
					ImGui::PushFont(g_pDefaultFontSmall);
					ImGui::Text("RCS stands for recoil control system.");
					ImGui::PopFont();
					ImGui::PopStyleColor();

					ImGui::SliderFloat("X axis:", &settings.m_fRCSX, 0, 2, "%.3f");
					ImGui::SliderFloat("Y axis:", &settings.m_fRCSY, 0, 2, "%.3f");
					ImGui::Text("Shoot delay");

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.369f, 0.369f, 0.369f, 1.f));
					ImGui::PushFont(g_pDefaultFontSmall);
					ImGui::Text("As long as you hold left-click");
					ImGui::Text("this delay will shoot when the timer");
					ImGui::Text("ends or we're on target.");//bruh
					ImGui::PopFont();
					ImGui::PopStyleColor();

					ImGui::SliderFloat("##delay", &settings.m_fDelay, 0, 200, "%.3f");

				}
				ImGui::EndGroupBox();

				ImGui::BeginGroupBox("Other", ImVec2(275, 69));//hehe
				{
					ImGui::Checkbox("Backtrack", g_Options.misc_backtrack);
					ImGui::Checkbox("Extend backtrack window", g_Options.fake_latency);

				}
				ImGui::EndGroupBox();
			}
			ImGui::EndGroup();


			ImGui::SameLine();

			ImGui::BeginGroup();
			{
				ImGui::BeginGroupBox("Filters", ImVec2(199, 235));
				{
					ImGui::Checkbox("Attack enemies", &settings.m_bAttackEnemies);
					ImGui::Checkbox("Attack friendlies", &settings.m_bAttackFriendlies);
					ImGui::Checkbox("Target backtrack", &settings.m_bTargetBacktrack);//currently does nothing :(
					ImGui::Checkbox("Ignore jumping", &settings.m_bIgnoreJumping);
					ImGui::Checkbox("Ignore smoke", &settings.m_bIgnoreSmoke);
					ImGui::Text("Flash Tolerance");

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.369f, 0.369f, 0.369f, 1.f));
					ImGui::PushFont(g_pDefaultFontSmall);
					ImGui::Text("Aimbot doesn't aim at target if the");
					ImGui::Text("tolerance is smaller than the flash");
					ImGui::Text("effect.");//bruh
					ImGui::PopFont();
					ImGui::PopStyleColor();

					ImGui::SliderFloat("##tolerance", &settings.m_fFlashTolerance, 0, 100, "%.3f");

				}
				ImGui::EndGroupBox();

				ImGui::BeginGroupBox("Visuals", ImVec2(199, 153));
				{
					ImGui::Checkbox("Render FOV##aimbot", g_Options.esp_draw_fov);

					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.369f, 0.369f, 0.369f, 1.f));
					ImGui::PushFont(g_pDefaultFontSmall);
					ImGui::Text("Renders your field of view, a.k.a where");
					ImGui::Text("within the circle you can shoot.");
					ImGui::PopFont();
					ImGui::PopStyleColor();

					if (g_Options.esp_draw_fov)
					{
						ImGui::Checkbox("Filled FOV circle##aimbot", g_Options.esp_fov_filled);
					}
					ImGui::Checkbox("Backtrack dots", g_Options.esp_bt_dots);
					ImGui::Checkbox("Backtrack models", g_Options.esp_bt_chams);
				}
				ImGui::EndGroupBox();

			}
			ImGui::EndGroup();
		}
		if (selected_aimbot_window == 1)
		{
			ImGui::BeginGroup();
			{
				ImGui::BeginGroupBox("Triggerbot##aimbotxd", ImVec2(275, 320));
				{
					ImGui::Checkbox("Hitchance", &settings.m_bTriggerHitchanceToggle);
					ImGui::SliderFloat("Amount:##hitchance", &settings.m_fTriggerHitchance, 1, 100, "%.3f");
					ImGui::Checkbox("Attack head", &settings.m_bAttackHead);
					ImGui::Checkbox("Attack chest", &settings.m_bAttackChest);
					ImGui::Checkbox("Attack stomach", &settings.m_bAttackStomach);
					ImGui::Checkbox("Attack arms", &settings.m_bAttackArms);
					ImGui::Checkbox("Attack legs", &settings.m_bAttackLegs);
				}
				ImGui::EndGroupBox();
			}
			ImGui::EndGroup();

			ImGui::SameLine();

			ImGui::BeginGroup();
			{
				ImGui::BeginGroupBox("Filters##fortriggfer", ImVec2(199, 235));
				{
					ImGui::Checkbox("Target backtrack##fortrigger", &settings.m_bTriggerTargetBacktrack);
					ImGui::Checkbox("Target enemies##fortrigger", &settings.m_bTriggerTargetEnemies);
					ImGui::Checkbox("Target friendlies##fortrigger", &settings.m_bTriggerTargetFriendlies);
					ImGui::SliderFloat("Flash tolerance##fortrigger", &settings.m_fTriggerFlashTolerance, 0, 100, "%.3f");
				}
				ImGui::EndGroupBox();
			}
			ImGui::EndGroup();
		}
	}
	ImGui::EndGroup();

	
}

ImVec4 GetColorByItemRarity(ItemRarity item)
{
	if (item == ItemRarity::ITEM_RARITY_COMMON)
		return ImVec4{ 0.623f, 0.639f, 0.670f , 1.f};
	if (item == ItemRarity::ITEM_RARITY_UNCOMMON) //100, 150, 225
		return ImVec4{ 0.392f, 0.588f, 0.882f , 1.f };
	if (item == ItemRarity::ITEM_RARITY_RARE) //75, 105, 205
		return ImVec4{ 0.294f, 0.411f, 0.803f , 1.f };
	if (item == ItemRarity::ITEM_RARITY_MYTHICAL) //135, 70, 250
		return ImVec4{ 0.529f, 0.274f, 0.980f , 1.f };
	if (item == ItemRarity::ITEM_RARITY_LEGENDARY) //215, 45, 230
		return ImVec4{ 0.843f, 0.176f, 0.901f , 1.f };
	if (item == ItemRarity::ITEM_RARITY_ANCIENT) //235, 75, 75
		return ImVec4{ 0.921f, 0.294f, 0.294f , 1.f };
	if (item == ItemRarity::ITEM_RARITY_IMMORTAL) //215, 185, 5
		return ImVec4{ 0.843f, 0.725f, 0.019f , 1.f };
}

void RenderConfigTab()
{
	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	bool placeholder_true = true;
	//static int pk = 0;
	//static int paintkit = 0;

	
#ifdef NDEBUG
	ImGui::BeginGroupBox("##nikgga_content", ImVec2(300, 175));
	{
		for (const auto& items : CParser::Get().m_mItems)
		{
			if (items.second.m_vSkins.empty())
				continue;

			bool selected = items.first == g_Options.Skin.current_weapon;

			if (ImGui::Selectable(items.second.m_szDisplayName.c_str(), selected))
			{
				g_Options.Skin.current_weapon = items.first;
			}
		}
	}
	ImGui::EndGroupBox();

	ImGui::SameLine();

	ImGui::BeginGroupBox("##paintkit_content", ImVec2(300, 175));
	{
		if (g_Options.Skin.current_weapon > 0)
		{
			for (const auto& paintkits : CParser::Get().m_mItems[g_Options.Skin.current_weapon].m_vSkins)
			{
				bool selected = paintkits == g_Options.SkinStruct[g_Options.Skin.current_weapon].nFallbackPaintKit;

				ImGui::PushStyleColor(ImGuiCol_Text, GetColorByItemRarity(CParser::Get().m_mPaintKits[paintkits].m_SkinRarity));
				if (ImGui::Selectable(CParser::Get().m_mPaintKits[paintkits].m_szDisplayName.c_str(), selected))
				{
					g_Options.SkinStruct[g_Options.Skin.current_weapon].nFallbackPaintKit = paintkits;
				}
				ImGui::PopStyleColor();


				//ImGui::Text(CParser::Get().m_mPaintKits[paintkits].m_szDisplayName.c_str());
			}
		}
	}
	ImGui::EndGroupBox();
#else

#endif
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}
static int pagetab = 0;


void Menu::Render()
{
	ImGui::GetIO().MouseDrawCursor = _visible;

    if(!_visible)
        return;

    const auto sidebar_size = get_sidebar_size();
    static int active_sidebar_tab = 0;

    //ImGui::PushStyle(_style);

    ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiSetCond_Once);
    ImGui::SetNextWindowSize(ImVec2{ 700, 500 }, ImGuiSetCond_Once);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
	if (ImGui::Begin("ANTION for Counter-Strike: Global Offensive", false,
		&_visible,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize )) {
	ImGui::PopStyleColor();


	     int tabs = _countof(sidebar_tabs);
		 float flW = (ImGui::GetContentRegionAvailWidth() / tabs) - (ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().ItemSpacing.x / tabs);

		 ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		 if (ImGui::ButtonCustomIcon("Visuals","C", ImVec2(flW + 6.25f, 43), 0 , pagetab == 0 ? true : false)) pagetab = 0; ImGui::SameLine();
		 if (ImGui::ButtonCustomIcon("Aimbot","B", ImVec2(flW + 6.25f, 43), 0, pagetab == 1 ? true : false)) pagetab = 1; ImGui::SameLine();
		 if (ImGui::ButtonCustomIcon("Misc", "D" ,ImVec2(flW + 6.25f, 43), 0, pagetab == 2 ? true : false)) pagetab = 2; ImGui::SameLine();
		 if (ImGui::ButtonCustomIcon("Skinchanger","E", ImVec2(flW + 6.5f, 43), 0, pagetab == 3 ? true : false)) pagetab = 3;
		 ImGui::PopStyleVar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
		ImGui::Spacing();
		ImGui::Button("##ghettofix1", ImVec2(0, 0));
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::SameLine();
		float breuh = (ImGui::GetContentRegionAvailWidth()) - (ImGui::GetStyle().ItemSpacing.x - ImGui::GetStyle().ItemSpacing.x );
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);

		ImGui::BeginGroupBoxInvisible("##body", ImVec2(665, sidebar_size.y + 85), false);
		ImGui::PopStyleVar();
        if(pagetab == TAB_ESP) {
            RenderEspTab();
        } else if(pagetab == TAB_AIMBOT) {
            RenderEmptyTab();
        } else if(pagetab == TAB_MISC) {
            RenderMiscTab();
        } else if(pagetab == TAB_CONFIG) {
            RenderConfigTab();
        }
        ImGui::EndGroupBox();
		ImGui::PopStyleVar();

		ImGui::PopStyleVar();

        ImGui::End();
    }
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.WindowPadThickness = 4.f;
	_style.WindowBorderSize = 0.f;
	_style.ChildRounding = 2.f;
	_style.FrameRounding = 6.f;
	_style.WindowRounding = 3.f;
	_style.WindowPadding = ImVec2(0, 0);
	_style.GrabRounding = 12.f;
	_style.ScrollbarSize = 7.f;
	_style.Colors[ImGuiCol_Text] = ImVec4(0.f, 0.f, 0.f, 1.f);
	_style.Colors[ImGuiCol_TitleBg] = ImVec4(0.847f, 0.141f, 0.141f, 0.313f);
	_style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.847f, 0.141f, 0.141f, 0.313f);
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(1.f, 1.f, 1.f, 1.f);
	_style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.929, 0.929, 0.929, 1);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	_style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	_style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.78f, 0.78f, 0.67f);
	_style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.929, 0.929, 0.929, 1.f);
	_style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.f);
	_style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.f, 1.f, 1.f, 1.f);

	_style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.75f, 0.71f, 0.71f, 1.00f);
	_style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);


	_style.Colors[ImGuiCol_Button] = ImVec4(0.84f, 0.86f, 0.88f, 1.00f);
	_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.82f, 0.83f, 0.84f, 1.00f);
	_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.94f, 0.94f, 0.95f, 1.00f);





	ImGui::GetStyle() = _style;
}

