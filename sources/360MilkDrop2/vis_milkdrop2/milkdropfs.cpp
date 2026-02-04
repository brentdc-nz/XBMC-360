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

#include "plugin.h"
#include "support.h"
#include "utility.h"
#include <assert.h>
#include <math.h>
#include "..\xbox\legacyshaders.h"

#define D3DCOLOR_RGBA_01(r,g,b,a) D3DCOLOR_RGBA(((int)(r*255)),((int)(g*255)),((int)(b*255)),((int)(a*255)))
#define FRAND ((rand() % 7381)/7380.0f)
#define warand() rand()


// This function evaluates whether the floating-point
// control Word is set to single precision/round to nearest/
// exceptions disabled. If not, the
// function changes the control Word to set them and returns
// TRUE, putting the old control Word value in the passback
// location pointed to by pwOldCW.

int GetNumToSpawn(float fTime, float fDeltaT, float fRate, float fRegularity, int iNumSpawnedSoFar)
{
    // PARAMETERS
    // ------------
    // fTime:          sum of all fDeltaT's so far (excluding this one)
    // fDeltaT:        time window for this frame
    // fRate:          avg. rate (spawns per second) of generation
    // fRegularity:    regularity of generation
    //					0.0: totally chaotic
    //					0.2: getting chaotic / very jittered
    //					0.4: nicely jittered
    //					0.6: slightly jittered
    //					0.8: almost perfectly regular
    //					1.0: perfectly regular
    // iNumSpawnedSoFar: the total number of spawnings so far
    //
    // RETURN VALUE
    // ------------
    // The number to spawn for this frame (add this to your net count!).
    //
    // COMMENTS
    // ------------
    // The spawn values returned will, over time, match
    // (within 1%) the theoretical totals expected based on the
    // amount of time passed and the average generation rate.
    //
    // UNRESOLVED ISSUES
    // -----------------
    // actual results of mixed gen. (0 < reg < 1) are about 1% too low
    // in the long run (vs. analytical expectations).  Decided not
    // to bother fixing it since it's only 1% (and VERY consistent).

    float fNumToSpawnReg;
    float fNumToSpawnIrreg;
    float fNumToSpawn;

    // compute # spawned based on regular generation
    fNumToSpawnReg = ((fTime + fDeltaT) * fRate) - iNumSpawnedSoFar;

    // compute # spawned based on irregular (random) generation
    // Adjust rate check for potentially very small fDeltaT
    if (fRate <= 0.0f || fDeltaT <= 0.0f || fDeltaT * fRate < 1e-6f) // Avoid division by zero or near-zero rate
    {
        fNumToSpawnIrreg = 0.0f;
    }
    else if (fDeltaT <= 1.0f / fRate)
    {
        // case 1: avg. less than 1 spawn per frame
        if ((warand() % 16384)/16384.0f < fDeltaT * fRate)
            fNumToSpawnIrreg = 1.0f;
        else
            fNumToSpawnIrreg = 0.0f;
    }
    else
    {
        // case 2: avg. more than 1 spawn per frame
        fNumToSpawnIrreg = fDeltaT * fRate;
        fNumToSpawnIrreg *= 2.0f*(warand() % 16384)/16384.0f;
    }

    // Ensure fRegularity is clamped between 0 and 1
    fRegularity = max(0.0f, min(1.0f, fRegularity));

    // get linear combo. of regular & irregular
    fNumToSpawn = fNumToSpawnReg*fRegularity + fNumToSpawnIrreg*(1.0f - fRegularity);

    // round to nearest integer for result
    // Add check for negative spawn count which can happen with large negative iNumSpawnedSoFar
    if (fNumToSpawn < 0) fNumToSpawn = 0;
    return (int)(fNumToSpawn + 0.49f);
}

void CPlugin::LoadPerFrameEvallibVars(CState* pState)
{
    // load the 'var_pf_*' variables in this CState object with the correct values.
    // for vars that affect pixel motion, that means evaluating them at time==-1,
    //    (i.e. no blending w/blendto value); the blending of the file dx/dy 
    //    will be done *after* execution of the per-vertex code.
    // for vars that do NOT affect pixel motion, evaluate them at the current time,
    //    so that if they're blending, both states see the blended value.

    // 1. vars that affect pixel motion: (eval at time==-1)
	*pState->var_pf_zoom		= (double)pState->m_fZoom.eval(-1);//GetTime()); 
	*pState->var_pf_zoomexp		= (double)pState->m_fZoomExponent.eval(-1);//GetTime());
	*pState->var_pf_rot			= (double)pState->m_fRot.eval(-1);//GetTime());  
	*pState->var_pf_warp		= (double)pState->m_fWarpAmount.eval(-1);//GetTime()); 
	*pState->var_pf_cx			= (double)pState->m_fRotCX.eval(-1);//GetTime());	
	*pState->var_pf_cy			= (double)pState->m_fRotCY.eval(-1);//GetTime()); 
	*pState->var_pf_dx			= (double)pState->m_fXPush.eval(-1);//GetTime());
	*pState->var_pf_dy			= (double)pState->m_fYPush.eval(-1);//GetTime());
	*pState->var_pf_sx			= (double)pState->m_fStretchX.eval(-1);//GetTime());
	*pState->var_pf_sy			= (double)pState->m_fStretchY.eval(-1);//GetTime());
	// read-only:
	*pState->var_pf_time		= (double)(GetTime() - m_fStartTime);
	*pState->var_pf_fps         = (double)GetFps();
	*pState->var_pf_bass		= (double)mysound.imm_rel[0];
	*pState->var_pf_mid			= (double)mysound.imm_rel[1];
	*pState->var_pf_treb		= (double)mysound.imm_rel[2];
	*pState->var_pf_bass_att	= (double)mysound.avg_rel[0];
	*pState->var_pf_mid_att		= (double)mysound.avg_rel[1];
	*pState->var_pf_treb_att	= (double)mysound.avg_rel[2];
	*pState->var_pf_frame		= (double)GetFrame();
	//*pState->var_pf_monitor     = 0;   -leave this as it was set in the per-frame INIT code!
    for (int vi=0; vi<NUM_Q_VAR; vi++)
	    *pState->var_pf_q[vi]	= pState->q_values_after_init_code[vi];//0.0f;
    *pState->var_pf_monitor     = pState->monitor_after_init_code;
	*pState->var_pf_progress    = (GetTime() - m_fPresetStartTime) / (m_fNextPresetTime - m_fPresetStartTime);

	// 2. vars that do NOT affect pixel motion: (eval at time==now)
	*pState->var_pf_decay		= (double)pState->m_fDecay.eval(GetTime());
	*pState->var_pf_wave_a		= (double)pState->m_fWaveAlpha.eval(GetTime());
	*pState->var_pf_wave_r		= (double)pState->m_fWaveR.eval(GetTime());
	*pState->var_pf_wave_g		= (double)pState->m_fWaveG.eval(GetTime());
	*pState->var_pf_wave_b		= (double)pState->m_fWaveB.eval(GetTime());
	*pState->var_pf_wave_x		= (double)pState->m_fWaveX.eval(GetTime());
	*pState->var_pf_wave_y		= (double)pState->m_fWaveY.eval(GetTime());
	*pState->var_pf_wave_mystery= (double)pState->m_fWaveParam.eval(GetTime());
	*pState->var_pf_wave_mode   = (double)pState->m_nWaveMode;	//?!?! -why won't it work if set to pState->m_nWaveMode???
	*pState->var_pf_ob_size		= (double)pState->m_fOuterBorderSize.eval(GetTime());
	*pState->var_pf_ob_r		= (double)pState->m_fOuterBorderR.eval(GetTime());
	*pState->var_pf_ob_g		= (double)pState->m_fOuterBorderG.eval(GetTime());
	*pState->var_pf_ob_b		= (double)pState->m_fOuterBorderB.eval(GetTime());
	*pState->var_pf_ob_a		= (double)pState->m_fOuterBorderA.eval(GetTime());
	*pState->var_pf_ib_size		= (double)pState->m_fInnerBorderSize.eval(GetTime());
	*pState->var_pf_ib_r		= (double)pState->m_fInnerBorderR.eval(GetTime());
	*pState->var_pf_ib_g		= (double)pState->m_fInnerBorderG.eval(GetTime());
	*pState->var_pf_ib_b		= (double)pState->m_fInnerBorderB.eval(GetTime());
	*pState->var_pf_ib_a		= (double)pState->m_fInnerBorderA.eval(GetTime());
	*pState->var_pf_mv_x        = (double)pState->m_fMvX.eval(GetTime());
	*pState->var_pf_mv_y        = (double)pState->m_fMvY.eval(GetTime());
	*pState->var_pf_mv_dx       = (double)pState->m_fMvDX.eval(GetTime());
	*pState->var_pf_mv_dy       = (double)pState->m_fMvDY.eval(GetTime());
	*pState->var_pf_mv_l        = (double)pState->m_fMvL.eval(GetTime());
	*pState->var_pf_mv_r        = (double)pState->m_fMvR.eval(GetTime());
	*pState->var_pf_mv_g        = (double)pState->m_fMvG.eval(GetTime());
	*pState->var_pf_mv_b        = (double)pState->m_fMvB.eval(GetTime());
	*pState->var_pf_mv_a        = (double)pState->m_fMvA.eval(GetTime());
	*pState->var_pf_echo_zoom   = (double)pState->m_fVideoEchoZoom.eval(GetTime());
	*pState->var_pf_echo_alpha  = (double)pState->m_fVideoEchoAlpha.eval(GetTime());
	*pState->var_pf_echo_orient = (double)pState->m_nVideoEchoOrientation;
    // new in v1.04:
    *pState->var_pf_wave_usedots  = (double)pState->m_bWaveDots;
    *pState->var_pf_wave_thick    = (double)pState->m_bWaveThick;
    *pState->var_pf_wave_additive = (double)pState->m_bAdditiveWaves;
    *pState->var_pf_wave_brighten = (double)pState->m_bMaximizeWaveColor;
    *pState->var_pf_darken_center = (double)pState->m_bDarkenCenter;
    *pState->var_pf_gamma         = (double)pState->m_fGammaAdj.eval(GetTime());
    *pState->var_pf_wrap          = (double)pState->m_bTexWrap;
    *pState->var_pf_invert        = (double)pState->m_bInvert;
    *pState->var_pf_brighten      = (double)pState->m_bBrighten;
    *pState->var_pf_darken        = (double)pState->m_bDarken;
    *pState->var_pf_solarize      = (double)pState->m_bSolarize;
    *pState->var_pf_meshx         = (double)m_nGridX;
    *pState->var_pf_meshy         = (double)m_nGridY;
    *pState->var_pf_pixelsx       = (double)GetWidth();
    *pState->var_pf_pixelsy       = (double)GetHeight();
    *pState->var_pf_aspectx       = (double)m_fInvAspectX;
    *pState->var_pf_aspecty       = (double)m_fInvAspectY;
    // new in v2.0:
    *pState->var_pf_blur1min      = (double)pState->m_fBlur1Min.eval(GetTime());
    *pState->var_pf_blur2min      = (double)pState->m_fBlur2Min.eval(GetTime());
    *pState->var_pf_blur3min      = (double)pState->m_fBlur3Min.eval(GetTime());
    *pState->var_pf_blur1max      = (double)pState->m_fBlur1Max.eval(GetTime());
    *pState->var_pf_blur2max      = (double)pState->m_fBlur2Max.eval(GetTime());
    *pState->var_pf_blur3max      = (double)pState->m_fBlur3Max.eval(GetTime());
    *pState->var_pf_blur1_edge_darken = (double)pState->m_fBlur1EdgeDarken.eval(GetTime());
}

void CPlugin::RunPerFrameEquations(int code)
{
	// run per-frame calculations

    /*
      code is only valid when blending.
          OLDcomp ~ blend-from preset has a composite shader;
          NEWwarp ~ blend-to preset has a warp shader; etc.

      code OLDcomp NEWcomp OLDwarp NEWwarp
        0    
        1            1
        2                            1
        3            1               1
        4     1
        5     1      1
        6     1                      1
        7     1      1               1
        8                    1
        9            1       1
        10                   1       1
        11           1       1       1
        12    1              1
        13    1      1       1
        14    1              1       1
        15    1      1       1       1
    */

    // when blending booleans (like darken, invert, etc) for pre-shader presets,
    // if blending to/from a pixel-shader preset, we can tune the snap point
    // (when it changes during the blend) for a less jumpy transition:
    m_fSnapPoint = 0.5f;
    if (m_pState->m_bBlending)
    {
        switch(code)
        {
        case 4:
        case 6:
        case 12:
        case 14:
            // old preset (only) had a comp shader
            m_fSnapPoint = -0.01f;
            break;
        case 1:
        case 3:
        case 9:
        case 11:
            // new preset (only) has a comp shader
            m_fSnapPoint = 1.01f;
            break;
        case 0:
        case 2:
        case 8:
        case 10:
            // neither old or new preset had a comp shader
            m_fSnapPoint = 0.5f;
            break;
        case 5:
        case 7:
        case 13:
        case 15:
            // both old and new presets use a comp shader - so it won't matter
            m_fSnapPoint = 0.5f;
            break;
        }
    }

	int num_reps = (m_pState->m_bBlending) ? 2 : 1;
	for (int rep=0; rep<num_reps; rep++)
	{
		CState *pState;

		if (rep==0)
			pState = m_pState;
		else
			pState = m_pOldState;

		// values that will affect the pixel motion (and will be automatically blended 
		//	LATER, when the results of 2 sets of these params creates 2 different U/V
		//  meshes that get blended together.)
        LoadPerFrameEvallibVars(pState);

		// also do just a once-per-frame init for the *per-**VERTEX*** *READ-ONLY* variables
		// (the non-read-only ones will be reset/restored at the start of each vertex)
		*pState->var_pv_time		= *pState->var_pf_time;	
		*pState->var_pv_fps         = *pState->var_pf_fps;
		*pState->var_pv_frame		= *pState->var_pf_frame;
		*pState->var_pv_progress    = *pState->var_pf_progress;
		*pState->var_pv_bass		= *pState->var_pf_bass;	
		*pState->var_pv_mid			= *pState->var_pf_mid;		
		*pState->var_pv_treb		= *pState->var_pf_treb;	
		*pState->var_pv_bass_att	= *pState->var_pf_bass_att;
		*pState->var_pv_mid_att		= *pState->var_pf_mid_att;	
		*pState->var_pv_treb_att	= *pState->var_pf_treb_att;
        *pState->var_pv_meshx       = (double)m_nGridX;
        *pState->var_pv_meshy       = (double)m_nGridY;
        *pState->var_pv_pixelsx     = (double)GetWidth();
        *pState->var_pv_pixelsy     = (double)GetHeight();
        *pState->var_pv_aspectx     = (double)m_fInvAspectX;
        *pState->var_pv_aspecty     = (double)m_fInvAspectY;
        //*pState->var_pv_monitor     = *pState->var_pf_monitor;

		// execute once-per-frame expressions:
#ifndef _NO_EXPR_
		if (pState->m_pf_codehandle)
		{
			if (pState->m_pf_codehandle)
			{
				NSEEL_code_execute(pState->m_pf_codehandle);
			}
		}
#endif

        // save some things for next frame:
        pState->monitor_after_init_code = *pState->var_pf_monitor;

        // save some things for per-vertex code:
        for (int vi=0; vi<NUM_Q_VAR; vi++)
            *pState->var_pv_q[vi] = *pState->var_pf_q[vi];

        // (a few range checks:)
        *pState->var_pf_gamma     = max(0    , min(    8, *pState->var_pf_gamma    ));
        *pState->var_pf_echo_zoom = max(0.001, min( 1000, *pState->var_pf_echo_zoom));
	}

	if (m_pState->m_bBlending)
	{
		// For all variables that do NOT affect pixel motion, blend them NOW,
        // so later the user can just access m_pState->m_pf_whatever.  
		double mix  = (double)CosineInterp(m_pState->m_fBlendProgress);
		double mix2 = 1.0 - mix;
        *m_pState->var_pf_decay        = mix*(*m_pState->var_pf_decay       ) + mix2*(*m_pOldState->var_pf_decay       );
        *m_pState->var_pf_wave_a       = mix*(*m_pState->var_pf_wave_a      ) + mix2*(*m_pOldState->var_pf_wave_a      );
        *m_pState->var_pf_wave_r       = mix*(*m_pState->var_pf_wave_r      ) + mix2*(*m_pOldState->var_pf_wave_r      );
        *m_pState->var_pf_wave_g       = mix*(*m_pState->var_pf_wave_g      ) + mix2*(*m_pOldState->var_pf_wave_g      );
        *m_pState->var_pf_wave_b       = mix*(*m_pState->var_pf_wave_b      ) + mix2*(*m_pOldState->var_pf_wave_b      );
        *m_pState->var_pf_wave_x       = mix*(*m_pState->var_pf_wave_x      ) + mix2*(*m_pOldState->var_pf_wave_x      );
        *m_pState->var_pf_wave_y       = mix*(*m_pState->var_pf_wave_y      ) + mix2*(*m_pOldState->var_pf_wave_y      );
        *m_pState->var_pf_wave_mystery = mix*(*m_pState->var_pf_wave_mystery) + mix2*(*m_pOldState->var_pf_wave_mystery);
		// wave_mode: exempt (integer)
        *m_pState->var_pf_ob_size      = mix*(*m_pState->var_pf_ob_size     ) + mix2*(*m_pOldState->var_pf_ob_size     ); 
        *m_pState->var_pf_ob_r         = mix*(*m_pState->var_pf_ob_r        ) + mix2*(*m_pOldState->var_pf_ob_r        );
        *m_pState->var_pf_ob_g         = mix*(*m_pState->var_pf_ob_g        ) + mix2*(*m_pOldState->var_pf_ob_g        );
        *m_pState->var_pf_ob_b         = mix*(*m_pState->var_pf_ob_b        ) + mix2*(*m_pOldState->var_pf_ob_b        );
        *m_pState->var_pf_ob_a         = mix*(*m_pState->var_pf_ob_a        ) + mix2*(*m_pOldState->var_pf_ob_a        );
        *m_pState->var_pf_ib_size      = mix*(*m_pState->var_pf_ib_size     ) + mix2*(*m_pOldState->var_pf_ib_size     );
        *m_pState->var_pf_ib_r         = mix*(*m_pState->var_pf_ib_r        ) + mix2*(*m_pOldState->var_pf_ib_r        );
        *m_pState->var_pf_ib_g         = mix*(*m_pState->var_pf_ib_g        ) + mix2*(*m_pOldState->var_pf_ib_g        );
        *m_pState->var_pf_ib_b         = mix*(*m_pState->var_pf_ib_b        ) + mix2*(*m_pOldState->var_pf_ib_b        );
        *m_pState->var_pf_ib_a         = mix*(*m_pState->var_pf_ib_a        ) + mix2*(*m_pOldState->var_pf_ib_a        );
        *m_pState->var_pf_mv_x         = mix*(*m_pState->var_pf_mv_x        ) + mix2*(*m_pOldState->var_pf_mv_x        );
        *m_pState->var_pf_mv_y         = mix*(*m_pState->var_pf_mv_y        ) + mix2*(*m_pOldState->var_pf_mv_y        );
        *m_pState->var_pf_mv_dx        = mix*(*m_pState->var_pf_mv_dx       ) + mix2*(*m_pOldState->var_pf_mv_dx       );
        *m_pState->var_pf_mv_dy        = mix*(*m_pState->var_pf_mv_dy       ) + mix2*(*m_pOldState->var_pf_mv_dy       );
        *m_pState->var_pf_mv_l         = mix*(*m_pState->var_pf_mv_l        ) + mix2*(*m_pOldState->var_pf_mv_l        );
        *m_pState->var_pf_mv_r         = mix*(*m_pState->var_pf_mv_r        ) + mix2*(*m_pOldState->var_pf_mv_r        );
        *m_pState->var_pf_mv_g         = mix*(*m_pState->var_pf_mv_g        ) + mix2*(*m_pOldState->var_pf_mv_g        );
        *m_pState->var_pf_mv_b         = mix*(*m_pState->var_pf_mv_b        ) + mix2*(*m_pOldState->var_pf_mv_b        );
        *m_pState->var_pf_mv_a         = mix*(*m_pState->var_pf_mv_a        ) + mix2*(*m_pOldState->var_pf_mv_a        );
		*m_pState->var_pf_echo_zoom    = mix*(*m_pState->var_pf_echo_zoom   ) + mix2*(*m_pOldState->var_pf_echo_zoom   );
		*m_pState->var_pf_echo_alpha   = mix*(*m_pState->var_pf_echo_alpha  ) + mix2*(*m_pOldState->var_pf_echo_alpha  );
        *m_pState->var_pf_echo_orient  = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_echo_orient : *m_pState->var_pf_echo_orient;
        // added in v1.04:
        *m_pState->var_pf_wave_usedots = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_wave_usedots  : *m_pState->var_pf_wave_usedots ;
        *m_pState->var_pf_wave_thick   = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_wave_thick    : *m_pState->var_pf_wave_thick   ;
        *m_pState->var_pf_wave_additive= (mix < m_fSnapPoint) ? *m_pOldState->var_pf_wave_additive : *m_pState->var_pf_wave_additive;
        *m_pState->var_pf_wave_brighten= (mix < m_fSnapPoint) ? *m_pOldState->var_pf_wave_brighten : *m_pState->var_pf_wave_brighten;
        *m_pState->var_pf_darken_center= (mix < m_fSnapPoint) ? *m_pOldState->var_pf_darken_center : *m_pState->var_pf_darken_center;
        *m_pState->var_pf_gamma        = mix*(*m_pState->var_pf_gamma       ) + mix2*(*m_pOldState->var_pf_gamma       );
        *m_pState->var_pf_wrap         = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_wrap          : *m_pState->var_pf_wrap         ;
        *m_pState->var_pf_invert       = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_invert        : *m_pState->var_pf_invert       ;
        *m_pState->var_pf_brighten     = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_brighten      : *m_pState->var_pf_brighten     ;
        *m_pState->var_pf_darken       = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_darken        : *m_pState->var_pf_darken       ;
        *m_pState->var_pf_solarize     = (mix < m_fSnapPoint) ? *m_pOldState->var_pf_solarize      : *m_pState->var_pf_solarize     ;
        // added in v2.0:
        *m_pState->var_pf_blur1min  = mix*(*m_pState->var_pf_blur1min ) + mix2*(*m_pOldState->var_pf_blur1min );
        *m_pState->var_pf_blur2min  = mix*(*m_pState->var_pf_blur2min ) + mix2*(*m_pOldState->var_pf_blur2min );
        *m_pState->var_pf_blur3min  = mix*(*m_pState->var_pf_blur3min ) + mix2*(*m_pOldState->var_pf_blur3min );
        *m_pState->var_pf_blur1max  = mix*(*m_pState->var_pf_blur1max ) + mix2*(*m_pOldState->var_pf_blur1max );
        *m_pState->var_pf_blur2max  = mix*(*m_pState->var_pf_blur2max ) + mix2*(*m_pOldState->var_pf_blur2max );
        *m_pState->var_pf_blur3max  = mix*(*m_pState->var_pf_blur3max ) + mix2*(*m_pOldState->var_pf_blur3max );
        *m_pState->var_pf_blur1_edge_darken = mix*(*m_pState->var_pf_blur1_edge_darken) + mix2*(*m_pOldState->var_pf_blur1_edge_darken);
    }
}

