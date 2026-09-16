#include "JSound.h"

// 0x00412140, 170 bytes.
JSound::JSound(IDirectSound* _lpDSound, char* fileName, unsigned short id, unsigned long _count)
    : lpDSound(_lpDSound), resId(id), count(_count), waveLength(0), waveData(NULL), playIndex(0), volume(100)
{
    CreateWaveData(fileName);
}

// 0x004121F0, 167 bytes.
JSound::JSound(IDirectSound* _lpDSound, unsigned short _resId, unsigned long _count)
    : lpDSound(_lpDSound), resId(_resId), count(_count), waveLength(0), waveData(NULL), playIndex(0)
{
    CreateWaveData(resId);
}

// 0x004122A0, 214 bytes.
JSound::~JSound()
{
    if (waveData != NULL) {
        delete waveData;
        waveData = NULL;
    }
    for (unsigned long i = 0; i < bufferList.Count(); i++) {
        IDirectSoundBuffer* buffer = (IDirectSoundBuffer*)bufferList.GetData(i);
        if (buffer != NULL) {
            buffer->Release();
            buffer = NULL;
        }
    }
    bufferList.Empty();
}

// 0x00412380, 18 bytes.
unsigned short JSound::GetResId() const
{
    return resId;
}

// 0x004123A0, 40 bytes.
void JSound::SetVolume(unsigned char _volume)
{
    if (_volume > 100) {
        volume = 100;
    } else {
        volume = _volume;
    }
}

// 0x004123D0, 251 bytes.
void JSound::Play(int looping)
{
    if (volume == 0) {
        return;
    }
    IDirectSoundBuffer* buffer = (IDirectSoundBuffer*)bufferList.GetData(playIndex);
    if (buffer != NULL) {
        buffer->Stop();
        buffer->SetCurrentPosition(0);
        long dsVolume = (long)((float)volume / 100.0f * 10000.0f) - 10000;
        buffer->SetVolume(dsVolume);
        HRESULT hr = buffer->Play(0, 0, (looping == 1) ? 1 : 0);
        if (hr == DSERR_BUFFERLOST) {
            buffer->Restore();
            FillDSBuffer();
        }
    }
    playIndex = (playIndex + 1) % count;
}

// 0x004124D0, 103 bytes.
void JSound::Stop()
{
    for (unsigned long i = 0; i < bufferList.Count(); i++) {
        IDirectSoundBuffer* buffer = (IDirectSoundBuffer*)bufferList.GetData(i);
        if (buffer != NULL) {
            buffer->Stop();
            buffer->SetCurrentPosition(0);
        }
    }
}

// 0x00412540, 289 bytes.
void JSound::CreateWaveData(char* fileName)
{
    HMMIO hmmio = mmioOpenA(fileName, NULL, MMIO_READ | MMIO_ALLOCBUF);

    MMCKINFO riffChunk;
    memset(&riffChunk, 0, sizeof(MMCKINFO));
    riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    MMRESULT mmr = mmioDescend(hmmio, &riffChunk, NULL, MMIO_FINDRIFF);

    MMCKINFO chunk;
    memset(&chunk, 0, sizeof(MMCKINFO));
    chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mmr = mmioDescend(hmmio, &chunk, &riffChunk, 0);
    WAVEFORMATEX fmtBuf;
    LONG bytesRead = mmioRead(hmmio, (HPSTR)&fmtBuf, sizeof(WAVEFORMATEX));
    mmr = mmioAscend(hmmio, &chunk, 0);

    chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    mmr = mmioDescend(hmmio, &chunk, &riffChunk, MMIO_FINDCHUNK);

    waveLength = chunk.cksize;
    waveData = new unsigned char[chunk.cksize];
    bytesRead = mmioRead(hmmio, (HPSTR)waveData, waveLength);

    CreateDSBuffer();
    FillDSBuffer();
}

// 0x00412670, 350 bytes.
void JSound::CreateWaveData(unsigned short resId)
{
    HRSRC hResInfo = FindResourceA(NULL, MAKEINTRESOURCEA(resId), "WAVE");
    HGLOBAL hResData = LoadResource(NULL, hResInfo);
    unsigned char* data = (unsigned char*)LockResource(hResData);

    unsigned long* p = (unsigned long*)data;
    unsigned long riffId = *p++;
    unsigned long chunkSize = *p++;
    unsigned long chunkId = *p++;
    unsigned char* end = (unsigned char*)p + chunkSize - 4;

    unsigned char* fmtData = NULL;
    unsigned char* waveDataPtr = NULL;
    unsigned long dataSize = 0;
    do {
        if ((unsigned char*)p >= end) {
            break;
        }
        chunkId = *p++;
        chunkSize = *p++;
        if (chunkId == mmioFOURCC('f', 'm', 't', ' ')) {
            if (fmtData == NULL) {
                fmtData = (unsigned char*)p;
            }
        } else if (chunkId == mmioFOURCC('d', 'a', 't', 'a')) {
            if (waveDataPtr == NULL) {
                waveDataPtr = (unsigned char*)p;
                dataSize = chunkSize;
            }
        }
        p = (unsigned long*)((unsigned char*)p + ((chunkSize + 1) & ~1));
    } while (fmtData == NULL || waveDataPtr == NULL);

    waveLength = dataSize;
    waveData = new unsigned char[dataSize];
    memcpy(waveData, waveDataPtr, dataSize);

    CreateDSBuffer();
    FillDSBuffer();
}

