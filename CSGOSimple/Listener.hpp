#pragma once
#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "Globals.h"
#include "options.hpp"
#include "HitmarkerSounds.h"
#pragma comment(lib, "winmm.lib")
class item_purchase

	: public Singleton<item_purchase>
{
	class item_purchase_listener
		: public IGameEventListener2
	{
	public:
		void start()
		{
			g_GameEvents->AddListener(this, "item_purchase", false);
			g_GameEvents->AddListener(this, "player_hurt", false);
		}
		void stop()
		{
			g_GameEvents->RemoveListener(this);
		}
		void FireGameEvent(IGameEvent* event) override
		{
			singleton()->on_fire_event(event);
		}
		int GetEventDebugID(void) override
		{
			return 42 /*0x2A*/;
		}
	};

public:

	static item_purchase* singleton()
	{
		static item_purchase* instance = new item_purchase;
		return instance;
	}

	void initialize()
	{
		listener.start();
	}

	void remove()
	{
		listener.stop();
	}

	void on_fire_event(IGameEvent* event)
	{
		if (!strcmp(event->GetName(), "player_hurt"))
		{
			auto bitch = event->GetInt("userid");
			auto coolguy49 = event->GetInt("attacker");
			int dmg = event->GetInt("dmg_health");


			auto bitch_index = g_EngineClient->GetPlayerForUserID(bitch);
			auto coolguy49_index = g_EngineClient->GetPlayerForUserID(coolguy49);
			C_BaseEntity* bitch_ = (C_BaseEntity*)g_EntityList->GetClientEntity(bitch_index);
			C_BaseEntity* coolguy49_ = (C_BaseEntity*)g_EntityList->GetClientEntity(coolguy49_index);
			player_info_t bitch_info;
			player_info_t coolguy49_info;

			if (coolguy49_ == g_LocalPlayer)
			{
				G::hitmarkeralpha = 1.f;
				switch (g_Options.esp_hitmarker_sound)
				{
				case 0: break;
				case 1: PlaySoundA(HitSounds::rawData, NULL, SND_ASYNC | SND_MEMORY); break; // Default
				case 2:	PlaySoundA(HitSounds::roblox, NULL, SND_ASYNC | SND_MEMORY); break; // Anime
				case 3:	PlaySoundA(HitSounds::penguware_hitmarker, NULL, SND_ASYNC | SND_MEMORY); break; // CHINA
				case 4: PlaySoundA(HitSounds::gamesense, NULL, SND_ASYNC | SND_MEMORY); break; // Skeet
				}

			}

		}

	}
private:
	item_purchase_listener  listener;
};

item_purchase purchase;