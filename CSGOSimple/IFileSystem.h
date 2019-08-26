#pragma once

using FileNameHandle_t = void*;

class IFileSystem
{
public:
	FileNameHandle_t FindOrAddFilename(const char* pszFileName)
	{
		return CallVFunc<41, FileNameHandle_t>(this, pszFileName);
	}

	bool String(const FileNameHandle_t& handle, char* buf, int buflen)
	{
		return CallVFunc<42, bool>(this, handle, buf, buflen);
	}

}; //extern IFileSystem* g_pFileSystem;