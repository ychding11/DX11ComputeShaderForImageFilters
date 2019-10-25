//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "Exceptions.h"
#include "myassert.h"

namespace GHI
{

    inline std::wstring StrToWstr(const char* ansiString)
    {
        wchar buffer[512];
        Win32Call(MultiByteToWideChar(CP_ACP, 0, ansiString, -1, buffer, 512));
        return std::wstring(buffer);
    }

    inline std::string WstrToStr(const wchar* wideString)
    {
        char buffer[512];
        Win32Call(WideCharToMultiByte(CP_ACP, 0, wideString, -1, buffer, 612, NULL, NULL));
        return std::string(buffer);
    }

    // Splits up a string using a delimiter
    inline void Split(const std::wstring& str, std::vector<std::wstring>& parts, const std::wstring& delimiters = L" ")
    {
        // Skip delimiters at beginning
        std::wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);

        // Find first "non-delimiter"
        std::wstring::size_type pos = str.find_first_of(delimiters, lastPos);

        while(std::wstring::npos != pos || std::wstring::npos != lastPos)
        {
            // Found a token, add it to the vector
            parts.push_back(str.substr(lastPos, pos - lastPos));

            // Skip delimiters.  Note the "not_of"
            lastPos = str.find_first_not_of(delimiters, pos);

            // Find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }
    }

    // Splits up a string using a delimiter
    inline std::vector<std::wstring> Split(const std::wstring& str, const std::wstring& delimiters = L" ")
    {
        std::vector<std::wstring> parts;
        Split(str, parts, delimiters);
        return parts;
    }

    // Splits up a string using a delimiter
    inline void Split(const std::string& str, std::vector<std::string>& parts, const std::string& delimiters = " ")
    {
        // Skip delimiters at beginning
        std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);

        // Find first "non-delimiter"
        std::string::size_type pos = str.find_first_of(delimiters, lastPos);

        while(std::string::npos != pos || std::string::npos != lastPos)
        {
            // Found a token, add it to the vector
            parts.push_back(str.substr(lastPos, pos - lastPos));

            // Skip delimiters.  Note the "not_of"
            lastPos = str.find_first_not_of(delimiters, pos);

            // Find next "non-delimiter"
            pos = str.find_first_of(delimiters, lastPos);
        }
    }

    // Splits up a string by a delimiter
    inline std::vector<std::string> Split(const std::string& str, const std::string& delimiters = " ")
    {
        std::vector<std::string> parts;
        Split(str, parts, delimiters);
        return parts;
    }

    // Parses a string into a number
    template<typename T> inline T Parse(const std::wstring& str)
    {
        std::wistringstream stream(str);
        wchar_t c;
        T x;
        if(!(str >> x) || stream.get(c))
            throw Exception(L"Can't parse string \"" + str + L"\"");
        return x;
    }

    // Converts a number to a string
    template<typename T> inline std::wstring ToString(const T& val)
    {
        std::wostringstream stream;
        if(!(stream << val))
            throw Exception(L"Error converting value to string");
        return stream.str();
    }

    // Converts a number to an ansi string
    template<typename T> inline std::string ToAnsiString(const T& val)
    {
        std::ostringstream stream;
        if(!(stream << val))
            throw Exception(L"Error converting value to string");
        return stream.str();
    }

    std::wstring WriteLog(const wchar* format, ...);
    std::string  WriteLog(const char* format, ...);

    #if 0
    #if defined(_DEBUG)
    #define DLOG  WriteLog
    #else
    #define DLOG  
    #endif
    #endif

    #if defined( DEBUG ) || defined( _DEBUG )
    #define DLOG(fmt,...)  //WriteLog("- [Debug] " fmt, ##__VA_ARGS__)
    #else
    #define DLOG(fmt,...)
    #endif

    #if defined( DEBUG ) || defined( _DEBUG )
    #define ELOG(fmt,...)  WriteLog("- [Error] %s():%s\t" fmt, __FUNCTION__, __FILE__, ##__VA_ARGS__)
    #else
    #define ELOG(fmt,...)
    #endif

    std::wstring MakeString(const wchar* format, ...);
    std::string MakeString(const char* format, ...);

    std::wstring SampleFrameworkDir();

    inline void toOutputWindow(const std::wstring& str)
    {
        std::wstring output = L"Error:" + str + L"\n";
        OutputDebugStringW(output.c_str());
        std::printf("%ls", output.c_str());
    }


    // Gets an index from an index buffer
    inline uint32 GetIndex(const void* indices, uint32 idx, uint32 indexSize)
    {
        if(indexSize == 2)
            return reinterpret_cast<const uint16*>(indices)[idx];
        else
            return reinterpret_cast<const uint32*>(indices)[idx];
    }

    // Gets an index from an index buffer
    template<typename T>
    T GetValue(const void* indices, uint32 idx)
    {
        return reinterpret_cast<const T*>(indices)[idx];
    }

    template<typename T, uint64 N>
    uint64 ArraySize(T(&)[N])
    {
        return N;
    }

    #define ArraySize_(x) ((sizeof(x) / sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

    inline uint64 AlignTo(uint64 num, uint64 alignment)
    {
        Assert_(alignment > 0);
        return ((num + alignment - 1) / alignment) * alignment;
    }


    ////// File IO related Function //////

    // Returns the extension of the file path: 
    // For example: xxx.cpp return cpp
    std::wstring FileExtension(const wchar_t* filePath_);
    std::string  FileExtension(const char* filePath_);

    // Returns the name of the file given the path (extension included)
    std::wstring FullFileName(const wchar* filePath_);

    // Load file as a whole string
    std::string LoadStr(std::string filename);
    void ArchiveStr(const std::string &str, std::string filename);
    
    // Returns the extension of the file path
    inline std::string ExtractExtension(const std::string & filePath)
    {
        Assert_(filePath.size() > 0);
        size_t idx = filePath.rfind('.');
        if (idx != std::string::npos)
            return filePath.substr(idx + 1, filePath.length() - idx - 1);
        else
            return std::string("");
    }

    // Returns the directory containing a file
    inline std::string ExtractDirectory(const std::string & filePath)
    {
        Assert_(filePath.size() > 0);

        size_t idx = filePath.rfind('\\');
        if (idx != std::wstring::npos)
            return filePath.substr(0, idx + 1);
        else
            return std::string("");
    }

}////namespace