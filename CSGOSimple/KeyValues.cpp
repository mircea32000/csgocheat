#pragma once

#include "KeyValues.h"
#include "helpers/utils.hpp"
#include "helpers/vfunc_hook.hpp"

PVOID KeyValues::operator new(size_t iAllocSize)
{
	static oKeyValuesSystem KeyValuesSystemFn = (oKeyValuesSystem)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem");
	auto KeyValuesSystem = KeyValuesSystemFn();

	return CallVFunc<1, PVOID, size_t>(KeyValuesSystem, iAllocSize);
}

void KeyValues::operator delete(PVOID pMem)
{
	static oKeyValuesSystem KeyValuesSystemFn = (oKeyValuesSystem)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem");
	auto KeyValuesSystem = KeyValuesSystemFn();

	CallVFunc<2, void, PVOID>(KeyValuesSystem, pMem);
}

const char* KeyValues::GetName() const
{
	static oKeyValuesSystem KeyValuesSystemFn = (oKeyValuesSystem)GetProcAddress(GetModuleHandleA("vstdlib.dll"), "KeyValuesSystem");
	auto KeyValuesSystem = KeyValuesSystemFn();

	auto a2 = (DWORD)this;

	return CallVFunc<4, const char*, int>(KeyValuesSystem, *(BYTE*)(a2 + 3) | (*(WORD*)(a2 + 18) << 8));
}

KeyValues* KeyValues::FindKey(const char* keyName, bool bCreate)
{
	static auto key_values_find_key = reinterpret_cast<KeyValues * (__thiscall*)(void*, const char*, bool)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 EC 1C 53 8B D9 85 DB"));
	return key_values_find_key(this, keyName, bCreate);
}

void KeyValues::SetString(const char* keyName, const char* value)
{
	auto key = FindKey(keyName, true);
	if (key)
	{
		static auto key_values_set_string = reinterpret_cast<void(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC A1 ? ? ? ? 53 56 57 8B F9 8B 08 8B 01"));
		key_values_set_string(key, value);
	}
}

void KeyValues::InitKeyValues(const char* name)
{
	static auto key_values = reinterpret_cast<void(__thiscall*)(void*, const char*)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 51 33 C0 C7 45 ? ? ? ? ? 56 8B F1 81 26 ? ? ? ? C6 46 03 ? 89 46 10 89 46 18 89 46 14 89 46 1C 89 46 04 89 46 08 89 46 0C FF 15 ? ? ? ? 6A 01 FF 75 08 8D 4D FC 8B 10 51 8B C8 FF 52 24 8B 0E 33 4D FC 81 E1 ? ? ? ? 31 0E 88 46 03"));
	key_values(this, name);
}

void KeyValues::SetUint64(const char* keyName, int value, int value2)
{
	static auto key_values_set_uint64 = reinterpret_cast<void(__thiscall*)(void*, const char*, int, int)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 56 6A 01 FF 75 08"));
	key_values_set_uint64(this, keyName, value, value2);
}

const char* KeyValues::GetString(const char* keyName, const char* defaultValue)
{
	static auto key_values_get_string = reinterpret_cast<const char* (__thiscall*)(void*, const char*, const char*)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 8B 5D 08"));
	return key_values_get_string(this, keyName, defaultValue);
}

int KeyValues::GetInt(const char* keyName, int defaultValue)
{
	static auto key_values_get_int = reinterpret_cast<int(__thiscall*)(void*, const char*, int)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 6A ? FF 75 08 E8 ? ? ? ? 85 C0 74 45"));
	return key_values_get_int(this, keyName, defaultValue);
}

void KeyValues::SetInt(const char* keyName, int Value)
{
	auto key_int = FindKey(keyName, true);
	if (key_int)
	{
		*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(key_int) + 0xC) = Value;
		*reinterpret_cast<char*>(reinterpret_cast<uintptr_t>(key_int) + 0x10) = 2;
	}
}

bool KeyValues::LoadFromBuffer(const char* resourceName, const char* pBuffer, void* pFileSystem, const char* pPathID, void* pfnEvaluateSymbolProc)
{
	using LoadFromBufferFn =
		bool(__thiscall*)
		(
			void* thisptr, const char* resourceName, const char* pBuffer,
			void* pFileSystem, const char* pPathID, void* pfnEvaluateSymbolProc, void* pUnk
			);
	static auto sig = Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89 4C 24 04");
	static LoadFromBufferFn LoadFromBufferEx = (LoadFromBufferFn)(sig);
	return LoadFromBufferEx(this, resourceName, pBuffer, pFileSystem, pPathID, pfnEvaluateSymbolProc, nullptr);
}