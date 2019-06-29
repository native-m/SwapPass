#ifndef __MAIN_H
#define __MAIN_H

#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <thread>
#include "MinHook.h"

typedef HRESULT(STDMETHODCALLTYPE *IDXGISwapChain$Present_Pfn)(
	IDXGISwapChain * This,
	UINT SyncInterval,
	UINT Flags);

static bool g_bInitialized = false;
static bool g_bD3DInitialized = false;
static bool g_bShaderInitialized = false;
static bool g_bZBufInitialized = false;
static POINT g_ptTargetSize = { 0 };
static DWORD_PTR g_dwPresentAddr = 0;
static IDXGISwapChain$Present_Pfn g_dwPresentFunc = 0;
static ID3D11Device* g_pDev;
static ID3D11DeviceContext* g_pCtx;
static ID3D11Texture2D* g_pRTTexture;
static ID3D11SamplerState* g_pRTState;
static ID3D11ShaderResourceView* g_pRTTextureSRV;
static ID3D11Texture2D* g_pZBufTexture;
static ID3D11ShaderResourceView* g_pZBufSRV;
static ID3D11VertexShader* g_pVS;
static ID3D11PixelShader* g_pPS;

template <typename T>
MH_STATUS CreateHook(LPVOID lpvTarget, LPVOID lpvDetour, T** pOriginal)
{
	return MH_CreateHook(lpvTarget, lpvDetour, (LPVOID*)pOriginal);
}

DWORD WINAPI Main(LPVOID lpvParam);
void SetupHook();
HRESULT WINAPI HookPresent(IDXGISwapChain* This, UINT SyncInterval, UINT Flags);
HWND FindMainWindowInProcess(DWORD dwPID);
void InitD3D(IDXGISwapChain* This, DXGI_SWAP_CHAIN_DESC& scDesc);
void InitResource(const DXGI_SWAP_CHAIN_DESC& scDesc);
void InitZbuf(const D3D11_TEXTURE2D_DESC& zdesc);
void InitShader();

#endif