// 0x004127D0, 229 bytes.
void JSound::CreateDSBuffer()
{
    PCMWAVEFORMAT wfx;
    memset(&wfx, 0, sizeof(PCMWAVEFORMAT));
    wfx.wf.wFormatTag = WAVE_FORMAT_PCM;
    wfx.wf.nChannels = 1;
    wfx.wf.nSamplesPerSec = 22050;
    wfx.wf.nBlockAlign = 1;
    wfx.wf.nAvgBytesPerSec = wfx.wf.nBlockAlign * 22050;
    wfx.wBitsPerSample = 8;

    DSBUFFERDESC dsbd;
    memset(&dsbd, 0, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_CTRLVOLUME;
    dsbd.dwBufferBytes = waveLength;
    dsbd.lpwfxFormat = (WAVEFORMATEX*)&wfx;

    for (unsigned long i = 0; i < count; i++) {
        LPDIRECTSOUNDBUFFER buffer;
        HRESULT hr = lpDSound->CreateSoundBuffer(&dsbd, &buffer, NULL);
        bufferList.Append(buffer);
    }
}

// 0x004128C0, 283 bytes.
void JSound::FillDSBuffer()
{
    for (unsigned long i = 0; i < bufferList.Count(); i++) {
        IDirectSoundBuffer* buffer = (IDirectSoundBuffer*)bufferList.GetData(i);
        if (buffer != NULL) {
            void* ptr1;
            unsigned long bytes1;
            void* ptr2;
            unsigned long bytes2;
            HRESULT hr = buffer->Lock(0, waveLength, &ptr1, &bytes1, &ptr2, &bytes2, 0);
            if (hr == DSERR_BUFFERLOST) {
                buffer->Restore();
                hr = buffer->Lock(0, waveLength, &ptr1, &bytes1, &ptr2, &bytes2, 0);
            }
            memcpy(ptr1, waveData, bytes1);
            if (ptr2 != NULL) {
                memcpy(ptr2, waveData + bytes1, bytes2);
            }
            hr = buffer->Unlock(ptr1, bytes1, ptr2, bytes2);
        }
    }
}

// 0x004129E0, 86 bytes.
JSoundList::JSoundList(HWND _hWind)
    : hWind(_hWind), lpDSound(NULL)
{
    for (int i = 0; i < 30; i++) {
        soundArray[i] = NULL;
    }
}

// 0x00412A40, 214 bytes.
JSoundList::~JSoundList()
{
    StopAll();
    for (int i = 0; i < 30; i++) {
        if (soundArray[i] != NULL) {
            delete soundArray[i];
            soundArray[i] = NULL;
        }
    }
    if (lpDSound != NULL) {
        lpDSound->Release();
        lpDSound = NULL;
    }
}

// 0x00412B20, 138 bytes.
int JSoundList::Create()
{
    HRESULT hr = DirectSoundCreate(NULL, &lpDSound, NULL);
    if (hr < 0) {
        lpDSound = NULL;
        return 0;
    }
    hr = lpDSound->SetCooperativeLevel(hWind, DSSCL_NORMAL);
    if (hr < 0) {
        if (lpDSound != NULL) {
            lpDSound->Release();
            lpDSound = NULL;
        }
        return 0;
    }
    return 1;
}

// 0x00412BB0, 177 bytes.
void JSoundList::CreateSound(char* fileName, unsigned short id, unsigned long count)
{
    if (lpDSound == NULL) {
        return;
    }
    int index = GetNullIndex();
    if (index >= 0) {
        soundArray[index] = new JSound(lpDSound, fileName, id, count);
    }
}

// 0x00412C70, 173 bytes.
void JSoundList::CreateSound(unsigned short resId, unsigned long count)
{
    if (lpDSound == NULL) {
        return;
    }
    int index = GetNullIndex();
    if (index >= 0) {
        soundArray[index] = new JSound(lpDSound, resId, count);
    }
}

// 0x00412D20, 87 bytes.
void JSoundList::Play(unsigned short resId, unsigned char volume, int looping)
{
    if (lpDSound == NULL) {
        return;
    }
    int index = FindIndexOfResId(resId);
    if (index >= 0) {
        soundArray[index]->SetVolume(volume);
        soundArray[index]->Play(looping);
    }
}

// 0x00412D80, 63 bytes.
void JSoundList::Stop(unsigned short resId)
{
    if (lpDSound == NULL) {
        return;
    }
    int index = FindIndexOfResId(resId);
    if (index >= 0) {
        soundArray[index]->Stop();
    }
}

// 0x00412DC0, 67 bytes.
void JSoundList::StopAll()
{
    for (int i = 0; i < 30; i++) {
        if (soundArray[i] != NULL) {
            soundArray[i]->Stop();
        }
    }
}

// 0x00412E10, 70 bytes.
int JSoundList::GetNullIndex() const
{
    int result = -1;
    for (int i = 0; i < 30; i++) {
        if (soundArray[i] == NULL) {
            result = i;
            break;
        }
    }
    return result;
}

// 0x00412E60, 98 bytes.
int JSoundList::FindIndexOfResId(unsigned short resId) const
{
    int result = -1;
    for (int i = 0; i < 30; i++) {
        if (soundArray[i] != NULL && soundArray[i]->GetResId() == resId) {
            result = i;
            break;
        }
    }
    return result;
}
