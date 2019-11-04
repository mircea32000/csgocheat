#include "C_CSPlayer.h"
#include "hooks.hpp"

void __fastcall Animations::DoExtraBoneProcessing(C_BasePlayer* ecx, void* edx, studiohdr_t* hdr, Vector* vector, Quaternion* something, matrix3x4_t* matrix, void* /* CBoneBitList& */ bone_list, void* /* CIKContext* */ context)
{
	float flBackupFraction(ecx->GetPlayerAnimState()->m_flUnknownFraction);
	ecx->GetPlayerAnimState()->m_flUnknownFraction = 0.f;

	//CCSPlayerHook.GetOriginal<DoExtraBoneProcessing_t>
	//	(VT::CCSPlayer::DoExtraBoneProcessing)(ecx, hdr, vector, something, matrix, bone_list, context);
	Hooks::vfuncs::player_hook.get_original<Typedefs::DoExtraBoneProcessing_t>(197)(ecx, hdr, vector, something, matrix, bone_list, context);
	//ofunc(ecx, hdr, vector, something, matrix, bone_list, context);
	ecx->GetPlayerAnimState()->m_flUnknownFraction = flBackupFraction;
}