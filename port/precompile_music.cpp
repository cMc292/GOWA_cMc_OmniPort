/*
 * The precompile screen's music.
 *
 * The theme is "Intro Launchers 2", converted from its MP3 to PCM s16le and
 * embedded in port/intro_launchers_2.cpp. It plays only on the precompile
 * screen, from 2.5 s after the screen opens, loops while the screen is up,
 * fades in, and fades out over a second when the game window opens.
 *
 * [system] precompile_music (GOWA_PRECOMPILE_MUSIC): off, low, normal, high.
 * XAudio2 2.9 is loaded from System32 when it plays.
 *
 * Ascension-R.exe --music-preview <file.wav> [seconds] writes the same audio
 * to a WAV file, for checking it without the screen.
 */
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <xaudio2.h>

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ps3emu/ps3_settings.h"
#include "precompile_music.h"

#pragma comment(lib, "ole32.lib")

extern "C" const int16_t kIntroLaunchersPcm[];
extern "C" const unsigned kIntroLaunchersFrames;

namespace {

const int RATE = 44100;
const ULONGLONG START_MS = 2500;
const ULONGLONG FADE_IN_MS = 2500;
const ULONGLONG FADE_OUT_MS = 1000;

HANDLE s_thread, s_stop_event;
volatile LONG s_stop;
ULONGLONG s_t0;
double s_level;

typedef HRESULT(WINAPI* xaudio2_create_fn)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR);

void write_wav(const char* path, const int16_t* pcm, unsigned frames)
{
    FILE* f = fopen(path, "wb");
    if (!f) return;
    const uint32_t bytes = frames * 4;
    const uint32_t riff = 36 + bytes;
    const uint32_t fmt[] = { 16, 0x00020001, (uint32_t)RATE, (uint32_t)RATE * 4, 0x00100004 };
    fwrite("RIFF", 1, 4, f);
    fwrite(&riff, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);
    fwrite(fmt, 4, 5, f);
    fwrite("data", 1, 4, f);
    fwrite(&bytes, 4, 1, f);
    fwrite(pcm, 2, (size_t)frames * 2, f);
    fclose(f);
}

/* At normal priority: the build is on every core and would otherwise hold it
 * off. It only waits, plays and fades. */
DWORD WINAPI music_main(LPVOID)
{
    for (;;) {
        const ULONGLONG now = GetTickCount64();
        if (s_stop) return 0;
        if (now >= s_t0 + START_MS) break;
        if (WaitForSingleObject(s_stop_event, (DWORD)(s_t0 + START_MS - now)) == WAIT_OBJECT_0) return 0;
    }

    const HRESULT co = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    IXAudio2* xa = NULL;
    IXAudio2MasteringVoice* master = NULL;
    IXAudio2SourceVoice* voice = NULL;
    HMODULE dll = LoadLibraryExW(L"xaudio2_9.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    xaudio2_create_fn create = dll ? (xaudio2_create_fn)(void*)GetProcAddress(dll, "XAudio2Create") : NULL;

    WAVEFORMATEX wf = {};
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 2;
    wf.nSamplesPerSec = RATE;
    wf.wBitsPerSample = 16;
    wf.nBlockAlign = 4;
    wf.nAvgBytesPerSec = RATE * 4;

    HRESULT hr = create ? create(&xa, 0, XAUDIO2_DEFAULT_PROCESSOR) : E_FAIL;
    if (SUCCEEDED(hr)) hr = xa->CreateMasteringVoice(&master);
    if (SUCCEEDED(hr)) hr = xa->CreateSourceVoice(&voice, &wf);
    if (SUCCEEDED(hr)) {
        XAUDIO2_BUFFER b = {};
        b.LoopCount = XAUDIO2_LOOP_INFINITE;
        b.AudioBytes = kIntroLaunchersFrames * 4;
        b.pAudioData = (const BYTE*)kIntroLaunchersPcm;
        hr = voice->SubmitSourceBuffer(&b);
    }
    if (FAILED(hr)) {
        printf("[music] no sound: XAudio2 %s (0x%08lX)\n", dll ? "did not start" : "is not there", (unsigned long)hr);
    } else {
        master->SetVolume((float)s_level);
        voice->SetVolume(0.0f);
        voice->Start();
        const ULONGLONG start = GetTickCount64();
        printf("[music] playing (%s)\n",
               ps3_settings_str("system", "precompile_music", "GOWA_PRECOMPILE_MUSIC", "normal"));
        float vol = 0.0f;
        for (;;) {
            const ULONGLONG now = GetTickCount64();
            vol = (float)fmin(1.0, (now - start) / (double)FADE_IN_MS);
            voice->SetVolume(vol);
            if (WaitForSingleObject(s_stop_event, 100) == WAIT_OBJECT_0) break;
        }
        const ULONGLONG fade0 = GetTickCount64();
        for (;;) {
            const double t = (GetTickCount64() - fade0) / (double)FADE_OUT_MS;
            if (t >= 1.0) break;
            voice->SetVolume(vol * (float)(1.0 - t));
            Sleep(20);
        }
        printf("[music] faded out\n");
    }
    if (voice) voice->DestroyVoice();
    if (master) master->DestroyVoice();
    if (xa) xa->Release();
    if (SUCCEEDED(co)) CoUninitialize();
    return 0;
}

}  // namespace

void gowa_music_begin(uint64_t t0_ms)
{
    if (s_thread) return;
    const char* v = ps3_settings_str("system", "precompile_music", "GOWA_PRECOMPILE_MUSIC", "normal");
    double level = 1.0;
    if (ps3_settings_ieq(v, "off")) level = 0.0;
    else if (ps3_settings_ieq(v, "low")) level = 0.5;
    else if (ps3_settings_ieq(v, "high")) level = 1.25;
    if (level <= 0.0) {
        printf("[music] off (system.precompile_music)\n");
        return;
    }
    s_level = level;
    s_t0 = t0_ms;
    s_stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    s_thread = CreateThread(NULL, 0, music_main, NULL, 0, NULL);
}

void gowa_music_end(void)
{
    if (!s_thread) return;
    InterlockedExchange(&s_stop, 1);
    SetEvent(s_stop_event);
}

int gowa_music_preview(const char* path, double seconds)
{
    unsigned frames = kIntroLaunchersFrames;
    if (seconds > 0.0) {
        const unsigned want = (unsigned)(seconds * RATE);
        if (want < frames) frames = want;
    }
    write_wav(path, kIntroLaunchersPcm, frames);
    printf("%s: %.1f s\n", path, frames / (double)RATE);
    return 0;
}
