//=================================================================================================
//
// 
//
//  All code licensed under the MIT license
//
//=================================================================================================

#include "DX11.h"
#include "Exceptions.h"
#include "Utility.h"

#ifdef _DEBUG
    #define UseDebugDevice_ 1
    #define BreakOnDXError_ (UseDebugDevice_ && 1)
    #define UseGPUValidation_ 0
#else
    #define UseDebugDevice_ 0
    #define BreakOnDXError_ 0
    #define UseGPUValidation_ 0
#endif

namespace GHI
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

struct VideoAdapter
{
    DXGI_ADAPTER_DESC desc;
    IDXGIAdapter* adapter;
};

static std::unordered_map<std::wstring, std::vector<VideoAdapter>> sAvailableAdapters;
static UINT sBestAdapterIndex;

static void EnumerateAdapters(void)
{
    IDXGIAdapter * pAdapter;
    IDXGIFactory* pFactory = NULL;
    bool hasNvCard = false;
    bool hasAMDCard = false;

    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory)))
    {
        throw Exception(L"Unable to create a DXGI Factory.\n "
                        L"Make sure that your OS and driver support DirectX");
    }

    WriteLog("================== Adapter List =========================");
    for (UINT i = 0;
        pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND;
        ++i)
    {
        DXGI_ADAPTER_DESC desc = { };
        pAdapter->GetDesc(&desc);
        WriteLog("adapter Index: %d  name:'%ls'", i, desc.Description);
        std::wstring strDesc(desc.Description);
        VideoAdapter adapter = { desc, pAdapter };
        if (strDesc.find(L"NVIDIA") != std::wstring::npos)
        {
            sAvailableAdapters[L"NV"].push_back(adapter);
        }
        else if (strDesc.find(L"AMD") != std::wstring::npos)
        {
            sAvailableAdapters[L"AMD"].push_back(adapter);
        }
        else if (strDesc.find(L"Intel") != std::wstring::npos)
        {
            sAvailableAdapters[L"Intel"].push_back(adapter);
        }
        else
        {
            WriteLog("\t\tVideo Adapter:'%ls' NOT recorded.", desc.Description);
        }
    }
    WriteLog("================== Adapter List =========================");
        
    if (pFactory)
    {
        pFactory->Release();
    }
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

    EnumerateAdapters();

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

