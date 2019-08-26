#pragma once
#include "singleton.hpp"

class CSpectators : public Singleton<CSpectators>
{
public:
	void RenderSpectators();
};