#include "Main.h"
#include "ShaderTest.h"

DWORD WINAPI Main(LPVOID lpvParam)
{
	// An infinite loop that keeps this DLL alive
	while (true)
	{
		if (!g_bInitialized)
		{
			SetupHook();
			g_bInitialized = true;
		}

		Sleep(1000);
	}
}

void SetupHook()
{
	HWND tempWindow = nullptr;
	static const D3D_FEATURE_LEVEL level[] = {
		D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};
	D3D_FEATURE_LEVEL supported;

	while(!tempWindow)
	{
		tempWindow = FindMainWindowInProcess(GetCurrentProcessId());
		Sleep(17);
	}

	// temporary variables
	HRESULT hr = -1;
	DXGI_SWAP_CHAIN_DESC scd = { 0 };
	IDXGISwapChain* swapchain;
	ID3D11Device* device;
	ID3D11DeviceContext* ctx;

	scd.BufferCount = 1;
	scd.BufferDesc.Width = 0;
	scd.BufferDesc.Height = 0;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = tempWindow;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = (GetWindowLong(tempWindow, GWL_STYLE) & WS_POPUP) != 0 ? FALSE : TRUE;
	//scd.Windowed = FALSE;

	// Loop until initialized
	// TODO: Add timer
	while(FAILED(hr))
	{
		if (FAILED(hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			level,
			7,
			D3D11_SDK_VERSION,
			&scd,
			&swapchain,
			&device,
			&supported,
			&ctx)))
		{
			if (FAILED(hr = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				0,
				&level[1],
				6,
				D3D11_SDK_VERSION,
				&scd,
				&swapchain,
				&device,
				&supported,
				&ctx)))
			{
				continue;
			}
		}
	}

	// take Present() function address from vtable
	g_dwPresentAddr = (*(DWORD_PTR**)swapchain)[8];

	// TODO: Detour here
	if (CreateHook((LPVOID)g_dwPresentAddr, &HookPresent, &g_dwPresentFunc) != MH_OK)
		MessageBox(nullptr, L"Cannot hook Present() function", L"Error", MB_OK);
	else
		MH_EnableHook(MH_ALL_HOOKS);

	ctx->Release();
	device->Release();
	swapchain->Release();
	DestroyWindow(tempWindow);
}

HRESULT WINAPI HookPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags)
{
	ID3D11Resource* tex = nullptr;
	ID3D11Texture2D* zbuf = nullptr;
	ID3D11RenderTargetView* view = nullptr;
	ID3D11DepthStencilView* zview = nullptr;
	DXGI_SWAP_CHAIN_DESC scDesc;
	D3D11_TEXTURE2D_DESC zbDesc;
	bool resChanged = false;

	This->GetDesc(&scDesc);

	if (!g_bD3DInitialized)
		InitD3D(This, scDesc);

	if (g_ptTargetSize.x != scDesc.BufferDesc.Width ||
		g_ptTargetSize.y != scDesc.BufferDesc.Height)
	{
		InitD3D(This, scDesc);
		resChanged = true;
	}

	g_pCtx->OMGetRenderTargets(1, &view, &zview); // actually we can do this through DXGI

	// Apply our custom shader pass
	if(view)
	{
		// Paste screen buffer into our shader pass
		view->GetResource(&tex);
		g_pCtx->CopyResource(g_pRTTexture, tex);

		// Setup everything
		g_pCtx->OMSetBlendState(nullptr, nullptr, 0xffffffff); // Disable blend state
		g_pCtx->IASetInputLayout(nullptr);
		g_pCtx->VSSetShader(g_pVS, nullptr, 0);
		g_pCtx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		g_pCtx->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
		g_pCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		g_pCtx->PSSetShader(g_pPS, nullptr, 0);
		g_pCtx->PSSetShaderResources(0, 1, &g_pRTTextureSRV);
		g_pCtx->PSSetSamplers(0, 1, &g_pRTState);

		// TAKE DEPTH BUFFER
		// it doesnt work lmao
		if (zview)
		{
			zview->GetResource((ID3D11Resource**)&zbuf);
			if (resChanged || !g_bZBufInitialized)
			{
				zbuf->GetDesc(&zbDesc);
				InitZbuf(zbDesc);
			}
			g_pCtx->CopyResource(g_pZBufTexture, zbuf);

			g_pCtx->PSSetShaderResources(1, 1, &g_pZBufSRV);
		}

		g_pCtx->GSSetShader(nullptr, nullptr, 0); // just in case
		g_pCtx->Draw(6, 0); // Draw our pass into screen

		// Release all resources
		view->Release();
		if (zview) zview->Release();
	}

	return g_dwPresentFunc(This, SyncInterval, Flags);
}