void CPlugin::RenderFrame(int bRedraw)
{
    int i;

    float fDeltaT = 1.0f/GetFps(); // Be cautious if GetFps() can be zero or negative

    // --- Ensure GetFps() is safe ---
    if (fDeltaT <= 0) fDeltaT = 1.0f / 30.0f; // Default to 30fps if GetFps() is invalid

    // --- State Updates ---
    if (GetFrame()==0)
    {
        m_fStartTime = GetTime();
        m_fPresetStartTime = GetTime();
    }

    if (m_fNextPresetTime < 0)
    {
        float dt = m_fTimeBetweenPresetsRand * (warand()%1000)*0.001f;
        m_fNextPresetTime = GetTime() + m_fBlendTimeAuto + m_fTimeBetweenPresets + dt;
    }

    if (!bRedraw)
    {
        m_rand_frame = D3DXVECTOR4(FRAND, FRAND, FRAND, FRAND);

        // randomly change the preset, if it's time
        if (m_fNextPresetTime < GetTime())
        {
            if (m_nLoadingPreset==0) // don't start a load if one is already underway!
                LoadRandomPreset(m_fBlendTimeAuto);
        }

        // update m_fBlendProgress;
        if (m_pState->m_bBlending)
        {
            m_pState->m_fBlendProgress = (GetTime() - m_pState->m_fBlendStartTime) / m_pState->m_fBlendDuration;
            if (m_pState->m_fBlendProgress >= 1.0f) // Use >= to ensure it terminates
            {
                m_pState->m_fBlendProgress = 1.0f; // Clamp
                m_pState->m_bBlending = false;
            }
            else if (m_pState->m_fBlendProgress < 0.0f) // Clamp negative case too
            {
                m_pState->m_fBlendProgress = 0.0f;
            }
        }

        // handle hard cuts here (just after new sound analysis)
        static float m_fHardCutThresh = m_fHardCutLoudnessThresh * 2.0f; // Initialize here
        //if (GetFrame() == 0) // Moved initialization outside
        //    m_fHardCutThresh = m_fHardCutLoudnessThresh*2.0f;
        if (GetFps() > 1.0f && !m_bHardCutsDisabled && !m_bPresetLockedByUser && !m_bPresetLockedByCode)
        {
            if (mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2] > m_fHardCutThresh*3.0f)
            {
                if (m_nLoadingPreset==0) // don't start a load if one is already underway!
                    LoadRandomPreset(0.0f);
                m_fHardCutThresh *= 2.0f;
            }
            else
            {
                // Ensure GetFps() is valid before division
                float currentFps = GetFps();
                if (currentFps > 0.0f && m_fHardCutHalflife > 0.0f) {
                    float k = -1.3863f / (m_fHardCutHalflife * currentFps);
                    float single_frame_multiplier = expf(k);
                    m_fHardCutThresh = (m_fHardCutThresh - m_fHardCutLoudnessThresh)*single_frame_multiplier + m_fHardCutLoudnessThresh;
                } else {
                    // Fallback or do nothing if FPS or halflife is invalid
                    m_fHardCutThresh = m_fHardCutLoudnessThresh; // Reset or decay slowly
                }
            }
        }

        // smooth & scale the audio data, according to m_state, for display purposes
        float scale = m_pState->m_fWaveScale.eval(GetTime()) / 128.0f;
        mysound.fWave[0][0] *= scale;
        mysound.fWave[1][0] *= scale;
        float mix2 = m_pState->m_fWaveSmoothing.eval(GetTime());
        mix2 = max(0.0f, min(1.0f, mix2)); // Clamp smoothing factor
        float mix1 = scale*(1.0f - mix2);
        for (i=1; i<576; i++)
        {
            mysound.fWave[0][i] = mysound.fWave[0][i]*mix1 + mysound.fWave[0][i-1]*mix2;
            mysound.fWave[1][i] = mysound.fWave[1][i]*mix1 + mysound.fWave[1][i-1]*mix2;
        }
    }

    // --- Determine Shader Usage ---
#if 1 // Hardcode these here rather than Winamp UI
    m_pOldState->m_nWarpPSVersion = MD2_PS_3_0;
    m_pState->m_nWarpPSVersion = MD2_PS_3_0;
    m_pOldState->m_nCompPSVersion = MD2_PS_3_0; // Composite shader usage might be irrelevant now
    m_pState->m_nCompPSVersion = MD2_PS_3_0;   // Composite shader usage might be irrelevant now
#else
    m_pOldState->m_nWarpPSVersion = MD2_PS_NONE;
    m_pState->m_nWarpPSVersion = MD2_PS_NONE;
    m_pOldState->m_nCompPSVersion = MD2_PS_NONE;
    m_pState->m_nCompPSVersion = MD2_PS_NONE;
#endif

    bool bOldPresetUsesWarpShader = (m_pOldState->m_nWarpPSVersion > 0);
    bool bNewPresetUsesWarpShader = (m_pState->m_nWarpPSVersion > 0);
    // Composite shader flags are less relevant now as logic is merged into warp pass
    // bool bOldPresetUsesCompShader = (m_pOldState->m_nCompPSVersion > 0);
    // bool bNewPresetUsesCompShader = (m_pState->m_nCompPSVersion > 0);

    // note: 'code' is only meaningful if we are BLENDING.
    // Code might need adjustment if composite shader flags are no longer used for branching
    int code = (bOldPresetUsesWarpShader ? 8 : 0) |
               //(bOldPresetUsesCompShader ? 4 : 0) | // Removed Comp shader flags from code calculation
               (bNewPresetUsesWarpShader ? 2 : 0);// |
               //(bNewPresetUsesCompShader ? 1 : 0); // Removed Comp shader flags from code calculation

    RunPerFrameEquations(code);

    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    // --- Set up common render states ---
    {
        DWORD texaddr = (*m_pState->var_pf_wrap > m_fSnapPoint) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
        // lpDevice->SetRenderState(D3DRS_WRAP0, 0); // WRAP0 is deprecated/not standard D3D9
        lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, texaddr);
        lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, texaddr);
        lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, texaddr); // Often unused for 2D textures
        lpDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, texaddr); // Assuming stage 1 also needs wrapping control
        lpDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, texaddr);
        lpDevice->SetSamplerState(1, D3DSAMP_ADDRESSW, texaddr);

        lpDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
        lpDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
        lpDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
        lpDevice->SetRenderState( D3DRS_FILLMODE,  D3DFILL_SOLID );
        lpDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ); // Default, will be overridden by drawing functions

        // Set default filtering for stages 0 and 1
        lpDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); // Or D3DTEXF_NONE if not using mipmaps
        lpDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); // Or D3DTEXF_NONE

        // Removed fixed-function texture stage states (BRENT comments) as shaders handle this

        lpDevice->SetTexture(0, NULL); // Ensure textures are unbound initially
        lpDevice->SetTexture(1, NULL);
    }

    // --- Blur Pass ---
    // NOTE: Disabled because BlurPasses likely requires render targets, which are disallowed.
    // If blur textures (m_lpBlur) can be created *without* D3DUSAGE_RENDERTARGET
    // and BlurPasses can be modified to write to them via other means (e.g., LockRect/UnlockRect,
    // or if the platform allows specific texture copy operations without RT usage),
    // then this could potentially be re-enabled.
    // if (m_nMaxPSVersion > 0)
    //    BlurPasses();

    // --- Main Warp/Composite Pass (Renders to Back Buffer) ---
    // This pass reads m_lpPrevFrame, applies warp, decay, gamma, echo, etc.
    ComputeGridAlphaValues(); // Prepare vertex data (including warp info and blend alpha)

    // Set the previous frame texture for the warp pass to read
    // lpDevice->SetTexture(0, m_lpPrevFrame); // WarpedBlit functions should set this

    if (!m_pState->m_bBlending)
    {
        // no blend
        if (bNewPresetUsesWarpShader)
            WarpedBlit_Shaders(1, false, false, false, false); // Pass 1 = New Preset
        else
            WarpedBlit_NoShaders(1, false, false, false, false); // Pass 1 = New Preset
    }
    else
    {
        // blending
        // WarpedBlit now handles both warp and composite effects.
        // Alpha blending is controlled by vertex alpha calculated in ComputeGridAlphaValues.
        // bAlphaBlend = true enables blending based on vertex alpha.
        // bFlipAlpha reverses the blend (useful for fade-out).
        // bCullTiles optimizes by skipping fully transparent triangles.
        // bFlipCulling changes which alpha values cause culling (0 or 255).

        if (bOldPresetUsesWarpShader && bNewPresetUsesWarpShader)
        {
            // Draw Old Preset (Pass 0), alpha based on (1-blend), cull fully blended-out tiles
            WarpedBlit_Shaders  (0, true, true,   true, true);
            // Draw New Preset (Pass 1) blended over the old, alpha based on blend, cull fully blended-out tiles
            WarpedBlit_Shaders  (1, true, false,   true, false);
        }
        else if (!bOldPresetUsesWarpShader && bNewPresetUsesWarpShader)
        {
            // Draw Old Preset (Pass 0), alpha based on (1-blend), cull fully blended-out tiles
            WarpedBlit_NoShaders(0, true, true,   true, true);
            // Draw New Preset (Pass 1) blended over the old, alpha based on blend, cull fully blended-out tiles
            WarpedBlit_Shaders  (1, true, false,   true, false);
        }
        else if (bOldPresetUsesWarpShader && !bNewPresetUsesWarpShader)
        {
            // Draw Old Preset (Pass 0), alpha based on (1-blend), cull fully blended-out tiles
            WarpedBlit_Shaders  (0, true, true,   true, true);
            // Draw New Preset (Pass 1) blended over the old, alpha based on blend, cull fully blended-out tiles
            WarpedBlit_NoShaders(1, true, false,   true, false);
        }
        else // !bOldPresetUsesWarpShader && !bNewPresetUsesWarpShader
        {
            // Vertex positions/UVs are already blended in ComputeGridAlphaValues.
            // Just draw the final blended state using Pass 1 (New Preset's non-shader logic).
            // No alpha blending needed here as vertex data handles the mix.
            WarpedBlit_NoShaders(1, false, false, false, false);
        }
    }
    // --- End Main Warp/Composite Pass ---


    // --- Overlay Passes (Render to Back Buffer) ---
    // These draw over the result of the WarpedBlit pass.
    DrawMotionVectors(); // Draw motion vectors if enabled
    DrawCustomShapes();
    DrawCustomWaves();
    DrawWave(mysound.fWave[0], mysound.fWave[1]); // Draw standard waveform
    DrawSprites(); // Draw borders and darken center effect
    DrawUserSprites(); // Draw user-loaded sprites (texmgr)
    // --- End Overlay Passes ---


    // --- Remove ShowToUser calls ---
    // The logic from ShowToUser_NoShaders is now in WarpedBlit_NoShaders.
    // The logic from ShowToUser_Shaders should be integrated into the composite/warp shader itself.
    /*
    // show it to the user [composite shader] - REMOVED
    if (!m_pState->m_bBlending)
    { ... }
    else
    { ... }
    */


    // --- Feedback Update ---
    // Copy the final back buffer to m_lpPrevFrame for the next frame's feedback
    // This MUST happen AFTER all drawing for the current frame is complete.
    LPDIRECT3DSURFACE9 pBackBufferB = NULL;
    HRESULT hr_rt = GetDevice()->GetRenderTarget(0, &pBackBufferB);

    if (SUCCEEDED(hr_rt) && pBackBufferB)
    {
        // Use Resolve to copy the potentially anti-aliased back buffer to the texture.
        // Ensure m_lpPrevFrame was created with compatible format and in D3DPOOL_DEFAULT.
        // If m_lpPrevFrame is D3DPOOL_MANAGED, you might need GetRenderTargetData instead,
        // but Resolve is generally preferred on Xbox 360 if possible.
        D3DRECT srcRect = { 0, 0, 1280, 720 }; // Use actual dimensions
        D3DPOINT destPoint = { 0, 0 };

        HRESULT hr_res = GetDevice()->Resolve(
            D3DRESOLVE_RENDERTARGET0,
            &srcRect,
            m_lpPrevFrame,
            &destPoint,
            0,
            0,
            NULL,
            0.0f,
            0,
            NULL
        );
        // Optional: Add error checking for hr_res if needed

        // Cleanup
        pBackBufferB->Release();
    }
    // --- End Feedback Update ---
}

void CPlugin::DrawMotionVectors()
{
	// FLEXIBLE MOTION VECTOR FIELD
	if ((float)*m_pState->var_pf_mv_a >= 0.001f)
	{
        //-------------------------------------------------------
        LPDIRECT3DDEVICE9 lpDevice = GetDevice();
        if (!lpDevice)
            return;

        lpDevice->SetTexture(0, NULL);
        lpDevice->SetVertexShader(g_pVertexShader_Legacy1);
		lpDevice->SetPixelShader(g_pPixelShader_Legacy1);
        lpDevice->SetFVF(WFVERTEX_FORMAT);

        //-------------------------------------------------------

		int x,y;

		int nX = (int)(*m_pState->var_pf_mv_x);// + 0.999f);
		int nY = (int)(*m_pState->var_pf_mv_y);// + 0.999f);
		float dx = (float)*m_pState->var_pf_mv_x - nX;
		float dy = (float)*m_pState->var_pf_mv_y - nY;
		if (nX > 64) { nX = 64; dx = 0; }
		if (nY > 48) { nY = 48; dy = 0; }

		if (nX > 0 && nY > 0)
		{
			float dx2 = (float)(*m_pState->var_pf_mv_dx);
			float dy2 = (float)(*m_pState->var_pf_mv_dy);

			float len_mult = (float)*m_pState->var_pf_mv_l;
			if (dx < 0) dx = 0;
			if (dy < 0) dy = 0;
			if (dx > 1) dx = 1;
			if (dy > 1) dy = 1;
			//dx = dx * 1.0f/(float)nX;
			//dy = dy * 1.0f/(float)nY;
			float inv_texsize = 1.0f/(float)m_nTexSizeX;
			float min_len = 1.0f*inv_texsize;

			WFVERTEX v[(64+1)*2];
			ZeroMemory(v, sizeof(WFVERTEX)*(64+1)*2);
			v[0].Diffuse = D3DCOLOR_RGBA_01((float)*m_pState->var_pf_mv_r,(float)*m_pState->var_pf_mv_g,(float)*m_pState->var_pf_mv_b,(float)*m_pState->var_pf_mv_a);
			for (x=1; x<(nX+1)*2; x++)
				v[x].Diffuse = v[0].Diffuse;

			lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
			lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			for (y=0; y<nY; y++)
			{
				float fy = (y + 0.25f)/(float)(nY + dy + 0.25f - 1.0f);

				// now move by offset
				fy -= dy2;

				if (fy > 0.0001f && fy < 0.9999f)
				{
					int n = 0;
					for (x=0; x<nX; x++)
					{
						//float fx = (x + 0.25f)/(float)(nX + dx + 0.25f - 1.0f);
						float fx = (x + 0.25f)/(float)(nX + dx + 0.25f - 1.0f);

						// now move by offset
						fx += dx2;

						if (fx > 0.0001f && fx < 0.9999f)
						{
							float fx2, fy2;
							ReversePropagatePoint(fx, fy, &fx2, &fy2);	// NOTE: THIS IS REALLY A REVERSE-PROPAGATION
							//fx2 = fx*2 - fx2;
							//fy2 = fy*2 - fy2;
							//fx2 = fx + 1.0f/(float)m_nTexSize;
							//fy2 = 1-(fy + 1.0f/(float)m_nTexSize);

							// enforce minimum trail lengths:
							{	
								float dx = (fx2 - fx);
								float dy = (fy2 - fy);
								dx *= len_mult;
								dy *= len_mult;
								float len = sqrtf(dx*dx + dy*dy);

								if (len > min_len)
								{

								}
								else if (len > 0.00000001f)
								{
									len = min_len/len;
									dx *= len;
									dy *= len;
								}
								else
								{
									dx = min_len;
									dy = min_len;
								}
									
								fx2 = fx + dx;
								fy2 = fy + dy;
							}
							/**/

							v[n].x = fx * 2.0f - 1.0f;
							v[n].y = fy * 2.0f - 1.0f;
							v[n+1].x = fx2 * 2.0f - 1.0f;
							v[n+1].y = fy2 * 2.0f - 1.0f; 

							// actually, project it in the reverse direction
							//v[n+1].x = v[n].x*2.0f - v[n+1].x;// + dx*2; 
							//v[n+1].y = v[n].y*2.0f - v[n+1].y;// + dy*2; 
							//v[n].x += dx*2;
							//v[n].y += dy*2;

							n += 2;
						}
					}

					// draw it
					lpDevice->DrawPrimitiveUP(D3DPT_LINELIST, n/2, v, sizeof(WFVERTEX));
				}
			}

			lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		}
	}
}

