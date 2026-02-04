/*
  LICENSE
  -------
Copyright 2005-2013 Nullsoft, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. 

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

  * Neither the name of Nullsoft nor the names of its contributors may be used to 
    endorse or promote products derived from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __NULLSOFT_DX9_PLUGIN_SHELL_H__
#define __NULLSOFT_DX9_PLUGIN_SHELL_H__ 1

# include <xtl.h>
#include "shell_defines.h"
#include "fft.h"
#include "defines.h"
//#include "textmgr.h"

#define TIME_HIST_SLOTS 128     // # of slots used if fps > 60.  half this many if fps==30.
#define MAX_SONGS_PER_PAGE 40

typedef struct
{
    wchar_t szFace[256];
    int nSize;  // size requested @ font creation time
    int bBold;
    int bItalic;
    int bAntiAliased;
} td_fontinfo;

typedef struct
{
    float   imm[2][3];                // bass, mids, treble, no damping, for each channel (long-term average is 1)
    float    avg[2][3];               // bass, mids, treble, some damping, for each channel (long-term average is 1)
    float     med_avg[2][3];          // bass, mids, treble, more damping, for each channel (long-term average is 1)
    float      long_avg[2][3];        // bass, mids, treble, heavy damping, for each channel (long-term average is 1)
    float       infinite_avg[2][3];   // bass, mids, treble: winamp's average output levels. (1)
    float   fWaveform[2][576];             // Not all 576 are valid! - only NUM_WAVEFORM_SAMPLES samples are valid for each channel (note: NUM_WAVEFORM_SAMPLES is declared in shell_defines.h)
    float   fSpectrum[2][NUM_FREQUENCIES]; // NUM_FREQUENCIES samples for each channel (note: NUM_FREQUENCIES is declared in shell_defines.h)
} td_soundinfo;                    // ...range is 0 Hz to 22050 Hz, evenly spaced.

class CPluginShell
{
public:    
    // GET METHODS
    // ------------------------------------------------------------
    int       GetFrame();          // returns current frame # (starts at zero)
    float     GetTime();           // returns current animation time (in seconds) (starts at zero) (updated once per frame)
    float     GetFps();            // returns current estimate of framerate (frames per second)
    eScrMode  GetScreenMode();     // returns WINDOWED, FULLSCREEN, FAKE_FULLSCREEN, DESKTOP, or NOT_YET_KNOWN (if called before or during OverrideDefaults()).

	virtual void CleanUpMyDX9Stuff(int final_cleanup) = 0;
	virtual void MyRenderFn(int redraw) = 0;
	virtual int  AllocateMyDX9Stuff() = 0;
protected:

    // GET METHODS THAT ONLY WORK ONCE DIRECTX IS READY
    // ------------------------------------------------------------
    //  The following 'Get' methods are only available after DirectX has been initialized.
    //  If you call these from OverrideDefaults, MyPreInitialize, or MyReadConfig, 
    //    they will return NULL (zero).
    // ------------------------------------------------------------
public:
	D3DDevice* GetDevice();
    int GetWidth(); // returns width of plugin window interior, in pixels.
    int GetHeight(); // returns height of plugin window interior, in pixels.
    D3DCAPS9*    GetCaps();              // returns a pointer to the D3DCAPS9 structer for the device.  NOT persistent; can change.

    // MISC
    // ------------------------------------------------------------
    td_soundinfo m_sound;                   // a structure always containing the most recent sound analysis information; defined in pluginshell.h.

    // CONFIG PANEL SETTINGS
    // ------------------------------------------------------------
    int          m_max_fps_fs;              // 1-120, or 0 for 'unlimited'
    int          m_max_fps_dm;              // 1-120, or 0 for 'unlimited'
    int          m_max_fps_w;               // 1-120, or 0 for 'unlimited'
    int          m_allow_page_tearing_w;    // 0 or 1
    int          m_save_cpu;                // 0 or 1

    // PURE VIRTUAL FUNCTIONS (...must be implemented by derived classes)
    // ------------------------------------------------------------
    virtual void OverrideDefaults()      = 0;
    virtual void MyPreInitialize()       = 0;
    virtual void MyReadConfig()          = 0;
    virtual int  AllocateMyNonDx9Stuff() = 0;

private:
	int  InitNondx9Stuff();

	// PRIVATE - MORE TIMEKEEPING
	double m_last_raw_time;
	int m_time_hist_pos;
	LARGE_INTEGER m_high_perf_timer_freq; // 0 if high-precision timer not available
	float  m_time_hist[TIME_HIST_SLOTS]; // Cumulative

	int m_iWidth;
	int m_iHeight;
    // GENERAL PRIVATE STUFF
    eScrMode     m_screenmode;      // // WINDOWED, FULLSCREEN, or FAKE_FULLSCREEN (i.e. running in a full-screen-sized window)
    int          m_frame;           // current frame #, starting at zero
    float        m_time;            // current animation time in seconds; starts at zero.
    float        m_fps;             // current estimate of frames per second
    LARGE_INTEGER m_prev_end_of_frame;

    // PRIVATE AUDIO PROCESSING DATA
    FFT   m_fftobj;
    float m_oldwave[2][576];        // for wave alignment
    int   m_prev_align_offset[2];   // for wave alignment
    int   m_align_weights_ready;

public:
    CPluginShell();
    ~CPluginShell();
    
	// called by vis.cpp, on behalf of Winamp:
    int  PluginPreInitialize(/*HWND hWinampWnd, HINSTANCE hWinampInstance*/);    
    int  PluginInitialize(D3DDevice* pD3dDevice, int iPosX, int iPosY, int iWidth, int iHeight);                                         
    int  PluginRender(unsigned char *pWaveL, unsigned char *pWaveR);
    void PluginQuit();
    void DrawAndDisplay(int redraw);
    void ReadConfig();
    void DoTime();
    void AnalyzeNewSound(unsigned char *pWaveL, unsigned char *pWaveR);
    void AlignWaves();
protected:
    void EnforceMaxFPS();

	int m_iExiting;
	LPDIRECT3DDEVICE9 m_pD3DDevice;
};

#endif //__NULLSOFT_DX9_PLUGIN_SHELL_H__