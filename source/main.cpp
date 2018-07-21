#include <windows.h>
#include <d3d11.h>                                 
#include "DX11EffectViewer.h"
#include "EffectManager.h"
#include "Logger.h"


HWND			g_hWnd = NULL;
DX11EffectViewer	application;

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message )
	{
    	case WM_PAINT:
        {
    		hdc = BeginPaint( hWnd, &ps );
    		EndPaint( hWnd, &ps );
    		break;
        }
    	case WM_KEYUP:
        {
            char key = tolower((int)wParam);
			if (wParam == VK_F1)
			{
				//application.m_csShaderFilename = L"data/Desaturate.hlsl";
    			//application.RunComputeShader();
                application.NextEffect();
			}
			else if (wParam == VK_F2)
			{
				//application.m_csShaderFilename = L"data/Circles.hlsl";
    			//application.RunComputeShader();
                application.PrevEffect();
			}
			else if (wParam == VK_ESCAPE)
			{
                SendMessage(hWnd, WM_CLOSE, 0, 0);
			}
    		else if ( key == 'q' )
    		{
    			PostQuitMessage( 0 );
    		}
			else if ( key == 'u' )
			{
    			application.RunComputeShader();
			}
    		else if ( key == 'd' )
    		{
                application.mDisplayMode = DisplayMode( (1 + application.mDisplayMode) % DisplayMode::ALL_MODE );
    		}
    		break;
        }
    	case WM_DESTROY:
    		PostQuitMessage( 0 );
    		break;
    
    	default:
    		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}

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
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = NULL;
	if( !RegisterClassEx( &wcex ) )
		return E_FAIL;

    int width  = (LONG)::GetSystemMetrics(SM_CXSCREEN);
    int height = (LONG)::GetSystemMetrics(SM_CYSCREEN);

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	g_hWnd = CreateWindow( L"TutorialWindowClass", L"Compute Shader - Filters",
		                    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL );
	if( !g_hWnd ) return E_FAIL;

	ShowWindow( g_hWnd, nCmdShow );
	return S_OK;
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
	{
		MessageBox(NULL,L"Initialize window failed, exit.", L"Warning", MB_ICONWARNING);
		return 0;
	}
	if (!application.initialize(g_hWnd))
	{
		MessageBox(NULL,L"Initialize App failed, exit!", L"Warning", MB_ICONWARNING);
		return 0;
	}
	MSG msg = {0};
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			application.Render();
		}
	}
    
    Logger::flushLogger();
	return ( int )msg.wParam;
}