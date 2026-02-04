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

/*
    TO DO
    -----
    -done/v1.06:
        -(nothing yet)
        -
        -
    -to do/v1.06:
        -FFT: high freq. data kinda sucks because of the 8-bit samples we get in;
            look for justin to put 16-bit vis data into wa5.
        -make an 'advanced view' button on config panel; hide complicated stuff
            til they click that.
        -put an asterisk(*) next to the 'max framerate' values that
            are ideal (given the current windows display mode or selected FS dispmode).
        -or add checkbox: "smart sync"
            -> matches FPS limit to nearest integer divisor of refresh rate.
        -debug.txt/logging support!
        -audio: make it a DSP plugin? then we could get the complete, continuous waveform
            and overlap our waveform windows, so we'd never miss a brief high note.
        -bugs:
            -vms plugins sometimes freeze after a several-minute pause; I've seen it
                with most of them.  hard to repro, though.
            -running FS on monitor 2, hit ALT-TAB -> minimizes!!!
                -but only if you let go of TAB first.  Let go of ALT first and it's fine!
                -> means it's related to the keyup...
            -fix delayloadhelper leak; one for each launch to config panel/plugin.
            -also, delayload(d3d9.dll) still leaks, if plugin has error initializing and
                quits by returning false from PluginInitialize().
        -add config panel option to ignore fake-fullscreen tips
            -"tip" boxes in dxcontext.cpp
            -"notice" box on WM_ACTIVATEAPP?
        -desktop mode:
            -icon context menus: 'send to', 'cut', and 'copy' links do nothing.
                -http://netez.com/2xExplorer/shellFAQ/bas_context.html
            -create a 2nd texture to render all icon text labels into
                (they're the sole reason that desktop mode is slow)
            -in UpdateIconBitmaps, don't read the whole bitmap and THEN
                realize it's a dupe; try to compare icon filename+index or somethign?
            -DRAG AND DROP.  COMPLICATED; MANY DETAILS.
                -http://netez.com/2xExplorer/shellFAQ/adv_drag.html
                -http://www.codeproject.com/shell/explorerdragdrop.asp
                -hmm... you can't drag icons between the 2 desktops (ugh)
            -multiple delete/open/props/etc
            -delete + enter + arrow keys.
            -try to solve mysteries w/ShellExecuteEx() and desktop *shortcuts* (*.lnk).
            -(notice that when icons are selected, they get modulated by the
                highlight color, when they should be blended 50% with that color.)

    ---------------------------
    final touches:
        -Tests:
            -make sure desktop still functions/responds properly when winamp paused
            -desktop mode + multimon:
                -try desktop mode on all monitors
                -try moving taskbar around; make sure icons are in the
                    right place, that context menus (general & for
                    specific icons) pop up in the right place, and that
                    text-off-left-edge is ok.
                -try setting the 2 monitors to different/same resolutions
        -check tab order of config panel controls!
        -Clean All
        -build in release mode to include in the ZIP
        -leave only one file open in workspace: README.TXT.
        -TEMPORARILY "ATTRIB -R" ALL FILES BEFORE ZIPPING THEM!

    ---------------------------
    KEEP IN VIEW:
        -EMBEDWND:
            -kiv: on resize of embedwnd, it's out of our control; winamp
                resizes the child every time the mouse position changes,
                and we have to cleanup & reallocate everything, b/c we
                can't tell when the resize begins & ends.
                [justin said he'd fix in wa5, though]
            -kiv: with embedded windows of any type (plugin, playlist, etc.)
                you can't place the winamp main wnd over them.
            -kiv: embedded windows are child windows and don't get the
                WM_SETFOCUS or WM_KILLFOCUS messages when they get or lose
                the focus.  (For a workaround, see milkdrop & scroll lock key.)
            -kiv: tiny bug (IGNORE): when switching between embedwnd &
                no-embedding, the window gets scooted a tiny tiny bit.
        -kiv: fake fullscreen mode w/multiple monitors: there is no way
            to keep the taskbar from popping up [potentially overtop of
            the plugin] when you click on something besides the plugin.
            To get around this, use true fullscreen mode.
        -kiv: max_fps implementation assumptions:
            -that most computers support high-precision timer
            -that no computers [regularly] sleep for more than 1-2 ms
                when you call Sleep(1) after timeBeginPeriod(1).
        -reminder: if vms_desktop.dll's interface needs changed,
            it will have to be renamed!  (version # upgrades are ok
            as long as it won't break on an old version; if the
            new functionality is essential, rename the DLL.)

    ---------------------------
    REMEMBER:
        -GF2MX + GF4 have icon scooting probs in desktop mode
            (when taskbar is on upper or left edge of screen)
        -Radeon is the one w/super slow text probs @ 1280x1024.
            (it goes unstable after you show playlist AND helpscr; -> ~1 fps)
        -Mark's win98 machine has hidden cursor (in all modes),
            but no one else seems to have this problem.
        -links:
            -win2k-only-style desktop mode: (uses VirtualAllocEx, vs. DLL Injection)
                http://www.digiwar.com/scripts/renderpage.php?section=2&subsection=2
            -http://www.experts-exchange.com/Programming/Programming_Platforms/Win_Prog/Q_20096218.html
*/

