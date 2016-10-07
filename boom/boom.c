
#include <windows.h>
#include <d3d9.h>
#include <stdlib.h>

#pragma comment(lib, "d3d9.lib")

HINSTANCE g_hInst = NULL;
HWND g_hWnd = NULL;
LPDIRECT3D9 g_pd3d = NULL;
LPDIRECT3DDEVICE9 g_pdev = NULL;

HRESULT InitD3D(HWND hWnd)
{
	D3DPRESENT_PARAMETERS d3dpp;
	D3DDISPLAYMODE dispmode;
	HRESULT hr = NULL;

	if (NULL == (g_pd3d = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	memset(&dispmode, 0, sizeof dispmode);
	hr = g_pd3d->lpVtbl->GetAdapterDisplayMode(g_pd3d, D3DADAPTER_DEFAULT, &dispmode);

	if (FAILED(hr))
		return hr;

	memset(&d3dpp, 0, sizeof d3dpp);
	d3dpp.Windowed = FALSE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	d3dpp.BackBufferFormat = dispmode.Format;
	d3dpp.BackBufferHeight = dispmode.Height;
	d3dpp.BackBufferWidth = dispmode.Width;
	d3dpp.FullScreen_RefreshRateInHz = dispmode.RefreshRate;

	hr = g_pd3d->lpVtbl->CreateDevice(g_pd3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pdev);

	ShowCursor(FALSE);

	return hr;
}

void Cleanup()
{
	if (g_pdev) g_pdev->lpVtbl->Release(g_pdev);
	if (g_pd3d) g_pd3d->lpVtbl->Release(g_pd3d);
}

void FillScreen(D3DSURFACE_DESC desc, D3DLOCKED_RECT lockedrect)
{
	char *buf = lockedrect.pBits;
	int x = 0, tx = 0;
	int y = 0, ty = 0;
	int idx = 0;

	for (y = 0; y < desc.Height; y++)
	{
		for (x = 0; x < desc.Width; x++)
		{
			idx = y * lockedrect.Pitch + x * 4;
			buf[idx + 0] = rand();
			buf[idx + 1] = rand();
			buf[idx + 2] = rand();
			buf[idx + 3] = 0;
		}
	}
}

void Render()
{
	HRESULT hr = S_OK;
	LPDIRECT3DSURFACE9 backbuffer = NULL;
	D3DLOCKED_RECT lockedrect = { 0 };
	D3DSURFACE_DESC desc = { 0 };
	int i = 0;

	hr = g_pdev->lpVtbl->GetBackBuffer(g_pdev, 0, i, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
	if (SUCCEEDED(hr))
	{
		backbuffer->lpVtbl->GetDesc(backbuffer, &desc);
		if (desc.Format == D3DFMT_X8R8G8B8)
		{
			hr = backbuffer->lpVtbl->LockRect(backbuffer, &lockedrect, NULL, 0);
			if (SUCCEEDED(hr))
			{
				FillScreen(desc, lockedrect);
				backbuffer->lpVtbl->UnlockRect(backbuffer);
			}
			else
			{
				DestroyWindow(g_hWnd);
			}
		}
		else
		{
			DestroyWindow(g_hWnd);
		}
		backbuffer->lpVtbl->Release(backbuffer);
	}
	else
	{
		DestroyWindow(g_hWnd);
	}

	g_pdev->lpVtbl->Present(g_pdev, NULL, NULL, NULL, NULL);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CREATE)
	{
		if (FAILED(InitD3D(hWnd)))
			DestroyWindow(hWnd);
		srand(time(NULL));
		return 0;
	}
	if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST)
	{
		DestroyWindow(hWnd);
	}
	if (msg == WM_DESTROY)
	{
		Cleanup();
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
{
	WNDCLASS wc = {
		CS_CLASSDC,
		WndProc,
		0, 0,
		hInstance,
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL,
		TEXT("MainWndClass")
	};
	MSG msg = { 0 };

	if (!RegisterClass(&wc))
		return 0;

	g_hInst = hInstance;
	
	g_hWnd = CreateWindow(TEXT("MainWndClass"), TEXT("Boom!"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);
	if (g_hWnd == NULL)
		return 0;

	ShowWindow(g_hWnd, nShowCmd);
	UpdateWindow(g_hWnd);

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	return (int)msg.wParam;
}