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

#include "utility.h"

float AdjustRateToFPS(float per_frame_decay_rate_at_fps1, float fps1, float actual_fps)
{
    // returns the equivalent per-frame decay rate at actual_fps

    // basically, do all your testing at fps1 and get a good decay rate;
    // then, in the real application, adjust that rate by the actual fps each time you use it.
    
    float per_second_decay_rate_at_fps1 = powf(per_frame_decay_rate_at_fps1, fps1);
    float per_frame_decay_rate_at_fps2 = powf(per_second_decay_rate_at_fps1, 1.0f/actual_fps);

    return per_frame_decay_rate_at_fps2;
}

inline float fmaxf(float a, float b) {
    return a > b ? a : b;
}

void RemoveExtension(wchar_t *str)
{
    wchar_t *p = wcsrchr(str, L'.');
    if (p) *p = 0;
}

void FAKE_time_to_frequency_domain(float *in_wavedata, float *out_spectraldata)
{
    // Fake FFT implementation generating plausible spectral data
    const int numBins = 4400 / 2;  // Assuming NFREQ is initialized elsewhere
    
    // Generate base spectrum with frequency characteristics
    for (int i = 0; i < numBins; i++) {
        // Base amplitude decreases exponentially with frequency
        float base = 1.0f / (1.0f + i * 0.1f);
        
        // Add pink noise characteristics (more energy in lower frequencies)
        float noise = static_cast<float>(rand()) / RAND_MAX * 0.15f * base;
        
        // Simulate random peaks in different frequency ranges
        float peak = 0.0f;
        if (rand() % 100 < 3) {  // 3% chance of random peak
            peak = static_cast<float>(rand()) / RAND_MAX * 2.5f;
        }
        
        // Boost typical frequency ranges (bass, mid, treble)
        float frequency_boost = 1.0f;
        float normalized_freq = static_cast<float>(i) / numBins;
        
        // Bass boost (0-15% of spectrum)
        if (normalized_freq < 0.15f) {
            frequency_boost += 2.0f - normalized_freq * 10.0f;
        }
        // Mid boost (15-40% of spectrum)
        else if (normalized_freq < 0.4f) {
            frequency_boost += 1.5f - normalized_freq * 2.0f;
        }
        
        // Combine components with frequency-specific boosts
        float magnitude = (base + noise + peak) * frequency_boost;
        
        // Simulate high-frequency roll-off
        if (normalized_freq > 0.7f) {
            magnitude *= 1.0f - (normalized_freq - 0.7f) * 0.5f;
        }
        
        // Apply final noise floor
        magnitude += static_cast<float>(rand()) / RAND_MAX * 0.05f;
        
        out_spectraldata[i] = fmaxf(magnitude, 0.01f);  // Ensure non-negative values
    }
}