#include "pluginshell.h"
#include "utility.h"

CPluginShell::CPluginShell()
{
	m_pD3DDevice = NULL;
}

CPluginShell::~CPluginShell()
{
	// This should remain empty!
}

LPDIRECT3DDEVICE9 CPluginShell::GetDevice()
{
	if (m_pD3DDevice)
		return m_pD3DDevice;
	else
		return NULL;
}

eScrMode  CPluginShell::GetScreenMode()
{
	return m_screenmode;
};
int       CPluginShell::GetFrame()
{
	return m_frame;
};
float     CPluginShell::GetTime()
{
	return m_time;
};
float     CPluginShell::GetFps()
{
	return m_fps;
};

int CPluginShell::GetWidth()
{
	if (m_pD3DDevice)
		return m_iWidth;  else return 0;
};
int CPluginShell::GetHeight()
{
	if (m_pD3DDevice)
		return m_iHeight; else return 0;
};

D3DCAPS9* CPluginShell::GetCaps()
{
	D3DCAPS9 pD3DCaps;

	if (m_pD3DDevice)
	{
		m_pD3DDevice->GetDeviceCaps(&pD3DCaps);
		return &pD3DCaps;
	}
	else
		return NULL;
};



int CPluginShell::PluginPreInitialize(/*HWND hWinampWnd, HINSTANCE hWinampInstance*/)
{
    // GENERAL PRIVATE STUFF
	m_iWidth = 0;
	m_iHeight = 0;
	m_iExiting = 0;


	m_max_fps_fs            = 30;
	m_max_fps_dm            = 30;
	m_max_fps_w             = 30;
	m_save_cpu              = 1;
	ZeroMemory(&m_sound, sizeof(td_soundinfo));
	for (int ch=0; ch<2; ch++)
		for (int i=0; i<3; i++)
		{
			m_sound.infinite_avg[ch][i] = m_sound.avg[ch][i] = m_sound.med_avg[ch][i] = m_sound.long_avg[ch][i] = 1.0f;
		}

	// GENERAL PRIVATE STUFF
	//m_screenmode: set at end (derived setting)
	m_frame = 0;
	m_time = 0;
	m_fps = 30;

	// PRIVATE - MORE TIMEKEEPING
	m_last_raw_time = 0;
	memset(m_time_hist, 0, sizeof(m_time_hist));
	m_time_hist_pos = 0;

	if (!QueryPerformanceFrequency(&m_high_perf_timer_freq))
		m_high_perf_timer_freq.QuadPart = 0;
	m_prev_end_of_frame.QuadPart = 0;

	// PRIVATE AUDIO PROCESSING DATA
	//(m_fftobj needs no init)
	memset(m_oldwave[0], 0, sizeof(float)*576);
	memset(m_oldwave[1], 0, sizeof(float)*576);
	m_prev_align_offset[0] = 0;
	m_prev_align_offset[1] = 0;
	m_align_weights_ready = 0;


	OverrideDefaults();
	ReadConfig();

			m_screenmode = FAKE_FULLSCREEN;
	MyPreInitialize();
	MyReadConfig();

	//-----

	return TRUE;
}

