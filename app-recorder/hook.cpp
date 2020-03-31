#include<Windows.h>
#include"../minhook/include/MinHook.h"
#pragma comment(lib,"dsound.lib")

#include"custom_dsound.h"
#include <vector>
#include <assert.h>

#ifdef _DEBUG
#define C(x) assert(x)
#else
#define C(x) x
#endif

typedef HRESULT(WINAPI* PFIDirectSoundBuffer_Lock)(LPDIRECTSOUNDBUFFER,DWORD,DWORD,LPVOID*,LPDWORD,LPVOID*,LPDWORD,DWORD);
typedef HRESULT(WINAPI* PFIDirectSoundBuffer_Unlock)(LPDIRECTSOUNDBUFFER,LPVOID,DWORD,LPVOID,DWORD);
typedef HRESULT(WINAPI* PFIDirectSoundBuffer_Play)(LPDIRECTSOUNDBUFFER,DWORD,DWORD,DWORD);
typedef HRESULT(WINAPI* PFIDirectSoundBuffer_Stop)(LPDIRECTSOUNDBUFFER);
typedef ULONG(WINAPI* PFIDirectSoundBuffer_Release)(LPDIRECTSOUNDBUFFER);
static PFIDirectSoundBuffer_Lock pfLock = nullptr, pfOriginalLock = nullptr;
static PFIDirectSoundBuffer_Unlock pfUnlock = nullptr, pfOriginalUnlock = nullptr;
static PFIDirectSoundBuffer_Play pfPlay = nullptr, pfOriginalPlay = nullptr;
static PFIDirectSoundBuffer_Stop pfStop = nullptr, pfOriginalStop = nullptr;
static PFIDirectSoundBuffer_Release pfRelease = nullptr, pfOriginalRelease = nullptr;
static HMODULE hDllModule;

DWORD GetDLLPath(LPTSTR path, DWORD max_length)
{
	return GetModuleFileName(hDllModule, path, max_length);
}

HRESULT WINAPI HookedIDirectSoundBuffer_Lock(LPDIRECTSOUNDBUFFER a, DWORD b, DWORD c, LPVOID*d, LPDWORD e, LPVOID*f, LPDWORD g, DWORD h)
{
	CustomLock(a, b, c, d, e, f, g, h);
	return pfOriginalLock(a, b, c, d, e, f, g, h);
}

HRESULT WINAPI HookedIDirectSoundBuffer_Unlock(LPDIRECTSOUNDBUFFER a, LPVOID b, DWORD c, LPVOID d, DWORD e)
{
	CustomUnlock(a, b, c, d, e);
	return pfOriginalUnlock(a, b, c, d, e);
}

HRESULT WINAPI HookedIDirectSoundBuffer_Play(LPDIRECTSOUNDBUFFER a, DWORD b, DWORD c, DWORD d)
{
	CustomPlay(a, b, c, d);
	return pfOriginalPlay(a, b, c, d);
}

HRESULT WINAPI HookedIDirectSoundBuffer_Stop(LPDIRECTSOUNDBUFFER a)
{
	CustomStop(a);
	return pfOriginalStop(a);
}

ULONG WINAPI HookedIDirectSoundBuffer_Release(LPDIRECTSOUNDBUFFER a)
{
	CustomRelease(a);
	return pfOriginalRelease(a);
}

BOOL GetPresentVAddr()
{
	LPDIRECTSOUND pDS;
	LPDIRECTSOUNDBUFFER pBuffer;
	C(SUCCEEDED(DirectSoundCreate(NULL, &pDS, NULL)));
	int bytesPerVar = 2;
	int onebufbytes = 44100 * bytesPerVar * 2 * 50 / 1000;
	WAVEFORMATEX w = { WAVE_FORMAT_PCM,2,44100,88200*bytesPerVar,4,bytesPerVar*8,sizeof w };
	DSBUFFERDESC desc = { sizeof desc,DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY |
		DSBCAPS_GLOBALFOCUS,onebufbytes*4,0,&w,DS3DALG_DEFAULT };
	HRESULT hr;
	C(SUCCEEDED(hr = pDS->CreateSoundBuffer(&desc, &pBuffer, NULL)));
	pfLock = reinterpret_cast<PFIDirectSoundBuffer_Lock>(reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pBuffer)[0])[11]);
	pfUnlock = reinterpret_cast<PFIDirectSoundBuffer_Unlock>(reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pBuffer)[0])[19]);
	pfPlay = reinterpret_cast<PFIDirectSoundBuffer_Play>(reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pBuffer)[0])[12]);
	pfStop = reinterpret_cast<PFIDirectSoundBuffer_Stop>(reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pBuffer)[0])[18]);
	pfRelease = reinterpret_cast<PFIDirectSoundBuffer_Release>(reinterpret_cast<INT_PTR*>(reinterpret_cast<INT_PTR*>(pBuffer)[0])[2]);
	pBuffer->Release();
	pDS->Release();
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StartHook()
{
	if (!GetPresentVAddr())
		return FALSE;
	if (MH_Initialize() != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfLock, HookedIDirectSoundBuffer_Lock, reinterpret_cast<void**>(&pfOriginalLock)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfUnlock, HookedIDirectSoundBuffer_Unlock, reinterpret_cast<void**>(&pfOriginalUnlock)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfPlay, HookedIDirectSoundBuffer_Play, reinterpret_cast<void**>(&pfOriginalPlay)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfStop, HookedIDirectSoundBuffer_Stop, reinterpret_cast<void**>(&pfOriginalStop)) != MH_OK)
		return FALSE;
	if (MH_CreateHook(pfRelease, HookedIDirectSoundBuffer_Release, reinterpret_cast<void**>(&pfOriginalRelease)) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfLock) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfUnlock) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfPlay) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfStop) != MH_OK)
		return FALSE;
	if (MH_EnableHook(pfRelease) != MH_OK)
		return FALSE;
	return TRUE;
}

//导出以方便在没有DllMain时调用
extern "C" __declspec(dllexport) BOOL StopHook()
{
	if (MH_DisableHook(pfLock) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfUnlock) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfPlay) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfStop) != MH_OK)
		return FALSE;
	if (MH_DisableHook(pfRelease) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfLock) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfUnlock) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfPlay) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfStop) != MH_OK)
		return FALSE;
	if (MH_RemoveHook(pfRelease) != MH_OK)
		return FALSE;
	if (MH_Uninitialize() != MH_OK)
		return FALSE;
	return TRUE;
}

DWORD WINAPI TInitHook(LPVOID param)
{
	return StartHook();
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	hDllModule = hInstDll;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInstDll);
		CreateThread(NULL, 0, TInitHook, NULL, 0, NULL);
		break;
	case DLL_PROCESS_DETACH:
		StopHook();
		break;
	case DLL_THREAD_ATTACH:break;
	case DLL_THREAD_DETACH:break;
	}
	return TRUE;
}

//SetWindowHookEx需要一个导出函数，否则DLL不会被加载
extern "C" __declspec(dllexport) LRESULT WINAPI HookProc(int code, WPARAM w, LPARAM l)
{
	return CallNextHookEx(NULL, code, w, l);
}