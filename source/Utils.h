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
        sprintf_s(buf, 512, "- Error @%s:%d\t  %s 0x%x\t \n",__FUNCTION__,__FILE__,__LINE__, #x, (ret) );  \
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
        sprintf_s(buf, 512, "- Error @%s,%s:%d\t  %s 0x%x\t \n",__FUNCTION__, __FILE__,__LINE__, #x, (ret) );  \
        OutputDebugStringA(buf);                      \
	    Logger::getLogger() << buf;                   \
		return false;                                 \
    }                                                 \
} while(0)

inline void output(const char *format, ...)
{
	va_list ptr_arg;
	va_start(ptr_arg, format);

	char tmps[1024];
	vsprintf(tmps, format, ptr_arg);

	OutputDebugStringA(tmps);
	OutputDebugStringA("\n");

	va_end(ptr_arg);
}

#define INFO(fmt,...)  output("- [Info] " fmt, ##__VA_ARGS__)
//#define ERROR(fmt,...) output("- [Error] " fmt, ##__VA_ARGS__)

#if defined( DEBUG ) || defined( _DEBUG )
#define DEBUG(fmt,...)  output("- [Debug] " fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt,...)
#endif

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