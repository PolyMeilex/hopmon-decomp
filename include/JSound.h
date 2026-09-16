#pragma once
#include "common.h"
#include <dsound.h>
#include "JList.h"

// PDB type 0x1574
class JSound {
    friend struct LayoutCheck;

    JSound(const JSound&);
    JSound();
public:
    JSound(IDirectSound* _lpDSound, unsigned short _resId, unsigned long _count);
    JSound(IDirectSound* _lpDSound, char* fileName, unsigned short id, unsigned long _count);
    virtual ~JSound(); // vtable 0x00
    unsigned short GetResId() const;
    void SetVolume(unsigned char _volume);
    void Play(int looping);
    void Stop();
protected:
    void CreateWaveData(unsigned short resId);
    void CreateWaveData(char* fileName);
    void CreateDSBuffer();
    void FillDSBuffer();
private:
    IDirectSound* lpDSound; // 0x04
    unsigned short resId; // 0x08
    unsigned long count; // 0x0C
    unsigned long waveLength; // 0x10
    unsigned char* waveData; // 0x14
    unsigned long playIndex; // 0x18
    unsigned char volume; // 0x1C
    JBaseList bufferList; // 0x20
    const JSound& operator=(const JSound&);
};
ASSERT_SIZE(JSound, 0x2C);

// PDB type 0x13E0
class JSoundList {
    friend struct LayoutCheck;

    JSoundList(const JSoundList&);
    JSoundList();
public:
    JSoundList(HWND _hWind);
    virtual ~JSoundList(); // vtable 0x00
    int Create();
    void CreateSound(unsigned short resId, unsigned long count);
    void CreateSound(char* fileName, unsigned short id, unsigned long count);
    void Play(unsigned short resId, unsigned char volume, int looping);
    void Stop(unsigned short resId);
    void StopAll();
protected:
    int GetNullIndex() const;
    int FindIndexOfResId(unsigned short resId) const;
private:
    HWND hWind; // 0x04
    IDirectSound* lpDSound; // 0x08
    JSound* soundArray[30]; // 0x0C
    const JSoundList& operator=(const JSoundList&);
};
ASSERT_SIZE(JSoundList, 0x84);
