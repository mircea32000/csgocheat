#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"

bool C_BaseEntity::IsPlayer()
{
	//index: 152
	//ref: "effects/nightvision"
	//sig: 8B 92 ? ? ? ? FF D2 84 C0 0F 45 F7 85 F6
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 156)(this);
}

bool C_BaseEntity::IsLoot() {
	return GetClientClass()->m_ClassID == ClassId_CPhysPropAmmoBox ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropLootCrate ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropRadarJammer ||
		GetClientClass()->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		GetClientClass()->m_ClassID == ClassId_CDrone ||
		GetClientClass()->m_ClassID == ClassId_CDronegun ||
		GetClientClass()->m_ClassID == ClassId_CItem_Healthshot ||
		GetClientClass()->m_ClassID == ClassId_CItemCash || 
		GetClientClass()->m_ClassID == ClassId_CBumpMine;
}

bool C_BaseEntity::IsWeapon()
{
	//index: 160
	//ref: "CNewParticleEffect::DrawModel"
	//sig: 8B 80 ? ? ? ? FF D0 84 C0 74 6F 8B 4D A4
	return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 164)(this);
}


bool C_BaseEntity::IsPlantedC4()
{
	return GetClientClass()->m_ClassID == ClassId_CPlantedC4;
}

bool C_BaseEntity::IsC4()
{
	return GetClientClass()->m_ClassID == ClassId_CC4;
}

bool C_BaseEntity::IsDefuseKit()
{
	return GetClientClass()->m_ClassID == ClassId_CBaseAnimating;
}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
	return CallVFunction<CCSWeaponInfo*(__thiscall*)(void*)>(this, 455)(this);
	/*
	static auto fnGetWpnData
	= reinterpret_cast<CCSWeaponInfo*(__thiscall*)(void*)>(
	Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 81 EC ? ? ? ? 53 8B D9 56 57 8D 8B")
	);
	return fnGetWpnData(this);*/
}

bool C_BaseCombatWeapon::HasBullets()
{
	return !IsReloading() && m_iClip1() > 0;
}

bool C_BaseCombatWeapon::CanFire()
{
	auto owner = this->m_hOwnerEntity().Get();
	if (!owner)
		return false;

	if (IsReloading() || m_iClip1() <= 0)
		return false;

	auto flServerTime = owner->m_nTickBase() * g_GlobalVars->interval_per_tick;

	if (owner->m_flNextAttack() > flServerTime)
		return false;


	return m_flNextPrimaryAttack() <= flServerTime;
}

bool C_BaseCombatWeapon::IsGrenade()
{
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_GRENADE;
}

bool C_BaseCombatWeapon::IsGun()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool C_BaseCombatWeapon::IsKnife()
{
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER) return false;
	return GetCSWeaponData()->iWeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsRifle()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_RIFLE:
		return true;
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	case WEAPONTYPE_SHOTGUN:
		return true;
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsPistol()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->iWeaponType)
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsReloading()
{
	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
	return *(bool*)((uintptr_t)this + inReload);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 477)(this);
}

float C_BaseCombatWeapon::GetSpread()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 447)(this);
}

void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
	CallVFunction<void(__thiscall*)(void*)>(this, 478)(this);
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{	static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

CUtlVector<IRefCounted*>& CEconItemView::m_VisualsDataProcessors()
{
	static auto m_varVisualsDataProcessors = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "81 C7 ? ? ? ? 8B 4F 0C 8B 57 04 89 4C") + 2);
	return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + m_varVisualsDataProcessors);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
	return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
	static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
	return *(CUserCmd**)((uintptr_t)this + currentCommand);
}

int C_BasePlayer::GetNumAnimOverlays()
{
	return *(int*)((DWORD)this + 0x298C);
}

AnimationLayer *C_BasePlayer::GetAnimOverlays()
{
	return *(AnimationLayer**)((DWORD)this + 0x2980);
}

