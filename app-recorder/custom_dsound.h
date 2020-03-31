#pragma once
#include<dsound.h>
#ifdef __cplusplus
extern "C" {
#endif
void CustomLock(LPDIRECTSOUNDBUFFER a, DWORD b, DWORD c, LPVOID* d, LPDWORD e, LPVOID* f, LPDWORD g, DWORD h);
void CustomUnlock(LPDIRECTSOUNDBUFFER a, LPVOID b, DWORD c, LPVOID d, DWORD e);
void CustomPlay(LPDIRECTSOUNDBUFFER a, DWORD b, DWORD c, DWORD d);
void CustomStop(LPDIRECTSOUNDBUFFER a);
void CustomRelease(LPDIRECTSOUNDBUFFER a);
DWORD GetDLLPath(LPTSTR path, DWORD max_length);
#ifdef __cplusplus
}
#endif
