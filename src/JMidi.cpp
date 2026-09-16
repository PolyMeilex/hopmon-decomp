#include "JMidi.h"
#include "JStringList.h"
#include "JString.h"
#include <mmsystem.h>
#include <stdio.h>

// 0x0040EF30, 166 bytes.
JMidiList::JMidiList(HWND _hWind)
    : hWind(_hWind), paused(0), currentMidiIndex(-1), midiFileList(new JStringList())
{
}

// 0x0040EFE0, 82 bytes.
JMidiList::~JMidiList()
{
    if (midiFileList != NULL) {
        delete midiFileList;
    }
    midiFileList = NULL;
}

// 0x0040F040, 106 bytes.
void JMidiList::Append(const char* midiFileName)
{
    JString str(midiFileName);
    midiFileList->Append(str);
}

// 0x0040F0B0, 201 bytes.
void JMidiList::Play(int index)
{
    if (index >= 0) {
        if (index < (int)midiFileList->Count() && currentMidiIndex != index) {
            Stop();
            JString fullPath = ConvertToFullPath(midiFileList->GetString(index), 1);
            PlayMidi(hWind, fullPath.c_str());
            currentMidiIndex = index;
            if (paused != 0) {
                PauseMidi();
            }
        }
    }
}

// 0x0040F180, 49 bytes.
void JMidiList::Replay()
{
    if (currentMidiIndex >= 0) {
        ReplayMidi(hWind);
        if (paused != 0) {
            PauseMidi();
        }
    }
}

// 0x0040F1C0, 35 bytes.
void JMidiList::Pause()
{
    paused = 1;
    if (currentMidiIndex >= 0) {
        PauseMidi();
    }
}

// 0x0040F1F0, 45 bytes.
void JMidiList::Resume()
{
    paused = 0;
    if (currentMidiIndex >= 0) {
        ResumeMidi(hWind);
    }
}

// 0x0040F220, 26 bytes.
void JMidiList::Stop()
{
    currentMidiIndex = -1;
    StopMidi();
}

// 0x0040F240, 119 bytes.
int PlayMidi(HWND hWnd, const char* sFileName)
{
    static char buf[256];

    if (sFileName == NULL) {
        return 0;
    }
    sprintf(buf, "open %s type sequencer alias MUSIC", sFileName);
    if (mciSendStringA("close all", NULL, 0, NULL) != 0) {
        return 0;
    }
    if (mciSendStringA(buf, NULL, 0, NULL) != 0) {
        return 0;
    }
    if (mciSendStringA("play MUSIC from 0 notify", NULL, 0, hWnd) != 0) {
        return 0;
    }
    return 1;
}

// 0x0040F2C0, 35 bytes.
int PauseMidi()
{
    if (mciSendStringA("stop MUSIC", NULL, 0, NULL) != 0) {
        return 0;
    }
    return 1;
}

// 0x0040F2F0, 37 bytes.
int ResumeMidi(HWND hWnd)
{
    if (mciSendStringA("play MUSIC notify", NULL, 0, hWnd) != 0) {
        return 0;
    }
    return 1;
}

// 0x0040F320, 35 bytes.
int StopMidi()
{
    if (mciSendStringA("close all", NULL, 0, NULL) != 0) {
        return 0;
    }
    return 1;
}

// 0x0040F350, 37 bytes.
int ReplayMidi(HWND hWnd)
{
    if (mciSendStringA("play MUSIC from 0 notify", NULL, 0, hWnd) != 0) {
        return 0;
    }
    return 1;
}