AnimationLayer *C_BasePlayer::GetAnimOverlay(int i)
{
	if (i < 15)
		return &GetAnimOverlays()[i];
	return nullptr;
}

int C_BasePlayer::GetSequenceActivity(int sequence)
{
	auto hdr = g_MdlInfo->GetStudiomodel(this->GetModel());

	if (!hdr)
		return -1;

	// sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
	// sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
	// c_csplayer vfunc 242, follow calls to find the function.
	// Thanks @Kron1Q for merge request
	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "55 8B EC 53 8B 5D 08 56 8B F1 83"));

	return get_sequence_activity(this, hdr, sequence);
}

CCSGOPlayerAnimState *C_BasePlayer::GetPlayerAnimState()
{
	return *(CCSGOPlayerAnimState**)((DWORD)this + 0x3900);
}

void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState *state, QAngle angle)
{
	static auto UpdateAnimState = Utils::PatternScan(
		GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");

	if (!UpdateAnimState)
		return;

	__asm {
		push 0
	}

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void C_BasePlayer::ResetAnimationState(CCSGOPlayerAnimState *state)
{
	using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
	static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
	if (!ResetAnimState)
		return;

	ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState *state)
{
	using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
	static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
	if (!CreateAnimState)
		return;

	CreateAnimState(state, this);
}
std::string C_BaseCombatWeapon::GetWeaponIcon()
{
	switch (this->m_Item().m_iItemDefinitionIndex())
	{
	case ItemDefinitionIndex::WEAPON_DEAGLE:
		return "\\";
	case ItemDefinitionIndex::WEAPON_ELITE:
		return "]";
	case ItemDefinitionIndex::WEAPON_FIVESEVEN:
		return "^";
	case ItemDefinitionIndex::WEAPON_GLOCK:
		return "_";
	case ItemDefinitionIndex::WEAPON_AK47:
		return "`";
	case ItemDefinitionIndex::WEAPON_AUG:
		return "a";
	case ItemDefinitionIndex::WEAPON_AWP:
		return "b";
	case ItemDefinitionIndex::WEAPON_FAMAS:
		return "c";
	case ItemDefinitionIndex::WEAPON_G3SG1:
		return "d";
	case ItemDefinitionIndex::WEAPON_GALILAR:
		return "e";
	case ItemDefinitionIndex::WEAPON_M249:
		return "f";
	case ItemDefinitionIndex::WEAPON_M4A1:
		return "g";
	case ItemDefinitionIndex::WEAPON_MAC10:
		return "h";
	case ItemDefinitionIndex::WEAPON_P90:
		return "i";
	case ItemDefinitionIndex::WEAPON_MP5:
		return "j";
	case ItemDefinitionIndex::WEAPON_UMP45:
		return "k";
	case ItemDefinitionIndex::WEAPON_XM1014:
		return "l";
	case ItemDefinitionIndex::WEAPON_BIZON:
		return "m";
	case ItemDefinitionIndex::WEAPON_MAG7:
		return "n";
	case ItemDefinitionIndex::WEAPON_NEGEV:
		return "o";
	case ItemDefinitionIndex::WEAPON_SAWEDOFF:
		return "p";
	case ItemDefinitionIndex::WEAPON_TEC9:
		return "q";
	case ItemDefinitionIndex::WEAPON_TASER:
		return "r";
	case ItemDefinitionIndex::WEAPON_HKP2000:
		return "B";
	case ItemDefinitionIndex::WEAPON_MP7:
		return "t";
	case ItemDefinitionIndex::WEAPON_MP9:
		return "u";
	case ItemDefinitionIndex::WEAPON_NOVA:
		return "A";
	case ItemDefinitionIndex::WEAPON_P250:
		return "s";
	case ItemDefinitionIndex::WEAPON_SCAR20:
		return "C";
	case ItemDefinitionIndex::WEAPON_SG556:
		return "D";
	case ItemDefinitionIndex::WEAPON_SSG08:
		return "E";
	case ItemDefinitionIndex::WEAPON_KNIFE:
	case ItemDefinitionIndex::WEAPON_KNIFEGG:
	case ItemDefinitionIndex::WEAPON_MELEE:
		return "F";
	case ItemDefinitionIndex::WEAPON_FLASHBANG:
		return "G";
	case ItemDefinitionIndex::WEAPON_HEGRENADE:
		return "H";
	case ItemDefinitionIndex::WEAPON_SMOKEGRENADE:
		return "I";
	case ItemDefinitionIndex::WEAPON_MOLOTOV:
	case ItemDefinitionIndex::WEAPON_FIREBOMB:
		return "J";
	case ItemDefinitionIndex::WEAPON_DECOY:
	case ItemDefinitionIndex::WEAPON_DIVERSION:
		return "K";
	case ItemDefinitionIndex::WEAPON_INCGRENADE:
		return "L";
	case ItemDefinitionIndex::WEAPON_C4:
		return "M";
	case ItemDefinitionIndex::WEAPON_HEALTHSHOT:
		return "@";
	case ItemDefinitionIndex::WEAPON_KNIFE_T:
		return "N";
	case ItemDefinitionIndex::WEAPON_M4A1_SILENCER:
		return "O";
	case ItemDefinitionIndex::WEAPON_USP_SILENCER:
		return "P";
	case ItemDefinitionIndex::WEAPON_CZ75A:
		return "Q";
	case ItemDefinitionIndex::WEAPON_REVOLVER:
		return "R";
	case ItemDefinitionIndex::WEAPON_TAGRENADE:
		return ";";
	case ItemDefinitionIndex::WEAPON_FISTS:
		return "<";
	case ItemDefinitionIndex::WEAPON_BREACHCHARGE:
		return "=";
	case ItemDefinitionIndex::WEAPON_TABLET:
		return ">";
	case ItemDefinitionIndex::WEAPON_AXE:
		return ":";
	case ItemDefinitionIndex::WEAPON_HAMMER:
		return "?";
	case ItemDefinitionIndex::WEAPON_SPANNER:
		return "[";
	case ItemDefinitionIndex::WEAPON_FRAG_GRENADE:
		return "9";
	case ItemDefinitionIndex::WEAPON_BAYONET:
		return "S";
	case ItemDefinitionIndex::WEAPON_KNIFE_FLIP:
		return "T";
	case ItemDefinitionIndex::WEAPON_KNIFE_GUT:
		return "U";
	case ItemDefinitionIndex::WEAPON_KNIFE_KARAMBIT:
		return "V";
	case ItemDefinitionIndex::WEAPON_KNIFE_M9_BAYONET:
		return "W";
	case ItemDefinitionIndex::WEAPON_KNIFE_TACTICAL:
		return "X";
	case ItemDefinitionIndex::WEAPON_KNIFE_FALCHION:
		return "Y";
	case ItemDefinitionIndex::WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "Z";
	case ItemDefinitionIndex::WEAPON_KNIFE_BUTTERFLY:
		return "0";
	case ItemDefinitionIndex::WEAPON_KNIFE_PUSH: //Shadow Daggers
		return "1";
	case ItemDefinitionIndex::WEAPON_KNIFE_URSUS:
		return "2";
	case ItemDefinitionIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE: //Navaja
		return "3";
	case ItemDefinitionIndex::WEAPON_KNIFE_STILETTO:
		return "4";
	case ItemDefinitionIndex::WEAPON_KNIFE_WIDOWMAKER: //Talon
		return "5";
	default://\uE006 kevlar, \uE007 k+h, \uE008 defuser
		return " ";
	}
}
Vector C_BasePlayer::GetEyePos()
{
	if (!this)
		return Vector(0, 0, 0);

	Vector result;
	CallVFunction<void(__thiscall*)(void*, Vector&)>(this, 282)(this, result);
	return result;
}

player_info_t C_BasePlayer::GetPlayerInfo()
{
	player_info_t info;
	g_EngineClient->GetPlayerInfo(EntIndex(), &info);
	return info;
}

bool C_BasePlayer::IsAlive()
{
	return m_lifeState() == LIFE_ALIVE;
}

bool C_BasePlayer::IsTeammate()
{
	return this != g_LocalPlayer && this->m_iTeamNum() == g_LocalPlayer->m_iTeamNum(); //@TODO : dangerzone check
}

bool C_BasePlayer::IsFlashed()
{
	static auto m_flFlashMaxAlpha = NetvarSys::Get().GetOffset("DT_CSPlayer", "m_flFlashMaxAlpha");
	return *(float*)((uintptr_t)this + m_flFlashMaxAlpha - 0x8) > 200.0;
}

bool C_BasePlayer::HasC4()
{
	static auto fnHasC4
		= reinterpret_cast<bool(__thiscall*)(void*)>(
			Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "56 8B F1 85 F6 74 31")
			);

	return fnHasC4(this);
}

void C_BasePlayer::SetAbsOrigin(const Vector& vOrigin)
{
	using SetAbsOrigin_t = void(__thiscall*)(void*, const Vector & origin);
	static SetAbsOrigin_t SetAbsOriginFn = (SetAbsOrigin_t)(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), ("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8 ? ?")));
	SetAbsOriginFn(this, vOrigin);
}