int CPluginShell::PluginInitialize(D3DDevice* pD3dDevice, int iPosX, int iPosY, int iWidth, int iHeight)
{
	// Note: initialize GDI before DirectX. Also separate them because
	// when we change windowed<->fullscreen, or lose the device and restore it,
	// we don't want to mess with any (persistent) GDI stuff.

#ifdef _XBOX
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	// TODO Better for Xbox??
	m_pD3DDevice = pD3dDevice;

	if (!InitNondx9Stuff())    return FALSE;  // gives its own error messages

	AllocateMyDX9Stuff(); // Should be inside AllocateDX9Stuff() // BRENT

#else
	if (!InitDirectX())        return FALSE;  // gives its own error messages
	if (!InitNondx9Stuff())    return FALSE;  // gives its own error messages
	if (!AllocateDX9Stuff())   return FALSE;  // gives its own error messages
	if (!InitVJStuff())        return FALSE;
#endif

	return TRUE;
}

void CPluginShell::ReadConfig()
{
}

//----------------------------------------------------------------------

int CPluginShell::PluginRender(unsigned char *pWaveL, unsigned char *pWaveR)
{
	// return FALSE here to tell Winamp to terminate the plugin

    if (!m_pD3DDevice)
    {
        // Note: 'm_ready' will go false when a device reset fatally fails
        //       (for example, when user resizes window, or toggles fullscreen.)
        m_iExiting = 1;
        return false;   // EXIT THE PLUGIN
    }

	DoTime();
	AnalyzeNewSound(pWaveL, pWaveR);
	AlignWaves();

	DrawAndDisplay(0);

	EnforceMaxFPS();

	m_frame++;

	return true;
}

void CPluginShell::PluginQuit()
{
	CleanUpMyDX9Stuff(1);
}


void CPluginShell::DrawAndDisplay(int redraw)
{
	MyRenderFn(redraw);

}


		

void CPluginShell::EnforceMaxFPS()
{
	int max_fps;
	switch (m_screenmode)
	{
	case WINDOWED:        max_fps = m_max_fps_w;  break;
	case FULLSCREEN:      max_fps = m_max_fps_fs; break;
	case FAKE_FULLSCREEN: max_fps = m_max_fps_fs; break;
	case DESKTOP:         max_fps = m_max_fps_dm; break;
	}

	if (max_fps <= 0)
		return;

	float fps_lo = (float)max_fps;
	float fps_hi = (float)max_fps;

	if (m_save_cpu)
	{
		// Find the optimal lo/hi bounds for the fps
		// that will result in a maximum difference,
		// in the time for a single frame, of 0.003 seconds -
		// the assumed granularity for Sleep(1) -

		// Using this range of acceptable fps
		// will allow us to do (sloppy) fps limiting
		// using only Sleep(1), and never the
		// second half of it: Sleep(0) in a tight loop,
		// which sucks up the CPU (whereas Sleep(1)
		// leaves it idle).

		// The original equation:
		//   1/(max_fps*t1) = 1/(max*fps/t1) - 0.003
		// where:
		//   t1 > 0
		//   max_fps*t1 is the upper range for fps
		//   max_fps/t1 is the lower range for fps

		float a = 1;
		float b = -0.003f * max_fps;
		float c = -1.0f;
		float det = b*b - 4*a*c;
		if (det>0)
		{
			float t1 = (-b + sqrtf(det)) / (2*a);
			//float t2 = (-b - sqrtf(det)) / (2*a);

			if (t1 > 1.0f)
			{
				fps_lo = max_fps / t1;
				fps_hi = max_fps * t1;
				// verify: now [1.0f/fps_lo - 1.0f/fps_hi] should equal 0.003 seconds.
				// note: allowing tolerance to go beyond these values for
				// fps_lo and fps_hi would gain nothing.
			}
		}
	}

	if (m_high_perf_timer_freq.QuadPart > 0)
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);

		if (m_prev_end_of_frame.QuadPart != 0)
		{
			int ticks_to_wait_lo = (int)((float)m_high_perf_timer_freq.QuadPart / (float)fps_hi);
			int ticks_to_wait_hi = (int)((float)m_high_perf_timer_freq.QuadPart / (float)fps_lo);
			int done = 0;
			int loops = 0;
			do
			{
				QueryPerformanceCounter(&t);

				__int64 t2 = t.QuadPart - m_prev_end_of_frame.QuadPart;
				if (t2 > 2147483000)
					done = 1;
				if (t.QuadPart < m_prev_end_of_frame.QuadPart)    // time wrap
					done = 1;

				// this is sloppy - if your freq. is high, this can overflow (to a (-) int) in just a few minutes
				// but it's ok, we have protection for that above.
				int ticks_passed = (int)(t.QuadPart - m_prev_end_of_frame.QuadPart);
				if (ticks_passed >= ticks_to_wait_lo)
					done = 1;

				if (!done)
				{
					// if > 0.01s left, do Sleep(1), which will actually sleep some
					//   steady amount of up to 3 ms (depending on the OS),
					//   and do so in a nice way (cpu meter drops; laptop battery spared).
					// otherwise, do a few Sleep(0)'s, which just give up the timeslice,
					//   but don't really save cpu or battery, but do pass a tiny
					//   amount of time.

					//if (ticks_left > (int)m_high_perf_timer_freq.QuadPart/500)
					if (ticks_to_wait_hi - ticks_passed > (int)m_high_perf_timer_freq.QuadPart/100)
						Sleep(5);
					else if (ticks_to_wait_hi - ticks_passed > (int)m_high_perf_timer_freq.QuadPart/1000)
						Sleep(1);
					else
						for (int i=0; i<10; i++)
							Sleep(0);  // causes thread to give up its timeslice
				}
			}
			while (!done);
		}

		m_prev_end_of_frame = t;
	}
	else
	{
		Sleep(1000/max_fps);
	}
}

