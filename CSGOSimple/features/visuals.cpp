#include <algorithm>

#include "visuals.hpp"
#include "../OverrideView.hpp"
#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../lagcomp.h"
#include "../spectators.h"
#include "../nade_bullshit.h"
RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}


void ThreeDBox(Vector minin, Vector maxin, Vector pos, Color col)
{
	Vector min = minin + pos;
	Vector max = maxin + pos;

	Vector corners[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(min.x, min.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(max.x, max.y, max.z),
		Vector(max.x, min.y, max.z) };


	int edges[12][2] = { { 0, 1 },{ 1, 2 },{ 2, 3 },{ 3, 0 },{ 4, 5 },{ 5, 6 },{ 6, 7 },{ 7, 4 },{ 0, 4 },{ 1, 5 },{ 2, 6 },{ 3, 7 }, };

	for (const auto edge : edges)
	{
		Vector p1, p2;
		if (!Math::WorldToScreen(corners[edge[0]], p1) || !Math::WorldToScreen(corners[edge[1]], p2))
			return;
		int red = 0;
		int green = 0;
		int blue = 0;
		int alpha = 0;

		Render::Get().RenderLine(p1.x, p1.y, p2.x, p2.y, col);
		//Render::Get().RenderLine(p1.x - 1, p1.y - 1, p2.x + 1, p2.y + 1, Color::Black);
		//Render::Get().RenderLine(p1.x + 1, p1.y + 1, p2.x - 1, p2.y - 1, Color::Black);
	}
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Visuals::Render() {
}
//--------------------------------------------------------------------------------
bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (pl->IsDormant() || !pl->IsAlive())
		return false;

	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy && g_Options.esp_enemies_only)
		return false;

	ctx.clr = ctx.is_enemy ? (ctx.is_visible ? g_Options.color_esp_enemy_visible : g_Options.color_esp_enemy_occluded) : (ctx.is_visible ? g_Options.color_esp_ally_visible : g_Options.color_esp_ally_occluded);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 15;

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
	auto w = h / 1.65f;

	ctx.bbox.left = static_cast<long>(ctx.feet_pos.x - w * 0.5f);
	ctx.bbox.right = static_cast<long>(ctx.bbox.left + w);
	ctx.bbox.bottom = static_cast<long>(ctx.feet_pos.y + 5.f);//meh
	ctx.bbox.top = static_cast<long>(ctx.head_pos.y);


	return true;
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderBTDots()
{
	for (auto& record : TimeWarp::Get().m_Records[ctx.pl->EntIndex()].m_vecRecords)
	{
		Vector thisTick = record.m_arrHitboxes[HITBOX_HEAD].m_vecHitboxPos;

		Vector screenThisTick;

		if (Math::WorldToScreen(thisTick, screenThisTick))
		{
			Render::Get().RenderBoxFilled(screenThisTick.x, screenThisTick.y, screenThisTick.x + 3, screenThisTick.y + 3, Color(255, 255, 255, 200));
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderBox() 
{

	float
		length_horizontal = (ctx.bbox.right - ctx.bbox.left) * 0.2f,
		length_vertical = (ctx.bbox.bottom - ctx.bbox.top) * 0.2f;

	switch(g_Options.esp_box_type)
	{
	case 0:
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.clr, 1);
		Render::Get().RenderBoxByType(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color::Black, 1);
		Render::Get().RenderBoxByType(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color::Black, 1);
	break;
	case 1:
		Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.clr, 1,1);
		Render::Get().RenderBoxByType(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, Color::Black, 1, 1);
		Render::Get().RenderBoxByType(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, Color::Black, 1, 1);
	break;
	case 2:
		ThreeDBox(ctx.pl->GetCollideable()->OBBMins(), ctx.pl->GetCollideable()->OBBMaxs(), ctx.pl->GetRenderOrigin(), ctx.clr);
	break;
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName()
{
	player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);

	Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y, 14.f,  Color::White);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth()
{
	auto  hp = ctx.pl->m_iHealth();
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	//float off = (box_h / 6.f) + 5;
	float off = 8;

	int height = (box_h * hp) / 100;

	int green = int(hp * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	Render::Get().RenderBox(x, y, x + w, y + h, Color::Black, 1.f, true);
	Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + height - 2, Color(red, green, 0, 255), 1.f, true);
}

//--------------------------------------------------------------------------------
void Visuals::Player::RenderSkeleton()
{

	studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(ctx.pl->GetModel());
	if (studioHdr)
	{
		static matrix3x4_t boneToWorldOut[128];
		if (ctx.pl->SetupBones(boneToWorldOut, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_GlobalVars->curtime))
		{
			for (int i = 0; i < studioHdr->numbones; i++)
			{
				mstudiobone_t* bone = studioHdr->GetBone(i);
				if (!bone || !(bone->flags & BONE_USED_BY_HITBOX) || bone->parent == -1)
					continue;

				Vector bonePos1;
				if (!Math::WorldToScreen(Vector(boneToWorldOut[i][0][3], boneToWorldOut[i][1][3], boneToWorldOut[i][2][3]), bonePos1))
					continue;

				Vector bonePos2;
				if (!Math::WorldToScreen(Vector(boneToWorldOut[bone->parent][0][3], boneToWorldOut[bone->parent][1][3], boneToWorldOut[bone->parent][2][3]), bonePos2))
					continue;

				Render::Get().RenderLine((int)bonePos1.x, (int)bonePos1.y, (int)bonePos2.x, (int)bonePos2.y, g_Options.color_esp_skeleton);
			}
		}
	}
}
void Visuals::Player::RenderFlags()
{
	int flag_count = 0;
	std::string armor_flag;


    Render::Get().RenderText("$" + std::to_string(ctx.pl->m_iAccount()), ctx.bbox.right + 1, ctx.bbox.top + flag_count++ * 8, 12, Color(28, 191, 13,255), false, true, g_pDefaultFont);

	if (ctx.pl->m_bHasHelmet())
		armor_flag += "H";

	if (ctx.pl->m_ArmorValue() > 0)
		armor_flag += "K";

	if (!armor_flag.empty())
		Render::Get().RenderText(armor_flag, ctx.bbox.right + 1, ctx.bbox.top + flag_count++ * 8, 10, Color::White, false, true, g_pDefaultFont);

	if (ctx.pl->IsFlashed())
		Render::Get().RenderText("Flashed", ctx.bbox.right + 1, ctx.bbox.top + flag_count++ * 8, 10, Color::White, false, true, g_pDefaultFont);

	if (ctx.pl->m_bIsScoped())
		Render::Get().RenderText("Scoped", ctx.bbox.right + 1, ctx.bbox.top + flag_count++ * 8, 10, Color::White, false, true, g_pDefaultFont);

}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderWeaponName()
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;

	if (g_Options.esp_weaponname_type == 0)
	{
	    auto texta = weapon->GetCSWeaponData()->szWeaponName + 7;
		auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, texta);
		Render::Get().RenderText(texta, ctx.feet_pos.x, (float)ctx.bbox.bottom, 14.f, Color::White, true,
			g_pDefaultFont);
	}
	if (g_Options.esp_weaponname_type == 1)
	{
		auto textb = weapon->GetWeaponIcon();
		auto sz = g_pWeaponIcons->CalcTextSizeA(14.f, FLT_MAX, 0.0f, textb.c_str());
		Render::Get().PureRenderText(textb, ctx.feet_pos.x, (float)ctx.bbox.bottom, 14.f, Color::White, true, true ,g_pWeaponIcons);
	}
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderSnapline()
{

	int screen_w, screen_h;
	g_EngineClient->GetScreenSize(screen_w, screen_h);

	Render::Get().RenderLine(screen_w / 2.f, (float)screen_h,
		ctx.feet_pos.x, ctx.feet_pos.y, ctx.clr);
}
//--------------------------------------------------------------------------------
void Visuals::RenderFOV()
{
	int w, h;
	g_EngineClient->GetScreenSize(w, h);

	if (!g_LocalPlayer)
		return;

	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	
	if (!weapon)
		return;

	float radius = 0;
	int aimbotFov = g_Options.m_mapAim[weapon->m_Item().m_iItemDefinitionIndex()].m_iFOV;
	float fov = OverrideView::Get().currentFOV;
	radius = tanf(DEG2RAD(aimbotFov) / 2) / tanf(DEG2RAD(fov) / 2) * w;

	if (g_Options.esp_fov_filled)
		Render::Get().RenderCircleFilled(w / 2, h / 2, radius, 40, g_Options.color_fov_filled);
	else
		Render::Get().RenderCircle(w / 2, h / 2, radius, 40, g_Options.color_fov_outline);

}
//--------------------------------------------------------------------------------
void Visuals::RenderCrosshair()
{
	int w, h;

	g_EngineClient->GetScreenSize(w, h);

	int x = w / 2;
	int y = h / 2;
	int dy = h / 97;
	int dx = w / 97;

	QAngle punchAngle = g_LocalPlayer->m_aimPunchAngle();
	x -= (dx * (punchAngle.yaw));
	y += (dy * (punchAngle.pitch));

	Render::Get().RenderLine(x - 8, y, x + 8, y, g_Options.color_esp_crosshair);
	Render::Get().RenderLine(x, y - 8, x, y + 8, g_Options.color_esp_crosshair);
}
//--------------------------------------------------------------------------------
void Visuals::RenderScope()
{
	int screenX, screenY;
	g_EngineClient->GetScreenSize(screenX, screenY);

	Render::Get().RenderLine(screenX / 2, 0, screenX / 2, screenY, Color::Black);
	Render::Get().RenderLine(0, screenY / 2, screenX, screenY / 2, Color::Black);
}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_vecOrigin().IsZero())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_weapons);
	//Render::Get().RenderBox(bbox.left - 1, bbox.top - 1, bbox.right + 1, bbox.bottom + 1, g_Options.color_esp_weapons);
	//Render::Get().RenderBox(bbox.left + 1, bbox.top + 1, bbox.right - 1, bbox.bottom - 1, g_Options.color_esp_weapons);


	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;


	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_weapons);
}
//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, g_Options.color_esp_defuse);

	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_defuse);
}
//--------------------------------------------------------------------------------
void Visuals::RenderPlantedC4Info(C_BaseEntity* ent)
{
	int x, y; g_EngineClient->GetScreenSize(x, y);//getscreensize in pixels for width of countdown bars

	auto bbox = GetBBox(ent);

	auto pLocal = g_LocalPlayer;

	float flblow = ent->m_flC4Blow();//the time when the bomb will detonate
	float ExplodeTimeRemaining = flblow - (pLocal->m_nTickBase() * g_GlobalVars->interval_per_tick);//subtract current time to get time remaining

	float fldefuse = ent->m_flDefuseCountDown();//time bomb is expected to defuse. if defuse is cancelled and started again this will be changed to the new value
	float DefuseTimeRemaining = fldefuse - (pLocal->m_nTickBase() * g_GlobalVars->interval_per_tick);//subtract current time to get time remaining

	char TimeToExplode[64]; sprintf_s(TimeToExplode, "Explode in: %.1f", ExplodeTimeRemaining);//Text we gonna display for explosion

	char TimeToDefuse[64]; sprintf_s(TimeToDefuse, "Defuse in: %.1f", DefuseTimeRemaining);//Text we gonna display for defuse

	ImVec2 textsize;
	int width, height;//text width and height for rendering in correct place. your cheat may get text height as a rect with both width and height
	textsize = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, TimeToExplode);
	width = textsize.x;
	height = textsize.y;

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, TimeToExplode);
	int w = bbox.right - bbox.left;

	if (ExplodeTimeRemaining > 0 && !ent->m_bBombDefused())//there is a period when u cant defuse the bomb and it hasn't exploded. > 0 check stops text showing then
	{									//also need to check if the bomb has been defused, cos otherwise it will just display time remaining when bomb was defused

		float fraction = ExplodeTimeRemaining / ent->m_flTimerLength();//the proportion of time remaining, use fltimerlength cos bomb detonation time can vary by gamemode
		int onscreenwidth = fraction * x;//the width of the bomb timer bar. proportion of time remaining multiplied by width of screen

		int red = 255 - (fraction * 255);//make our bar fade from complete green to complete red
		int green = fraction * 255;



		Render::Get().RenderText(TimeToExplode, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color::White);

		Color masturbation(red, green, 0, 255);
		Render::Get().RenderBoxFilled(0, 0, onscreenwidth + 1, 11, masturbation);
		Render::Get().RenderText(TimeToExplode, ImVec2(onscreenwidth - width,10), 14.f, Color::White);

	}//could remove the "explode in" but why make things more complicated

	ImVec2 cacamaca;
	cacamaca = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, TimeToDefuse);
	width = cacamaca.x;//now we gonna do defuse bar. why add a new variable for text width when we can use the old one...

	C_BasePlayer* Defuser = (C_BasePlayer*)C_BasePlayer::get_entity_from_handle(ent->m_hBombDefuser());//this is the player whos is defusing the bomb

	if (Defuser)//if there is a player defusing the bomb. this check is needed or it will continue showing time if a player stops defusing
	{
		float fraction = DefuseTimeRemaining / ent->m_flTimerLength();
		int onscreenwidth = fraction * x;

		//text for defuse countdown, below explode countdown
		Render::Get().RenderText(TimeToDefuse, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 15), 14.f, Color::White);

		Color uhh(0, 0, 255, 255);
		Render::Get().RenderBoxFilled(0, 10, onscreenwidth, 20, uhh, 0);
		Render::Get().RenderText(TimeToDefuse, ImVec2(onscreenwidth - width, 20), 14.f, Color::White);
	}

}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;


	//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, g_Options.color_esp_item);
}
//--------------------------------------------------------------------------------
void Visuals::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	if (g_Options.misc_thirdperson && g_LocalPlayer->IsAlive())
	{
		if (!g_Input->m_fCameraInThirdPerson)
		{
			g_Input->m_fCameraInThirdPerson = true;
		}

		float dist = g_Options.misc_thirdperson_dist;

		QAngle *view = g_LocalPlayer->GetVAngles();
		trace_t tr;
		Ray_t ray;

		Vector desiredCamOffset = Vector(cos(DEG2RAD(view->yaw)) * dist,
			sin(DEG2RAD(view->yaw)) * dist,
			sin(DEG2RAD(-view->pitch)) * dist
		);

		//cast a ray from the Current camera Origin to the Desired 3rd person Camera origin
		ray.Init(g_LocalPlayer->GetEyePos(), (g_LocalPlayer->GetEyePos() - desiredCamOffset));
		CTraceFilter traceFilter;
		traceFilter.pSkip = g_LocalPlayer;
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &traceFilter, &tr);

		Vector diff = g_LocalPlayer->GetEyePos() - tr.endpos;

		float distance2D = sqrt(abs(diff.x * diff.x) + abs(diff.y * diff.y));// Pythagorean

		bool horOK = distance2D > (dist - 2.0f);
		bool vertOK = (abs(diff.z) - abs(desiredCamOffset.z) < 3.0f);

		float cameraDistance;

		if (horOK && vertOK)  // If we are clear of obstacles
		{
			cameraDistance = dist; // go ahead and set the distance to the setting
		}
		else
		{
			if (vertOK) // if the Vertical Axis is OK
			{
				cameraDistance = distance2D * 0.95f;
			}
			else// otherwise we need to move closer to not go into the floor/ceiling
			{
				cameraDistance = abs(diff.z) * 0.95f;
			}
		}
		g_Input->m_fCameraInThirdPerson = true;

		g_Input->m_vecCameraOffset.z = cameraDistance;
	}
	else
	{
		g_Input->m_fCameraInThirdPerson = false;
	}
}


