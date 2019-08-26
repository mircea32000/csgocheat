#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "imgui/imgui.h"
#include "singleton.hpp"
#include <d3dx9.h>

class CRadar : public Singleton<CRadar>
{
public:
	void Render();

	LPDIRECT3DTEXTURE9	m_pMapTexture;
	float				m_flPosX;
	float				m_flPosY;
	float				m_flScale;
	bool				m_bCanDraw = false;

private:
	void VectorYawRotate(const ImVec2& in, float flYaw, ImVec2& out);
	void GetSize(int& x, int& y);
	void GetPos(int& x, int& y);
	ImVec2 WorldToMap(const Vector& worldpos);
	ImVec2 MapToRadar(const ImVec2& mappos);
	void VectorYawRotate2(ImVec2 center, float angle, ImVec2* p);

}; //extern CRadar g_Radar;