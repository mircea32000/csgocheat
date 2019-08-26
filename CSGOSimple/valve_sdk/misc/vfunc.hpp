#pragma once

template<unsigned int IIdx, typename TRet, typename ... TArgs>
static auto CallVFunc(void* thisptr, TArgs ... argList) -> TRet
{
	using Fn = TRet(__thiscall*)(void*, decltype(argList)...);
	return (*static_cast<Fn * *>(thisptr))[IIdx](thisptr, argList...);
}

template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
    int* pVTable = *(int**)ppClass;
    int dwAddress = pVTable[index];
    return (FuncType)(dwAddress);
}
