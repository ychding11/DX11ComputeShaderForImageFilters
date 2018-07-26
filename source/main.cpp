#include <windows.h>
#include <d3d11.h>                                 
#include "DX11EffectViewer.h"
#include "EffectManager.h"
#include "Logger.h"


HWND			g_hWnd = NULL;
DX11EffectViewer	application;

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
#define WINDOW_NAME L"Compute Shader - Filters"

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
		return E_FAIL;

    int width  = (LONG)::GetSystemMetrics(SM_CXSCREEN);
    int height = (LONG)::GetSystemMetrics(SM_CYSCREEN);

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	g_hWnd = CreateWindow( CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL );
	if( !g_hWnd ) return E_FAIL;

	ShowWindow( g_hWnd, nCmdShow );
	return S_OK;
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
	{
        Logger::getLogger() << "Initialize window failed, exit." << "\n";
		return 0;
	}
	if (!application.Initialize(g_hWnd))
	{
        Logger::getLogger() << "Initialize App failed, exit!" << "\n";
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