void Visuals::AddToDrawList() {
	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;
		
		if (entity == g_LocalPlayer && !g_Input->m_fCameraInThirdPerson)
			continue;

		if (i < 65) {
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) {
				if (g_Options.esp_player_snaplines) player.RenderSnapline();
				if (g_Options.esp_player_boxes)     player.RenderBox();
				if (g_Options.esp_player_weapons)   player.RenderWeaponName();
				if (g_Options.esp_spooky_shit)      player.RenderSkeleton();
 				if (g_Options.esp_player_names)     player.RenderName();
				if (g_Options.esp_player_health)    player.RenderHealth();
				if (g_Options.esp_player_flags)     player.RenderFlags();
				if (g_Options.esp_bt_dots)          player.RenderBTDots();
			}
		}
		else if (g_Options.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		else if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		else if (entity->IsPlantedC4() && g_Options.esp_planted_c4)
			RenderPlantedC4Info(entity);
		else if (entity->IsLoot() && g_Options.esp_items)
			RenderItemEsp(entity);
	}


	if (g_Options.esp_crosshair)
		RenderCrosshair();

	if (g_Options.esp_draw_fov)
		RenderFOV();

	if (g_Options.esp_removescope && (g_LocalPlayer && g_LocalPlayer->m_hActiveWeapon().Get() && g_LocalPlayer->m_hActiveWeapon().Get()->IsSniper() && g_LocalPlayer->m_bIsScoped()))
		RenderScope();

	CNadePred::Get().draw();

	if (g_Options.esp_spectator_list)
		CSpectators::Get().RenderSpectators();
}
