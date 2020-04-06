#include"custom_dsound.h"
#include<cstdio>
#include<iostream>
#include<map>

struct WaveStructure
{
	char strRIFF[4];
	int chunkSize;
	char strFormat[4];
	char strFmt[4];
	int subchunk1Size;
	short audioFormat;
	short numChannels;
	int sampleRate;
	int byteRate;
	short blockAlign;
	short bpsample;//Bits per sample
	char strData[4];
	int subchunk2Size;//Data size（字节数）
};

#define FCLOSE(f){fclose(f);f=NULL;}

class DSCustom
{
private:
	LPDIRECTSOUNDBUFFER m_pBuffer;
	FILE* fWaveOut;
	int sWaveSize;
public:
	DSCustom():m_pBuffer(),fWaveOut(),sWaveSize(){}
	void Init(LPDIRECTSOUNDBUFFER pBuffer)
	{
		m_pBuffer = pBuffer;
	}
	void Lock(DWORD b, DWORD c, LPVOID* d, LPDWORD e, LPVOID* f, LPDWORD g, DWORD h)
	{
	}
	void Unlock(LPVOID pLockedBuffer, DWORD lockedBufferBytes, LPVOID d, DWORD e)
	{
		if (!fWaveOut)
			return;
		fwrite(pLockedBuffer, lockedBufferBytes, 1, fWaveOut);
		sWaveSize += lockedBufferBytes;
	}
	void Play(DWORD b, DWORD c, DWORD d)
	{
		WCHAR filename[MAX_PATH],dllpath[MAX_PATH],hostname[MAX_PATH];
		GetDLLPath(dllpath, MAX_PATH-1);
		GetModuleFileName(GetModuleHandle(NULL), hostname, MAX_PATH - 1);
		wcsrchr(dllpath, '\\')[1] = 0;
		wsprintf(filename, TEXT("%s%s_%p.wav"), dllpath, wcsrchr(hostname, '\\') + 1, m_pBuffer);
		if (fWaveOut)
			FCLOSE(fWaveOut);
		_wfopen_s(&fWaveOut, filename, TEXT("wb"));
		if (fWaveOut)
			fseek(fWaveOut,sizeof(WaveStructure),SEEK_SET);
		else
			MessageBox(NULL, TEXT("打开文件失败。"), NULL, MB_ICONERROR);
		sWaveSize = 0;
	}
	void Stop()
	{
		if (!fWaveOut)
			return;
		WAVEFORMATEX fmt;
		m_pBuffer->GetFormat(&fmt, sizeof(WAVEFORMATEX), nullptr);
		WaveStructure wavfileheader =
		{
			'R','I','F','F',//strRIFF
			0,//chunkSize
			'W','A','V','E',//strFormat
			'f','m','t',' ',//strFmt
			16,//subchunk1Size
			WAVE_FORMAT_PCM,//audioFormat
			(short)fmt.nChannels,//numChannels
			(int)fmt.nSamplesPerSec,//sampleRate
			(int)fmt.nSamplesPerSec * fmt.nChannels * fmt.wBitsPerSample/8,//byteRate
			(short)(fmt.nChannels * fmt.wBitsPerSample/8),//blockAlign
			(short)(fmt.wBitsPerSample),//bpsample
			'd','a','t','a',//strData
			sWaveSize//subchunk2Size
		};
		wavfileheader.chunkSize = 36 + wavfileheader.subchunk2Size;
		fseek(fWaveOut, 0, SEEK_SET);
		fwrite(&wavfileheader, sizeof(WaveStructure), 1, fWaveOut);
		FCLOSE(fWaveOut);
	}
	void Release()
	{
		Stop();
		if (fWaveOut)
			FCLOSE(fWaveOut);
	}
};

static std::map<LPDIRECTSOUNDBUFFER, DSCustom> cp;

void CustomLock(LPDIRECTSOUNDBUFFER a, DWORD b, DWORD c, LPVOID* d, LPDWORD e, LPVOID* f, LPDWORD g, DWORD h)
{
	if (cp.find(a) == cp.end())
		return;
	cp[a].Lock(b,c,d,e,f,g,h);
}

void CustomUnlock(LPDIRECTSOUNDBUFFER a, LPVOID b, DWORD c, LPVOID d, DWORD e)
{
	if (cp.find(a) == cp.end())
		return;
	cp[a].Unlock(b,c,d,e);
}

void CustomPlay(LPDIRECTSOUNDBUFFER a, DWORD b, DWORD c, DWORD d)
{
	if (cp.find(a) == cp.end())
	{
		cp.insert(std::make_pair(a, DSCustom()));
		cp[a].Init(a);
	}
	cp[a].Play(b, c, d);
}

void CustomStop(LPDIRECTSOUNDBUFFER a)
{
	if (cp.find(a) == cp.end())
		return;
	cp[a].Stop();
}

void CustomRelease(LPDIRECTSOUNDBUFFER a)
{
	if (cp.find(a) == cp.end())
		return;
	cp[a].Release();
}
