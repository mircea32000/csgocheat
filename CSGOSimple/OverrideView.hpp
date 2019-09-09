#pragma once
#include "singleton.hpp"
class OverrideView : public Singleton<OverrideView>
{
public:
	float currentFOV = 0;
};