HWND FindMainWindowInProcess(DWORD dwPID)
{
	typedef struct {
		DWORD dwPID;
		HWND hWnd;
	} Param;

	// UwU
	WNDENUMPROC enumWindowProc = [](HWND hWnd, LPARAM lParam) -> BOOL {
		Param* p = (Param*)lParam;
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		if (p->dwPID != pid || !(GetWindow(hWnd, GW_OWNER) == nullptr && IsWindowVisible(hWnd)))
			return TRUE;
		p->hWnd = hWnd;
		return FALSE;
	};

	Param p;
	p.dwPID = dwPID;
	p.hWnd = nullptr;

	EnumWindows(enumWindowProc, (LPARAM)&p);

	return p.hWnd;
}

void InitD3D(IDXGISwapChain* This, DXGI_SWAP_CHAIN_DESC& scDesc)
{
	This->GetDevice(__uuidof(g_pDev), (void**)&g_pDev);
	g_pDev->GetImmediateContext(&g_pCtx);

	InitResource(scDesc);
	if (!g_bShaderInitialized)
		InitShader();
	g_bD3DInitialized = true;
}

void InitResource(const DXGI_SWAP_CHAIN_DESC& scDesc)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC desc = { 0 };
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	D3D11_SAMPLER_DESC stDesc;
	FLOAT borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	g_ptTargetSize.x = scDesc.BufferDesc.Width;
	g_ptTargetSize.y = scDesc.BufferDesc.Height;

	desc.Width = g_ptTargetSize.x;
	desc.Height = g_ptTargetSize.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = scDesc.BufferDesc.Format;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	stDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	stDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	stDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	stDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	stDesc.MipLODBias = 0.0f;
	stDesc.MaxAnisotropy = 1;
	stDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	memcpy_s(stDesc.BorderColor, sizeof(FLOAT) * 4, borderColor, sizeof(FLOAT) * 4);
	stDesc.MinLOD = -FLT_MAX;
	stDesc.MaxLOD = FLT_MAX;

	if (g_pDev)
	{
		if (g_pRTTexture)
			g_pRTTexture->Release();
		if (g_pRTTextureSRV)
			g_pRTTextureSRV->Release();
		if (g_pRTState)
			g_pRTState->Release();

		hr = g_pDev->CreateTexture2D(&desc, nullptr, &g_pRTTexture);
		hr = g_pDev->CreateShaderResourceView(g_pRTTexture, &srvDesc, &g_pRTTextureSRV);
		hr = g_pDev->CreateSamplerState(&stDesc, &g_pRTState);
	}
}

void InitZbuf(const D3D11_TEXTURE2D_DESC & zdesc)
{
	HRESULT hr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	srvDesc.Format = zdesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = zdesc.MipLevels - 1;
	srvDesc.Texture2D.MipLevels = zdesc.MipLevels;

	if (g_pDev)
	{
		if (g_pZBufTexture)
			g_pZBufTexture->Release();
		if (g_pZBufSRV)
			g_pZBufSRV->Release();

		hr = g_pDev->CreateTexture2D(&zdesc, nullptr, &g_pZBufTexture);
		hr = g_pDev->CreateShaderResourceView(g_pZBufTexture, &srvDesc, &g_pZBufSRV);

		g_bZBufInitialized = true;
	}
}

void InitShader()
{
	HRESULT hr;
	ID3D10Blob* vsBlob = nullptr, *vsErrBlob = nullptr;
	ID3D10Blob* psBlob = nullptr, *psErrBlob = nullptr;

	if (FAILED(D3DCompile(g_strVS, sizeof(g_strVS), "SwapPassVS", nullptr, nullptr, "MainVS", "vs_4_0", 0, 0, &vsBlob, &vsErrBlob)))
	{
		OutputDebugString(L"Cannot compile VS");
		if (vsErrBlob)
		{
			OutputDebugStringA((LPCSTR)vsErrBlob->GetBufferPointer());
			vsErrBlob->Release();
		}
		return;
	}

	if (FAILED(D3DCompile(g_strPS, sizeof(g_strPS), "SwapPassPS", nullptr, nullptr, "MainPS", "ps_4_0", 0, 0, &psBlob, &psErrBlob)))
	{
		OutputDebugString(L"Cannot compile PS");
		if (psErrBlob)
		{
			OutputDebugStringA((LPCSTR)psErrBlob->GetBufferPointer());
			psErrBlob->Release();
		}
		return;
	}

	hr = g_pDev->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_pVS);
	hr = g_pDev->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pPS);

	g_bShaderInitialized = true;
	vsBlob->Release();
	psBlob->Release();
}