bool CPlugin::ReversePropagatePoint(float fx, float fy, float *fx2, float *fy2)
{
	//float fy = y/(float)nMotionVectorsY;
	int   y0 = (int)(fy*m_nGridY);
	float dy = fy*m_nGridY - y0;

	//float fx = x/(float)nMotionVectorsX;
	int   x0 = (int)(fx*m_nGridX);
	float dx = fx*m_nGridX - x0;

	int x1 = x0 + 1;
	int y1 = y0 + 1;

	if (x0 < 0) return false;
	if (y0 < 0) return false;
	//if (x1 < 0) return false;
	//if (y1 < 0) return false;
	//if (x0 > m_nGridX) return false;
	//if (y0 > m_nGridY) return false;
	if (x1 > m_nGridX) return false;
	if (y1 > m_nGridY) return false;

	float tu, tv;
	tu  = m_verts[y0*(m_nGridX+1)+x0].tu * (1-dx)*(1-dy);
	tv  = m_verts[y0*(m_nGridX+1)+x0].tv * (1-dx)*(1-dy);
	tu += m_verts[y0*(m_nGridX+1)+x1].tu * (dx)*(1-dy);
	tv += m_verts[y0*(m_nGridX+1)+x1].tv * (dx)*(1-dy);
	tu += m_verts[y1*(m_nGridX+1)+x0].tu * (1-dx)*(dy);
	tv += m_verts[y1*(m_nGridX+1)+x0].tv * (1-dx)*(dy);
	tu += m_verts[y1*(m_nGridX+1)+x1].tu * (dx)*(dy);
	tv += m_verts[y1*(m_nGridX+1)+x1].tv * (dx)*(dy);

	*fx2 = tu;
	*fy2 = 1.0f - tv;
	return true;
}

void CPlugin::GetSafeBlurMinMax(CState* pState, float* blur_min, float* blur_max)
{
    blur_min[0] = (float)*pState->var_pf_blur1min;
    blur_min[1] = (float)*pState->var_pf_blur2min;
    blur_min[2] = (float)*pState->var_pf_blur3min;
    blur_max[0] = (float)*pState->var_pf_blur1max;
    blur_max[1] = (float)*pState->var_pf_blur2max;
    blur_max[2] = (float)*pState->var_pf_blur3max;

    // check that precision isn't wasted in later blur passes [...min-max gap can't grow!]
    // also, if min-max are close to each other, push them apart:
    const float fMinDist = 0.1f;
    if (blur_max[0] - blur_min[0] < fMinDist) {
        float avg = (blur_min[0] + blur_max[0])*0.5f;
        blur_min[0] = avg - fMinDist*0.5f;
        blur_max[0] = avg - fMinDist*0.5f;
    }
    blur_max[1] = min(blur_max[0], blur_max[1]);
    blur_min[1] = max(blur_min[0], blur_min[1]);
    if (blur_max[1] - blur_min[1] < fMinDist) {
        float avg = (blur_min[1] + blur_max[1])*0.5f;
        blur_min[1] = avg - fMinDist*0.5f;
        blur_max[1] = avg - fMinDist*0.5f;
    }
    blur_max[2] = min(blur_max[1], blur_max[2]);
    blur_min[2] = max(blur_min[1], blur_min[2]);
    if (blur_max[2] - blur_min[2] < fMinDist) {
        float avg = (blur_min[2] + blur_max[2])*0.5f;
        blur_min[2] = avg - fMinDist*0.5f;
        blur_max[2] = avg - fMinDist*0.5f;
    }
}

void CPlugin::BlurPasses()
{
    // NOTE: This function likely requires render targets (m_lpBlur textures).
    // If render targets are strictly disallowed on the target platform (Xbox 360 EDRAM limits),
    // this function needs to be disabled or completely rewritten.
    // The code below assumes m_lpBlur CAN be render targets, but reads initial data
    // from m_lpPrevFrame instead of an intermediate texture.

    #if (NUM_BLUR_TEX > 0)
        LPDIRECT3DDEVICE9 lpDevice = GetDevice();
        if (!lpDevice)
            return;

        // Determine how many blur passes are actually needed based on shader usage
        int passes = min(NUM_BLUR_TEX, m_nHighestBlurTexUsedThisFrame * 2); // *2 for H/V passes
        if (passes == 0) {
            m_nHighestBlurTexUsedThisFrame = 0; // Reset if no blur used
            return;
        }

        // --- Check if m_lpBlur textures are valid render targets ---
        // This is a placeholder check. Real validation depends on how they were created.
        bool blurTargetsValid = true;
        for(int i = 0; i < passes; ++i) {
            if (!m_lpBlur[i]) {
                blurTargetsValid = false;
                break;
            }
            // Optionally, check texture description for D3DUSAGE_RENDERTARGET if possible
        }
        if (!blurTargetsValid) {
             m_nHighestBlurTexUsedThisFrame = 0; // Reset if textures invalid
             return; // Cannot perform blur if textures aren't valid RTs
        }
        // --- End Check ---


        // Set common states for blur passes
        lpDevice->SetVertexShader( m_BlurShaders[0].vs.ptr ); // Assuming one VS for all blur passes
        lpDevice->SetVertexDeclaration(m_pMyVertDecl);
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        DWORD wrap = D3DTADDRESS_CLAMP; // Blur usually clamps
        lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, wrap);
        lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, wrap);
        lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, wrap);
        lpDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); // Or NONE
        // lpDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 1); // Anisotropy usually not needed for blur

        // Clear texture bindings for safety
        for (int stage = 0; stage < 16; stage++) // Use a reasonable upper limit
            lpDevice->SetTexture(stage, NULL);

        // Set up fullscreen quad vertices
        MYVERTEX v[4];
        v[0].x = -1; v[0].y =  1; v[0].z = 0; v[0].tu = 0; v[0].tv = 0; // Top-left
        v[1].x =  1; v[1].y =  1; v[1].z = 0; v[1].tu = 1; v[1].tv = 0; // Top-right
        v[2].x = -1; v[2].y = -1; v[2].z = 0; v[2].tu = 0; v[2].tv = 1; // Bottom-left
        v[3].x =  1; v[3].y = -1; v[3].z = 0; v[3].tu = 1; v[3].tv = 1; // Bottom-right
        // Set dummy diffuse color if needed by FVF/shader
        v[0].Diffuse = v[1].Diffuse = v[2].Diffuse = v[3].Diffuse = 0xFFFFFFFF;

        // Get blur parameters
        const float w[8] = { 4.0f, 3.8f, 3.5f, 2.9f, 1.9f, 1.2f, 0.7f, 0.3f }; // Example weights
        float edge_darken = (float)*m_pState->var_pf_blur1_edge_darken;
        float blur_min[3], blur_max[3];
        GetSafeBlurMinMax(m_pState, blur_min, blur_max);
        float fscale[3], fbias[3];
        // Calculate scale/bias for progressive blur range mapping (as before)
        // ... (calculation code remains the same) ...
        float temp_min, temp_max;
        if (blur_max[0] - blur_min[0] < 1e-6f) { // Avoid division by zero
             fscale[0] = 1.0f; fbias[0] = 0.0f;
        } else {
             fscale[0] = 1.0f / (blur_max[0] - blur_min[0]);
             fbias [0] = -blur_min[0] * fscale[0];
        }
        temp_min  = (blur_min[1] - blur_min[0]) * fscale[0] + fbias[0]; // Map to [0..1] range of previous pass
        temp_max  = (blur_max[1] - blur_min[0]) * fscale[0] + fbias[0];
        if (temp_max - temp_min < 1e-6f) {
             fscale[1] = 1.0f; fbias[1] = 0.0f;
        } else {
             fscale[1] = 1.0f / (temp_max - temp_min);
             fbias [1] = -temp_min * fscale[1];
        }
        temp_min  = (blur_min[2] - blur_min[1]) * fscale[1] + fbias[1]; // Map to [0..1] range of previous pass
        temp_max  = (blur_max[2] - blur_min[1]) * fscale[1] + fbias[1];
        if (temp_max - temp_min < 1e-6f) {
             fscale[2] = 1.0f; fbias[2] = 0.0f;
        } else {
             fscale[2] = 1.0f / (temp_max - temp_min);
             fbias [2] = -temp_min * fscale[2];
        }


        // --- Blur Loop ---
        LPDIRECT3DSURFACE9 pOldRT = NULL;
        lpDevice->GetRenderTarget(0, &pOldRT); // Save original back buffer RT

        for (int i = 0; i < passes; i++)
        {
            // Set source texture: Previous frame for first pass, previous blur tex otherwise
            lpDevice->SetTexture(0, (i == 0) ? m_lpPrevFrame : m_lpBlur[i - 1]);

            // Set Render Target to the current blur texture
            LPDIRECT3DSURFACE9 pBlurTargetSurface = NULL;
            // m_lpBlur[i] should be valid based on earlier check
            m_lpBlur[i]->GetSurfaceLevel(0, &pBlurTargetSurface);

            if (!pBlurTargetSurface) {
                // Should not happen if initial check passed, but handle defensively
                lpDevice->SetTexture(0, NULL); // Unbind source
                continue;
            }

            lpDevice->SetRenderTarget(0, pBlurTargetSurface);
            pBlurTargetSurface->Release(); // RT holds a reference now

            // Set pixel shader for this pass (horizontal/vertical)
            lpDevice->SetPixelShader(m_BlurShaders[i % 2].ps.ptr);

            // Set shader constants
            LPD3DXCONSTANTTABLE pCT = m_BlurShaders[i % 2].ps.CT;
            D3DXHANDLE* h = m_BlurShaders[i % 2].ps.params.const_handles;

            int srcw = (i == 0) ? GetWidth() : m_nBlurTexW[i - 1];
            int srch = (i == 0) ? GetHeight() : m_nBlurTexH[i - 1];
            D3DXVECTOR4 srctexsize = D3DXVECTOR4((float)srcw, (float)srch, (srcw > 0) ? 1.0f / (float)srcw : 0.0f, (srch > 0) ? 1.0f / (float)srch : 0.0f);

            float fscale_now = fscale[i / 2];
            float fbias_now = fbias[i / 2];

            // Set constants based on pass type (horizontal/vertical)
            // ... (constant setting logic remains the same) ...
            if (i%2==0)
            {
                // pass 1 (long horizontal pass)
                const float w1 = w[0] + w[1];
                const float w2 = w[2] + w[3];
                const float w3 = w[4] + w[5];
                const float w4 = w[6] + w[7];
                const float d1 = (w1 > 1e-6f) ? (0 + 2*w[1]/w1) : 0;
                const float d2 = (w2 > 1e-6f) ? (2 + 2*w[3]/w2) : 2;
                const float d3 = (w3 > 1e-6f) ? (4 + 2*w[5]/w3) : 4;
                const float d4 = (w4 > 1e-6f) ? (6 + 2*w[7]/w4) : 6;
                const float w_sum = w1+w2+w3+w4;
                const float w_div = (w_sum > 1e-6f) ? (0.5f/w_sum) : 0.0f;
                if (h[0]) pCT->SetVector( lpDevice, h[0], &srctexsize );
                if (h[1]) pCT->SetVector( lpDevice, h[1], &D3DXVECTOR4( w1,w2,w3,w4 ));
                if (h[2]) pCT->SetVector( lpDevice, h[2], &D3DXVECTOR4( d1,d2,d3,d4 ));
                if (h[3]) pCT->SetVector( lpDevice, h[3], &D3DXVECTOR4( fscale_now,fbias_now,w_div,0));
            }
            else
            {
                // pass 2 (short vertical pass)
                const float w1 = w[0]+w[1] + w[2]+w[3];
                const float w2 = w[4]+w[5] + w[6]+w[7];
                const float d1 = (w1 > 1e-6f) ? (0 + 2*((w[2]+w[3])/w1)) : 0;
                const float d2 = (w2 > 1e-6f) ? (2 + 2*((w[6]+w[7])/w2)) : 2;
                const float w_sum = (w1+w2);
                const float w_div = (w_sum > 1e-6f) ? (1.0f/(w_sum*2)) : 0.0f;
                if (h[0]) pCT->SetVector( lpDevice, h[0], &srctexsize );
                if (h[5]) pCT->SetVector( lpDevice, h[5], &D3DXVECTOR4( w1,w2,d1,d2 ));
                if (h[6])
                {
                    if (i==1) // Only darken edges on the very first vertical pass
                        pCT->SetVector( lpDevice, h[6], &D3DXVECTOR4( w_div,(1-edge_darken),edge_darken,5.0f ));
                    else
                        pCT->SetVector( lpDevice, h[6], &D3DXVECTOR4( w_div,1.0f,0.0f,5.0f ));
                }
            }


            // Draw fullscreen quad (renders to m_lpBlur[i])
            // Use DrawPrimitiveUP with TRIANGLESTRIP
            lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, v, sizeof(MYVERTEX));

            // Clear texture binding for the next pass
            lpDevice->SetTexture(0, NULL);
        }

        // Restore original render target (back buffer)
        if (pOldRT) {
            lpDevice->SetRenderTarget(0, pOldRT);
            pOldRT->Release();
        }

        // Restore default shaders and FVF
        lpDevice->SetPixelShader( NULL ); // Or restore previous if needed
        lpDevice->SetVertexShader( NULL ); // Or restore previous if needed
        // lpDevice->SetFVF( MYVERTEX_FORMAT ); // Restore FVF if changed

    #endif // (NUM_BLUR_TEX > 0)

    m_nHighestBlurTexUsedThisFrame = 0; // Reset for next frame regardless
}


void CPlugin::ComputeGridAlphaValues()
{
    float fBlend = m_pState->m_fBlendProgress;//max(0,min(1,(m_pState->m_fBlendProgress*1.6f - 0.3f)));
    /*switch(code) //if (nPassOverride==0)
    {
    //case 8:
    //case 9:
    //case 12:
    //case 13:
        // note - these are the 4 cases where the old preset uses a warp shader, but new preset doesn't.
        fBlend = 1-fBlend;  // <-- THIS IS THE KEY - FLIPS THE ALPHAS AND EVERYTHING ELSE JUST WORKS.
        break;
    }*/
    //fBlend = 1-fBlend;  // <-- THIS IS THE KEY - FLIPS THE ALPHAS AND EVERYTHING ELSE JUST WORKS.
    bool bBlending = m_pState->m_bBlending;//(fBlend >= 0.0001f && fBlend <= 0.9999f);

	// warp stuff
	float fWarpTime = GetTime() * m_pState->m_fWarpAnimSpeed;
	float fWarpScaleInv = 1.0f / m_pState->m_fWarpScale.eval(GetTime());
	float f[4];
	f[0] = 11.68f + 4.0f*cosf(fWarpTime*1.413f + 10);
	f[1] =  8.77f + 3.0f*cosf(fWarpTime*1.113f + 7);
	f[2] = 10.54f + 3.0f*cosf(fWarpTime*1.233f + 3);
	f[3] = 11.49f + 4.0f*cosf(fWarpTime*0.933f + 5);

	// texel alignment
	float texel_offset_x = 0.5f / (float)m_nTexSizeX;
	float texel_offset_y = 0.5f / (float)m_nTexSizeY;

    int num_reps = (m_pState->m_bBlending) ? 2 : 1;
    int start_rep = 0;

    // FIRST WE HAVE 1-2 PASSES FOR CRUNCHING THE PER-VERTEX EQUATIONS
    for (int rep=start_rep; rep<num_reps; rep++)
	{
        // to blend the two PV equations together, we simulate both to get the final UV coords,
        // then we blend those final UV coords.  We also write out an alpha value so that
        // the second DRAW pass below (which might use a different shader) can do blending.
		CState *pState;

		if (rep==0)
			pState = m_pState;
		else
			pState = m_pOldState;

		// cache the doubles as floats so that computations are a bit faster
		float fZoom		= (float)(*pState->var_pf_zoom);
		float fZoomExp	= (float)(*pState->var_pf_zoomexp);
		float fRot		= (float)(*pState->var_pf_rot);
		float fWarp		= (float)(*pState->var_pf_warp);
		float fCX		= (float)(*pState->var_pf_cx);
		float fCY		= (float)(*pState->var_pf_cy);
		float fDX		= (float)(*pState->var_pf_dx);
		float fDY		= (float)(*pState->var_pf_dy);
		float fSX		= (float)(*pState->var_pf_sx);
		float fSY		= (float)(*pState->var_pf_sy);
		
		int n = 0;

		for (int y=0; y<=m_nGridY; y++)
		{
			for (int x=0; x<=m_nGridX; x++)
			{
				// Note: x, y, z are now set at init. time - no need to mess with them!
				//m_verts[n].x = i/(float)m_nGridX*2.0f - 1.0f;
				//m_verts[n].y = j/(float)m_nGridY*2.0f - 1.0f;
				//m_verts[n].z = 0.0f;
				
				if (pState->m_pp_codehandle)
				{
					// restore all the variables to their original states,
					//  run the user-defined equations,
					//  then move the results into local vars for computation as floats

					*pState->var_pv_x		= (double)(m_verts[n].x* 0.5f*m_fAspectX + 0.5f);
					*pState->var_pv_y		= (double)(m_verts[n].y*-0.5f*m_fAspectY + 0.5f);
					*pState->var_pv_rad		= (double)m_vertinfo[n].rad;
					*pState->var_pv_ang		= (double)m_vertinfo[n].ang;
					*pState->var_pv_zoom	= *pState->var_pf_zoom;
					*pState->var_pv_zoomexp	= *pState->var_pf_zoomexp;
					*pState->var_pv_rot		= *pState->var_pf_rot;
					*pState->var_pv_warp	= *pState->var_pf_warp;
					*pState->var_pv_cx		= *pState->var_pf_cx;
					*pState->var_pv_cy		= *pState->var_pf_cy;
					*pState->var_pv_dx		= *pState->var_pf_dx;
					*pState->var_pv_dy		= *pState->var_pf_dy;
					*pState->var_pv_sx		= *pState->var_pf_sx;
					*pState->var_pv_sy		= *pState->var_pf_sy;
					//*pState->var_pv_time		= *pState->var_pv_time;		// (these are all now initialized 
					//*pState->var_pv_bass		= *pState->var_pv_bass;		//  just once per frame)
					//*pState->var_pv_mid		= *pState->var_pv_mid;		
					//*pState->var_pv_treb		= *pState->var_pv_treb;	
					//*pState->var_pv_bass_att	= *pState->var_pv_bass_att;
					//*pState->var_pv_mid_att	= *pState->var_pv_mid_att;	
					//*pState->var_pv_treb_att	= *pState->var_pv_treb_att;

#ifndef _NO_EXPR_
					NSEEL_code_execute(pState->m_pp_codehandle);
#endif

					fZoom = (float)(*pState->var_pv_zoom);
					fZoomExp = (float)(*pState->var_pv_zoomexp);
					fRot  = (float)(*pState->var_pv_rot);
					fWarp = (float)(*pState->var_pv_warp);
					fCX   = (float)(*pState->var_pv_cx);
					fCY   = (float)(*pState->var_pv_cy);
					fDX   = (float)(*pState->var_pv_dx);
					fDY   = (float)(*pState->var_pv_dy);
					fSX   = (float)(*pState->var_pv_sx);
					fSY   = (float)(*pState->var_pv_sy);
				}

				float fZoom2 = powf(fZoom, powf(fZoomExp, m_vertinfo[n].rad*2.0f - 1.0f));

				// initial texcoords, w/built-in zoom factor
				float fZoom2Inv = 1.0f/fZoom2;
				float u =  m_verts[n].x*m_fAspectX*0.5f*fZoom2Inv + 0.5f;
				float v = -m_verts[n].y*m_fAspectY*0.5f*fZoom2Inv + 0.5f;
                    //float u_orig = u;
                    //float v_orig = v;
                    //m_verts[n].tr = u_orig + texel_offset_x;
                    //m_verts[n].ts = v_orig + texel_offset_y;

				// stretch on X, Y:
				u = (u - fCX)/fSX + fCX;
				v = (v - fCY)/fSY + fCY;

				// warping:
				//if (fWarp > 0.001f || fWarp < -0.001f)
				//{
					u += fWarp*0.0035f*sinf(fWarpTime*0.333f + fWarpScaleInv*(m_verts[n].x*f[0] - m_verts[n].y*f[3]));
					v += fWarp*0.0035f*cosf(fWarpTime*0.375f - fWarpScaleInv*(m_verts[n].x*f[2] + m_verts[n].y*f[1]));
					u += fWarp*0.0035f*cosf(fWarpTime*0.753f - fWarpScaleInv*(m_verts[n].x*f[1] - m_verts[n].y*f[2]));
					v += fWarp*0.0035f*sinf(fWarpTime*0.825f + fWarpScaleInv*(m_verts[n].x*f[0] + m_verts[n].y*f[3]));
				//}

				// rotation:
				float u2 = u - fCX;
				float v2 = v - fCY;
				
				float cos_rot = cosf(fRot);
				float sin_rot = sinf(fRot);
				u = u2*cos_rot - v2*sin_rot + fCX;
				v = u2*sin_rot + v2*cos_rot + fCY;

				// translation:
				u -= fDX;
				v -= fDY;

                // undo aspect ratio fix:
                u = (u-0.5f)*m_fInvAspectX + 0.5f;
                v = (v-0.5f)*m_fInvAspectY + 0.5f;

				// final half-texel-offset translation:
				u += texel_offset_x;
				v += texel_offset_y;
                
                if (rep==0)
				{
                    // UV's for m_pState
					m_verts[n].tu = u;
					m_verts[n].tv = v;
					m_verts[n].Diffuse = 0xFFFFFFFF;		
				}
				else
				{
                    // blend to UV's for m_pOldState
                    float mix2 = m_vertinfo[n].a*fBlend + m_vertinfo[n].c;//fCosineBlend2;
                    mix2 = max(0,min(1,mix2));   
                    //     if fBlend un-flipped, then mix2 is 0 at the beginning of a blend, 1 at the end...
                    //                           and alphas are 0 at the beginning, 1 at the end.
					m_verts[n].tu = m_verts[n].tu*(mix2) + u*(1-mix2);
					m_verts[n].tv = m_verts[n].tv*(mix2) + v*(1-mix2);
                    // this sets the alpha values for blending between two presets:
					m_verts[n].Diffuse = 0x00FFFFFF | (((DWORD)(mix2*255))<<24);		
				}

				n++;
			}
		}

	}
}

