#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "valve_sdk/Misc/Color.hpp"
#include <map>
#define A( s ) #s
#define OPTION(type, var, val) Var<type> var = {A(var), val}

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T*() { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

struct LegitBotConfig
{
	bool m_bEnabled = false;
	bool m_bRCS = false;
	int m_iFOV = 0;
	int m_iHitbox = 0;
	int m_iSmoothingMethod = 0;
	float m_fSmooth = 0.f;
	float m_fRCSX = 0.f; 
	float m_fRCSY = 0.f; 
	float m_fDelay = 0.f; 

	//FILTERS

	bool m_bIgnoreSmoke = false;
	bool m_bIgnoreJumping = false;
	bool m_bTargetBacktrack = false;
	bool m_bAttackEnemies = true;
	bool m_bAttackFriendlies = false;
	float m_fFlashTolerance = 0.f;

	//TRIGGERBOT

	bool m_bAttackHead = false;
	bool m_bAttackChest = false;
	bool m_bAttackStomach = false;
	bool m_bAttackArms = false;
	bool m_bAttackLegs = false;

	//TRIGGERBOT FILTERS

	bool  m_bTriggerTargetBacktrack = false;
	bool  m_bTriggerTargetEnemies = false;
	bool  m_bTriggerTargetFriendlies = false;
	bool  m_bTriggerHitchanceToggle = false;
	float m_fTriggerFlashTolerance = 0.f;
	float m_fTriggerHitchance = 0.f;

};

class Options
{
public:
		// 
		// ESP
		// 

	    OPTION(bool, esp_outline, true);
		OPTION(bool, esp_enabled, false);
		OPTION(bool, esp_enemies_only, false);
		OPTION(int, esp_box_type, 0);
		OPTION(int, esp_weaponname_type, 0);
		OPTION(bool, esp_player_boxes, false);
		OPTION(bool, esp_draw_fov, false);
		OPTION(bool, esp_fov_filled, false);
		OPTION(bool, esp_grenade_prediction, false);
		OPTION(bool, esp_player_names, false);
		OPTION(bool, esp_player_health, false);
		OPTION(bool, esp_player_flags, false);
		OPTION(bool, esp_player_weapons, false);
		OPTION(bool, esp_spooky_shit, false);
		OPTION(bool, esp_bt_dots, false);
		OPTION(bool, esp_bt_chams, false);

		OPTION(bool, esp_spectator_list, false);
		OPTION(bool, esp_player_snaplines, false);
		OPTION(bool, esp_crosshair, false);
		OPTION(bool, esp_removescope, false);
		OPTION(bool, esp_dropped_weapons, false);
		OPTION(bool, esp_defuse_kit, false);
		OPTION(bool, esp_planted_c4, false);
		OPTION(bool, esp_items, false);
		OPTION(bool, esp_radar, false);
		OPTION(bool, bHideHeader, false);
		OPTION(int, iZoom, 3000);
		OPTION(int, iAlpha, 100);
		// 
		// GLOW
		// 
		OPTION(bool, glow_enabled, false);
		OPTION(bool, glow_enemies_only, false);
		OPTION(bool, glow_players, false);
		OPTION(bool, glow_chickens, false);
		OPTION(bool, glow_c4_carrier, false);
		OPTION(bool, glow_planted_c4, false);
		OPTION(bool, glow_defuse_kits, false);
		OPTION(bool, glow_weapons, false);

		//
		// CHAMS
		//
		OPTION(bool, chams_player_enabled, false);
		OPTION(bool, chams_disable_occlusion, false);
		OPTION(bool, chams_player_enemies_only, false);
		OPTION(bool, chams_player_wireframe, false);
		OPTION(bool, chams_player_flat, false);
		OPTION(bool, chams_player_ignorez, false);
		OPTION(bool, chams_player_glass, false);
		OPTION(bool, chams_player_metalic, false);
		OPTION(bool, chams_player_glow, false);
		OPTION(bool, chams_arms_enabled, false);
		OPTION(bool, chams_backtrack_rainbow, false);
		OPTION(bool, chams_arms_wireframe, false);
		OPTION(bool, chams_arms_flat, false);
		OPTION(int , chams_type, 0);
		OPTION(int, chams_type_weapon, 0);
		OPTION(int, chams_type_arms, 0);
		OPTION(bool, chams_arms_ignorez, false);
		OPTION(bool, chams_arms_glass, false);

		//
		// MISC
		//
		OPTION(bool, misc_bhop, false);
		OPTION(bool, misc_backtrack, false);
		OPTION(bool, fake_latency, false);
		OPTION(bool, misc_no_hands, false);
		OPTION(bool, misc_thirdperson, false);
		OPTION(bool, misc_showranks, true);
		OPTION(bool, chams_gloss, true);
		OPTION(bool, misc_watermark, true);
		OPTION(float, misc_thirdperson_dist, 50.f);
		OPTION(int, viewmodel_fov, 68);
		OPTION(int, legit_fov, 1);
		OPTION(bool, legit_rcs, true);
		OPTION(float, legit_rcs_x, 2);
		OPTION(float, legit_rcs_y, 2);
		OPTION(float, legit_target_delay, 0);
		OPTION(int, legit_hitbox, 0);
		OPTION(int, legit_smoothing_method, 0);
		OPTION(float, legit_smooth, 1.f);
		OPTION(float, mat_ambient_light_r, 0.0f);
		OPTION(float, mat_ambient_light_g, 0.0f);
		OPTION(float, mat_ambient_light_b, 0.0f);

		// 
		// COLORS
		// 
		OPTION(Color, color_chams_gloss, Color(0, 128, 255));
		OPTION(Color, color_fov_outline, Color(0, 128, 255));
		OPTION(Color, color_fov_filled, Color(0, 128, 255));
		OPTION(Color, color_esp_ally_visible, Color(0, 128, 255));
		OPTION(Color, color_esp_enemy_visible, Color(255, 0, 0));
		OPTION(Color, color_esp_ally_occluded, Color(0, 128, 255));
		OPTION(Color, color_esp_enemy_occluded, Color(255, 0, 0));
		OPTION(Color, color_esp_crosshair, Color(255, 255, 255));
		OPTION(Color, color_esp_weapons, Color(128, 0, 128));
		OPTION(Color, color_esp_skeleton, Color(255, 255, 255));
		OPTION(Color, color_esp_defuse, Color(0, 128, 255));
		OPTION(Color, color_esp_c4, Color(255, 255, 0));
		OPTION(Color, color_esp_item, Color(255, 255, 255));

		OPTION(Color, color_glow_ally, Color(0, 128, 255));
		OPTION(Color, color_glow_enemy, Color(255, 0, 0));
		OPTION(Color, color_glow_chickens, Color(0, 128, 0));
		OPTION(Color, color_glow_c4_carrier, Color(255, 255, 0));
		OPTION(Color, color_glow_planted_c4, Color(128, 0, 128));
		OPTION(Color, color_glow_defuse, Color(255, 255, 255));
		OPTION(Color, color_glow_weapons, Color(255, 128, 0));

		OPTION(Color, color_chams_player_ally_visible, Color(0, 128, 255));
		OPTION(Color, color_chams_player_ally_occluded, Color(0, 255, 128));
		OPTION(Color, color_chams_player_enemy_visible, Color(255, 0, 0));
		OPTION(Color, color_chams_player_enemy_occluded, Color(255, 128, 0));
		OPTION(Color, color_chams_arms_visible, Color(0, 128, 255));
		OPTION(Color, color_chams_arms_occluded, Color(0, 128, 255));
		OPTION(Color, color_watermark, Color(255, 255, 255)); // no menu config cuz its useless

		std::map <int, LegitBotConfig> m_mapAim;
};

inline Options g_Options;
inline bool   g_Unload;