void CPluginShell::DoTime()
{
	if (m_frame==0)
	{
		m_fps = 30;
		m_time = 0;
		m_time_hist_pos = 0;
	}

	double new_raw_time;
	float elapsed;

	if (m_high_perf_timer_freq.QuadPart != 0)
	{
		// get high-precision time
		// precision: usually from 1..6 us (MICROseconds), depending on the cpu speed.
		// (higher cpu speeds tend to have better precision here)
		LARGE_INTEGER t;
		if (!QueryPerformanceCounter(&t))
		{
			m_high_perf_timer_freq.QuadPart = 0;   // something went wrong (exception thrown) -> revert to crappy timer
		}
		else
		{
			new_raw_time = (double)t.QuadPart;
			elapsed = (float)((new_raw_time - m_last_raw_time)/(double)m_high_perf_timer_freq.QuadPart);
		}
	}

	if (m_high_perf_timer_freq.QuadPart == 0)
	{
		// get low-precision time
		// precision: usually 1 ms (MILLIsecond) for win98, and 10 ms for win2k.
		new_raw_time = (double)(GetTime()*0.001);
		elapsed = (float)(new_raw_time - m_last_raw_time);
	}

	m_last_raw_time = new_raw_time;
	int slots_to_look_back = (m_high_perf_timer_freq.QuadPart==0) ? TIME_HIST_SLOTS : TIME_HIST_SLOTS/2;

	m_time += 1.0f/m_fps;

	// timekeeping goals:
	//    1. keep 'm_time' increasing SMOOTHLY: (smooth animation depends on it)
	//          m_time += 1.0f/m_fps;     // where m_fps is a bit damped
	//    2. keep m_time_hist[] 100% accurate (except for filtering out pauses),
	//       so that when we look take the difference between two entries,
	//       we get the real amount of time that passed between those 2 frames.
	//          m_time_hist[i] = m_last_raw_time + elapsed_corrected;

	if (m_frame > TIME_HIST_SLOTS)
	{
		if (m_fps < 60.0f)
			slots_to_look_back = (int)(slots_to_look_back*(0.1f + 0.9f*(m_fps/60.0f)));

		if (elapsed > 5.0f/m_fps || elapsed > 1.0f || elapsed < 0)
			elapsed = 1.0f / 30.0f;

		float old_hist_time = m_time_hist[(m_time_hist_pos - slots_to_look_back + TIME_HIST_SLOTS) % TIME_HIST_SLOTS];
		float new_hist_time = m_time_hist[(m_time_hist_pos - 1 + TIME_HIST_SLOTS) % TIME_HIST_SLOTS]
		                      + elapsed;

		m_time_hist[m_time_hist_pos] = new_hist_time;
		m_time_hist_pos = (m_time_hist_pos+1) % TIME_HIST_SLOTS;

		float new_fps = slots_to_look_back / (float)(new_hist_time - old_hist_time);
		float damping = (m_high_perf_timer_freq.QuadPart==0) ? 0.93f : 0.87f;

		// damp heavily, so that crappy timer precision doesn't make animation jerky
		if (fabsf(m_fps - new_fps) > 3.0f)
			m_fps = new_fps;
		else
			m_fps = damping*m_fps + (1-damping)*new_fps;
	}
	else
	{
		float damping = (m_high_perf_timer_freq.QuadPart==0) ? 0.8f : 0.6f;

		if (m_frame < 2)
			elapsed = 1.0f / 30.0f;
		else if (elapsed > 1.0f || elapsed < 0)
			elapsed = 1.0f / m_fps;

		float old_hist_time = m_time_hist[0];
		float new_hist_time = m_time_hist[(m_time_hist_pos - 1 + TIME_HIST_SLOTS) % TIME_HIST_SLOTS]
		                      + elapsed;

		m_time_hist[m_time_hist_pos] = new_hist_time;
		m_time_hist_pos = (m_time_hist_pos+1) % TIME_HIST_SLOTS;

		if (m_frame > 0)
		{
			float new_fps = (m_frame) / (new_hist_time - old_hist_time);
			m_fps = damping*m_fps + (1-damping)*new_fps;
		}
	}
}