void CPlugin::WarpedBlit_NoShaders(int nPass, bool bAlphaBlend, bool bFlipAlpha, bool bCullTiles, bool bFlipCulling)
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    // If preset is invalid, just clear to black and exit.
    if (!wcscmp(m_pState->m_szDesc, INVALID_PRESET_DESC))
    {
        lpDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
        return;
    }

    // --- Setup for Warped Blit ---
    lpDevice->SetTexture(0, m_lpPrevFrame); // Read previous frame for feedback/warp source
    lpDevice->SetVertexShader( g_pVertexShader_Legacy2 ); // Assumes this handles vertex warp
    lpDevice->SetPixelShader( g_pPixelShader_Legacy2 );   // Basic texture sampling
    lpDevice->SetFVF( MYVERTEX_FORMAT );

    // Set sampler states (filtering, wrapping)
    DWORD texaddr = (*m_pState->var_pf_wrap > m_fSnapPoint) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
    lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, texaddr);
    lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, texaddr);
    lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, texaddr);
    lpDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    lpDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    lpDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR); // Or NONE if not using mipmaps

    // --- Apply Decay ---
    float fDecay = (float)(*m_pState->var_pf_decay);
    // Adjust decay for 16-bit targets if necessary (code omitted for brevity, but was present)
    // Clamp decay to avoid full black/white unless intended
    fDecay = max(0.0f, min(1.0f, fDecay));
    D3DCOLOR cDecay = D3DCOLOR_RGBA_01(fDecay, fDecay, fDecay, 1.0f); // Base color for modulation

    // --- Setup Blend States for Warp Pass ---
    if (bAlphaBlend)
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        if (bFlipAlpha)
        {
            lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_INVSRCALPHA);
            lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
        }
        else
        {
            lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
            lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
    }
    else
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    // --- Draw Warped Mesh ---
    int nAlphaTestValue = bFlipCulling ? 1 : 0;
    MYVERTEX tempv[1024 * 3]; // Consider making this dynamic or checking against max verts
    int max_verts_in_batch = sizeof(tempv)/sizeof(tempv[0]);
    int max_prims_per_batch = min( GetCaps()->MaxPrimitiveCount, max_verts_in_batch / 3) - 4; // -4 for safety margin
    if (max_prims_per_batch <= 0) max_prims_per_batch = 1; // Ensure at least 1 primitive can be drawn

    int primCount = m_nGridX * m_nGridY * 2;
    int src_idx = 0;
    while (src_idx < primCount * 3)
    {
        int prims_queued = 0;
        int verts_queued = 0; // Track vertices added to tempv
        while (prims_queued < max_prims_per_batch && src_idx < primCount * 3)
        {
            // Check if adding 3 more verts exceeds buffer
            if (verts_queued + 3 > max_verts_in_batch) break;

            // Store starting index in case we need to remove the triangle
            int current_vert_idx = verts_queued;

            // Copy 3 verts for a triangle
            for (int j = 0; j < 3; j++)
            {
                int mesh_vert_index = m_indices_list[src_idx++];
                // Basic bounds check for safety, though indices should be valid
                if (mesh_vert_index < 0 || mesh_vert_index >= (m_nGridX + 1) * (m_nGridY + 1)) {
                     // Handle error: skip triangle, log, assert?
                     src_idx -= j; // Backtrack index
                     verts_queued = current_vert_idx; // Reset vertex count for this attempt
                     goto next_triangle_attempt; // Skip rest of loop for this triangle
                }

                tempv[verts_queued] = m_verts[mesh_vert_index];
                // Apply decay color (modulates texture), keep blend alpha from vertex
                tempv[verts_queued].Diffuse = (cDecay & 0x00FFFFFF) | (tempv[verts_queued].Diffuse & 0xFF000000);
                // Flip Y for D3D coordinate system if mesh wasn't already set up this way
                // tempv[verts_queued].y *= -1.0f; // Assuming mesh Y is already -1 to 1 based on ComputeGridAlphaValues
                verts_queued++;
            }

            // Cull triangle if blending and alpha is zero/one depending on flip
            if (bCullTiles)
            {
                DWORD d1 = (tempv[verts_queued - 3].Diffuse >> 24);
                DWORD d2 = (tempv[verts_queued - 2].Diffuse >> 24);
                DWORD d3 = (tempv[verts_queued - 1].Diffuse >> 24);
                bool bIsNeeded;
                if (nAlphaTestValue) // Cull if alpha is 255 (flipped)
                    bIsNeeded = ((d1 & d2 & d3) < 255);
                else // Cull if alpha is 0
                    bIsNeeded = ((d1 | d2 | d3) > 0);

                if (!bIsNeeded) {
                    verts_queued -= 3; // Remove triangle vertices from buffer
                } else {
                    prims_queued++;
                }
            }
            else
            {
                prims_queued++;
            }
            next_triangle_attempt:; // Label for goto
        }
        if (prims_queued > 0)
            lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, prims_queued, tempv, sizeof(MYVERTEX));
    }

    // --- Apply Post-Warp Effects (Integrated from ShowToUser_NoShaders) ---
    // Reset blend state after main warp draw
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    // Prepare a full-screen quad for post-processing effects
    SPRITEVERTEX vPost[4];
    // Setup vPost vertices for a full-screen quad (-1 to 1) with UVs (0 to 1)
    vPost[0].x = -1.0f; vPost[0].y =  1.0f; vPost[0].z = 0.0f; vPost[0].tu = 0.0f; vPost[0].tv = 0.0f; vPost[0].Diffuse = 0xFFFFFFFF;
    vPost[1].x =  1.0f; vPost[1].y =  1.0f; vPost[1].z = 0.0f; vPost[1].tu = 1.0f; vPost[1].tv = 0.0f; vPost[1].Diffuse = 0xFFFFFFFF;
    vPost[2].x = -1.0f; vPost[2].y = -1.0f; vPost[2].z = 0.0f; vPost[2].tu = 0.0f; vPost[2].tv = 1.0f; vPost[2].Diffuse = 0xFFFFFFFF;
    vPost[3].x =  1.0f; vPost[3].y = -1.0f; vPost[3].z = 0.0f; vPost[3].tu = 1.0f; vPost[3].tv = 1.0f; vPost[3].Diffuse = 0xFFFFFFFF;

    // Set FVF for post-processing quad
    lpDevice->SetFVF( SPRITEVERTEX_FORMAT );
    // Use simple pass-through shaders for post-processing via blend states
    lpDevice->SetVertexShader( g_pVertexShader_Legacy3 ); // Simple VS
    lpDevice->SetPixelShader( g_pPixelShader_Legacy3 );   // Simple PS (might just output diffuse color)


    // Apply effects using blend states. This requires careful ordering and
    // knowledge of available blend ops on Xbox 360.

    // --- Video Echo ---
    // This is difficult without render targets. A simple approach is to redraw
    // the *warped mesh* again with modified UVs and alpha blending.
    float fVideoEchoZoom        = (float)(*m_pState->var_pf_echo_zoom);
    float fVideoEchoAlpha       = (float)(*m_pState->var_pf_echo_alpha);
    int   nVideoEchoOrientation = (int)  (*m_pState->var_pf_echo_orient) % 4;

    // Handle blending transition for orientation change
    if (m_pState->m_bBlending &&
        m_pState->m_fVideoEchoAlpha.eval(GetTime()) > 0.01f && // Check current time eval
        m_pOldState->m_fVideoEchoAlpha.eval(-1) > 0.01f && // Check old state base value
        m_pState->m_nVideoEchoOrientation != m_pOldState->m_nVideoEchoOrientation)
    {
        if (m_pState->m_fBlendProgress < m_fSnapPoint)
        {
            nVideoEchoOrientation = m_pOldState->m_nVideoEchoOrientation;
            // Fade out the old orientation's echo
            fVideoEchoAlpha *= (1.0f - m_pState->m_fBlendProgress / m_fSnapPoint);
        }
        else
        {
             // Fade in the new orientation's echo
             fVideoEchoAlpha *= (m_pState->m_fBlendProgress - m_fSnapPoint) / (1.0f - m_fSnapPoint);
        }
    }
    fVideoEchoAlpha = max(0.0f, min(1.0f, fVideoEchoAlpha)); // Clamp alpha


    if (fVideoEchoAlpha > 0.001f && fVideoEchoZoom != 1.0f) // Only draw if echo is visible and zoom != 1
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        // Additive blending for echo is common, but alpha blend might be desired
        // lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        // lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

        // Need to redraw the warped mesh (m_verts) with modified UVs based on echo zoom/orientation
        // This requires recalculating UVs similar to ComputeGridAlphaValues but applying echo transform
        // For simplicity here, we'll skip the complex redraw. A full implementation
        // would iterate through m_verts, calculate echo UVs, apply echo alpha, and draw.
        // Placeholder: Draw a simple blended quad as a visual cue (won't look like real echo)
        /*
        D3DCOLOR echoColor = D3DCOLOR_RGBA_01(fVideoEchoAlpha, fVideoEchoAlpha, fVideoEchoAlpha, fVideoEchoAlpha);
        vPost[0].Diffuse = vPost[1].Diffuse = vPost[2].Diffuse = vPost[3].Diffuse = echoColor;
        lpDevice->SetTexture(0, m_lpPrevFrame); // Echo samples previous frame
        // Setup UVs based on zoom/orientation - complex part omitted
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        */
        // NOTE: Proper video echo without RTs is non-trivial. Consider disabling or simplifying.
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); // Reset blend state
    }


    // --- Gamma Correction ---
    float fGammaAdj = (float)(*m_pState->var_pf_gamma);
    fGammaAdj = max(1.0f, fGammaAdj); // Gamma adjustment usually starts from 1.0

    if (fGammaAdj > 1.001f) // Only apply if gamma > 1
    {
        int nPasses = (int)(fGammaAdj - 1.0f); // Number of full additive passes
        float gamma_remainder = fGammaAdj - 1.0f - nPasses; // Fractional part for final pass

        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE); // Additive blend
        lpDevice->SetTexture(0, NULL); // Don't sample texture for gamma pass

        // Draw full brightness quads additively
        vPost[0].Diffuse = vPost[1].Diffuse = vPost[2].Diffuse = vPost[3].Diffuse = 0xFFFFFFFF; // White, full alpha
        for (int nPass = 0; nPass < nPasses; nPass++)
        {
            lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        }

        // Draw final fractional gamma pass
        if (gamma_remainder > 0.001f)
        {
            BYTE intensity = (BYTE)(gamma_remainder * 255.0f);
            D3DCOLOR gammaColor = D3DCOLOR_RGBA(intensity, intensity, intensity, intensity); // Use intensity for alpha too? Or 255?
            vPost[0].Diffuse = vPost[1].Diffuse = vPost[2].Diffuse = vPost[3].Diffuse = gammaColor;
            lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        }
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }


    // --- Brighten/Darken/Invert/Solarize ---
    // These require specific D3DBLEND ops. Check Xbox 360 caps.
    lpDevice->SetTexture(0, NULL); // Effects applied via blend ops, no texture needed

    // Invert
    if (*m_pState->var_pf_invert > m_fSnapPoint && (GetCaps()->SrcBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR))
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR); // Source = 1 - DestColor
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);       // Dest = Source * SrcBlend = 1 - DestColor
        vPost[0].Diffuse = vPost[1].Diffuse = vPost[2].Diffuse = vPost[3].Diffuse = 0xFFFFFFFF; // Opaque white quad (doesn't matter)
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    // Darken (Modulate by self)
    if (*m_pState->var_pf_darken > m_fSnapPoint && (GetCaps()->DestBlendCaps & D3DPBLENDCAPS_DESTCOLOR))
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);        // Source = 0
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);  // Dest = Dest * DestBlend = DestColor * DestColor
        vPost[0].Diffuse = vPost[1].Diffuse = vPost[2].Diffuse = vPost[3].Diffuse = 0xFFFFFFFF; // Doesn't matter
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    // Brighten (Screen blend: 1 - (1-Dest)*(1-Dest)) - Approximated by additive blend?
    // True screen blend isn't directly available. Additive is a common substitute.
    if (*m_pState->var_pf_brighten > m_fSnapPoint)
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);       // Source = 1
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);      // Dest = Dest + Source = Dest + 1 (Clamped)
        // This brightens but doesn't exactly match screen blend.
        // A better approximation might involve multiple passes or shader logic.
        vPost[0].Diffuse = vPost[1].Diffuse = vPost[2].Diffuse = vPost[3].Diffuse = D3DCOLOR_RGBA(64,64,64,64); // Add a small amount?
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }


    // Solarize (Complex blend, often requires multiple passes or shader)
    // Example: Invert -> Darken -> Invert might approximate some solarize effects.
    if (*m_pState->var_pf_solarize > m_fSnapPoint &&
        (GetCaps()->SrcBlendCaps & D3DPBLENDCAPS_INVDESTCOLOR) &&
        (GetCaps()->DestBlendCaps & D3DPBLENDCAPS_DESTCOLOR))
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        // Invert
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        // Darken (Square the inverted result)
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));
        // Invert back
        lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVDESTCOLOR);
        lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vPost, sizeof(SPRITEVERTEX));

        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }


    // --- Cleanup ---
    lpDevice->SetTexture(0, NULL); // Unbind texture
    // Restore default blend state just in case
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CPlugin::WarpedBlit_Shaders(int nPass, bool bAlphaBlend, bool bFlipAlpha, bool bCullTiles, bool bFlipCulling)
{
    // if nPass==0, it draws old preset (blending 1 of 2).
    // if nPass==1, it draws new preset (blending 2 of 2, OR done blending)

    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    // If preset is invalid, just clear to black and exit.
    if (!wcscmp(m_pState->m_szDesc, INVALID_PRESET_DESC))
    {
        lpDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0);
        return;
    }

    // --- Setup Shaders and FVF ---
    // The vertex shader (m_fallbackShaders_vs.warp.ptr) likely handles vertex positions
    // based on the mesh grid. UVs for warping are calculated in ComputeGridAlphaValues
    // and passed via the MYVERTEX structure.
    // The pixel shader (si->ptr) handles reading m_lpPrevFrame (via TEX_VS in ApplyShaderParams),
    // applying decay, gamma, echo, etc., and outputting the final color.

    PShaderInfo* si = (nPass == 0) ? &m_OldShaders.warp : &m_shaders.warp;
    CState* state = (nPass == 0) ? m_pOldState : m_pState;

    // Check if shader pointers are valid
    if (!m_fallbackShaders_vs.warp.ptr || !si->ptr) {
        // Fallback or error handling if shaders are missing
        // For now, maybe call the NoShaders version?
        WarpedBlit_NoShaders(nPass, bAlphaBlend, bFlipAlpha, bCullTiles, bFlipCulling);
        return;
    }

    lpDevice->SetVertexDeclaration(m_pMyVertDecl); // Ensure correct vertex declaration
    lpDevice->SetVertexShader(m_fallbackShaders_vs.warp.ptr);
    lpDevice->SetPixelShader(si->ptr);

    // Apply shader constants and textures (including m_lpPrevFrame via TEX_VS)
    ApplyShaderParams(&(si->params), si->CT, state);

    // --- Setup Blend States ---
    int nAlphaTestValue = bFlipCulling ? 1 : 0;
    if (bAlphaBlend)
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        if (bFlipAlpha)
        {
            lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_INVSRCALPHA);
            lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);
        }
        else
        {
            lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
            lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        }
    }
    else
    {
        lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    }

    // --- Draw Warped Mesh ---
    // Hurl the triangles at the video card. Un-indexed drawing.
    MYVERTEX tempv[1024 * 3]; // Consider dynamic allocation or checking size
    int max_verts_in_batch = sizeof(tempv)/sizeof(tempv[0]);
    int max_prims_per_batch = min( GetCaps()->MaxPrimitiveCount, max_verts_in_batch / 3) - 4; // Safety margin
    if (max_prims_per_batch <= 0) max_prims_per_batch = 1;

    // The original code split drawing into two halves for an 'ang' hack.
    // If that hack is still needed, keep the loop. Otherwise, simplify.
    // Assuming the hack might still be relevant for some shaders:
    for (int half = 0; half < 2; half++)
    {
        // Hack / restore the ang values along the angle-wrap [0 <-> 2pi] seam...
        // This modifies the global m_verts array, which might be unexpected.
        // Consider passing angle differently if possible.
        float new_ang = half ? 3.14159265358979323846f : -3.14159265358979323846f;
        int y_offset = (m_nGridY / 2) * (m_nGridX + 1);
        for (int x = 0; x < m_nGridX / 2; x++) {
             int vert_idx = y_offset + x;
             // Bounds check
             if (vert_idx >= 0 && vert_idx < (m_nGridX + 1) * (m_nGridY + 1)) {
                 m_verts[vert_idx].ang = new_ang; // Modifying m_verts directly
             }
        }


        // Draw half of the polys
        int primCount = m_nGridX * m_nGridY; // Total triangles in half = gridX * gridY
        int src_idx_offset = half * primCount * 3; // Start index for this half
        int src_idx = 0; // Index within this half

        while (src_idx < primCount * 3)
        {
            int prims_queued = 0;
            int verts_queued = 0;
            while (prims_queued < max_prims_per_batch && src_idx < primCount * 3)
            {
                 // Check if adding 3 more verts exceeds buffer
                 if (verts_queued + 3 > max_verts_in_batch) break;

                 int current_vert_idx = verts_queued;

                 // Copy 3 verts
                 for (int j = 0; j < 3; j++) {
                     int mesh_vert_index = m_indices_list[src_idx_offset + src_idx++];
                     // Basic bounds check
                     if (mesh_vert_index < 0 || mesh_vert_index >= (m_nGridX + 1) * (m_nGridY + 1)) {
                          src_idx -= j; // Backtrack
                          verts_queued = current_vert_idx;
                          goto next_shader_triangle_attempt;
                     }
                     tempv[verts_queued++] = m_verts[mesh_vert_index];
                     // Note: Decay is now handled inside the pixel shader, not via vertex color modulation.
                 }

                 if (bCullTiles)
                 {
                     DWORD d1 = (tempv[verts_queued - 3].Diffuse >> 24);
                     DWORD d2 = (tempv[verts_queued - 2].Diffuse >> 24);
                     DWORD d3 = (tempv[verts_queued - 1].Diffuse >> 24);
                     bool bIsNeeded;
                     if (nAlphaTestValue)
                         bIsNeeded = ((d1 & d2 & d3) < 255);
                     else
                         bIsNeeded = ((d1 | d2 | d3) > 0);

                     if (!bIsNeeded) {
                         verts_queued -= 3;
                     } else {
                         prims_queued++;
                     }
                 }
                 else {
                     prims_queued++;
                 }
                 next_shader_triangle_attempt:;
            }
            if (prims_queued > 0) {
                // Ensure textures are set correctly before draw (ApplyShaderParams should handle this)
                lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, prims_queued, tempv, sizeof(MYVERTEX));
            }
        }
    }

    // --- Cleanup ---
    lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); // Reset blend state
    RestoreShaderParams(); // Unbind textures and restore sampler/shader states
}