mstudiohitboxset_t* C_BasePlayer::GetHitboxHandle(matrix3x4_t matrix[256])
{
	static bool* s_bEnableInvalidateBoneCache =
		(bool*)(*(uintptr_t*)(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), ("F3 0F 5C C1 C6 05 ? ? ? ? 00")) + 6));

	static auto InvalidateBoneCache = Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), ("80 3D ? ? ? ? 00 74 16 A1 ? ? ? ? 48 C7 81 ? ? 00 00 FF FF 7F FF 89 81 ? ? 00 00 C3"));

	bool bOrig = *s_bEnableInvalidateBoneCache;
	*s_bEnableInvalidateBoneCache = true;
	((int* (__thiscall*)(C_BasePlayer*))InvalidateBoneCache)(this);
	*s_bEnableInvalidateBoneCache = bOrig;

	static auto DidCheckForOcclusion =
		Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), ("A1 ? ? ? ? 8B B7 30 0A 00 00 89 75 F8 39 70 04"));
	static ptrdiff_t m_iDidCheckForOcclusion = *(uintptr_t*)(DidCheckForOcclusion + 7);
	static uintptr_t m_dwOcclusionArray = *(uintptr_t*)(DidCheckForOcclusion + 1);

	static int m_nForceBone = NetvarSys::Get().GetOffset("DT_BaseAnimating", "m_nForceBone");
	*(int*)(uintptr_t(this) + m_iDidCheckForOcclusion) = reinterpret_cast<int*>(m_dwOcclusionArray)[1];

	bool bIsLocal = EntIndex() == g_EngineClient->GetLocalPlayer();

	//PushAllowBoneSetup(EntIndex());

	Vector vAbsOrigin;
	vAbsOrigin = m_angAbsOrigin();
	SetAbsOrigin(m_vecOrigin());

	bool bSetupBones = SetupBones(matrix, 256, BONE_USED_BY_ANYTHING, 0.0);

	SetAbsOrigin(vAbsOrigin);

	if (!bSetupBones)
		return nullptr;
	auto model = GetModel();
	if (!model)
		return nullptr;
	studiohdr_t* hdr = g_MdlInfo->GetStudiomodel(model);
	if (!hdr)
		return nullptr;
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	if (!set)
		return nullptr;

	return set;
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				auto
					min = Vector{},
					max = Vector{};

				Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
				Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}
	return Vector{};
}

mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f)) {
		auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
		if (studio_model) {
			auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
			if (hitbox) {
				return hitbox;
			}
		}
	}
	return nullptr;
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector &output)
{
	if (hitbox >= HITBOX_MAX)
		return false;

	const model_t *model = this->GetModel();
	if (!model)
		return false;

	studiohdr_t *studioHdr = g_MdlInfo->GetStudiomodel(model);
	if (!studioHdr)
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
		return false;

	mstudiobbox_t *studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
	if (!studioBox)
		return false;

	Vector min, max;

	Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
	Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}

Vector C_BasePlayer::GetBonePos(int bone)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f)) {
		return boneMatrix[bone].at(3);
	}
	return Vector{};
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	auto endpos = player->GetHitboxPos(hitbox);

	ray.Init(GetEyePos(), endpos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(GetEyePos(), pos);
	g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::IsBehindSmoke(Vector vEndPos = Vector())
{
	Vector vStartPos = g_LocalPlayer->GetEyePos();

	using _LineGoesThroughSmoke = bool(__cdecl*) (Vector, Vector);

	static _LineGoesThroughSmoke LineGoesThroughSmokeFn = 0;

	static auto dwFunctionAddress =
		Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");

	if (dwFunctionAddress)
	{
		LineGoesThroughSmokeFn = (_LineGoesThroughSmoke)dwFunctionAddress;
	}

	if (vEndPos == Vector())
	{
		BYTE bHitBoxCheckVisible[2] = {
			HITBOX_HEAD,
			HITBOX_STOMACH
		};

		for (unsigned char nHit : bHitBoxCheckVisible)
		{
			Vector vHitBox = GetHitboxPos(nHit);

			if (vHitBox.IsZero())
				continue;

			if (LineGoesThroughSmokeFn)
			{
				return LineGoesThroughSmokeFn(vStartPos, vHitBox);
			}
		}
	}

	if (LineGoesThroughSmokeFn)
	{
		return LineGoesThroughSmokeFn(vStartPos, vEndPos);
	}

	return false;
}

void C_BasePlayer::UpdateClientSideAnimation()
{
	return CallVFunction<void(__thiscall*)(void*)>(this, 221)(this);
}

void C_BasePlayer::InvalidateBoneCache()
{
	static DWORD addr = (DWORD)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");

	*(int*)((uintptr_t)this + 0xA30) = g_GlobalVars->framecount; //we'll skip occlusion checks now
	*(int*)((uintptr_t)this + 0xA28) = 0;//clear occlusion flags

	unsigned long g_iModelBoneCounter = **(unsigned long**)(addr + 10);
	*(unsigned int*)((DWORD)this + 0x2924) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*(unsigned int*)((DWORD)this + 0x2690) = (g_iModelBoneCounter - 1); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

void C_BasePlayer::SetLocalViewangles(Vector angle)
{
	CallVFunc<369, void>(this, &angle);
}

int C_BasePlayer::PhysicsRunThink(int i)
{
	static auto pfnPhysicsRunThink =
		Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 EC 10 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8 16");

	using PhysicsRunThink_t = int(__thiscall*)(void*, int);

	return ((PhysicsRunThink_t)(pfnPhysicsRunThink))(this, i);
}

void C_BasePlayer::PreThink()
{
	CallVFunc<314, void>(this);
}

void C_BasePlayer::Think()
{
	CallVFunc<140, void>(this);
}

void C_BasePlayer::PostThink()
{
	CallVFunc<315, int>(this);
}

int C_BasePlayer::m_nMoveType()
{
	return *(int*)((uintptr_t)this + 0x25C);
}

QAngle* C_BasePlayer::GetVAngles()
{
	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
	return (QAngle*)((uintptr_t)this + deadflag + 0x4);
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
	return CallVFunction<void(__thiscall*)(void*, int)>(this, 244)(this, sequence);
}

float_t C_BasePlayer::m_flSpawnTime()
{
	return *(float_t*)((uintptr_t)this + 0xA360);
}
