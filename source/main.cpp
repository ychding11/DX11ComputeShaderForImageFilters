#include <windows.h>
#include <d3d11.h>                                 
#include "DX11EffectViewer.h"
#include "EffectManager.h"
#include "Logger.h"
#include "Utils.h"


HWND			    g_hWnd = NULL;
DX11EffectViewer	application;

ID3D11Device*			pd3dDevice = nullptr;
ID3D11DeviceContext*	pImmediateContext = nullptr;
IDXGISwapChain*			pSwapChain = nullptr;
ID3D11RenderTargetView*	pRenderTargetView = nullptr;

unsigned int widthSwapchain;
unsigned int heightSwapchain;

#ifdef SAFE_RELEASE
#undef SAFE_RELEASE
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p) = nullptr; } }
#endif

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
    	case WM_PAINT:
        {
	        PAINTSTRUCT ps;
	        HDC hdc;
    		hdc = BeginPaint( hWnd, &ps );
    		EndPaint( hWnd, &ps );
    		break;
        }
    	case WM_KEYUP:
        {
            char key = tolower((int)wParam);
			if (wParam == VK_F1)
			{
                std::string name;
                application.NextEffect(name);
                SetWindowTextA(g_hWnd, name.c_str());
			}
			else if (wParam == VK_F2)
			{
                std::string name;
                application.PrevEffect(name);
                SetWindowTextA(g_hWnd, name.c_str());
			}
			else if (wParam == VK_F3)
			{
                std::string name;
                application.NextImage(name);
                SetWindowTextA(g_hWnd, name.c_str());
			}
			else if (wParam == VK_ESCAPE)
			{
                SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
			else if ( key == 'u' )
			{
                std::string name;
                application.UpdateEffects();
                application.NextEffect(name);
                SetWindowTextA(g_hWnd, name.c_str());
			}
    		else if ( key == 'd' )
    		{
                application.mDisplayMode = DisplayMode( (1 + application.mDisplayMode) % DisplayMode::ALL_MODE );
    		}
    		else if ( key == 'q' )
    		{
    			PostQuitMessage( 0 );
    		}
    		break;
        }
    	case WM_DESTROY:
        {
    		PostQuitMessage( 0 );
    		break;
        }
    	default:
    		return DefWindowProc( hWnd, message, wParam, lParam );
	}
	return 0;
}

#define CLASS_NAME  L"TutorialWindowClass"
#define WINDOW_NAME L"Image Effects -Compute Shader"

HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = CLASS_NAME;
	wcex.hIconSm = NULL;
	if( !RegisterClassEx( &wcex ) )
    {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
    }

    int width  = (LONG)::GetSystemMetrics(SM_CXSCREEN);
    int height = (LONG)::GetSystemMetrics(SM_CYSCREEN);

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	g_hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL );
    if (g_hWnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
    }
	ShowWindow( g_hWnd, nCmdShow );
	return S_OK;
}

 HRESULT InitializeD3D11(HWND hWnd)
{
    HRESULT hr = S_OK;
    RECT rc;
    GetClientRect(hWnd, &rc);
    unsigned int width = rc.right - rc.left;
    unsigned int height = rc.bottom - rc.top;
	widthSwapchain = width;
	heightSwapchain = height;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    D3D_DRIVER_TYPE         driverType = D3D_DRIVER_TYPE_NULL;
    D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    // Create Device, DeviceContext, SwapChain, FeatureLevel
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &pSwapChain, &pd3dDevice, &featureLevel, &pImmediateContext);
        if (SUCCEEDED(hr)) break;
    }
    if (FAILED(hr))
    {
        MessageBox(NULL, L"Create D3D Device and Swap Chain Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return S_FALSE;
    }

    // Create Render Target View Object from SwapChain's Back Buffer.
    // Access one of swap chain's back buffer.[0-based buffer index, interface type which manipulates buffer, output param]
    ID3D11Texture2D* pBackBuffer = NULL;
    hr = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    if (FAILED(hr))
    {
        MessageBox(NULL, L"Get Back Buffer from SwapChain Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return S_FALSE;
    }
    hr = pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        MessageBox(NULL, L"Create render target from Back buffer failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return S_FALSE;
    }
	return S_OK;
}

HRESULT Render(ID3D11DeviceContext*	pImmediateContext, ID3D11RenderTargetView*	pRenderTargetView )
{
    float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	pImmediateContext->OMSetRenderTargets( 1, &pRenderTargetView, NULL );
    pImmediateContext->ClearRenderTargetView(pRenderTargetView, ClearColor);
    application.Render(pImmediateContext);
	pSwapChain->Present( 0, 0 );
	return S_OK;
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    WIN_CALL_CHECK(InitWindow(hInstance, nCmdShow));
    D3D11_CALL_CHECK(InitializeD3D11(g_hWnd));
    D3D11_CALL_CHECK(application.initialize(pd3dDevice, pImmediateContext));

	MSG msg = { 0 };
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
            Render(pImmediateContext, pRenderTargetView);
		}
	}
    
	application.SaveResult();
    application.Destory();
    Logger::flushLogger();

    SAFE_RELEASE(pRenderTargetView );
    SAFE_RELEASE(pSwapChain );
    SAFE_RELEASE(pImmediateContext );
    SAFE_RELEASE(pd3dDevice );

	return ( int )msg.wParam;
}