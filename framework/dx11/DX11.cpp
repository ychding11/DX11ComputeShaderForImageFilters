//=================================================================================================
//
//  MJP's DX12 Sample Framework
// 
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "DX11.h"
#include "Exceptions.h"

#ifdef _DEBUG
    #define UseDebugDevice_ 1
    #define BreakOnDXError_ (UseDebugDevice_ && 1)
    #define UseGPUValidation_ 0
#else
    #define UseDebugDevice_ 0
    #define BreakOnDXError_ 0
    #define UseGPUValidation_ 0
#endif

namespace SimpleFramework
{

namespace DX11
{

D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
IDXGIFactory* Factory = nullptr;
IDXGIAdapter* Adapter = nullptr;

static ID3D11Device* pDevice = nullptr;
static ID3D11DeviceContext     *pImmediateContext = nullptr;

uint64 CurrentCPUFrame = 0;
uint64 CurrentGPUFrame = 0;
uint64 CurrFrameIdx = 0;

static const uint64 NumCmdAllocators = RenderLatency;

static bool ShuttingDown = false;


ID3D11Device* Device()
{
	return pDevice;
}

ID3D11DeviceContext* ImmediateContext()
{
	return pImmediateContext;
}

// Converts a number to a string
template<typename T> inline std::wstring ToString(const T& val)
{
	std::wostringstream stream;
	if (!(stream << val))
		throw Exception(L"Error converting value to string");
	return stream.str();
}

// Converts a number to an ansi string
template<typename T> inline std::string ToAnsiString(const T& val)
{
	std::ostringstream stream;
	if (!(stream << val))
		throw Exception(L"Error converting value to string");
	return stream.str();
}


void WriteLog(const wchar* format, ...)
{
	wchar buffer[1024] = { 0 };
	va_list args;
	va_start(args, format);
	vswprintf_s(buffer, 1024, format, args);
	OutputDebugStringW(buffer);
	OutputDebugStringW(L"\n");
}

void WriteLog(const char* format, ...)
{
	char buffer[1024] = { 0 };
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, 1024, format, args);
	OutputDebugStringA(buffer);
	OutputDebugStringA("\n");
}


void Initialize(D3D_FEATURE_LEVEL minFeatureLevel, uint32 adapterIdx)
{
    ShuttingDown = false;

    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&Factory));
    if(FAILED(hr))
        throw Exception(L"Unable to create a DXGI 1.4 device.\n "
                        L"Make sure that your OS and driver support DirectX 12");

    LARGE_INTEGER umdVersion = { };
    Factory->EnumAdapters(adapterIdx, &Adapter);

    if(Adapter == nullptr)
        throw Exception(L"Unable to locate a DXGI 1.4 adapter that supports a D3D12 device.\n"
                        L"Make sure that your OS and driver support DirectX 12");

    DXGI_ADAPTER_DESC desc = { };
    Adapter->GetDesc(&desc);
    WriteLog("Creating DX11 device on adapter '%ls'", desc.Description);

	UINT createDeviceFlags = 0;

    #if UseDebugDevice_
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_FEATURE_LEVEL featureLevelsArray[1];
    featureLevelsArray[0] = D3D_FEATURE_LEVEL_11_0;

	DXCall(D3D11CreateDevice(Adapter,
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
		createDeviceFlags,
		featureLevelsArray, 1,
		D3D11_SDK_VERSION,
		&pDevice,
		&FeatureLevel,
		&pImmediateContext ));

    // Check the maximum feature level, and make sure it's above our minimum
    if(FeatureLevel < minFeatureLevel)
    {
        std::wstring majorLevel = ToString<int>(minFeatureLevel >> 12);
        std::wstring minorLevel = ToString<int>((minFeatureLevel >> 8) & 0xF);
        throw Exception(L"The device doesn't support the minimum feature level required to run this sample (DX" + majorLevel + L"." + minorLevel + L")");
    }
}

void Initialize(D3D_FEATURE_LEVEL minFeatureLevel)
{
    ShuttingDown = false;

	UINT createDeviceFlags = 0;

    #if UseDebugDevice_
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    D3D_FEATURE_LEVEL featureLevelsArray[1];
    featureLevelsArray[0] = D3D_FEATURE_LEVEL_11_0;

	DXCall(D3D11CreateDevice(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		featureLevelsArray, 1,
		D3D11_SDK_VERSION,
		&pDevice,
		&FeatureLevel,
		&pImmediateContext ));

    // Check the maximum feature level, and make sure it's above our minimum
    if(FeatureLevel < minFeatureLevel)
    {
        std::wstring majorLevel = ToString<int>(minFeatureLevel >> 12);
        std::wstring minorLevel = ToString<int>((minFeatureLevel >> 8) & 0xF);
        throw Exception(L"The device doesn't support the minimum feature level required to run this sample (DX" + majorLevel + L"." + minorLevel + L")");
    }

	IDXGIDevice * dxgiDevice = 0;
	DXCall(pDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)& dxgiDevice));
	DXCall(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void **)&  Adapter));
	DXCall(Adapter->GetParent(__uuidof(IDXGIFactory), (void **)& Factory));

	DXGI_ADAPTER_DESC desc = { };
	Adapter->GetDesc(&desc);
	WriteLog("Creating DX11 device on default adapter '%ls'", desc.Description);
}
void Shutdown()
{
    Assert_(CurrentCPUFrame == CurrentGPUFrame);
    ShuttingDown = true;

    Release(Factory);
    Release(Adapter);

    #if BreakOnDXError_
    #endif

    #if UseDebugDevice_ && 0
    #endif

    Release(pImmediateContext);
    Release(pDevice);
}



void DeferredRelease_(IUnknown* resource)
{
}

} // namespace
} // namespace

