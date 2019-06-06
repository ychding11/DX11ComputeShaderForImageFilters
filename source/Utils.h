#ifndef	_UTILS_H 
#define _UTILS_H 

#include <string>
#include <vector>
#include <iostream>
#include <windows.h>

#include <filesystem>
#include "Logger.h"

#define CHECK_WIN_CALL_FAIL  0xffff

#define WIN_CALL_CHECK(x)                             \
do{                                                   \
    LRESULT ret = x;                                  \
    if((ret) != S_OK)                                 \
    {                                                 \
        char buf[512];                                \
        sprintf_s(buf, 512, "- Error @%s:%d\t  %s %d\t \n",__FILE__,__LINE__, #x, (ret) );  \
        OutputDebugStringA(buf);                      \
        system("pause");                              \
        return CHECK_WIN_CALL_FAIL;                   \
    }                                                 \
} while(0)

#define D3D11_COMPILE_CALL_CHECK(x)                   \
do{                                                   \
    LRESULT ret = (x);                                \
    if((ret) != S_OK)                                 \
    {                                                 \
        char buf[512];                                \
        sprintf_s(buf, 512, "- Error @%s:%d\t  %s 0x%x\t \n",__FILE__,__LINE__, #x, (ret) );  \
        OutputDebugStringA(buf);                      \
	    Logger::getLogger() << buf;                   \
        if (pErrorBlob)                               \
		{                                             \
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());                        \
			Logger::getLogger() << ((char*)pErrorBlob->GetBufferPointer());                   \
		}											  \
		return false;                                 \
    }                                                 \
} while(0)

#define D3D11_CALL_CHECK(x)                           \
do{                                                   \
    LRESULT ret = (x);                                \
    if((ret) != S_OK)                                 \
    {                                                 \
        char buf[512];                                \
        sprintf_s(buf, 512, "- Error @%s:%d\t  %s 0x%x\t \n",__FILE__,__LINE__, #x, (ret) );  \
        OutputDebugStringA(buf);                      \
	    Logger::getLogger() << buf;                   \
		return false;                                 \
    }                                                 \
} while(0)


inline void Error(const char *format, ...)
{
	va_list ptr_arg;
	va_start(ptr_arg, format);

	static char tmps[1024];
	vsprintf(tmps, format, ptr_arg);

    //static char info[256];
    //sprintf_s(info, 256, "- Error @%s:%d\t",__FILE__,__LINE__);
	//Logger::getLogger() << info;

	Logger::getLogger() << tmps;

	va_end(ptr_arg);
}


inline void Info(const char *format, ...)
{
	va_list ptr_arg;
	va_start(ptr_arg, format);

	static char tmps[1024];
	vsprintf(tmps, format, ptr_arg);

    //static char info[256];
    //sprintf_s(info, 256, "- Info @%s:%d\t",__FILE__,__LINE__);
	//Logger::getLogger() << info;

	Logger::getLogger() << tmps;

	va_end(ptr_arg);
}


// locations used to store image files
#define IMAGE_REPO "..\\images"

// locations used to store effect files
#define EFFECT_REPO "..\\effects"

inline wchar_t* CharPtrToLPCWSTR(const char* charArray)
{
    wchar_t* wString = new wchar_t[4096];
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
    return wString;
}

//! It requires c++17 
struct path_leaf_string
{
	std::string operator()(const std::filesystem::directory_entry& entry) const
	{
		return entry.path().string();
	}
};

//! It requires c++17 
inline void getFiles(const std::string& name, std::vector<std::string>& v)
{
	std::filesystem::path p(name);
	std::filesystem::directory_iterator start(p);
	std::filesystem::directory_iterator end;
	std::transform(start, end, std::back_inserter(v), path_leaf_string());
}


#endif