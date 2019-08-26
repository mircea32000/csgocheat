#pragma once
#include "valve_sdk/csgostructs.hpp"
#include "options.hpp"

class CSkinChanger : public Singleton<CSkinChanger>
{
public:
	void Do();
};