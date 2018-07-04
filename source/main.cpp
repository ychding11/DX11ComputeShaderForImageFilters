#include <windows.h>
#include <d3d11.h>
#include "DXApplication.h"

HWND			g_hWnd = NULL;
int				width = 400;// 1920;// 720;
int				height = 400;// 1080; // 540 * 1;
DXApplication	application;

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message )
	{
    	case WM_PAINT:
    		hdc = BeginPaint( hWnd, &ps );
    		EndPaint( hWnd, &ps );
    		break;
    
    	case WM_KEYUP:
    		if(wParam == 112)
    			application.RunComputeShader(L"data/Desaturate.hlsl");
    		else if(wParam == 113)
    			application.RunComputeShader(L"data/Circles.hlsl");
    		else if (tolower((int)wParam) == 'q')
    		{
    			PostQuitMessage( 0 );
    		}
    		break;
    
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

	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	g_hWnd = CreateWindow( L"TutorialWindowClass", L"Compute Shader - Filters",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL );
	if( !g_hWnd )
		return E_FAIL;

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
	if (!application.initialize(g_hWnd, width, height))
	{
		MessageBox(NULL,L"Initialize App failed, exit!", L"Warning", MB_ICONWARNING);
		return 0;
	}
    SetWindowPos(g_hWnd, 0, 0, 0, application.imageWidth(), application.imageHeight() * 2, SWP_NOMOVE );
	// Main message loop
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
			application.RenderResult();
		}
	}

	return ( int )msg.wParam;
}