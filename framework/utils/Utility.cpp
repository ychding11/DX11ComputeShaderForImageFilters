//=================================================================================================
//
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "Utility.h"
#include "Exceptions.h"
#include "App.h"

namespace GHI
{

    void WriteLog(const wchar* format, ...)
    {
        wchar buffer[4096] = { 0 };
        va_list args;
        va_start(args, format);
        vswprintf_s(buffer, ArraySize_(buffer), format, args);

        OutputDebugStringW(buffer);
        OutputDebugStringW(L"\n");
    }

    void WriteLog(const char* format, ...)
    {
        char buffer[4096] = { 0 };
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, ArraySize_(buffer), format, args);
        OutputDebugStringA(buffer);
        OutputDebugStringA("\n");
    }

    std::wstring MakeString(const wchar* format, ...)
    {
        wchar buffer[1024] = { 0 };
        va_list args;
        va_start(args, format);
        vswprintf_s(buffer, ArraySize_(buffer), format, args);
        return std::wstring(buffer);
    }

    std::string MakeString(const char* format, ...)
    {
        char buffer[1024] = { 0 };
        va_list args;
        va_start(args, format);
        vsprintf_s(buffer, ArraySize_(buffer), format, args);
        return std::string(buffer);
    }

    std::wstring SampleFrameworkDir()
    {
        return std::wstring(L"");
    }

    std::wstring FileExtension(const wchar_t* filePath_)
    {
        Assert_(filePath_);

        std::wstring filePath(filePath_);
        size_t idx = filePath.rfind(L'.');
        if (idx != std::wstring::npos)
            return filePath.substr(idx + 1, filePath.length() - idx - 1);
        else
            return std::wstring(L"");
    }

    std::string FileExtension(const char* filePath_)
    {
        Assert_(filePath_);

        std::string filePath(filePath_);
        size_t idx = filePath.rfind('.');
        if (idx != std::string::npos)
            return filePath.substr(idx + 1, filePath.length() - idx - 1);
        else
            return std::string("");
    }

    std::wstring FullFileName(const wchar* filePath_)
    {
        Assert_(filePath_);

        std::wstring filePath(filePath_);
        size_t idx = filePath.rfind(L'\\');
        if (idx != std::wstring::npos && idx < filePath.length() - 1)
            return filePath.substr(idx + 1);
        else
        {
            idx = filePath.rfind(L'/');
            if (idx != std::wstring::npos && idx < filePath.length() - 1)
                return filePath.substr(idx + 1);
            else
                return filePath;
        }
    }
/*
    // Note: elegant but very slow 
    // See http://0x80.pl/notesen/2019-01-07-cpp-read-file.html For details
    std::string load1(const std::string& path)
    {
        std::ifstream file(path);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }

    std::string load2(const std::string& path)
    {
        auto ss = std::ostringstream{};
        std::ifstream file(path);
        ss << file.rdbuf();
        return ss.str();
    }

*/
    // Note: Use c stand read() is much faster than "stream" in C++.
    std::string LoadStr(std::string filename)
    {
        std::ifstream in(filename);
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t len = in.tellg();
            in.seekg(0);
            std::string contents(len + 1, '\0');
            in.read(&contents[0], len);
            in.close();
            return contents;
        }
        else
        {
            return "";
        }
    }

    void ArchiveStr(const std::string &str, std::string filename)
    {
        std::ofstream out(filename);
        if (out)
        {
            out.write(str.c_str(), str.size());
            out.close();
        }
        else
        {
        }
    }

}