void CPlugin::DrawCustomShapes()
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

	int num_reps = (m_pState->m_bBlending) ? 2 : 1;
	for (int rep=0; rep<num_reps; rep++)
	{
        CState *pState = (rep==0) ? m_pState : m_pOldState;
        float alpha_mult = 1;
        if (num_reps==2)
            alpha_mult = (rep==0) ? m_pState->m_fBlendProgress : (1-m_pState->m_fBlendProgress);

        for (int i=0; i<MAX_CUSTOM_SHAPES; i++)
        {
            if (pState->m_shape[i].enabled)
            {
                for (int instance=0; instance<pState->m_shape[i].instances; instance++)
                {
                    // 1. execute per-frame code
                    LoadCustomShapePerFrameEvallibVars(pState, i, instance);

			        #ifndef _NO_EXPR_
				        if (pState->m_shape[i].m_pf_codehandle)
				        {
					        NSEEL_code_execute(pState->m_shape[i].m_pf_codehandle);
				        }
			        #endif

                    // save changes to t1-t8 this frame
                    /*
		            pState->m_shape[i].t_values_after_init_code[0] = *pState->m_shape[i].var_pf_t1;
		            pState->m_shape[i].t_values_after_init_code[1] = *pState->m_shape[i].var_pf_t2;
		            pState->m_shape[i].t_values_after_init_code[2] = *pState->m_shape[i].var_pf_t3;
		            pState->m_shape[i].t_values_after_init_code[3] = *pState->m_shape[i].var_pf_t4;
		            pState->m_shape[i].t_values_after_init_code[4] = *pState->m_shape[i].var_pf_t5;
		            pState->m_shape[i].t_values_after_init_code[5] = *pState->m_shape[i].var_pf_t6;
		            pState->m_shape[i].t_values_after_init_code[6] = *pState->m_shape[i].var_pf_t7;
		            pState->m_shape[i].t_values_after_init_code[7] = *pState->m_shape[i].var_pf_t8;
                    */

                    int sides = (int)(*pState->m_shape[i].var_pf_sides);
                    if (sides<3) sides=3;
                    if (sides>100) sides=100;

	                lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                    lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
                    lpDevice->SetRenderState(D3DRS_DESTBLEND, ((int)(*pState->m_shape[i].var_pf_additive) != 0) ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);

                    SPRITEVERTEX v[512];  // for textured shapes (has texcoords)
                    WFVERTEX v2[512];     // for untextured shapes + borders

                    v[0].x = (float)(*pState->m_shape[i].var_pf_x* 2-1);// * ASPECT;
                    v[0].y = (float)(*pState->m_shape[i].var_pf_y*-2+1);
                    v[0].z = 0;
                    v[0].tu = 0.5f;
                    v[0].tv = 0.5f;
                    v[0].Diffuse = 
                        ((((int)(*pState->m_shape[i].var_pf_a * 255 * alpha_mult)) & 0xFF) << 24) |
                        ((((int)(*pState->m_shape[i].var_pf_r * 255)) & 0xFF) << 16) |
                        ((((int)(*pState->m_shape[i].var_pf_g * 255)) & 0xFF) <<  8) |
                        ((((int)(*pState->m_shape[i].var_pf_b * 255)) & 0xFF)      );
                    v[1].Diffuse = 
                        ((((int)(*pState->m_shape[i].var_pf_a2 * 255 * alpha_mult)) & 0xFF) << 24) |
                        ((((int)(*pState->m_shape[i].var_pf_r2 * 255)) & 0xFF) << 16) |
                        ((((int)(*pState->m_shape[i].var_pf_g2 * 255)) & 0xFF) <<  8) |
                        ((((int)(*pState->m_shape[i].var_pf_b2 * 255)) & 0xFF)      );

                    for (int j=1; j<sides+1; j++)
                    {
                        float t = (j-1)/(float)sides;
                        v[j].x = v[0].x + (float)*pState->m_shape[i].var_pf_rad*cosf(t*3.1415927f*2 + (float)*pState->m_shape[i].var_pf_ang + 3.1415927f*0.25f)*m_fAspectY;  // DON'T TOUCH!
                        v[j].y = v[0].y + (float)*pState->m_shape[i].var_pf_rad*sinf(t*3.1415927f*2 + (float)*pState->m_shape[i].var_pf_ang + 3.1415927f*0.25f);           // DON'T TOUCH!
                        v[j].z = 0;
                        v[j].tu = 0.5f + 0.5f*cosf(t*3.1415927f*2 + (float)*pState->m_shape[i].var_pf_tex_ang + 3.1415927f*0.25f)/((float)*pState->m_shape[i].var_pf_tex_zoom) * m_fAspectY; // DON'T TOUCH!
                        v[j].tv = 0.5f + 0.5f*sinf(t*3.1415927f*2 + (float)*pState->m_shape[i].var_pf_tex_ang + 3.1415927f*0.25f)/((float)*pState->m_shape[i].var_pf_tex_zoom);     // DON'T TOUCH!
                        v[j].Diffuse = v[1].Diffuse;
                    }
                    v[sides+1] = v[1];

                    if ((int)(*pState->m_shape[i].var_pf_textured) != 0)
                    {
                        // draw textured version
                        lpDevice->SetTexture(0, NULL);
//						lpDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
                        lpDevice->SetVertexShader( g_pVertexShader_Legacy3 );
						lpDevice->SetPixelShader(g_pPixelShader_Legacy3);
                        lpDevice->SetFVF( SPRITEVERTEX_FORMAT );
                        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, sides, (void*)v, sizeof(SPRITEVERTEX));
                    }
                    else
                    {
                        // no texture
                        for (int j=0; j < sides+2; j++)
                        {
                            v2[j].x       = v[j].x;
                            v2[j].y       = v[j].y;
                            v2[j].z       = v[j].z;
                            v2[j].Diffuse = v[j].Diffuse;
                        }
                        lpDevice->SetTexture(0, NULL);
	//					lpDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
                        lpDevice->SetVertexShader( g_pVertexShader_Legacy1 );
						lpDevice->SetPixelShader(g_pPixelShader_Legacy1);
					    lpDevice->SetFVF( WFVERTEX_FORMAT );
                        lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, sides, (void*)v2, sizeof(WFVERTEX));
                    }

                    // DRAW BORDER
                    if (*pState->m_shape[i].var_pf_border_a > 0)
                    {
                       lpDevice->SetTexture(0, NULL);
	//					lpDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
                        lpDevice->SetVertexShader( g_pVertexShader_Legacy1 );
						lpDevice->SetPixelShader( g_pPixelShader_Legacy1 );
                        lpDevice->SetFVF( WFVERTEX_FORMAT );

                        v2[0].Diffuse = 
                            ((((int)(*pState->m_shape[i].var_pf_border_a * 255 * alpha_mult)) & 0xFF) << 24) |
                            ((((int)(*pState->m_shape[i].var_pf_border_r * 255)) & 0xFF) << 16) |
                            ((((int)(*pState->m_shape[i].var_pf_border_g * 255)) & 0xFF) <<  8) |
                            ((((int)(*pState->m_shape[i].var_pf_border_b * 255)) & 0xFF)      );
                        for (int j=0; j<sides+2; j++)
                        {
                            v2[j].x = v[j].x;
                            v2[j].y = v[j].y;
                            v2[j].z = v[j].z;
                            v2[j].Diffuse = v2[0].Diffuse;
                        }
                    
                        int its = ((int)(*pState->m_shape[i].var_pf_thick) != 0) ? 4 : 1;
		                float x_inc = 2.0f / (float)m_nTexSizeX;
		                float y_inc = 2.0f / (float)m_nTexSizeY;
                        for (int it=0; it<its; it++)
                        {
			                switch(it)
			                {
			                case 0: break;
			                case 1: for (int j=0; j<sides+2; j++) v2[j].x += x_inc; break;		// draw fat dots
			                case 2: for (int j=0; j<sides+2; j++) v2[j].y += y_inc; break;		// draw fat dots
			                case 3: for (int j=0; j<sides+2; j++) v2[j].x -= x_inc; break;		// draw fat dots
			                }
                            lpDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, sides, (void*)&v2[1], sizeof(WFVERTEX));
                        }
                    }

                    lpDevice->SetTexture(0, NULL);
                    lpDevice->SetVertexShader( NULL );
                    lpDevice->SetFVF( SPRITEVERTEX_FORMAT );
                }
            }
        }
    }

	lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CPlugin::LoadCustomShapePerFrameEvallibVars(CState* pState, int i, int instance)
{
	*pState->m_shape[i].var_pf_time		= (double)(GetTime() - m_fStartTime);
	*pState->m_shape[i].var_pf_frame	= (double)GetFrame();
	*pState->m_shape[i].var_pf_fps      = (double)GetFps();
	*pState->m_shape[i].var_pf_progress = (GetTime() - m_fPresetStartTime) / (m_fNextPresetTime - m_fPresetStartTime);
	*pState->m_shape[i].var_pf_bass		= (double)mysound.imm_rel[0];
	*pState->m_shape[i].var_pf_mid		= (double)mysound.imm_rel[1];
	*pState->m_shape[i].var_pf_treb		= (double)mysound.imm_rel[2];
	*pState->m_shape[i].var_pf_bass_att	= (double)mysound.avg_rel[0];
	*pState->m_shape[i].var_pf_mid_att	= (double)mysound.avg_rel[1];
	*pState->m_shape[i].var_pf_treb_att	= (double)mysound.avg_rel[2];
    for (int vi=0; vi<NUM_Q_VAR; vi++)
        *pState->m_shape[i].var_pf_q[vi] = *pState->var_pf_q[vi];
    for (int vi=0; vi<NUM_T_VAR; vi++)
	    *pState->m_shape[i].var_pf_t[vi] = pState->m_shape[i].t_values_after_init_code[vi];
	*pState->m_shape[i].var_pf_x        = pState->m_shape[i].x;
	*pState->m_shape[i].var_pf_y        = pState->m_shape[i].y;
	*pState->m_shape[i].var_pf_rad      = pState->m_shape[i].rad;
	*pState->m_shape[i].var_pf_ang      = pState->m_shape[i].ang;
	*pState->m_shape[i].var_pf_tex_zoom = pState->m_shape[i].tex_zoom;
	*pState->m_shape[i].var_pf_tex_ang  = pState->m_shape[i].tex_ang;
	*pState->m_shape[i].var_pf_sides    = pState->m_shape[i].sides;
    *pState->m_shape[i].var_pf_additive = pState->m_shape[i].additive;
    *pState->m_shape[i].var_pf_textured = pState->m_shape[i].textured;
    *pState->m_shape[i].var_pf_instances = pState->m_shape[i].instances;
    *pState->m_shape[i].var_pf_instance = instance;
    *pState->m_shape[i].var_pf_thick    = pState->m_shape[i].thickOutline;
	*pState->m_shape[i].var_pf_r        = pState->m_shape[i].r;
	*pState->m_shape[i].var_pf_g        = pState->m_shape[i].g;
	*pState->m_shape[i].var_pf_b        = pState->m_shape[i].b;
	*pState->m_shape[i].var_pf_a        = pState->m_shape[i].a;
	*pState->m_shape[i].var_pf_r2       = pState->m_shape[i].r2;
	*pState->m_shape[i].var_pf_g2       = pState->m_shape[i].g2;
	*pState->m_shape[i].var_pf_b2       = pState->m_shape[i].b2;
	*pState->m_shape[i].var_pf_a2       = pState->m_shape[i].a2;
	*pState->m_shape[i].var_pf_border_r = pState->m_shape[i].border_r;
	*pState->m_shape[i].var_pf_border_g = pState->m_shape[i].border_g;
	*pState->m_shape[i].var_pf_border_b = pState->m_shape[i].border_b;
	*pState->m_shape[i].var_pf_border_a = pState->m_shape[i].border_a;
}

void CPlugin::LoadCustomWavePerFrameEvallibVars(CState* pState, int i)
{
	*pState->m_wave[i].var_pf_time		= (double)(GetTime() - m_fStartTime);
	*pState->m_wave[i].var_pf_frame		= (double)GetFrame();
	*pState->m_wave[i].var_pf_fps       = (double)GetFps();
	*pState->m_wave[i].var_pf_progress  = (GetTime() - m_fPresetStartTime) / (m_fNextPresetTime - m_fPresetStartTime);
	*pState->m_wave[i].var_pf_bass		= (double)mysound.imm_rel[0];
	*pState->m_wave[i].var_pf_mid		= (double)mysound.imm_rel[1];
	*pState->m_wave[i].var_pf_treb		= (double)mysound.imm_rel[2];
	*pState->m_wave[i].var_pf_bass_att	= (double)mysound.avg_rel[0];
	*pState->m_wave[i].var_pf_mid_att	= (double)mysound.avg_rel[1];
	*pState->m_wave[i].var_pf_treb_att	= (double)mysound.avg_rel[2];
    for (int vi=0; vi<NUM_Q_VAR; vi++)
	    *pState->m_wave[i].var_pf_q[vi] = *pState->var_pf_q[vi];
    for (int vi=0; vi<NUM_T_VAR; vi++)
	    *pState->m_wave[i].var_pf_t[vi] = pState->m_wave[i].t_values_after_init_code[vi];
	*pState->m_wave[i].var_pf_r         = pState->m_wave[i].r;
	*pState->m_wave[i].var_pf_g         = pState->m_wave[i].g;
	*pState->m_wave[i].var_pf_b         = pState->m_wave[i].b;
	*pState->m_wave[i].var_pf_a         = pState->m_wave[i].a;
    *pState->m_wave[i].var_pf_samples   = pState->m_wave[i].samples;
}

// does a better-than-linear smooth on a wave.  Roughly doubles the # of points.
int SmoothWave(WFVERTEX* vi, int nVertsIn, WFVERTEX* vo)
{
    const float c1 = -0.15f;
    const float c2 = 1.15f;
    const float c3 = 1.15f;
    const float c4 = -0.15f;
    const float inv_sum = 1.0f/(c1+c2+c3+c4);

    int j = 0;

    int i_below = 0;
    int i_above;
    int i_above2 = 1;
    for (int i=0; i<nVertsIn-1; i++)
    {
        i_above = i_above2;
        i_above2 = min(nVertsIn-1,i+2);
        vo[j] = vi[i];
        vo[j+1].x = (c1*vi[i_below].x + c2*vi[i].x + c3*vi[i_above].x + c4*vi[i_above2].x)*inv_sum;
        vo[j+1].y = (c1*vi[i_below].y + c2*vi[i].y + c3*vi[i_above].y + c4*vi[i_above2].y)*inv_sum;
        vo[j+1].z = 0;
        vo[j+1].Diffuse = vi[i].Diffuse;//0xFFFF0080;
        i_below = i;
        j += 2;
    }
    vo[j++] = vi[nVertsIn-1];

    return j;
}