void CPluginShell::AnalyzeNewSound(unsigned char *pWaveL, unsigned char *pWaveR)
{
	// we get 576 samples in from winamp.
	// the output of the fft has 'num_frequencies' samples,
	//   and represents the frequency range 0 hz - 22,050 hz.
	// usually, plugins only use half of this output (the range 0 hz - 11,025 hz),
	//   since >10 khz doesn't usually contribute much.

	int i;

	float temp_wave[2][576];

	int old_i = 0;
	for (i=0; i<576; i++)
	{
		m_sound.fWaveform[0][i] = (float)((pWaveL[i] ^ 128) - 128);
		m_sound.fWaveform[1][i] = (float)((pWaveR[i] ^ 128) - 128);

		// simulating single frequencies from 200 to 11,025 Hz:
		//float freq = 1.0f + 11050*(GetFrame() % 100)*0.01f;
		//m_sound.fWaveform[0][i] = 10*sinf(i*freq*6.28f/44100.0f);

		// damp the input into the FFT a bit, to reduce high-frequency noise:
		temp_wave[0][i] = 0.5f*(m_sound.fWaveform[0][i] + m_sound.fWaveform[0][old_i]);
		temp_wave[1][i] = 0.5f*(m_sound.fWaveform[1][i] + m_sound.fWaveform[1][old_i]);
		old_i = i;
	}

	m_fftobj.time_to_frequency_domain(temp_wave[0], m_sound.fSpectrum[0]);
	m_fftobj.time_to_frequency_domain(temp_wave[1], m_sound.fSpectrum[1]);

	// sum (left channel) spectrum up into 3 bands
	// [note: the new ranges do it so that the 3 bands are equally spaced, pitch-wise]
	float min_freq = 200.0f;
	float max_freq = 11025.0f;
	float net_octaves = (logf(max_freq/min_freq) / logf(2.0f));     // 5.7846348455575205777914165223593
	float octaves_per_band = net_octaves / 3.0f;                    // 1.9282116151858401925971388407864
	float mult = powf(2.0f, octaves_per_band); // each band's highest freq. divided by its lowest freq.; 3.805831305510122517035102576162
	// [to verify: min_freq * mult * mult * mult should equal max_freq.]
	for (int ch=0; ch<2; ch++)
	{
		for (i=0; i<3; i++)
		{
			// old guesswork code for this:
			//   float exp = 2.1f;
			//   int start = (int)(NUM_FREQUENCIES*0.5f*powf(i/3.0f, exp));
			//   int end   = (int)(NUM_FREQUENCIES*0.5f*powf((i+1)/3.0f, exp));
			// results:
			//          old range:      new range (ideal):
			//   bass:  0-1097          200-761
			//   mids:  1097-4705       761-2897
			//   treb:  4705-11025      2897-11025
			int start = (int)(NUM_FREQUENCIES * min_freq*powf(mult, (float)i)/11025.0f);
			int end   = (int)(NUM_FREQUENCIES * min_freq*powf(mult, (float)(i+1))/11025.0f);
			if (start < 0) start = 0;
			if (end > NUM_FREQUENCIES) end = NUM_FREQUENCIES;

			m_sound.imm[ch][i] = 0;
			for (int j=start; j<end; j++)
				m_sound.imm[ch][i] += m_sound.fSpectrum[ch][j];
			m_sound.imm[ch][i] /= (float)(end-start);
		}
	}

	// multiply by long-term, empirically-determined inverse averages:
	// (for a trial of 244 songs, 10 seconds each, somewhere in the 2nd or 3rd minute,
	//  the average levels were: 0.326781557	0.38087377	0.199888934
	for (int ch=0; ch<2; ch++)
	{
		m_sound.imm[ch][0] /= 0.326781557f;//0.270f;
		m_sound.imm[ch][1] /= 0.380873770f;//0.343f;
		m_sound.imm[ch][2] /= 0.199888934f;//0.295f;
	}

	// do temporal blending to create attenuated and super-attenuated versions
	for (int ch=0; ch<2; ch++)
	{
		for (i=0; i<3; i++)
		{
			// m_sound.avg[i]
			{
				float avg_mix;
				if (m_sound.imm[ch][i] > m_sound.avg[ch][i])
					avg_mix = AdjustRateToFPS(0.2f, 14.0f, m_fps);
				else
					avg_mix = AdjustRateToFPS(0.5f, 14.0f, m_fps);
				m_sound.avg[ch][i] = m_sound.avg[ch][i]*avg_mix + m_sound.imm[ch][i]*(1-avg_mix);
			}

			// m_sound.med_avg[i]
			// m_sound.long_avg[i]
			{
				float med_mix  = 0.91f;//0.800f + 0.11f*powf(t, 0.4f);    // primarily used for velocity_damping
				float long_mix = 0.96f;//0.800f + 0.16f*powf(t, 0.2f);    // primarily used for smoke plumes
				med_mix  = AdjustRateToFPS(med_mix, 14.0f, m_fps);
				long_mix = AdjustRateToFPS(long_mix, 14.0f, m_fps);
				m_sound.med_avg[ch][i]  =  m_sound.med_avg[ch][i]*(med_mix) + m_sound.imm[ch][i]*(1-med_mix);
				m_sound.long_avg[ch][i] = m_sound.long_avg[ch][i]*(long_mix) + m_sound.imm[ch][i]*(1-long_mix);
			}
		}
	}
}

