#pragma once
#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"

namespace Animations
{
	void __fastcall DoExtraBoneProcessing(C_BasePlayer* ecx, void* edx, studiohdr_t* hdr, Vector* vector, Quaternion* something, matrix3x4_t* matrix, void* /* CBoneBitList& */ bone_list, void* /* CIKContext* */ context);
	bool __fastcall ShouldInterpolate(uintptr_t ecx, uintptr_t edx);
	bool __fastcall SetupBones(IClientRenderable* ecx, void* edx, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);

	namespace Typedefs
	{
		using DoExtraBoneProcessing_t = void(__thiscall*)(C_BasePlayer*, studiohdr_t*, Vector*, Quaternion*, matrix3x4_t*, void*, void*);
		using SetupBones_t = bool(__thiscall*)(uintptr_t, matrix3x4_t*, int, int, float);
	}
}
class CCSPlayer : public Singleton<CCSPlayer>
{
public:
	void Initialise();
	void Shutdown();
};//could use namespaces but eh