void CPlugin::DrawCustomWaves()
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    lpDevice->SetTexture(0, m_lpBlur[1]); // Use your texture resource
    lpDevice->SetTexture(0, NULL);
    lpDevice->SetVertexShader( g_pVertexShader_Legacy3 );
	lpDevice->SetPixelShader(g_pPixelShader_Legacy3);
    lpDevice->SetFVF( WFVERTEX_FORMAT );

    // note: read in all sound data from CPluginShell's m_sound
	int num_reps = (m_pState->m_bBlending) ? 2 : 1;
	for (int rep=0; rep<num_reps; rep++)
	{
        CState *pState = (rep==0) ? m_pState : m_pOldState;
        float alpha_mult = 1;
        if (num_reps==2)
            alpha_mult = (rep==0) ? m_pState->m_fBlendProgress : (1-m_pState->m_fBlendProgress);

        for (int i=0; i<MAX_CUSTOM_WAVES; i++)
        {
            if (pState->m_wave[i].enabled)
            {
                int nSamples = pState->m_wave[i].samples;
                int max_samples = pState->m_wave[i].bSpectrum ? 512 : NUM_WAVEFORM_SAMPLES;
                if (nSamples > max_samples)
                    nSamples = max_samples;
                nSamples -= pState->m_wave[i].sep;

                // 1. execute per-frame code
                LoadCustomWavePerFrameEvallibVars(pState, i);

			    // 2.a. do just a once-per-frame init for the *per-point* *READ-ONLY* variables
			    //     (the non-read-only ones will be reset/restored at the start of each vertex)
			    *pState->m_wave[i].var_pp_time		= *pState->m_wave[i].var_pf_time;	
			    *pState->m_wave[i].var_pp_fps       = *pState->m_wave[i].var_pf_fps;
			    *pState->m_wave[i].var_pp_frame		= *pState->m_wave[i].var_pf_frame;
		        *pState->m_wave[i].var_pp_progress  = *pState->m_wave[i].var_pf_progress;
			    *pState->m_wave[i].var_pp_bass		= *pState->m_wave[i].var_pf_bass;	
			    *pState->m_wave[i].var_pp_mid		= *pState->m_wave[i].var_pf_mid;		
			    *pState->m_wave[i].var_pp_treb		= *pState->m_wave[i].var_pf_treb;	
			    *pState->m_wave[i].var_pp_bass_att	= *pState->m_wave[i].var_pf_bass_att;
			    *pState->m_wave[i].var_pp_mid_att	= *pState->m_wave[i].var_pf_mid_att;	
			    *pState->m_wave[i].var_pp_treb_att	= *pState->m_wave[i].var_pf_treb_att;

				NSEEL_code_execute(pState->m_wave[i].m_pf_codehandle);

                for (int vi=0; vi<NUM_Q_VAR; vi++)
                    *pState->m_wave[i].var_pp_q[vi] = *pState->m_wave[i].var_pf_q[vi];
                for (int vi=0; vi<NUM_T_VAR; vi++)
                    *pState->m_wave[i].var_pp_t[vi] = *pState->m_wave[i].var_pf_t[vi];

                nSamples = (int)*pState->m_wave[i].var_pf_samples;
                nSamples = min(512, nSamples);

                if ((nSamples >= 2) || (pState->m_wave[i].bUseDots && nSamples >= 1))
                {
                    int j;
                    float tempdata[2][512];
                    float mult = ((pState->m_wave[i].bSpectrum) ? 0.15f : 0.004f) * pState->m_wave[i].scaling * pState->m_fWaveScale.eval(-1);
                    float *pdata1 = (pState->m_wave[i].bSpectrum) ? m_sound.fSpectrum[0] : m_sound.fWaveform[0];
                    float *pdata2 = (pState->m_wave[i].bSpectrum) ? m_sound.fSpectrum[1] : m_sound.fWaveform[1];
                
                    // initialize tempdata[2][512]
                    int j0 = (pState->m_wave[i].bSpectrum) ? 0 : (max_samples - nSamples)/2/**(1-pState->m_wave[i].bSpectrum)*/ - pState->m_wave[i].sep/2;
                    int j1 = (pState->m_wave[i].bSpectrum) ? 0 : (max_samples - nSamples)/2/**(1-pState->m_wave[i].bSpectrum)*/ + pState->m_wave[i].sep/2;
                    float t = (pState->m_wave[i].bSpectrum) ? (max_samples - pState->m_wave[i].sep)/(float)nSamples : 1;
                    float mix1 = powf(pState->m_wave[i].smoothing*0.98f, 0.5f);  // lower exponent -> more default smoothing
                    float mix2 = 1-mix1;
                    // SMOOTHING:
                    tempdata[0][0] = pdata1[j0];
                    tempdata[1][0] = pdata2[j1];
                    for (j=1; j<nSamples; j++) 
                    {
                        tempdata[0][j] = pdata1[(int)(j*t)+j0]*mix2 + tempdata[0][j-1]*mix1;
                        tempdata[1][j] = pdata2[(int)(j*t)+j1]*mix2 + tempdata[1][j-1]*mix1;
                    }
                    // smooth again, backwards: [this fixes the asymmetry of the beginning & end..]
                    for (j=nSamples-2; j>=0; j--)
                    {
                        tempdata[0][j] = tempdata[0][j]*mix2 + tempdata[0][j+1]*mix1;
                        tempdata[1][j] = tempdata[1][j]*mix2 + tempdata[1][j+1]*mix1;
                    }
                    // finally, scale to final size:
                    for (j=0; j<nSamples; j++) 
                    {
                        tempdata[0][j] *= mult;
                        tempdata[1][j] *= mult;
                    }

                    // 2. for each point, execute per-point code


                    // to do:
                    //  -add any of the m_wave[i].xxx menu-accessible vars to the code?
                    WFVERTEX v[1024];
                    float j_mult = 1.0f/(float)(nSamples-1); 
                    for (j=0; j<nSamples; j++)
                    {
                        float t = j*j_mult;
                        float value1 = tempdata[0][j];
                        float value2 = tempdata[1][j];
                        *pState->m_wave[i].var_pp_sample = t;
                        *pState->m_wave[i].var_pp_value1 = value1;
                        *pState->m_wave[i].var_pp_value2 = value2;
                        *pState->m_wave[i].var_pp_x      = 0.5f + value1;
                        *pState->m_wave[i].var_pp_y      = 0.5f + value2;
                        *pState->m_wave[i].var_pp_r      = *pState->m_wave[i].var_pf_r;
                        *pState->m_wave[i].var_pp_g      = *pState->m_wave[i].var_pf_g;
                        *pState->m_wave[i].var_pp_b      = *pState->m_wave[i].var_pf_b;
                        *pState->m_wave[i].var_pp_a      = *pState->m_wave[i].var_pf_a;

                        #ifndef _NO_EXPR_
                            NSEEL_code_execute(pState->m_wave[i].m_pp_codehandle);
                        #endif

                        v[j].x = (float)(*pState->m_wave[i].var_pp_x* 2-1)*m_fInvAspectX;
                        v[j].y = (float)(*pState->m_wave[i].var_pp_y*-2+1)*m_fInvAspectY;
                        v[j].z = 0;
                        v[j].Diffuse = 
                            ((((int)(*pState->m_wave[i].var_pp_a * 255 * alpha_mult)) & 0xFF) << 24) |
                            ((((int)(*pState->m_wave[i].var_pp_r * 255)) & 0xFF) << 16) |
                            ((((int)(*pState->m_wave[i].var_pp_g * 255)) & 0xFF) <<  8) |
                            ((((int)(*pState->m_wave[i].var_pp_b * 255)) & 0xFF)      );
                    }

                    // 3. smooth it
                    WFVERTEX v2[2048];
                    WFVERTEX *pVerts = v;
                    if (!pState->m_wave[i].bUseDots) 
                    {
                        nSamples = SmoothWave(v, nSamples, v2);
                        pVerts = v2;
                    }

                    // 4. draw it
	                lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	                lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
                    lpDevice->SetRenderState(D3DRS_DESTBLEND, pState->m_wave[i].bAdditive ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
                
                    float ptsize = (float)((m_nTexSizeX >= 1024) ? 2 : 1) + (pState->m_wave[i].bDrawThick ? 1 : 0);
                    if (pState->m_wave[i].bUseDots)
                        lpDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&ptsize) ); 

                    int its = (pState->m_wave[i].bDrawThick && !pState->m_wave[i].bUseDots) ? 4 : 1;
		            float x_inc = 2.0f / (float)m_nTexSizeX;
		            float y_inc = 2.0f / (float)m_nTexSizeY;
                    for (int it=0; it<its; it++)
                    {
			            switch(it)
			            {
			            case 0: break;
			            case 1: for (j=0; j<nSamples; j++) pVerts[j].x += x_inc; break;		// draw fat dots
			            case 2: for (j=0; j<nSamples; j++) pVerts[j].y += y_inc; break;		// draw fat dots
			            case 3: for (j=0; j<nSamples; j++) pVerts[j].x -= x_inc; break;		// draw fat dots
			            }
                        lpDevice->DrawPrimitiveUP(pState->m_wave[i].bUseDots ? D3DPT_POINTLIST : D3DPT_LINESTRIP, nSamples - (pState->m_wave[i].bUseDots ? 0 : 1), (void*)pVerts, sizeof(WFVERTEX));
                    }

                    ptsize = 1.0f;
                    if (pState->m_wave[i].bUseDots)
                        lpDevice->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&ptsize) ); 
                }
            }
        }
    }

	lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

void CPlugin::DrawWave(float *fL, float *fR)
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    lpDevice->SetTexture(0, NULL);
    lpDevice->SetVertexShader( g_pVertexShader_Legacy1 );
	lpDevice->SetPixelShader(g_pPixelShader_Legacy1);
    lpDevice->SetFVF( WFVERTEX_FORMAT );

	int i;
	WFVERTEX v1[576+1], v2[576+1];

	lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	lpDevice->SetRenderState(D3DRS_DESTBLEND, (*m_pState->var_pf_wave_additive) ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);

	//float cr = m_pState->m_waveR.eval(GetTime());
	//float cg = m_pState->m_waveG.eval(GetTime());
	//float cb = m_pState->m_waveB.eval(GetTime());
	float cr = (float)(*m_pState->var_pf_wave_r);
	float cg = (float)(*m_pState->var_pf_wave_g);
	float cb = (float)(*m_pState->var_pf_wave_b);
	float cx = (float)(*m_pState->var_pf_wave_x);
	float cy = (float)(*m_pState->var_pf_wave_y); // note: it was backwards (top==1) in the original milkdrop, so we keep it that way!
	float fWaveParam = (float)(*m_pState->var_pf_wave_mystery);

	if (cr < 0) cr = 0;
	if (cg < 0) cg = 0;
	if (cb < 0) cb = 0;
	if (cr > 1) cr = 1;
	if (cg > 1) cg = 1;
	if (cb > 1) cb = 1;

	// maximize color:
	if (*m_pState->var_pf_wave_brighten)
	{
		float fMaximizeWaveColorAmount = 1.0f;
		float max = cr;
		if (max < cg) max = cg;
		if (max < cb) max = cb;
		if (max > 0.01f)
		{
			cr = cr/max*fMaximizeWaveColorAmount + cr*(1.0f - fMaximizeWaveColorAmount);
			cg = cg/max*fMaximizeWaveColorAmount + cg*(1.0f - fMaximizeWaveColorAmount);
			cb = cb/max*fMaximizeWaveColorAmount + cb*(1.0f - fMaximizeWaveColorAmount);
		}
	}

	float fWavePosX = cx*2.0f - 1.0f; // go from 0..1 user-range to -1..1 D3D range
	float fWavePosY = cy*2.0f - 1.0f;

	float bass_rel   = mysound.imm[0];
	float mid_rel    = mysound.imm[1];
	float treble_rel = mysound.imm[2];

	int sample_offset = 0;
	int new_wavemode = (int)(*m_pState->var_pf_wave_mode) % NUM_WAVES;  // since it can be changed from per-frame code!

	int its = (m_pState->m_bBlending && (new_wavemode != m_pState->m_nOldWaveMode)) ? 2 : 1;
	int nVerts1 = 0;
	int nVerts2 = 0;
	int nBreak1 = -1;
	int nBreak2 = -1;
	float alpha1, alpha2;

	for (int it=0; it<its; it++)
	{
		int   wave   = (it==0) ? new_wavemode : m_pState->m_nOldWaveMode;
		int   nVerts = NUM_WAVEFORM_SAMPLES;		// allowed to peek ahead 64 (i.e. left is [i], right is [i+64])
		int   nBreak = -1;

        float fWaveParam2 = fWaveParam;
        //std::string fWaveParam; // kill its scope
        if ((wave==0 || wave==1 || wave==4) && (fWaveParam2 < -1 || fWaveParam2 > 1))
        {
            //fWaveParam2 = max(fWaveParam2, -1.0f);
            //fWaveParam2 = min(fWaveParam2,  1.0f);
            fWaveParam2 = fWaveParam2*0.5f + 0.5f;
            fWaveParam2 -= floorf(fWaveParam2);
            fWaveParam2 = fabsf(fWaveParam2);
            fWaveParam2 = fWaveParam2*2-1;
        }

		WFVERTEX *v = (it==0) ? v1 : v2;
		ZeroMemory(v, sizeof(WFVERTEX)*nVerts);

		float alpha = (float)(*m_pState->var_pf_wave_a);//m_pState->m_fWaveAlpha.eval(GetTime());

		switch(wave)
		{
		case 0:
			// circular wave

			nVerts /= 2;
			sample_offset = (NUM_WAVEFORM_SAMPLES-nVerts)/2;//mysound.GoGoAlignatron(nVerts * 12/10);	// only call this once nVerts is final!

			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);

			{
				float inv_nverts_minus_one = 1.0f/(float)(nVerts-1);

				for (i=0; i<nVerts; i++)
				{
					float rad = 0.5f + 0.4f*fR[i+sample_offset] + fWaveParam2;
					float ang = (i)*inv_nverts_minus_one*6.28f + GetTime()*0.2f;
					if (i < nVerts/10)
					{
						float mix = i/(nVerts*0.1f);
						mix = 0.5f - 0.5f*cosf(mix * 3.1416f);
						float rad_2 = 0.5f + 0.4f*fR[i + nVerts + sample_offset] + fWaveParam2;
						rad = rad_2*(1.0f-mix) + rad*(mix);
					}
					v[i].x = rad*cosf(ang) *m_fAspectY + fWavePosX;		// 0.75 = adj. for aspect ratio
					v[i].y = rad*sinf(ang) *m_fAspectX + fWavePosY;
					//v[i].Diffuse = color;
				}
			}

			// dupe last vertex to connect the lines; skip if blending
			if (!m_pState->m_bBlending)
			{
				nVerts++;
				memcpy(&v[nVerts-1], &v[0], sizeof(WFVERTEX));
			}

			break;

		case 1:
			// x-y osc. that goes around in a spiral, in time

			alpha *= 1.25f;
			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);

			nVerts /= 2;	

			for (i=0; i<nVerts; i++)
			{
				float rad = 0.53f + 0.43f*fR[i] + fWaveParam2;
				float ang = fL[i+32] * 1.57f + GetTime()*2.3f;
				v[i].x = rad*cosf(ang) *m_fAspectY + fWavePosX;		// 0.75 = adj. for aspect ratio
				v[i].y = rad*sinf(ang) *m_fAspectX + fWavePosY;
				//v[i].Diffuse = color;//(D3DCOLOR_RGBA_01(cr, cg, cb, alpha*min(1, max(0, fL[i])));
			}

			break;
			
		case 2:
			// centered spiro (alpha constant)
			//	 aimed at not being so sound-responsive, but being very "nebula-like"
			//   difference is that alpha is constant (and faint), and waves a scaled way up

			switch(m_nTexSizeX)
			{
			case 256:  alpha *= 0.07f; break;
			case 512:  alpha *= 0.09f; break;
			case 1024: alpha *= 0.11f; break;
			case 2048: alpha *= 0.13f; break;
			}

			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);
			
			for (i=0; i<nVerts; i++)
			{
				v[i].x = fR[i   ] *m_fAspectY + fWavePosX;//((pR[i] ^ 128) - 128)/90.0f * ASPECT; // 0.75 = adj. for aspect ratio
				v[i].y = fL[i+32] *m_fAspectX + fWavePosY;//((pL[i+32] ^ 128) - 128)/90.0f;
				//v[i].Diffuse = color;
			}

			break;
		case 3:
			// centered spiro (alpha tied to volume)
			//	 aimed at having a strong audio-visual tie-in
			//   colors are always bright (no darks)

			switch(m_nTexSizeX)
			{
			case 256:  alpha = 0.075f; break;
			case 512:  alpha = 0.150f; break;
			case 1024: alpha = 0.220f; break;
			case 2048: alpha = 0.330f; break;
			}

			alpha *= 1.3f;
			alpha *= powf(treble_rel, 2.0f);
			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);

			for (i=0; i<nVerts; i++)
			{
				v[i].x = fR[i   ] *m_fAspectY + fWavePosX;//((pR[i] ^ 128) - 128)/90.0f * ASPECT; // 0.75 = adj. for aspect ratio
				v[i].y = fL[i+32] *m_fAspectX + fWavePosY;//((pL[i+32] ^ 128) - 128)/90.0f;
				//v[i].Diffuse = color;
			}
			break;
		case 4:
			// horizontal "script", left channel

			if (nVerts > m_nTexSizeX/3) 
				nVerts = m_nTexSizeX/3;

			sample_offset = (NUM_WAVEFORM_SAMPLES-nVerts)/2;//mysound.GoGoAlignatron(nVerts + 25);	// only call this once nVerts is final!

			/*
			if (treble_rel > treb_thresh_for_wave6)
			{
				//alpha = 1.0f;
				treb_thresh_for_wave6 = treble_rel * 1.025f;
			}
			else
			{
				alpha *= 0.2f;
				treb_thresh_for_wave6 *= 0.996f;		// fixme: make this fps-independent
			}
			*/

			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);

			{
				float w1 = 0.45f + 0.5f*(fWaveParam2*0.5f + 0.5f);		// 0.1 - 0.9
				float w2 = 1.0f - w1;

				float inv_nverts = 1.0f/(float)(nVerts);

				for (i=0; i<nVerts; i++)
				{
					v[i].x = -1.0f + 2.0f*(i*inv_nverts) + fWavePosX;
					v[i].y = fL[i+sample_offset]*0.47f + fWavePosY;//((pL[i] ^ 128) - 128)/270.0f;
					v[i].x += fR[i+25+sample_offset]*0.44f;//((pR[i+25] ^ 128) - 128)/290.0f;
					//v[i].Diffuse = color;

					// momentum
					if (i>1)
					{
						v[i].x = v[i].x*w2 + w1*(v[i-1].x*2.0f - v[i-2].x);
						v[i].y = v[i].y*w2 + w1*(v[i-1].y*2.0f - v[i-2].y);
					}
				}

				/*
				// center on Y
				float avg_y = 0;
				for (i=0; i<nVerts; i++) 
					avg_y += v[i].y;
				avg_y /= (float)nVerts;
				avg_y *= 0.5f;		// damp the movement
				for (i=0; i<nVerts; i++) 
					v[i].y -= avg_y;
				*/
			}

			break;

		case 5:
			// weird explosive complex # thingy

			switch(m_nTexSizeX)
			{
			case 256:  alpha *= 0.07f; break;
			case 512:  alpha *= 0.09f; break;
			case 1024: alpha *= 0.11f; break;
			case 2048: alpha *= 0.13f; break;
			}

			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);
			
			{
				float cos_rot = cosf(GetTime()*0.3f);
				float sin_rot = sinf(GetTime()*0.3f);

				for (i=0; i<nVerts; i++)
				{
					float x0 = (fR[i]*fL[i+32] + fL[i]*fR[i+32]);
					float y0 = (fR[i]*fR[i] - fL[i+32]*fL[i+32]);
					v[i].x = (x0*cos_rot - y0*sin_rot)*m_fAspectY + fWavePosX;
					v[i].y = (x0*sin_rot + y0*cos_rot)*m_fAspectX + fWavePosY;
					//v[i].Diffuse = color;
				}
			}

			break;

		case 6:
		case 7:
		case 8:
			// 6: angle-adjustable left channel, with temporal wave alignment;
			//   fWaveParam2 controls the angle at which it's drawn
			//	 fWavePosX slides the wave away from the center, transversely.
			//   fWavePosY does nothing
			//
			// 7: same, except there are two channels shown, and
			//   fWavePosY determines the separation distance.
			// 
			// 8: same as 6, except using the spectrum analyzer (UNFINISHED)
			// 
			nVerts /= 2;

			if (nVerts > m_nTexSizeX/3) 
				nVerts = m_nTexSizeX/3;

			if (wave==8)
				nVerts = 256;
			else
				sample_offset = (NUM_WAVEFORM_SAMPLES-nVerts)/2;//mysound.GoGoAlignatron(nVerts);	// only call this once nVerts is final!

			if (m_pState->m_bModWaveAlphaByVolume)
				alpha *= ((mysound.imm_rel[0] + mysound.imm_rel[1] + mysound.imm_rel[2])*0.333f - m_pState->m_fModWaveAlphaStart.eval(GetTime()))/(m_pState->m_fModWaveAlphaEnd.eval(GetTime()) - m_pState->m_fModWaveAlphaStart.eval(GetTime()));
			if (alpha < 0) alpha = 0;
			if (alpha > 1) alpha = 1;
			//color = D3DCOLOR_RGBA_01(cr, cg, cb, alpha);

			{
				float ang = 1.57f*fWaveParam2;	// from -PI/2 to PI/2
				float dx  = cosf(ang);
				float dy  = sinf(ang);

				float edge_x[2], edge_y[2];

				//edge_x[0] = fWavePosX - dx*3.0f;
				//edge_y[0] = fWavePosY - dy*3.0f;
				//edge_x[1] = fWavePosX + dx*3.0f;
				//edge_y[1] = fWavePosY + dy*3.0f;
				edge_x[0] = fWavePosX*cosf(ang + 1.57f) - dx*3.0f;
				edge_y[0] = fWavePosX*sinf(ang + 1.57f) - dy*3.0f;
				edge_x[1] = fWavePosX*cosf(ang + 1.57f) + dx*3.0f;
				edge_y[1] = fWavePosX*sinf(ang + 1.57f) + dy*3.0f;

				for (i=0; i<2; i++)	// for each point defining the line
				{
					// clip the point against 4 edges of screen
					// be a bit lenient (use +/-1.1 instead of +/-1.0) 
					//	 so the dual-wave doesn't end too soon, after the channels are moved apart
					for (int j=0; j<4; j++)
					{
						float t;
						bool bClip = false;

						switch(j)
						{
						case 0:
							if (edge_x[i] > 1.1f)
							{
								t = (1.1f - edge_x[1-i]) / (edge_x[i] - edge_x[1-i]);
								bClip = true;
							}
							break;
						case 1:
							if (edge_x[i] < -1.1f)
							{
								t = (-1.1f - edge_x[1-i]) / (edge_x[i] - edge_x[1-i]);
								bClip = true;
								}
							break;
						case 2:
							if (edge_y[i] > 1.1f)
							{
								t = (1.1f - edge_y[1-i]) / (edge_y[i] - edge_y[1-i]);
								bClip = true;
							}
							break;
						case 3:
							if (edge_y[i] < -1.1f)
							{
								t = (-1.1f - edge_y[1-i]) / (edge_y[i] - edge_y[1-i]);
								bClip = true;
							}
							break;
						}

						if (bClip)
						{
							float dx = edge_x[i] - edge_x[1-i];
							float dy = edge_y[i] - edge_y[1-i];
							edge_x[i] = edge_x[1-i] + dx*t;
							edge_y[i] = edge_y[1-i] + dy*t;
						}
					}
				}

				dx = (edge_x[1] - edge_x[0]) / (float)nVerts;
				dy = (edge_y[1] - edge_y[0]) / (float)nVerts;
				float ang2 = atan2f(dy,dx);
				float perp_dx = cosf(ang2 + 1.57f);
				float perp_dy = sinf(ang2 + 1.57f);

				if (wave == 6)
					for (i=0; i<nVerts; i++)
					{
						v[i].x = edge_x[0] + dx*i + perp_dx*0.25f*fL[i + sample_offset];
						v[i].y = edge_y[0] + dy*i + perp_dy*0.25f*fL[i + sample_offset];
						//v[i].Diffuse = color;
					}
				else if (wave == 8)
					//256 verts
					for (i=0; i<nVerts; i++)
					{
						float f = 0.1f*logf(mysound.fSpecLeft[i*2] + mysound.fSpecLeft[i*2+1]);
						v[i].x = edge_x[0] + dx*i + perp_dx*f;
						v[i].y = edge_y[0] + dy*i + perp_dy*f;
						//v[i].Diffuse = color;
					}
				else
				{
					float sep = powf(fWavePosY*0.5f + 0.5f, 2.0f);
					for (i=0; i<nVerts; i++)
					{
						v[i].x = edge_x[0] + dx*i + perp_dx*(0.25f*fL[i + sample_offset] + sep);
						v[i].y = edge_y[0] + dy*i + perp_dy*(0.25f*fL[i + sample_offset] + sep);
						//v[i].Diffuse = color;
					}

					for (i=0; i<nVerts; i++)
					{
						v[i+nVerts].x = edge_x[0] + dx*i + perp_dx*(0.25f*fR[i + sample_offset] - sep);
						v[i+nVerts].y = edge_y[0] + dy*i + perp_dy*(0.25f*fR[i + sample_offset] - sep);
						//v[i+nVerts].Diffuse = color;
					}

					nBreak = nVerts;
					nVerts *= 2;
				}
			}

			break;
		}

		if (it==0)
		{
			nVerts1 = nVerts;
			nBreak1 = nBreak;
			alpha1  = alpha;
		}
		else
		{
			nVerts2 = nVerts;
			nBreak2 = nBreak;
			alpha2  = alpha;
		}
	}	

	// v1[] is for the current waveform
	// v2[] is for the old waveform (from prev. preset - only used if blending)
	// nVerts1 is the # of vertices in v1
	// nVerts2 is the # of vertices in v2
	// nBreak1 is the index of the point at which to break the solid line in v1[] (-1 if no break)
	// nBreak2 is the index of the point at which to break the solid line in v2[] (-1 if no break)

	float mix = CosineInterp(m_pState->m_fBlendProgress);
	float mix2 = 1.0f - mix;

	// blend 2 waveforms
	if (nVerts2 > 0)
	{
		// note: this won't yet handle the case where (nBreak1 > 0 && nBreak2 > 0)
		//       in this case, code must break wave into THREE segments
		float m = (nVerts2-1)/(float)nVerts1;
		float x,y;
		for (int i=0; i<nVerts1; i++)
		{
			float fIdx = i*m;
			int   nIdx = (int)fIdx;
			float t = fIdx - nIdx;
			if (nIdx == nBreak2-1)
			{
				x = v2[nIdx].x;
				y = v2[nIdx].y;
				nBreak1 = i+1;
			}
			else
			{
				x = v2[nIdx].x*(1-t) + v2[nIdx+1].x*(t);
				y = v2[nIdx].y*(1-t) + v2[nIdx+1].y*(t);
			}
			v1[i].x = v1[i].x*(mix) + x*(mix2);
			v1[i].y = v1[i].y*(mix) + y*(mix2);
		}
	}

	// determine alpha
	if (nVerts2 > 0)
	{
		alpha1 = alpha1*(mix) + alpha2*(1.0f-mix);
	}

	// apply color & alpha
    // ALSO reverse all y values, to stay consistent with the pre-VMS milkdrop,
    //  which DIDN'T:
	v1[0].Diffuse = D3DCOLOR_RGBA_01(cr, cg, cb, alpha1);
	for (i=0; i<nVerts1; i++)
    {
		v1[i].Diffuse = v1[0].Diffuse;
        v1[i].y = -v1[i].y;
    }
	
    // don't draw wave if (possibly blended) alpha is less than zero.
    if (alpha1 < 0.004f)
        goto SKIP_DRAW_WAVE;

    // TESSELLATE - smooth the wave, one time.
    WFVERTEX* pVerts = v1;
    WFVERTEX vTess[(576+3)*2];
    if (1)
    {
        if (nBreak1==-1)
        {
            nVerts1 = SmoothWave(v1, nVerts1, vTess);
        }
        else 
        {
            int oldBreak = nBreak1;
            nBreak1 = SmoothWave(v1, nBreak1, vTess);
            nVerts1 = SmoothWave(&v1[oldBreak], nVerts1-oldBreak, &vTess[nBreak1]) + nBreak1;
        }
        pVerts = vTess;
    }

	// draw primitives
	{
		//D3DPRIMITIVETYPE primtype = (*m_pState->var_pf_wave_usedots) ? D3DPT_POINTLIST : D3DPT_LINESTRIP;
		float x_inc = 2.0f / (float)m_nTexSizeX;
		float y_inc = 2.0f / (float)m_nTexSizeY;
		int drawing_its = ((*m_pState->var_pf_wave_thick || *m_pState->var_pf_wave_usedots) && (m_nTexSizeX >= 512)) ? 4 : 1;

		for (int it=0; it<drawing_its; it++)
		{
			int j;

			switch(it)
			{
			case 0: break;
			case 1: for (j=0; j<nVerts1; j++) pVerts[j].x += x_inc; break;		// draw fat dots
			case 2: for (j=0; j<nVerts1; j++) pVerts[j].y += y_inc; break;		// draw fat dots
			case 3: for (j=0; j<nVerts1; j++) pVerts[j].x -= x_inc; break;		// draw fat dots
			}

			if (nBreak1 == -1)
			{
                if (*m_pState->var_pf_wave_usedots)
                    lpDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nVerts1, (void*)pVerts, sizeof(WFVERTEX));
                else
                    lpDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, nVerts1-1, (void*)pVerts, sizeof(WFVERTEX));
			}
			else
			{
                if (*m_pState->var_pf_wave_usedots)
                {
                    lpDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nBreak1, (void*)pVerts, sizeof(WFVERTEX));
                    lpDevice->DrawPrimitiveUP(D3DPT_POINTLIST, nVerts1-nBreak1, (void*)&pVerts[nBreak1], sizeof(WFVERTEX));
                }
                else
                {
                    lpDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, nBreak1-1, (void*)pVerts, sizeof(WFVERTEX));
                    lpDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, nVerts1-nBreak1-1, (void*)&pVerts[nBreak1], sizeof(WFVERTEX));
                }
			}
		}
	}