int CPluginShell::InitNondx9Stuff()
{
	return AllocateMyNonDx9Stuff();
}


void CPluginShell::AlignWaves()
{
	// align waves, using recursive (mipmap-style) least-error matching
	// note: NUM_WAVEFORM_SAMPLES must be between 32 and 576.

	int align_offset[2] = { 0, 0 };

#if (NUM_WAVEFORM_SAMPLES < 576) // [don't let this code bloat our DLL size if it's not going to be used]

	int nSamples = NUM_WAVEFORM_SAMPLES;

#define MAX_OCTAVES 10

	int octaves = (int)floorf(logf((float)(576-nSamples))/logf(2.0f));
	if (octaves < 4)
		return;
	if (octaves > MAX_OCTAVES)
		octaves = MAX_OCTAVES;

	for (int ch=0; ch<2; ch++)
	{
		// only worry about matching the lower 'nSamples' samples
		float temp_new[MAX_OCTAVES][576];
		float temp_old[MAX_OCTAVES][576];
		static float temp_weight[MAX_OCTAVES][576];
		static int   first_nonzero_weight[MAX_OCTAVES];
		static int   last_nonzero_weight[MAX_OCTAVES];
		int spls[MAX_OCTAVES];
		int space[MAX_OCTAVES];

		memcpy(temp_new[0], m_sound.fWaveform[ch], sizeof(float)*576);
		memcpy(temp_old[0], &m_oldwave[ch][m_prev_align_offset[ch]], sizeof(float)*nSamples);
		spls[0] = 576;
		space[0] = 576 - nSamples;

		// potential optimization: could reuse (instead of recompute) mip levels for m_oldwave[2][]?
		for (int octave=1; octave<octaves; octave++)
		{
			spls[octave] = spls[octave-1]/2;
			space[octave] = space[octave-1]/2;
			for (int n=0; n<spls[octave]; n++)
			{
				temp_new[octave][n] = 0.5f*(temp_new[octave-1][n*2] + temp_new[octave-1][n*2+1]);
				temp_old[octave][n] = 0.5f*(temp_old[octave-1][n*2] + temp_old[octave-1][n*2+1]);
			}
		}

		if (!m_align_weights_ready)
		{
			m_align_weights_ready = 1;
			for (int octave=0; octave<octaves; octave++)
			{
				int compare_samples = spls[octave] - space[octave];
				for (int n=0; n<compare_samples; n++)
				{
					// start with pyramid-shaped pdf, from 0..1..0
					if (n < compare_samples/2)
						temp_weight[octave][n] = n*2/(float)compare_samples;
					else
						temp_weight[octave][n] = (compare_samples-1 - n)*2/(float)compare_samples;

					// TWEAK how much the center matters, vs. the edges:
					temp_weight[octave][n] = (temp_weight[octave][n] - 0.8f)*5.0f + 0.8f;

					// clip:
					if (temp_weight[octave][n]>1) temp_weight[octave][n] = 1;
					if (temp_weight[octave][n]<0) temp_weight[octave][n] = 0;
				}

				int n = 0;
				while (temp_weight[octave][n] == 0 && n < compare_samples)
					n++;
				first_nonzero_weight[octave] = n;

				n = compare_samples-1;
				while (temp_weight[octave][n] == 0 && n >= 0)
					n--;
				last_nonzero_weight[octave] = n;
			}
		}

		int n1 = 0;
		int n2 = space[octaves-1];
		for (int octave = octaves-1; octave>=0; octave--)
		{
			// for example:
			//  space[octave] == 4
			//  spls[octave] == 36
			//  (so we test 32 samples, w/4 offsets)
			int compare_samples = spls[octave]-space[octave];

			int lowest_err_offset = -1;
			float lowest_err_amount = 0;
			for (int n=n1; n<n2; n++)
			{
				float err_sum = 0;
				//for (int i=0; i<compare_samples; i++)
				for (int i=first_nonzero_weight[octave]; i<=last_nonzero_weight[octave]; i++)
				{
					float x = (temp_new[octave][i+n] - temp_old[octave][i]) * temp_weight[octave][i];
					if (x>0)
						err_sum += x;
					else
						err_sum -= x;
				}

				if (lowest_err_offset == -1 || err_sum < lowest_err_amount)
				{
					lowest_err_offset = n;
					lowest_err_amount = err_sum;
				}
			}

			// now use 'lowest_err_offset' to guide bounds of search in next octave:
			//  space[octave] == 8
			//  spls[octave] == 72
			//     -say 'lowest_err_offset' was 2
			//     -that corresponds to samples 4 & 5 of the next octave
			//     -also, expand about this by 2 samples?  YES.
			//  (so we'd test 64 samples, w/8->4 offsets)
			if (octave > 0)
			{
				n1 = lowest_err_offset*2  -1;
				n2 = lowest_err_offset*2+2+1;
				if (n1 < 0) n1=0;
				if (n2 > space[octave-1]) n2 = space[octave-1];
			}
			else
				align_offset[ch] = lowest_err_offset;
		}
	}
#endif
	memcpy(m_oldwave[0], m_sound.fWaveform[0], sizeof(float)*576);
	memcpy(m_oldwave[1], m_sound.fWaveform[1], sizeof(float)*576);
	m_prev_align_offset[0] = align_offset[0];
	m_prev_align_offset[1] = align_offset[1];

	// finally, apply the results: modify m_sound.fWaveform[2][0..576]
	// by scooting the aligned samples so that they start at m_sound.fWaveform[2][0].
	for (int ch=0; ch<2; ch++)
		if (align_offset[ch]>0)
		{
			for (int i=0; i<nSamples; i++)
				m_sound.fWaveform[ch][i] = m_sound.fWaveform[ch][i+align_offset[ch]];
			// zero the rest out, so it's visually evident that these samples are now bogus:
			memset(&m_sound.fWaveform[ch][nSamples], 0, (576-nSamples)*sizeof(float));
		}

}