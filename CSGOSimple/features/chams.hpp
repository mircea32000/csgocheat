#pragma once

#include "../singleton.hpp"

class IMatRenderContext;
struct DrawModelState_t;
struct ModelRenderInfo_t;
class matrix3x4_t;
class IMaterial;
class Color;

class Chams
	: public Singleton<Chams>
{
	friend class Singleton<Chams>;

	Chams();
	~Chams();

public:
	void OnDrawModelExecute(
		IMatRenderContext* ctx,
		const DrawModelState_t& state,
		const ModelRenderInfo_t& pInfo,
		matrix3x4_t* pCustomBoneToWorld);

private:
	void OverrideMaterial(bool ignoreZ, int option, const Color& rgba);

	IMaterial* materialRegular = nullptr;
	IMaterial* materialFlat = nullptr;
	IMaterial* materialReflective = nullptr;
	IMaterial* materialGlass = nullptr;
	IMaterial* materialGlow = nullptr;
};