SKIP_DRAW_WAVE:
	lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
}

void CPlugin::DrawSprites()
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    lpDevice->SetTexture(0, NULL);
    lpDevice->SetVertexShader( g_pVertexShader_Legacy1 );
	lpDevice->SetPixelShader (g_pPixelShader_Legacy1 );
    lpDevice->SetFVF( WFVERTEX_FORMAT );

	if (*m_pState->var_pf_darken_center)
	{
		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);//SRCALPHA);
		lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		WFVERTEX v3[6];
		ZeroMemory(v3, sizeof(WFVERTEX)*6);

		// colors:
		v3[0].Diffuse = D3DCOLOR_RGBA_01(0, 0, 0, 3.0f/32.0f);
		v3[1].Diffuse = D3DCOLOR_RGBA_01(0, 0, 0, 0.0f/32.0f);
		v3[2].Diffuse = v3[1].Diffuse;
		v3[3].Diffuse = v3[1].Diffuse;
		v3[4].Diffuse = v3[1].Diffuse;
		v3[5].Diffuse = v3[1].Diffuse;

		// positioning:
		float fHalfSize = 0.05f;
		v3[0].x = 0.0f; 	
		v3[1].x = 0.0f - fHalfSize*m_fAspectY; 	
		v3[2].x = 0.0f; 	
		v3[3].x = 0.0f + fHalfSize*m_fAspectY; 	
		v3[4].x = 0.0f; 
		v3[5].x = v3[1].x;
		v3[0].y = 0.0f; 	
		v3[1].y = 0.0f;
		v3[2].y = 0.0f - fHalfSize; 	
		v3[3].y = 0.0f;
		v3[4].y = 0.0f + fHalfSize; 	
		v3[5].y = v3[1].y;
		//v3[0].tu = 0;	v3[1].tu = 1;	v3[2].tu = 0;	v3[3].tu = 1;
		//v3[0].tv = 1;	v3[1].tv = 1;	v3[2].tv = 0;	v3[3].tv = 0;

		lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 4, (LPVOID)v3, sizeof(WFVERTEX));

		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}

	// do borders
	{
		float fOuterBorderSize = (float)*m_pState->var_pf_ob_size;
		float fInnerBorderSize = (float)*m_pState->var_pf_ib_size;

		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
		lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		for (int it=0; it<2; it++)
		{
			WFVERTEX v3[4];
			ZeroMemory(v3, sizeof(WFVERTEX)*4);

			// colors:
			float r = (it==0) ? (float)*m_pState->var_pf_ob_r : (float)*m_pState->var_pf_ib_r;
			float g = (it==0) ? (float)*m_pState->var_pf_ob_g : (float)*m_pState->var_pf_ib_g;
			float b = (it==0) ? (float)*m_pState->var_pf_ob_b : (float)*m_pState->var_pf_ib_b;
			float a = (it==0) ? (float)*m_pState->var_pf_ob_a : (float)*m_pState->var_pf_ib_a;
			if (a > 0.001f)
			{
				v3[0].Diffuse = D3DCOLOR_RGBA_01(r,g,b,a);
				v3[1].Diffuse = v3[0].Diffuse;
				v3[2].Diffuse = v3[0].Diffuse;
				v3[3].Diffuse = v3[0].Diffuse;

				// positioning:
				float fInnerRad = (it==0) ? 1.0f - fOuterBorderSize : 1.0f - fOuterBorderSize - fInnerBorderSize;
				float fOuterRad = (it==0) ? 1.0f                    : 1.0f - fOuterBorderSize;
				v3[0].x =  fInnerRad;
				v3[1].x =  fOuterRad; 	
				v3[2].x =  fOuterRad;
				v3[3].x =  fInnerRad;
				v3[0].y =  fInnerRad;
				v3[1].y =  fOuterRad;
				v3[2].y = -fOuterRad;
				v3[3].y = -fInnerRad;

				for (int rot=0; rot<4; rot++)
				{
		            lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, (LPVOID)v3, sizeof(WFVERTEX));

					// rotate by 90 degrees
					for (int v=0; v<4; v++)
					{
						float t = 1.570796327f;
						float x = v3[v].x;
						float y = v3[v].y;
						v3[v].x = x*cosf(t) - y*sinf(t);
						v3[v].y = x*sinf(t) + y*cosf(t);
					}
				}
			}
		}
		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	}
}

void CPlugin::DrawUserSprites()	// from system memory, to back buffer.
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice)
        return;

    lpDevice->SetTexture(0, NULL);
    lpDevice->SetVertexShader( NULL );
    lpDevice->SetFVF( SPRITEVERTEX_FORMAT );

    lpDevice->SetRenderState(D3DRS_WRAP0, 0);
    lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    lpDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

    // reset these to the standard safe mode:
//    lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
//	lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
//	lpDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
//    lpDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
//	lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
 //   lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
 //   lpDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	for (int iSlot=0; iSlot < NUM_TEX; iSlot++)
	{
		if (m_texmgr.m_tex[iSlot].pSurface)
		{
			int k;

			// set values of input variables:
			*(m_texmgr.m_tex[iSlot].var_time)     = (double)(GetTime() - m_texmgr.m_tex[iSlot].fStartTime);
			*(m_texmgr.m_tex[iSlot].var_frame)    = (double)(GetFrame() - m_texmgr.m_tex[iSlot].nStartFrame);
			*(m_texmgr.m_tex[iSlot].var_fps)      = (double)GetFps();
			*(m_texmgr.m_tex[iSlot].var_progress) = (double)m_pState->m_fBlendProgress;
			*(m_texmgr.m_tex[iSlot].var_bass)     = (double)mysound.imm_rel[0];
			*(m_texmgr.m_tex[iSlot].var_mid)      = (double)mysound.imm_rel[1];
			*(m_texmgr.m_tex[iSlot].var_treb)     = (double)mysound.imm_rel[2];
			*(m_texmgr.m_tex[iSlot].var_bass_att) = (double)mysound.avg_rel[0];
			*(m_texmgr.m_tex[iSlot].var_mid_att)  = (double)mysound.avg_rel[1];
			*(m_texmgr.m_tex[iSlot].var_treb_att) = (double)mysound.avg_rel[2];

			// evaluate expressions
			#ifndef _NO_EXPR_
				if (m_texmgr.m_tex[iSlot].m_codehandle)
				{
					NSEEL_code_execute(m_texmgr.m_tex[iSlot].m_codehandle);
				}
			#endif

			bool bKillSprite = (*m_texmgr.m_tex[iSlot].var_done != 0.0);
			bool bBurnIn = (*m_texmgr.m_tex[iSlot].var_burn != 0.0);

            // Remember the original backbuffer and zbuffer
            LPDIRECT3DSURFACE9 pBackBuffer=NULL;//, pZBuffer=NULL;
            lpDevice->GetRenderTarget( 0, &pBackBuffer );
            //lpDevice->GetDepthStencilSurface( &pZBuffer );


			// finally, use the results to draw the sprite.
			if (lpDevice->SetTexture(0, m_texmgr.m_tex[iSlot].pSurface) != D3D_OK) 
				return;

			SPRITEVERTEX v3[4];
			ZeroMemory(v3, sizeof(SPRITEVERTEX)*4);

			float x   = min(1000.0f, max(-1000.0f, (float)(*m_texmgr.m_tex[iSlot].var_x) * 2.0f - 1.0f ));
			float y   = min(1000.0f, max(-1000.0f, (float)(*m_texmgr.m_tex[iSlot].var_y) * 2.0f - 1.0f ));
			float sx  = min(1000.0f, max(-1000.0f, (float)(*m_texmgr.m_tex[iSlot].var_sx) ));
			float sy  = min(1000.0f, max(-1000.0f, (float)(*m_texmgr.m_tex[iSlot].var_sy) ));
			float rot = (float)(*m_texmgr.m_tex[iSlot].var_rot);
			int flipx = (*m_texmgr.m_tex[iSlot].var_flipx == 0.0) ? 0 : 1;
			int flipy = (*m_texmgr.m_tex[iSlot].var_flipy == 0.0) ? 0 : 1;
			float repeatx = min(100.0f, max(0.01f, (float)(*m_texmgr.m_tex[iSlot].var_repeatx) ));
			float repeaty = min(100.0f, max(0.01f, (float)(*m_texmgr.m_tex[iSlot].var_repeaty) ));

			int blendmode = min(4, max(0, ((int)(*m_texmgr.m_tex[iSlot].var_blendmode))));
			float r = min(1.0f, max(0.0f, ((float)(*m_texmgr.m_tex[iSlot].var_r))));
			float g = min(1.0f, max(0.0f, ((float)(*m_texmgr.m_tex[iSlot].var_g))));
			float b = min(1.0f, max(0.0f, ((float)(*m_texmgr.m_tex[iSlot].var_b))));
			float a = min(1.0f, max(0.0f, ((float)(*m_texmgr.m_tex[iSlot].var_a))));

			// set x,y coords
			v3[0+flipx].x = -sx;
			v3[1-flipx].x =  sx;
			v3[2+flipx].x = -sx;
			v3[3-flipx].x =  sx;
			v3[0+flipy*2].y = -sy;
			v3[1+flipy*2].y = -sy;
			v3[2-flipy*2].y =  sy;
			v3[3-flipy*2].y =  sy;

			// first aspect ratio: adjust for non-1:1 images
			{
				float aspect = m_texmgr.m_tex[iSlot].img_h / (float)m_texmgr.m_tex[iSlot].img_w;

				if (aspect < 1)
					for (k=0; k<4; k++) v3[k].y *= aspect;		// wide image
				else
					for (k=0; k<4; k++) v3[k].x /= aspect;		// tall image
			}

			// 2D rotation
			{
				float cos_rot = cosf(rot);
				float sin_rot = sinf(rot);
				for (k=0; k<4; k++)
				{
					float x2 = v3[k].x*cos_rot - v3[k].y*sin_rot;
					float y2 = v3[k].x*sin_rot + v3[k].y*cos_rot;
					v3[k].x = x2;
					v3[k].y = y2;
				}
			}

			// translation
			for (k=0; k<4; k++)
			{
				v3[k].x += x;
				v3[k].y += y;
			}

			// second aspect ratio: normalize to width of screen
			{
				float aspect = GetWidth() / (float)(GetHeight());
				
				if (aspect > 1)
					for (k=0; k<4; k++) v3[k].y *= aspect;
				else
					for (k=0; k<4; k++) v3[k].x /= aspect;
			}

			// third aspect ratio: adjust for burn-in
			if (bKillSprite && bBurnIn)	// final render-to-VS1
			{
				float aspect = GetWidth()/(float)(GetHeight()*4.0f/3.0f);
				if (aspect < 1.0f)
					for (k=0; k<4; k++) v3[k].x *= aspect;
				else
					for (k=0; k<4; k++) v3[k].y /= aspect;
			}

			// finally, flip 'y' for annoying DirectX
			//for (k=0; k<4; k++) v3[k].y *= -1.0f;

			// set u,v coords
			{
				float dtu = 0.5f;// / (float)m_texmgr.m_tex[iSlot].tex_w;
				float dtv = 0.5f;// / (float)m_texmgr.m_tex[iSlot].tex_h;
				v3[0].tu = -dtu;
				v3[1].tu =  dtu;///*m_texmgr.m_tex[iSlot].img_w / (float)m_texmgr.m_tex[iSlot].tex_w*/ - dtu;	
				v3[2].tu = -dtu;	
				v3[3].tu =  dtu;///*m_texmgr.m_tex[iSlot].img_w / (float)m_texmgr.m_tex[iSlot].tex_w*/ - dtu;	
				v3[0].tv = -dtv;	
				v3[1].tv = -dtv;	
				v3[2].tv =  dtv;///*m_texmgr.m_tex[iSlot].img_h / (float)m_texmgr.m_tex[iSlot].tex_h*/ - dtv;	
				v3[3].tv =  dtv;///*m_texmgr.m_tex[iSlot].img_h / (float)m_texmgr.m_tex[iSlot].tex_h*/ - dtv;

				// repeat on x,y
				for (k=0; k<4; k++) 
				{
					v3[k].tu = (v3[k].tu - 0.0f)*repeatx + 0.5f;
					v3[k].tv = (v3[k].tv - 0.0f)*repeaty + 0.5f;
				}
			}

			// blendmodes                                      src alpha:        dest alpha:
			// 0   blend      r,g,b=modulate     a=opacity     SRCALPHA          INVSRCALPHA
			// 1   decal      r,g,b=modulate     a=modulate    D3DBLEND_ONE      D3DBLEND_ZERO
			// 2   additive   r,g,b=modulate     a=modulate    D3DBLEND_ONE      D3DBLEND_ONE
			// 3   srccolor   r,g,b=no effect    a=no effect   SRCCOLOR          INVSRCCOLOR
			// 4   colorkey   r,g,b=modulate     a=no effect   
			switch(blendmode)
			{
			case 0:
			default:
				// alpha blend

				/*
				Q. I am rendering with alpha blending and setting the alpha 
				of the diffuse vertex component to determine the opacity.  
				It works when there is no texture set, but as soon as I set 
				a texture the alpha that I set is no longer applied.  Why?

				The problem originates in the texture blending stages, rather 
				than in the subsequent alpha blending.  Alpha can come from 
				several possible sources.  If this has not been specified, 
				then the alpha will be taken from the texture, if one is selected.  
				If no texture is selected, then the default will use the alpha 
				channel of the diffuse vertex component.

				Explicitly specifying the diffuse vertex component as the source 
				for alpha will insure that the alpha is drawn from the alpha value 
				you set, whether a texture is selected or not:

				pDevice->SetSamplerState(D3DSAMP_ALPHAOP,D3DTOP_SELECTARG1);
				pDevice->SetSamplerState(D3DSAMP_ALPHAARG1,D3DTA_DIFFUSE);

				If you later need to use the texture alpha as the source, set 
				D3DSAMP_ALPHAARG1 to D3DTA_TEXTURE.
				*/

 //               lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
//				lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
				lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
				lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				for (k=0; k<4; k++) v3[k].Diffuse = D3DCOLOR_RGBA_01(r,g,b,a);
				break;
			case 1:
				// decal
				lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				//lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
				//lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
				for (k=0; k<4; k++) v3[k].Diffuse = D3DCOLOR_RGBA_01(r*a,g*a,b*a,1);
				break;
			case 2:
				// additive
				lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
				lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
				for (k=0; k<4; k++) v3[k].Diffuse = D3DCOLOR_RGBA_01(r*a,g*a,b*a,1);
				break;
			case 3:
				// srccolor
				lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR);
				lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);
				for (k=0; k<4; k++) v3[k].Diffuse = D3DCOLOR_RGBA_01(1,1,1,1);
				break;
			case 4:
				// color keyed texture: use the alpha value in the texture to 
				//  determine which texels get drawn.  
				/*lpDevice->SetRenderState(D3DRS_ALPHAREF, 0);
				lpDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL);
				lpDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
                */

//                lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
//	            lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
//	            lpDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
 //               lpDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
//	            lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
//                lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
//                lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
 //               lpDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

				// also, smoothly blend this in-between texels:
				lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				lpDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
				lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
				for (k=0; k<4; k++) v3[k].Diffuse = D3DCOLOR_RGBA_01(r,g,b,a);
				break;
			}

			lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID)v3, sizeof(SPRITEVERTEX));

			if (/*bKillSprite &&*/ bBurnIn)	// final render-to-VS1
			{
                // Change the rendertarget back to the original setup
                lpDevice->SetTexture(0, NULL);
             //   lpDevice->SetRenderTarget( 0, pBackBuffer );
				 //lpDevice->SetDepthStencilSurface( pZBuffer );
			    lpDevice->SetTexture(0, m_texmgr.m_tex[iSlot].pSurface);

				// undo aspect ratio changes (that were used to fit it to VS1):
				{
					float aspect = GetWidth()/(float)(GetHeight()*4.0f/3.0f);
					if (aspect < 1.0f)
						for (k=0; k<4; k++) v3[k].x /= aspect;
					else
						for (k=0; k<4; k++) v3[k].y *= aspect;
				}

			    lpDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, (LPVOID)v3, sizeof(SPRITEVERTEX));
			}

   //         SafeRelease(pBackBuffer);
            //SafeRelease(pZBuffer);

			if (bKillSprite)
			{
				KillSprite(iSlot);
			}

	 //       lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    //        lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
    //        lpDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		}
	}

	lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

    // reset these to the standard safe mode:
//    lpDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
//	lpDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
//	lpDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
//    lpDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
//	lpDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
 //   lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
 //   lpDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void CPlugin::UvToMathSpace(float u, float v, float* rad, float* ang)
{
    // (screen space = -1..1 on both axes; corresponds to UV space)
    // uv space = [0..1] on both axes
    // "math" space = what the preset authors are used to:
    //      upper left = [0,0]
    //      bottom right = [1,1]
    //      rad == 1 at corners of screen
    //      ang == 0 at three o'clock, and increases counter-clockwise (to 6.28).
    float px = (u*2-1) * m_fAspectX;  // probably 1.0
    float py = (v*2-1) * m_fAspectY;  // probably <1
    
    *rad = sqrtf(px*px + py*py) / sqrtf(m_fAspectX*m_fAspectX + m_fAspectY*m_fAspectY);
    *ang = atan2f(py, px);
    if (*ang < 0)
        *ang += 6.2831853071796f;
}

void CPlugin::RestoreShaderParams()
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    for (int i=0; i<2; i++) 
    {
        lpDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);//texaddr);
        lpDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);//texaddr);
        lpDevice->SetSamplerState(i, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);//texaddr);
	    lpDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        lpDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
    }

    for (int i=0; i<4; i++) 
        lpDevice->SetTexture( i, NULL );

    lpDevice->SetVertexShader(NULL);
     //lpDevice->SetVertexDeclaration(NULL);  -directx debug runtime complains heavily about this
    lpDevice->SetPixelShader(NULL);

}

void CPlugin::ApplyShaderParams(CShaderParams* p, LPD3DXCONSTANTTABLE pCT, CState* pState)
{
    LPDIRECT3DDEVICE9 lpDevice = GetDevice();
    if (!lpDevice || !p || !pCT || !pState) return; // Basic validation

    // bind textures
    for (int i=0; i<sizeof(p->m_texture_bindings)/sizeof(p->m_texture_bindings[0]); i++)
    {
        LPDIRECT3DBASETEXTURE9 texToBind = NULL;

        if (p->m_texcode[i] == TEX_VS) {
            texToBind = m_lpPrevFrame; // *** BIND PREVIOUS FRAME TEXTURE FOR TEX_VS ***
        } else if (p->m_texcode[i] >= TEX_BLUR1 && p->m_texcode[i] <= TEX_BLUR_LAST) {
            // NOTE: If BlurPasses is disabled, m_lpBlur might be NULL or contain stale data.
            // Shaders requesting blur textures might get unexpected results.
            int blurIdx = (int)p->m_texcode[i] - (int)TEX_BLUR1;
            if (blurIdx >= 0 && blurIdx < NUM_BLUR_TEX) {
                texToBind = m_lpBlur[blurIdx];
            }
        } else if (p->m_texcode[i] == TEX_BLUR_LAST) {
             // Explicitly handle TEX_PREV_FRAME if it's different from TEX_VS
             texToBind = m_lpPrevFrame;
        } else {
            // Handle other texture codes (noise, user textures, etc.)
            texToBind = p->m_texture_bindings[i].texptr; // Use pre-assigned pointer
        }
        // else TEX_NONE, texToBind remains NULL

        lpDevice->SetTexture(i, texToBind);

        // also set up sampler stage, if anything is bound here...
        if (texToBind)
        {
            bool bAniso = false; // Anisotropy likely not needed/supported well here
            DWORD HQFilter = bAniso ? D3DTEXF_ANISOTROPIC : D3DTEXF_LINEAR;
            DWORD wrap   = p->m_texture_bindings[i].bWrap ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP;
            DWORD filter = p->m_texture_bindings[i].bBilinear ? HQFilter : D3DTEXF_POINT;
            lpDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, wrap);
            lpDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, wrap);
            lpDevice->SetSamplerState(i, D3DSAMP_ADDRESSW, wrap); // Usually ignored for 2D
            lpDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, filter);
            lpDevice->SetSamplerState(i, D3DSAMP_MINFILTER, filter);
            lpDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, filter); // Use D3DTEXF_NONE if no mipmaps
            // lpDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, bAniso ? 4 : 1);
        } else {
             // Ensure sampler states are reasonable even if no texture bound? Optional.
             // lpDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
             // lpDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
             // lpDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
        }

        // Update highest blur texture used (only if blur is enabled)
        // if (texToBind && p->m_texcode[i] >= TEX_BLUR1 && p->m_texcode[i] <= TEX_BLUR_LAST)
        //     m_nHighestBlurTexUsedThisFrame = max(m_nHighestBlurTexUsedThisFrame, ((int)p->m_texcode[i] - (int)TEX_BLUR1) + 1);
    }

    // bind "texsize_XYZ" params
    int N = p->texsize_params.size();
    for (int i=0; i<N; i++)
    {
        TexSizeParamInfo* q = &(p->texsize_params[i]);
        // Check handle validity before setting
        if (q->texsize_param) {
            float w = (float)q->w;
            float h = (float)q->h;
            pCT->SetVector( lpDevice, q->texsize_param, &D3DXVECTOR4(w, h, (w > 0) ? 1.0f/w : 0.0f, (h > 0) ? 1.0f/h : 0.0f));
        }
    }

    // --- Bind standard shader constants (time, fps, audio, etc.) ---
    // (This part remains largely the same as the original code)
    float time_since_preset_start = GetTime() - pState->GetPresetStartTime();
    float time_since_preset_start_wrapped = time_since_preset_start - floorf(time_since_preset_start/10000.0f)*10000.0f; // Use floorf for safety
    float time = GetTime() - m_fStartTime;
    float progress = (m_fNextPresetTime - m_fPresetStartTime > 1e-6f) ? (GetTime() - m_fPresetStartTime) / (m_fNextPresetTime - m_fPresetStartTime) : 0.0f;
    progress = max(0.0f, min(1.0f, progress)); // Clamp progress

    float mip_x = (GetWidth() > 0) ? logf((float)GetWidth())/logf(2.0f) : 0.0f;
    float mip_y = (GetHeight() > 0) ? logf((float)GetHeight())/logf(2.0f) : 0.0f;
    float mip_avg = 0.5f*(mip_x + mip_y);

    float aspect_x = 1.0f;
    float aspect_y = 1.0f;
    if (GetWidth() > 0 && GetHeight() > 0) { // Avoid division by zero
        if (GetWidth() > GetHeight())
            aspect_y = (float)GetHeight() / (float)GetWidth();
        else
            aspect_x = (float)GetWidth() / (float)GetHeight();
    }

    float blur_min[3], blur_max[3];
    GetSafeBlurMinMax(pState, blur_min, blur_max);

    // bind float4's
    if (p->rand_frame ) pCT->SetVector( lpDevice, p->rand_frame , &m_rand_frame );
    if (p->rand_preset) pCT->SetVector( lpDevice, p->rand_preset, &pState->m_rand_preset );
    D3DXHANDLE* h = p->const_handles;
    if (h[0]) pCT->SetVector( lpDevice, h[0], &D3DXVECTOR4( aspect_x, aspect_y, (aspect_x != 0.0f) ? 1.0f/aspect_x : 1.0f, (aspect_y != 0.0f) ? 1.0f/aspect_y : 1.0f ));
    // h[1] seems unused (pixel)
    if (h[2]) pCT->SetVector( lpDevice, h[2], &D3DXVECTOR4(time_since_preset_start_wrapped, GetFps(), (float)GetFrame(), progress));
    float bass = mysound.imm_rel[0]; float mid = mysound.imm_rel[1]; float treb = mysound.imm_rel[2];
    if (h[3]) pCT->SetVector( lpDevice, h[3], &D3DXVECTOR4(bass, mid, treb, 0.3333f*(bass + mid + treb) ));
    float bass_att = mysound.avg_rel[0]; float mid_att = mysound.avg_rel[1]; float treb_att = mysound.avg_rel[2];
    if (h[4]) pCT->SetVector( lpDevice, h[4], &D3DXVECTOR4(bass_att, mid_att, treb_att, 0.3333f*(bass_att + mid_att + treb_att) ));
    // Blur constants might be irrelevant if blur is disabled
    if (h[5]) pCT->SetVector( lpDevice, h[5], &D3DXVECTOR4( blur_max[0]-blur_min[0], blur_min[0], blur_max[1]-blur_min[1], blur_min[1] ));
    if (h[6]) pCT->SetVector( lpDevice, h[6], &D3DXVECTOR4( blur_max[2]-blur_min[2], blur_min[2], blur_min[0], blur_max[0] ));
    float texw = (float)m_nTexSizeX; float texh = (float)m_nTexSizeY;
    if (h[7]) pCT->SetVector( lpDevice, h[7], &D3DXVECTOR4(texw, texh, (texw > 0) ? 1.0f/texw : 0.0f, (texh > 0) ? 1.0f/texh : 0.0f ));
    // Slow trig constants
    if (h[8]) pCT->SetVector( lpDevice, h[8], &D3DXVECTOR4( 0.5f+0.5f*cosf(time* 0.329f+1.2f), 0.5f+0.5f*cosf(time* 1.293f+3.9f), 0.5f+0.5f*cosf(time* 5.070f+2.5f), 0.5f+0.5f*cosf(time*20.051f+5.4f) ));
    if (h[9]) pCT->SetVector( lpDevice, h[9], &D3DXVECTOR4( 0.5f+0.5f*sinf(time* 0.329f+1.2f), 0.5f+0.5f*sinf(time* 1.293f+3.9f), 0.5f+0.5f*sinf(time* 5.070f+2.5f), 0.5f+0.5f*sinf(time*20.051f+5.4f) ));
    // Very slow trig constants
    if (h[10]) pCT->SetVector( lpDevice, h[10], &D3DXVECTOR4( 0.5f+0.5f*cosf(time*0.0050f+2.7f), 0.5f+0.5f*cosf(time*0.0085f+5.3f), 0.5f+0.5f*cosf(time*0.0133f+4.5f), 0.5f+0.5f*cosf(time*0.0217f+3.8f) ));
    if (h[11]) pCT->SetVector( lpDevice, h[11], &D3DXVECTOR4( 0.5f+0.5f*sinf(time*0.0050f+2.7f), 0.5f+0.5f*sinf(time*0.0085f+5.3f), 0.5f+0.5f*sinf(time*0.0133f+4.5f), 0.5f+0.5f*sinf(time*0.0217f+3.8f) ));
    if (h[12]) pCT->SetVector( lpDevice, h[12], &D3DXVECTOR4( mip_x, mip_y, mip_avg, 0 ));
    // More blur constants
    if (h[13]) pCT->SetVector( lpDevice, h[13], &D3DXVECTOR4( blur_min[1], blur_max[1], blur_min[2], blur_max[2] ));


    // write q vars
    int num_q_float4s = sizeof(p->q_const_handles)/sizeof(p->q_const_handles[0]);
    for (int i=0; i<num_q_float4s; i++)
    {
        if (p->q_const_handles[i])
            pCT->SetVector( lpDevice, p->q_const_handles[i], &D3DXVECTOR4(
                (float)*pState->var_pf_q[i*4+0],
                (float)*pState->var_pf_q[i*4+1],
                (float)*pState->var_pf_q[i*4+2],
                (float)*pState->var_pf_q[i*4+3] ));
    }

    // write matrices (remains the same)
    // ... (matrix setting code) ...
    for (int i=0; i<20; i++)
    {
        if (p->rot_mat[i])
        {
            D3DXMATRIX mx,my,mz,mxlate,temp;
            D3DXMatrixRotationX(&mx, pState->m_rot_base[i].x + pState->m_rot_speed[i].x*time);
            D3DXMatrixRotationY(&my, pState->m_rot_base[i].y + pState->m_rot_speed[i].y*time);
            D3DXMatrixRotationZ(&mz, pState->m_rot_base[i].z + pState->m_rot_speed[i].z*time);
            D3DXMatrixTranslation(&mxlate, pState->m_xlate[i].x, pState->m_xlate[i].y, pState->m_xlate[i].z);
            D3DXMatrixMultiply(&temp, &mx, &mxlate);
            D3DXMatrixMultiply(&temp, &temp, &mz);
            D3DXMatrixMultiply(&temp, &temp, &my);
            pCT->SetMatrix(lpDevice, p->rot_mat[i], &temp);
        }
    }
    // the last 4 are totally random, each frame
    for (int i=20; i<24; i++)
    {
        if (p->rot_mat[i])
        {
            D3DXMATRIX mx,my,mz,mxlate,temp;
            D3DXMatrixRotationX(&mx, FRAND * 6.283185f);
            D3DXMatrixRotationY(&my, FRAND * 6.283185f);
            D3DXMatrixRotationZ(&mz, FRAND * 6.283185f);
            D3DXMatrixTranslation(&mxlate, FRAND, FRAND, FRAND); // Random translation [-0.5..0.5]? Check FRAND range.
            D3DXMatrixMultiply(&temp, &mx, &mxlate);
            D3DXMatrixMultiply(&temp, &temp, &mz);
            D3DXMatrixMultiply(&temp, &temp, &my);
            pCT->SetMatrix(lpDevice, p->rot_mat[i], &temp);
        }
    }
}

// Remove or comment out ShowToUser_NoShaders and ShowToUser_Shaders as their
// functionality is merged into WarpedBlit_* or the shaders themselves.

void CPlugin::ShowToUser_NoShaders()
{
    // This function is deprecated. Its logic is merged into WarpedBlit_NoShaders.
    // If called, it might indicate an issue elsewhere.
    // For safety, ensure it does nothing or logs a warning.
    return;
}

void CPlugin::ShowToUser_Shaders(int nPass, bool bAlphaBlend, bool bFlipAlpha, bool bCullTiles, bool bFlipCulling)
{
    // This function is deprecated. Its logic should be merged into the warp/composite shader.
    // If called, it might indicate an issue elsewhere.
    // For safety, ensure it does nothing or logs a warning.
    return;
}