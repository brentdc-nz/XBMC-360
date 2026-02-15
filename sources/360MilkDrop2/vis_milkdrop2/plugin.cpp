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
  ##########################################################################################

  NOTE: 
  
  The DX9 SDK include & lib files for building MilkDrop 2 are right here, in the subdirectory:
    .\dx9sdk_summer04\

  The summer 2004 SDK statically links you to d3dx9.lib (5,820 kb).  It bloats vis_milk2.dll 
  a bit, but we (Ben Allison / Ryan Geiss) decided it's worth the stability.  We tinkered 
  with using the February 2005 sdk, which lets you dynamically link to d3dx9_31.dll (?), 
  but decided to skip it because it would cause too much hassle/confusion among users.

  Summer 2004 sdk: http://www.microsoft.com/downloads/details.aspx?FamilyID=fd044a42-9912-42a3-9a9e-d857199f888e&DisplayLang=en
  Feb 2005 sdk:    http://www.microsoft.com/downloads/details.aspx?FamilyId=77960733-06E9-47BA-914A-844575031B81&displaylang=en

  ##########################################################################################
*/

/*
Order of Function Calls
-----------------------
    The only code that will be called by the plugin framework are the
    12 virtual functions in plugin.h.  But in what order are they called?  
    A breakdown follows.  A function name in { } means that it is only 
    called under certain conditions.

    Order of function calls...
    
    When the PLUGIN launches
    ------------------------
        INITIALIZATION
            OverrideDefaults
            MyPreInitialize
            MyReadConfig
            << DirectX gets initialized at this point >>
            AllocateMyNonDx9Stuff
            AllocateMyDX9Stuff
        RUNNING
            +--> { CleanUpMyDX9Stuff + AllocateMyDX9Stuff }  // called together when user resizes window or toggles fullscreen<->windowed.
            |    MyRenderFn
            |    MyRenderUI
            |    { MyWindowProc }                            // called, between frames, on mouse/keyboard/system events.  100% threadsafe.
            +----<< repeat >>
        CLEANUP        
            CleanUpMyDX9Stuff
            CleanUpMyNonDx9Stuff
            << DirectX gets uninitialized at this point >>

    When the CONFIG PANEL launches
    ------------------------------
        INITIALIZATION
            OverrideDefaults
            MyPreInitialize
            MyReadConfig
            << DirectX gets initialized at this point >>
        RUNNING
            { MyConfigTabProc }                  // called on startup & on keyboard events
        CLEANUP
            [ MyWriteConfig ]                    // only called if user clicked 'OK' to exit
            << DirectX gets uninitialized at this point >>
*/

/*
  NOTES
  -----
  


  To do
  -----
    -VMS VERSION:
        -based on vms 1.05, but the 'fix slow text' option has been added.
            that includes m_lpDDSText, CTextManager (m_text), changes to 
            DrawDarkTranslucentBox, the replacement of all DrawText calls
            (now routed through m_text), and adding the 'fix slow text' cb
            to the config panel.

    -KILLED FEATURES:
        -vj mode
    
    -NEW FEATURES FOR 1.04:
            -added the following variables for per-frame scripting: (all booleans, except 'gamma')
	            wave_usedots, wave_thick, wave_additive, wave_brighten
                gamma, darken_center, wrap, invert, brighten, darken, solarize
                (also, note that echo_zoom, echo_alpha, and echo_orient were already in there,
                 but weren't covered in the documentation!)
        d   -fixed: spectrum w/512 samples + 256 separation -> infinite spike
        d   -reverted dumb changes to aspect ratio stuff
        d   -reverted wave_y fix; now it's backwards, just like it's always been
                (i.e. for wave's y position, 0=bottom and 1=top, which is opposite
                to the convention in the rest of milkdrop.  decided to keep the
                'bug' so presets don't need modified.)
        d   -fixed: Krash: Inconsistency bug - pressing Escape while in the code windows 
                for custom waves completely takes you out of the editing menus, 
                rather than back to the custom wave menu 
        d   -when editing code: fix display of '&' character 
        d   -internal texture size now has a little more bias toward a finer texture, 
                based on the window size, when set to 'Auto'.  (Before, for example,
                to reach 1024x1024, the window had to be 768x768 or greater; now, it
                only has to be 640x640 (25% of the way there).  I adjusted it because
                before, at in-between resolutions like 767x767, it looked very grainy;
                now it will always look nice and crisp, at any window size, but still
                won't cause too much aliasing (due to downsampling for display).
        d   -fixed: rova:
                When creating presets have commented code // in the per_pixel section when cause error in preset.
                Example nothing in per_frame and just comments in the per_pixel. EXamples on repuest I have a few.
        d   -added kill keys:
                -CTRL+K kills all running sprites
                -CTRL+T kills current song title anim
                -CTRL+Y kills current custom message
        d   -notice to sprite users:
                -in milk_img.ini, color key can't be a range anymore; it's
                    now limited to just a single color.  'colorkey_lo' and 
                    'colorkey_hi' have been replaced with just one setting, 
                    'colorkey'.
        d   -song titles + custom messages are working again
        ?   -fixed?: crashes on window resize [out of mem]
                -Rova: BTW the same bug as krash with the window resizing.
                -NOT due to the 'integrate w/winamp' option.
                -> might be fixed now (had forgotten to release m_lpDDSText)
        <AFTER BETA 3..>
        d   -added checkbox to config screen to automatically turn SCROLL LOCK on @ startup
        d   -added alphanumeric seeking to the playlist; while playlist is up,
                you can now press A-Z and 0-9 to seek to the next song in the playlist
                that starts with that character.
        d   -<fixed major bug w/illegal mem access on song title launches;
                could have been causing crashing madness @ startup on many systems>
        d   -<fixed bug w/saving 64x48 mesh size>
        d   -<fixed squashed shapes>
        d   -<fixed 'invert' variable>
        d   -<fixed squashed song titles + custom msgs>
        ?   -<might have fixed scroll lock stuff>  
        ?   -<might have fixed crashing; could have been due to null ptr for failed creation of song title texture.>
        ?   -<also might have solved any remaining resize or exit bugs by callign SetTexture(NULL)
                in DX8 cleanup.>
        d   -<fixed sizing issues with songtitle font.>
        d   -<fixed a potentially bogus call to deallocate memory on exit, when it was cleaning up the menus.>
        d   -<fixed more scroll lock issues>
        d   -<fixed broken Noughts & Crosses presets; max # of per-frame vars was one too few, after the additions of the new built-in variables.>
        d   -<hopefully fixed waveforms>
        <AFTER BETA 4>
            -now when playlist is up, SHIFT+A-Z seeks upward (while lowercase/regular a-z seeks downward).
            -custom shapes:
                -OH MY GOD
                -increased max. # of custom shapes (and waves) from 3 to 4
                -added 'texture' option, which allows you to use the last frame as a texture on the shape
                    -added "tex_ang" and "tex_zoom" params to control the texture coords
                -each frame, custom shapes now draw BEFORE regular waveform + custom waves
                -added init + per-frame vars: "texture", "additive", "thick", "tex_ang", "tex_zoom"
            -fixed valid characters for filenames when importing/exporting custom shapes/waves;
                also, it now gives error messages on error in import/export.
            -cranked max. meshsize up to 96x72
            -Krash, Rova: now the 'q' variables, as modified by the preset per-frame equations, are again 
                readable by the custom waves + custom shapes.  Sorry about that.  Should be the end of the 
                'q' confusion.
            -added 'meshx' and 'meshy' [read-only] variables to the preset init, per-frame, and per-pixel 
                equations (...and inc'd the size of the global variable pool by 2).
            -removed t1-t8 vars for Custom Shapes; they were unnecessary (since there's no per-point code there).
            -protected custom waves from trying to draw when # of sample minus the separation is < 2
                (or 1 if drawing with dots)
            -fixed some [minor] preset-blending bugs in the custom wave code 
            -created a visual map for the flow of values for the q1-q8 and t1-t8 variables:
                q_and_t_vars.gif (or something).
            -fixed clipping of onscreen text in low-video-memory situations.  Now, if there isn't enough
                video memory to create an offscreen texture that is at least 75% of the size of the 
                screen (or to create at least a 256x256 one), it won't bother using one, and will instead draw text directly to the screen.
                Otherwise, if the texture is from 75%-99% of the screen size, text will now at least
                appear in the correct position on the screen so that it will be visible; this will mean
                that the right- and bottom-aligned text will no longer be fully right/bottom-aligned 
                to the edge of the screen.                
            -fixed blurry text 
            -VJ mode is partially restored; the rest will come with beta 7 or the final release.  At the time of beta 6, VJ mode still has some glitches in it, but I'm working on them.  Most notably, it doesn't resize the text image when you resize the window; that's next on my list.
        <AFTER BETA 6:>            
            -now sprites can burn-in on any frame, not just on the last frame.
                set 'burn' to one (in the sprite code) on any frame to make it burn in.
                this will break some older sprites, but it's super easy to fix, and 
                I think it's worth it. =)  thanks to papaw00dy for the suggestion!
            -fixed the asymptotic-value bug with custom waves using spectral data & having < 512 samples (thanks to telek's example!)
            -fixed motion vectors' reversed Y positioning.
            -fixed truncation ("...") of long custom messages
            -fixed that pesky bug w/the last line of code on a page
            -fixed the x-positioning of custom waves & shapes.  Before, if you were 
                saving some coordinates from the preset's per-frame equations (say in q1 and q2)
                and then you set "x = q1; y = q2;" in a custom shape's per-frame code
                (or in a custom wave's per-point code), the x position wouldn't really be
                in the right place, because of aspect ratio multiplications.  Before, you had
                to actually write "x = (q1-0.5)*0.75 + 0.5; y = q2;" to get it to line up; 
                now it's fixed, though, and you can just write "x = q1; y = q2;".
            -fixed some bugs where the plugin start up, in windowed mode, on the wrong window
                (and hence run super slow).
            -fixed some bugs w/a munged window frame when the "integrate with winamp" option
                was checked.
        <AFTER BETA 7:>
            -preset ratings are no longer read in all at once; instead, they are scanned in
                1 per frame until they're all in.  This fixes the long pauses when you switch
                to a directory that has many hundreds of presets.  If you want to switch
                back to the old way (read them all in at once), there is an option for it
                in the config panel.
            -cranked max. mesh size up to 128x96
            -fixed bug in custom shape per-frame code, where t1-t8 vars were not 
                resetting, at the beginning of each frame, to the values that they had 
                @ the end of the custom shape init code's execution.
            -
            -
            -


        beta 2 thread: http://forums.winamp.com/showthread.php?threadid=142635
        beta 3 thread: http://forums.winamp.com/showthread.php?threadid=142760
        beta 4 thread: http://forums.winamp.com/showthread.php?threadid=143500
        beta 6 thread: http://forums.winamp.com/showthread.php?threadid=143974
        (+read about beat det: http://forums.winamp.com/showthread.php?threadid=102205)

@       -code editing: when cursor is on 1st posn. in line, wrong line is highlighted!?
        -requests:
            -random sprites (...they can just write a prog for this, tho)
            -Text-entry mode.
                -Like your favorite online game, hit T or something to enter 'text entry' mode. Type a message, then either hit ESC to clear and cancel text-entry mode, or ENTER to display the text on top of the vis. Easier for custom messages than editing the INI file (and probably stopping or minimizing milkdrop to do it) and reloading.
                -OR SKIP IT; EASY TO JUST EDIT, RELOAD, AND HIT 00.
            -add 'AA' parameter to custom message text file?
        -when mem is low, fonts get kicked out -> white boxes
            -probably happening b/c ID3DXFont can't create a 
             temp surface to use to draw text, since all the
             video memory is gobbled up.
*       -add to installer: q_and_t_vars.gif
*       -presets:
            1. pick final set
                    a. OK-do a pass weeding out slow presets (crank mesh size up)
                    b. OK-do 2nd pass; rate them & delete crappies
                    c. OK-merge w/set from 1.03; check for dupes; delete more suckies
            2. OK-check for cpu-guzzlers
            3. OK-check for big ones (>= 8kb)
            4. check for ultra-spastic-when-audio-quiet ones
            5. update all ratings
            6. zip 'em up for safekeeping
*       -docs: 
                -link to milkdrop.co.uk
                -preset authoring:
                    -there are 11 variable pools:
                        preset:
                            a) preset init & per-frame code
                            b) preset per-pixel code
                        custom wave 1:
                            c) init & per-frame code
                            d) per-point code
                        custom wave 2:
                            e) init & per-frame code
                            f) per-point code
                        custom wave 3:
                            g) init & per-frame code
                            h) per-point code
                        i) custom shape 1: init & per-frame code
                        j) custom shape 2: init & per-frame code
                        k) custom shape 3: init & per-frame code

                    -all of these have predefined variables, the values of many of which 
                        trickle down from init code, to per-frame code, to per-pixel code, 
                        when the same variable is defined for each of these.
                    -however, variables that you define ("my_var = 5;") do NOT trickle down.
                        To allow you to pass custom values from, say, your per-frame code
                        to your per-pixel code, the variables q1 through q8 were created.
                        For custom waves and custom shapes, t1 through t8 work similarly.
                    -q1-q8:
                        -purpose: to allow [custom] values to carry from {the preset init
                            and/or per-frame equations}, TO: {the per-pixel equations},
                            {custom waves}, and {custom shapes}.
                        -are first set in preset init code.
                        -are reset, at the beginning of each frame, to the values that 
                            they had at the end of the preset init code. 
                        -can be modified in per-frame code...
                            -changes WILL be passed on to the per-pixel code 
                            -changes WILL pass on to the q1-q8 vars in the custom waves
                                & custom shapes code
                            -changes will NOT pass on to the next frame, though;
                                use your own (custom) variables for that.
                        -can be modified in per-pixel code...
                            -changes will pass on to the next *pixel*, but no further
                            -changes will NOT pass on to the q1-q8 vars in the custom
                                waves or custom shapes code.
                            -changes will NOT pass on to the next frame, after the
                                last pixel, though.
                        -CUSTOM SHAPES: q1-q8...
                            -are readable in both the custom shape init & per-frame code
                            -start with the same values as q1-q8 had at the end of the *preset*
                                per-frame code, this frame
                            -can be modified in the init code, but only for a one-time
                                pass-on to the per-frame code.  For all subsequent frames
                                (after the first), the per-frame code will get the q1-q8
                                values as described above.
                            -can be modified in the custom shape per-frame code, but only 
                                as temporary variables; the changes will not pass on anywhere.
                        -CUSTOM WAVES: q1-q8...
                            -are readable in the custom wave init, per-frame, and per-point code
                            -start with the same values as q1-q8 had at the end of the *preset*
                                per-frame code, this frame
                            -can be modified in the init code, but only for a one-time
                                pass-on to the per-frame code.  For all subsequent frames
                                (after the first), the per-frame code will get the q1-q8
                                values as described above.
                            -can be modified in the custom wave per-frame code; changes will
                                pass on to the per-point code, but that's it.
                            -can be modified in the per-point code, and the modified values
                                will pass on from point to point, but won't carry beyond that.
                        -CUSTOM WAVES: t1-t8...
                            -allow you to generate & save values in the custom wave init code,
                                that can pass on to the per-frame and, more sigificantly,
                                per-point code (since it's in a different variable pool).
                            -...                                
                        


                        !-whatever the values of q1-q8 were at the end of the per-frame and per-pixel
                            code, these are copied to the q1-q8 variables in the custom wave & custom 
                            shape code, for that frame.  However, those values are separate.
                            For example, if you modify q1-q8 in the custom wave #1 code, those changes 
                            will not be visible anywhere else; if you modify q1-q8 in the custom shape
                            #2 code, same thing.  However, if you modify q1-q8 in the per-frame custom
                            wave code, those modified values WILL be visible to the per-point custom
                            wave code, and can be modified within it; but after the last point,
                            the values q1-q8 will be discarded; on the next frame, in custom wave #1
                            per-frame code, the values will be freshly copied from the values of the 
                            main q1-q8 after the preset's per-frame and per-point code have both been
                            executed.                          
                    -monitor: 
                        -can be read/written in preset init code & preset per-frame code.
                        -not accessible from per-pixel code.
                        -if you write it on one frame, then that value persists to the next frame.
                    -t1-t8:
                        -
                        -
                        -
                -regular docs:
                    -put in the stuff recommended by VMS (vidcap, etc.)
                    -add to troubleshooting:
                        1) desktop mode icons not appearing?  or
                           onscreen text looking like colored boxes?
                             -> try freeing up some video memory.  lower your res; drop to 16 bit;
                                etc.  TURN OFF AUTO SONGTITLES.
                        1) slow desktop/fullscr mode?  -> try disabling auto songtitles + desktop icons.
                            also try reducing texsize to 256x256, since that eats memory that the text surface could claim.
                        2) 
                        3) 
        *   -presets:
                -add new 
                -fix 3d presets (bring gammas back down to ~1.0)
                -check old ones, make sure they're ok
                    -"Rovastar - Bytes"
                    -check wave_y
        *   -document custom waves & shapes
        *   -slow text is mostly fixed... =(
                -desktop icons + playlist both have begin/end around them now, but in desktop mode,
                 if you bring up playlist or Load menu, fps drops in half; press Esc, and fps doesn't go back up.
            -
            -
            -
        -DONE / v1.04:
            -updated to VMS 1.05
                -[list benefits...]
                -
                -
            -3d mode: 
                a) SWAPPED DEFAULT L/R LENS COLORS!  All images on the web are left=red, right=blue!                    
                b) fixed image display when viewing a 3D preset in a non-4:3 aspect ratio window
                c) gamma now works for 3d presets!  (note: you might have to update your old presets.
                        if they were 3D presets, the gamma was ignored and 1.0 was used; now,
                        if gamma was >1.0 in the old preset, it will probably appear extremely bright.)
                d) added SHIFT+F9 and CTRL+C9 to inc/dec stereo separation
                e) added default stereo separation to config panel
            -cranked up the max. mesh size (was 48x36, now 64x48) and the default mesh size
                (was 24x18, now 32x24)
            -fixed aspect ratio for final display
            -auto-texsize is now computed slightly differently; for vertically or horizontally-stretched
                windows, the texsize is now biased more toward the larger dimension (vs. just the
                average).
            -added anisotropic filtering (for machines that support it)
            -fixed bug where the values of many variables in the preset init code were not set prior 
                to execution of the init code (e.g. time, bass, etc. were all broken!)
            -added various preset blend effects.  In addition to the old uniform fade, there is
                now a directional wipe, radial wipe, and plasma fade.
            -FIXED SLOW TEXT for DX8 (at least, on the geforce 4).  
                Not using DT_RIGHT or DT_BOTTOM was the key.

        
        -why does text kill it in desktop mode?
        -text is SLOOW
        -to do: add (and use) song title font + tooltip font
        -re-test: menus, text, boxes, etc.
        -re-test: TIME        
        -testing:
            -make sure sound works perfectly.  (had to repro old pre-vms sound analysis!)
            -autogamma: currently assumes/requires that GetFrame() resets to 0 on a mode change
                (i.e. windowed -> fullscreen)... is that the case?
            -restore motion vectors
            -
            -
        -restore lost surfaces
        -test bRedraw flag (desktop mode/paused)
        -search for //? in milkdropfs.cpp and fix things
            
        problem: no good soln for VJ mode
        problem: D3DX won't give you solid background for your text.
            soln: (for later-) create wrapper fn that draws w/solid bkg.

        SOLN?: use D3DX to draw all text (plugin.cpp stuff AND playlist); 
        then, for VJ mode, create a 2nd DxContext 
        w/its own window + windowproc + fonts.  (YUCK)
    1) config panel: test, and add WM_HELP's (copy from tooltips)
    2) keyboard input: test; and...
        -need to reset UI_MODE when playlist is turned on, and
        -need to reset m_show_playlist when UI_MODE is changed.  (?)
        -(otherwise they can both show @ same time and will fight 
            for keys and draw over each other)
    3) comment out most of D3D stuff in milkdropfs.cpp, and then 
        get it running w/o any milkdrop, but with text, etc.
    4) sound

  Issues / To Do Later
  --------------------
    1) sprites: color keying stuff probably won't work any more...
    2) scroll lock: pull code from Monkey
    3) m_nGridY should not always be m_nGridX*3/4!
    4) get solid backgrounds for menus, waitstring code, etc.
        (make a wrapper function!)

    99) at end: update help screen

  Things that will be different
  -----------------------------
    1) font sizes are no longer relative to size of window; they are absolute.
    2) 'don't clear screen at startup' option is gone
    3) 'always on top' option is gone
    4) text will not be black-on-white when an inverted-color preset is showing

                -VJ mode:
                    -notes
                        1. (remember window size/pos, and save it from session to session?  nah.)
                        2. (kiv: scroll lock)
                        3. (VJ window + desktop mode:)
                                -ok w/o VJ mode
                                -w/VJ mode, regardless of 'fix slow text' option, probs w/focus;
                                    click on vj window, and plugin window flashes to top of Z order!
                                -goes away if you comment out 1st call to PushWindowToJustBeforeDesktop()...
                                -when you disable PushWindowToJustBeforeDesktop:
                                    -..and click on EITHER window, milkdrop jumps in front of the taskbar.
                                    -..and click on a non-MD window, nothing happens.
                                d-FIXED somehow, magically, while fixing bugs w/true fullscreen mode!
                        4. (VJ window + true fullscreen mode:)
                                d-make sure VJ window gets placed on the right monitor, at startup,
                                    and respects taskbar posn.
                                d-bug - start in windowed mode, then dbl-clk to go [true] fullscreen 
                                    on 2nd monitor, all with VJ mode on, and it excepts somewhere 
                                    in m_text.DrawNow() in a call to DrawPrimitive()!
                                    FIXED - had to check m_vjd3d8_device->TestCooperativeLevel
                                    each frame, and destroy/reinit if device needed reset.
                                d-can't resize VJ window when grfx window is running true fullscreen!
                                    -FIXED, by dropping the Sleep(30)/return when m_lost_focus
                                        was true, and by not consuming WM_NCACTIVATE in true fullscreen
                                        mode when m_hTextWnd was present, since DX8 doesn't do its
                                        auto-minimize thing in that case.



*/

#include "plugin.h"
#include "utility.h"
#include "support.h"
#include "defines.h"
#include "shell_defines.h"
#include <assert.h>
#include <io.h>
#include "..\xbox\legacyshaders.h"
#include "gstring.h"
#include "AutoWide.h"
#include "Vector.h"
#include <string>

extern CPlugin* g_pPlugin;

#ifdef _XBOX
int warand(){ return rand(); }
#endif

#define FRAND ((warand() % 7381)/7380.0f)

#define D3DCOLOR_RGBA_01(r,g,b,a) D3DCOLOR_RGBA(((int)(r*255)),((int)(g*255)),((int)(b*255)),((int)(a*255)))
#define SPRITEVERTEX_FORMAT (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) )

void NSEEL_HOSTSTUB_EnterMutex(){}
void NSEEL_HOSTSTUB_LeaveMutex(){}

#define SafeRelease(x) { if (x) {x->Release(); x=NULL;} } // BRENT - DELETE ME

// note: these must match layouts in support.h!!
D3DVERTEXELEMENT9 g_MyVertDecl[] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
    { 0, 16, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    { 0, 32, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
    D3DDECL_END()
};

D3DVERTEXELEMENT9 g_WfVertDecl[] =
{
    { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
    D3DDECL_END()
};

D3DVERTEXELEMENT9 g_SpriteVertDecl[] =
{
    // matches D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1
    { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
    { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
    { 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
    D3DDECL_END()
};



#define IsAlphabetChar(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define IsAlphanumericChar(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9') || x == '.')
#define IsNumericChar(x) (x >= '0' && x <= '9')

const unsigned char LC2UC[256] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,255,
	33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,
	49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,
	97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
	113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,
	97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,
	113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,
	129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,
	145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,
	161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,
	177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,
	193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,
	209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
	225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,
	241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
};

/*
 * Copies the given string TO the clipboard.
 */
void CPlugin::ClearErrors(int category)  // 0=all categories
{
    int N = m_errors.size();
    for (int i=0; i<N; i++)

    if (category==ERR_ALL || m_errors[i].category == category)
    {
      ///  m_errors.eraseAt(i);
        i--;
        N--;
    }
}

int mystrcmpiW(const char *s1, const char *s2)
{
	// returns  1 if s1 comes before s2
	// returns  0 if equal
	// returns -1 if s1 comes after s2
	// treats all characters/symbols by their ASCII values, 
	//    except that it DOES ignore case.

	int i=0;

	while (LC2UC[s1[i]] == LC2UC[s2[i]] && s1[i] != 0)
		i++;

	//FIX THIS!

	if (s1[i]==0 && s2[i]==0)
		return 0;
	else if (s1[i]==0)
		return -1;
	else if (s2[i]==0)
		return 1;
	else 
		return (LC2UC[s1[i]] < LC2UC[s2[i]]) ? -1 : 1;
}

bool ReadFileToString(const wchar_t* szBaseFilename, char* szDestText, int nMaxBytes, bool bConvertLFsToSpecialChar)
{
    wchar_t szFile[MAX_PATH];
    swprintf(szFile, L"%s%s", L"", szBaseFilename);
    
    // read in all chars.  Replace char combos:  { 13;  13+10;  10 } with LINEFEED_CONTROL_CHAR, if bConvertLFsToSpecialChar is true.
    FILE* f = _wfopen(szFile, L"rb");
    if (!f)
    {
//        wchar_t buf[1024], title[64];
//		swprintf(buf, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_READ_DATA_FILE_X), szFile);
//		g_plugin.dumpmsg(buf); 
//		MessageBoxW(NULL, buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		return false;
    }

    int len = 0;
    int x;
    char prev_ch = 0;
    while ( (x = fgetc(f)) >= 0 && len < nMaxBytes-4 ) 
    {
        char orig_ch = (char)x;
        char ch = orig_ch;
        bool bSkipChar = false;
        if (bConvertLFsToSpecialChar)
        {
            if (ch==10)
            {
                if (prev_ch==13)
                    bSkipChar = true;
                else 
                    ch = LINEFEED_CONTROL_CHAR;
            }
            else if (ch==13)
                ch = LINEFEED_CONTROL_CHAR;
        }

        if (!bSkipChar)
            szDestText[len++] = ch;
        prev_ch = orig_ch;
    }
    szDestText[len] = 0;
    szDestText[len++] = ' ';   // make sure there is some whitespace after
    fclose(f);
    return true;
}



// Modify the help screen text here.
// Watch the # of lines, though; if there are too many, they will get cut off;
//   and watch the length of the lines, since there is no wordwrap.  
// A good guideline: your entire help screen should be visible when fullscreen 
//   @ 640x480 and using the default help screen font.
wchar_t* g_szHelp = 0;
int g_szHelp_W = 0;

// this is for integrating modern skins (with their Random button)
// and having it match our Scroll Lock (preset lock) state...
#define IPC_CB_VISRANDOM 628 

//----------------------------------------------------------------------

void CPlugin::OverrideDefaults()
{
    // Here, you have the option of overriding the "default defaults"
    //   for the stuff on tab 1 of the config panel, replacing them
    //   with custom defaults for your plugin.
    // To override any of the defaults, just uncomment the line 
    //   and change the value.
    // DO NOT modify these values from any function but this one!

    // This example plugin only changes the default width/height
    //   for fullscreen mode; the "default defaults" are just
    //   640 x 480.
    // If your plugin is very dependent on smooth animation and you
    //   wanted it plugin to have the 'save cpu' option OFF by default,
    //   for example, you could set 'm_save_cpu' to 0 here.

    // m_start_fullscreen      = 0;       // 0 or 1
    // m_start_desktop         = 0;       // 0 or 1
    // m_fake_fullscreen_mode  = 0;       // 0 or 1
    // m_max_fps_fs            = 30;      // 1-120, or 0 for 'unlimited'
    // m_max_fps_dm            = 30;      // 1-120, or 0 for 'unlimited'
    // m_max_fps_w             = 30;      // 1-120, or 0 for 'unlimited'
    // m_show_press_f1_msg     = 1;       // 0 or 1
       m_allow_page_tearing_w  = 0;       // 0 or 1
    // m_allow_page_tearing_fs = 0;       // 0 or 1
    // m_allow_page_tearing_dm = 1;       // 0 or 1
    // m_minimize_winamp       = 1;       // 0 or 1
    // m_desktop_textlabel_boxes = 1;     // 0 or 1
    // m_save_cpu              = 0;       // 0 or 1

    // lstrcpy(m_fontinfo[0].szFace, "Trebuchet MS"); // system font
    // m_fontinfo[0].nSize     = 18;
    // m_fontinfo[0].bBold     = 0;
    // m_fontinfo[0].bItalic   = 0;
    // lstrcpy(m_fontinfo[1].szFace, "Times New Roman"); // decorative font
    // m_fontinfo[1].nSize     = 24;
    // m_fontinfo[1].bBold     = 0;
    // m_fontinfo[1].bItalic   = 1;

    // Don't override default FS mode here; shell is now smart and sets it to match
    // the current desktop display mode, by default.

    //m_disp_mode_fs.Width    = 1024;             // normally 640
    //m_disp_mode_fs.Height   = 768;              // normally 480
    // use either D3DFMT_X8R8G8B8 or D3DFMT_R5G6B5.  
    // The former will match to any 32-bit color format available, 
    // and the latter will match to any 16-bit color available, 
    // if that exact format can't be found.
	//m_disp_mode_fs.Format   = D3DFMT_UNKNOWN; //<- this tells config panel & visualizer to use current display mode as a default!!   //D3DFMT_X8R8G8B8;
	// m_disp_mode_fs.RefreshRate = 60;
}

//----------------------------------------------------------------------

void CPlugin::MyPreInitialize()
{
    // Initialize EVERY data member you've added to CPlugin here;
    //   these will be the default values.
    // If you want to initialize any of your variables with random values
    //   (using rand()), be sure to seed the random number generator first!
    // (If you want to change the default values for settings that are part of
    //   the plugin shell (framework), do so from OverrideDefaults() above.)

    // seed the system's random number generator w/the current system time:
    //srand((unsigned)time(NULL));  -don't - let winamp do it

	// attempt to load a unicode F1 help message otherwise revert to the ansi version
/*
	g_szHelp = (wchar_t*)GetTextResource(IDR_TEXT2,1);
	if(!g_szHelp) g_szHelp = (wchar_t*)GetTextResource(IDR_TEXT1,0);
	else g_szHelp_W = 1;
*/
    // CONFIG PANEL SETTINGS THAT WE'VE ADDED (TAB #2)
	m_bFirstRun		            = true;
    m_bInitialPresetSelected    = false;
	m_fBlendTimeUser			= 1.7f;
	m_fBlendTimeAuto			= 2.7f;
	m_fTimeBetweenPresets		= 16.0f;
	m_fTimeBetweenPresetsRand	= 10.0f;
	m_bSequentialPresetOrder    = false;
	m_bHardCutsDisabled			= true;
	m_fHardCutLoudnessThresh	= 2.5f;
	m_fHardCutHalflife			= 60.0f;
	//m_nWidth			= 1024;
	//m_nHeight			= 768;
	//m_nDispBits		= 16;
    m_nCanvasStretch    = 0;
	m_nTexSizeX			= -1;	// -1 means "auto"
	m_nTexSizeY			= -1;	// -1 means "auto"
	m_nTexBitsPerCh     =  8;
	m_nGridX			= 48;//32;
	m_nGridY			= 36;//24;

/*	m_bShowPressF1ForHelp = true;
	//lstrcpy(m_szMonitorName, "[don't use multimon]");
	m_bShowMenuToolTips = true;	// NOTE: THIS IS CURRENTLY HARDWIRED TO TRUE - NO OPTION TO CHANGE
	m_n16BitGamma	= 2;
	m_bAutoGamma    = true;
	//m_nFpsLimit			= -1;
	m_bEnableRating			= true;*/
	m_bSongTitleAnims		= true;
	m_fSongTitleAnimDuration = 1.7f;
	m_fTimeBetweenRandomSongTitles = -1.0f;
	m_fTimeBetweenRandomCustomMsgs = -1.0f;
	m_nSongTitlesSpawned = 0;
	m_nCustMsgsSpawned = 0;
    m_nFramesSinceResize = 0;
    m_bWarningsDisabled2    = false;
    m_bPresetLockOnAtStartup = false;
/*	m_bPreventScollLockHandling = false;
    m_nMaxPSVersion_ConfigPanel = -1;  // -1 = auto, 0 = disable shaders, 2 = ps_2_0, 3 = ps_3_0
    m_nMaxPSVersion_DX9 = -1;          // 0 = no shader support, 2 = ps_2_0, 3 = ps_3_0
    m_nMaxPSVersion = -1;              // this one will be the ~min of the other two.  0/2/3.
    m_nMaxImages = 32;
    m_nMaxBytes  = 16000000;
*/
    #if 0
        m_dwShaderFlags = D3DXSHADER_DEBUG|(1<<16);
    #else
        m_dwShaderFlags = (1<<16);//D3DXSHADER_SKIPOPTIMIZATION|D3DXSHADER_NO_PRESHADER;          
    #endif

    m_pShaderCompileErrors = NULL;

    ZeroMemory(&m_shaders,    sizeof(PShaderSet));
    ZeroMemory(&m_OldShaders, sizeof(PShaderSet));
    ZeroMemory(&m_NewShaders, sizeof(PShaderSet));
    ZeroMemory(&m_fallbackShaders_vs, sizeof(VShaderSet));
    ZeroMemory(&m_fallbackShaders_ps, sizeof(PShaderSet));
    ZeroMemory(m_BlurShaders, sizeof(m_BlurShaders));
    m_bWarpShaderLock = false;
    m_bCompShaderLock = false;
    m_bNeedRescanTexturesDir = true;

    // vertex declarations:
    m_pSpriteVertDecl = NULL;
    m_pWfVertDecl = NULL;
    m_pMyVertDecl = NULL;

	/*
    m_gdi_title_font_doublesize  = NULL;
    m_d3dx_title_font_doublesize = NULL;*/

    // RUNTIME SETTINGS THAT WE'VE ADDED
    m_prev_time = GetTime() - 0.0333f; // note: this will be updated each frame, at bottom of MyRenderFn.
	m_bTexSizeWasAutoPow2	= false;
	m_bTexSizeWasAutoExact	= false;
	m_bPresetLockedByUser = false;  //NOW SET IN DERIVED SETTINGS //BRENT
	m_bPresetLockedByCode = false;
	m_fStartTime	= 0.0f;
	m_fPresetStartTime = 0.0f;
	m_fNextPresetTime  = -1.0f;	// negative value means no time set (...it will be auto-set on first call to UpdateTime)
    m_nLoadingPreset   = 0;
    m_nPresetsLoadedTotal = 0;
    m_fSnapPoint = 0.5f;
	m_pState    = &m_state_DO_NOT_USE[0];
	m_pOldState = &m_state_DO_NOT_USE[1];
	m_pNewState = &m_state_DO_NOT_USE[2];
	m_UI_mode			= UI_REGULAR;
 //	m_bShowShaderHelp = false;

    m_nMashSlot = 0;    //0..MASH_SLOTS-1
    for (int mash=0; mash<MASH_SLOTS; mash++)
        m_nLastMashChangeFrame[mash] = 0;

//	m_bUserPagedUp      = false;
//	m_bUserPagedDown    = false;
	m_fMotionVectorsTempDx = 0.0f;
	m_fMotionVectorsTempDy = 0.0f;
	
//	m_waitstring.bActive		= false;
//	m_waitstring.bOvertypeMode  = false;
//	m_waitstring.szClipboard[0] = 0;

	m_nPresets		= 0;
	m_nDirs			= 0;
    m_nPresetListCurPos = 0;
	m_nCurrentPreset = -1;
	m_szCurrentPresetFile[0] = 0;
    m_szLoadingPreset[0] = 0;
	//m_szPresetDir[0] = 0; // will be set @ end of this function
    m_bPresetListReady = false;
    m_szUpdatePresetMask[0] = 0;
    //m_nRatingReadProgress = -1;

    myfft.Init(576, MY_FFT_SAMPLES, -1);
	memset(&mysound, 0, sizeof(mysound));

    for (int i=0; i<PRESET_HIST_LEN; i++)
        m_presetHistory[i] = "";

    m_presetHistoryPos = 0;
    m_presetHistoryBackFence = 0;
    m_presetHistoryFwdFence = 0;
/*
	//m_nTextHeightPixels = -1;
	//m_nTextHeightPixels_Fancy = -1;
	m_bShowFPS			= false;
	m_bShowRating		= false;
	m_bShowPresetInfo	= false;
	m_bShowDebugInfo	= false;
	m_bShowSongTitle	= false;
	m_bShowSongTime		= false;
	m_bShowSongLen		= false;
	m_fShowRatingUntilThisTime = -1.0f;
	ClearErrors();
	m_szDebugMessage[0]	= 0;
    m_szSongTitle[0]    = 0;
    m_szSongTitlePrev[0] = 0;*/

//	m_lpVS[0]				= NULL;
//	m_lpVS[1]				= NULL;
	m_lpPrevFrame = NULL;
    m_lpCurFrame = NULL;
    m_bShaderParamsUseCurrent = false;


    #if (NUM_BLUR_TEX>0)
        for (int i=0; i<NUM_BLUR_TEX; i++)
            m_lpBlur[i] = NULL;
    #endif

	m_lpDDSTitle			= NULL;
    m_nTitleTexSizeX        = 0;
    m_nTitleTexSizeY        = 0;
	m_verts					= NULL;
	m_verts_temp            = NULL;
	m_vertinfo				= NULL;
	m_indices_list			= NULL;
	m_indices_strip			= NULL;

/*	m_bMMX			        = false;
    m_bHasFocus             = true;
    m_bHadFocus             = false;
    m_bOrigScrollLockState  = GetKeyState(VK_SCROLL) & 1;
    // m_bMilkdropScrollLockState is derived at end of MyReadConfig()

	m_nNumericInputMode   = NUMERIC_INPUT_MODE_CUST_MSG;
	m_nNumericInputNum    = 0;
	m_nNumericInputDigits = 0;
	//td_custom_msg_font   m_CustomMessageFont[MAX_CUSTOM_MESSAGE_FONTS];
	//td_custom_msg        m_CustomMessage[MAX_CUSTOM_MESSAGES];

    //texmgr      m_texmgr;		// for user sprites

	m_supertext.bRedrawSuperText = false;
	m_supertext.fStartTime = -1.0f;

	// --------------------other init--------------------

    g_bDebugOutput		= false;
	g_bDumpFileCleared	= false;

    swprintf(m_szMilkdrop2Path, L"%s%s", GetPluginsDirPath(), SUBDIR);
	swprintf(m_szPresetDir,  L"%spresets\\", m_szMilkdrop2Path );

    // note that the config dir can be under Program Files or Application Data!!
    wchar_t szConfigDir[MAX_PATH] = {0};
    lstrcpyW(szConfigDir, GetConfigIniFile());
    wchar_t* p = wcsrchr(szConfigDir, L'\\');
    if (p) *(p+1) = 0;
   	swprintf(m_szMsgIniFile, L"%s%s", szConfigDir, MSG_INIFILE );
	swprintf(m_szImgIniFile, L"%s%s", szConfigDir, IMG_INIFILE );*/
}

//----------------------------------------------------------------------

void CPlugin::MyReadConfig()
{
    // Read the user's settings from the .INI file.
    // If you've added any controls to the config panel, read their value in
    //   from the .INI file here.

    // use this function         declared in   to read a value of this type:
    // -----------------         -----------   ----------------------------
    // GetPrivateProfileInt      Win32 API     int
    // GetPrivateProfileBool     utility.h     bool
    // GetPrivateProfileBOOL     utility.h     BOOL
    // GetPrivateProfileFloat    utility.h     float
    // GetPrivateProfileString   Win32 API     string

#ifndef _XBOX
	int n=0;
    wchar_t *pIni = GetConfigIniFile();

	m_bFirstRun		= !GetPrivateProfileBoolW(L"settings",L"bConfigured" ,false,pIni);
	m_bEnableRating = GetPrivateProfileBoolW(L"settings",L"bEnableRating",m_bEnableRating,pIni);
	m_bHardCutsDisabled = GetPrivateProfileBoolW(L"settings",L"bHardCutsDisabled",m_bHardCutsDisabled,pIni);
	g_bDebugOutput	= GetPrivateProfileBoolW(L"settings",L"bDebugOutput",g_bDebugOutput,pIni);
	m_bShowPressF1ForHelp = GetPrivateProfileBoolW(L"settings",L"bShowPressF1ForHelp",m_bShowPressF1ForHelp,pIni);
	m_bSongTitleAnims   = GetPrivateProfileBoolW(L"settings",L"bSongTitleAnims",m_bSongTitleAnims,pIni);

	m_bShowFPS			= GetPrivateProfileBoolW(L"settings",L"bShowFPS",       m_bShowFPS			,pIni);
	m_bShowRating		= GetPrivateProfileBoolW(L"settings",L"bShowRating",    m_bShowRating		,pIni);
	m_bShowPresetInfo	= GetPrivateProfileBoolW(L"settings",L"bShowPresetInfo",m_bShowPresetInfo	,pIni);
	m_bShowSongTitle	= GetPrivateProfileBoolW(L"settings",L"bShowSongTitle", m_bShowSongTitle	,pIni);
	m_bShowSongTime		= GetPrivateProfileBoolW(L"settings",L"bShowSongTime",  m_bShowSongTime	,pIni);
	m_bShowSongLen		= GetPrivateProfileBoolW(L"settings",L"bShowSongLen",   m_bShowSongLen		,pIni);

	int nTemp = GetPrivateProfileBoolW(L"settings",L"bFixPinkBug",-1,pIni);
	if (nTemp == 0)
		m_n16BitGamma = 0;
	else if (nTemp == 1)
		m_n16BitGamma = 2;
	m_n16BitGamma		= GetPrivateProfileIntW(L"settings",L"n16BitGamma",m_n16BitGamma,pIni);
	m_bAutoGamma        = GetPrivateProfileBoolW(L"settings",L"bAutoGamma",m_bAutoGamma,pIni);
	m_bWarningsDisabled2    = GetPrivateProfileBoolW(L"settings",L"bWarningsDisabled2",m_bWarningsDisabled2,pIni);
    m_bPresetLockOnAtStartup = GetPrivateProfileBoolW(L"settings",L"bPresetLockOnAtStartup",m_bPresetLockOnAtStartup,pIni);
	m_bPreventScollLockHandling = GetPrivateProfileBoolW(L"settings",L"m_bPreventScollLockHandling",m_bPreventScollLockHandling,pIni);

    m_nCanvasStretch = GetPrivateProfileIntW(L"settings",L"nCanvasStretch"    ,m_nCanvasStretch,pIni);
	m_nTexSizeX		= GetPrivateProfileIntW(L"settings",L"nTexSize"    ,m_nTexSizeX   ,pIni);
	m_nTexSizeY		= m_nTexSizeX;
	m_bTexSizeWasAutoPow2   = (m_nTexSizeX == -2);
	m_bTexSizeWasAutoExact = (m_nTexSizeX == -1);
	m_nTexBitsPerCh = GetPrivateProfileIntW(L"settings", L"nTexBitsPerCh", m_nTexBitsPerCh, pIni);
	m_nGridX		= GetPrivateProfileIntW(L"settings",L"nMeshSize"   ,m_nGridX      ,pIni);
	m_nGridY        = m_nGridX*3/4;
    m_nMaxPSVersion_ConfigPanel = GetPrivateProfileIntW(L"settings",L"MaxPSVersion",m_nMaxPSVersion_ConfigPanel,pIni);
    m_nMaxImages    = GetPrivateProfileIntW(L"settings",L"MaxImages",m_nMaxImages,pIni);
    m_nMaxBytes     = GetPrivateProfileIntW(L"settings",L"MaxBytes" ,m_nMaxBytes ,pIni);

	m_fBlendTimeUser			= GetPrivateProfileFloatW(L"settings",L"fBlendTimeUser"         ,m_fBlendTimeUser         ,pIni);
	m_fBlendTimeAuto			= GetPrivateProfileFloatW(L"settings",L"fBlendTimeAuto"         ,m_fBlendTimeAuto         ,pIni);
	m_fTimeBetweenPresets		= GetPrivateProfileFloatW(L"settings",L"fTimeBetweenPresets"    ,m_fTimeBetweenPresets    ,pIni);
	m_fTimeBetweenPresetsRand	= GetPrivateProfileFloatW(L"settings",L"fTimeBetweenPresetsRand",m_fTimeBetweenPresetsRand,pIni);
	m_fHardCutLoudnessThresh	= GetPrivateProfileFloatW(L"settings",L"fHardCutLoudnessThresh" ,m_fHardCutLoudnessThresh ,pIni);
	m_fHardCutHalflife			= GetPrivateProfileFloatW(L"settings",L"fHardCutHalflife"       ,m_fHardCutHalflife       ,pIni);
	m_fSongTitleAnimDuration	= GetPrivateProfileFloatW(L"settings",L"fSongTitleAnimDuration" ,m_fSongTitleAnimDuration ,pIni);
	m_fTimeBetweenRandomSongTitles = GetPrivateProfileFloatW(L"settings",L"fTimeBetweenRandomSongTitles" ,m_fTimeBetweenRandomSongTitles,pIni);
	m_fTimeBetweenRandomCustomMsgs = GetPrivateProfileFloatW(L"settings",L"fTimeBetweenRandomCustomMsgs" ,m_fTimeBetweenRandomCustomMsgs,pIni);

    // --------

	GetPrivateProfileStringW(L"settings",L"szPresetDir",m_szPresetDir,m_szPresetDir,sizeof(m_szPresetDir),pIni);

	ReadCustomMessages();

	// bounds-checking:
	if (m_nGridX > MAX_GRID_X)
		m_nGridX = MAX_GRID_X;
	if (m_nGridY > MAX_GRID_Y)
		m_nGridY = MAX_GRID_Y;
	if (m_fTimeBetweenPresetsRand < 0)
		m_fTimeBetweenPresetsRand = 0;
	if (m_fTimeBetweenPresets < 0.1f)
		m_fTimeBetweenPresets = 0.1f;

    // DERIVED SETTINGS
    m_bPresetLockedByUser      = m_bPresetLockOnAtStartup;
#endif

	// XML version //
	
#if 1 //TEmp till loading the XML
int n=0;

m_bFirstRun		= true;
m_bEnableRating = false; // <EnableRating>false</EnableRating>
m_bHardCutsDisabled = false; // <HardCutsDisabled>false</HardCutsDisabled>
m_nTexSizeX		= 1280; // <TexSize>1024</TexSize> - Width
m_nTexSizeY = 720; // Height
m_nGridX		= 32; // <MeshSize>32</MeshSize>
m_fBlendTimeAuto = 2.0f; // <BlendTimeAuto>2.000000</BlendTimeAuto>
m_fTimeBetweenPresets = 20.0f; // <TimeBetweenPresets>20.000000</TimeBetweenPresets>
m_fTimeBetweenPresetsRand = 6.0f; // <TimeBetweenPresetsRand>6.000000</TimeBetweenPresetsRand>
m_fHardCutLoudnessThresh = 2.5f; // <HardCutLoudnessThresh>2.500000</HardCutLoudnessThresh>
m_fHardCutHalflife = 60.0f; // <HardCutHalflife>60.000000</HardCutHalflife>

// Derived/calculated values
m_nGridY = m_nGridX*3/4; // 32*3/4 = 24

#endif	
	
	
//	LoadSettings(); //BRENT - TODO Like XBMC does - Move into wrapper
#ifdef _XBOX
		// Normal folder
      strcpy(m_szPresetDir,  "D:\\visualisations\\");
      strcat(m_szPresetDir,  "Milkdrop");
      strcat(m_szPresetDir,  "\\");
      strcat(m_szPresetDir,  /*setting.entry[setting.current]*/"default");
      strcat(m_szPresetDir, "\\");

	  return;
#endif

}

#include <iostream>
#include <string>
#include <cstring> // for strcpy

void StripComments(char* str)
{
    if (!str || !str[0] || !str[1])
        return;

    char c0 = str[0];
    char c1 = str[1];
    char* dest = str;
    char* p = &str[1];
    bool bIgnoreTilEndOfLine = false;
    bool bIgnoreTilCloseComment = false; //this one takes precedence
    int nCharsToSkip = 0;
    while (1)
    {
        // handle '//' comments
        if (!bIgnoreTilCloseComment && c0=='/' && c1=='/')
            bIgnoreTilEndOfLine = true;
        if (bIgnoreTilEndOfLine && (c0==10 || c0==13))
        {
            bIgnoreTilEndOfLine = false;
            nCharsToSkip = 0;
        }

        // handle /* */ comments
        if (!bIgnoreTilEndOfLine && c0=='/' && c1=='*')
            bIgnoreTilCloseComment = true;
        if (bIgnoreTilCloseComment && c0=='*' && c1=='/') 
        {
            bIgnoreTilCloseComment = false;
            nCharsToSkip = 2;
        }

        if (!bIgnoreTilEndOfLine && !bIgnoreTilCloseComment)
        {
            if (nCharsToSkip > 0)
                nCharsToSkip--;
            else
                *dest++ = c0;
        }

        if (c1==0)
            break;

        p++;
        c0 = c1;
        c1 = *p;
    }

    *dest++ = 0;
}

int CPlugin::AllocateMyNonDx9Stuff()
{
    // This gets called only once, when your plugin is actually launched.
    // If only the config panel is launched, this does NOT get called.
    //   (whereas MyPreInitialize() still does).
    // If anything fails here, return FALSE to safely exit the plugin,
    //   but only after displaying a messagebox giving the user some information
    //   about what went wrong.

    ZeroMemory(m_szShaderIncludeText, 32768 * sizeof(char)); // For raw C++ memory
    // read in 'm_szShaderIncludeText'
    bool bSuccess = true;
    bSuccess = ReadFileToString(L"D:\\data\\include.fx", m_szShaderIncludeText, sizeof(m_szShaderIncludeText)-4, false);
    if (bSuccess)
        StripComments(m_szShaderIncludeText);

    bool bIncludeValid = bSuccess && m_szShaderIncludeText[0] &&
                         strstr(m_szShaderIncludeText, "sampler_main") &&
                         strstr(m_szShaderIncludeText, "GetBlur1");

    if (!bIncludeValid)
    {
        static const char kFallbackInclude[] =
            "float4 _c0;\n"
            "float4 _c1;\n"
            "float4 _c2;\n"
            "float4 _c3;\n"
            "float4 _c4;\n"
            "float4 _c5;\n"
            "float4 _c6;\n"
            "float4 _c7;\n"
            "float4 _c8;\n"
            "float4 _c9;\n"
            "float4 _c10;\n"
            "float4 _c11;\n"
            "float4 _c12;\n"
            "float4 _c13;\n"
            "float4 _qa;\n"
            "float4 _qb;\n"
            "float4 _qc;\n"
            "float4 _qd;\n"
            "float4 _qe;\n"
            "float4 _qf;\n"
            "float4 _qg;\n"
            "float4 _qh;\n"
            "float4 rand_frame;\n"
            "float4 rand_preset;\n"
            "#define aspect _c0\n"
            "#define texsize _c7\n"
            "#define time _c2.x\n"
            "#define fps _c2.y\n"
            "#define frame _c2.z\n"
            "#define progress _c2.w\n"
            "#define bass _c3.x\n"
            "#define mid _c3.y\n"
            "#define treb _c3.z\n"
            "#define bass_att _c4.x\n"
            "#define mid_att _c4.y\n"
            "#define treb_att _c4.z\n"
            "sampler sampler_main;\n"
            "sampler sampler_fc_main;\n"
            "sampler sampler_blur1;\n"
            "sampler sampler_blur2;\n"
            "sampler sampler_blur3;\n"
            "float3 GetBlur1(float2 uv) { return tex2D(sampler_blur1, uv).rgb; }\n"
            "float3 GetBlur2(float2 uv) { return tex2D(sampler_blur2, uv).rgb; }\n"
            "float3 GetBlur3(float2 uv) { return tex2D(sampler_blur3, uv).rgb; }\n";
        lstrcpy(m_szShaderIncludeText, kFallbackInclude);
    }
    m_nShaderIncludeTextLen = lstrlen(m_szShaderIncludeText);

	ZeroMemory(&m_szDefaultWarpVShaderText, 32768 * sizeof(char));
	bSuccess |= ReadFileToString(L"D:\\data\\warp_vs.fx", m_szDefaultWarpVShaderText, sizeof(m_szDefaultWarpVShaderText), true);
    if (!bSuccess) return false;
	ZeroMemory(&m_szDefaultWarpPShaderText, 32768 * sizeof(char));
    bSuccess |= ReadFileToString(L"D:\\data\\warp_ps.fx", m_szDefaultWarpPShaderText, sizeof(m_szDefaultWarpPShaderText), true);
	if (!bSuccess) return false;
	ZeroMemory(&m_szDefaultCompVShaderText, 32768 * sizeof(char));
    bSuccess |= ReadFileToString(L"D:\\data\\comp_vs.fx", m_szDefaultCompVShaderText, sizeof(m_szDefaultCompVShaderText), true);
    if (!bSuccess) return false;
	ZeroMemory(&m_szDefaultCompPShaderText, 32768 * sizeof(char));
    bSuccess |= ReadFileToString(L"D:\\data\\comp_ps.fx", m_szDefaultCompPShaderText, sizeof(m_szDefaultCompPShaderText), true);
    if (!bSuccess) return false;
	ZeroMemory(&m_szBlurVS, 32768 * sizeof(char));
    bSuccess |= ReadFileToString(L"D:\\data\\blur_vs.fx", m_szBlurVS, sizeof(m_szBlurVS), true);
    if (!bSuccess) return false;
	ZeroMemory(&m_szBlurPSX, 32768 * sizeof(char));
    bSuccess |= ReadFileToString(L"D:\\data\\blur1_ps.fx", m_szBlurPSX, sizeof(m_szBlurPSX), true);
    if (!bSuccess) return false;
	ZeroMemory(&m_szBlurPSY, 32768 * sizeof(char));
    bSuccess |= ReadFileToString(L"D:\\data\\blur2_ps.fx", m_szBlurPSY, sizeof(m_szBlurPSY), true);
    if (!bSuccess) return false;

	m_pState->Default();
	m_pOldState->Default();
    m_pNewState->Default();

	//LoadRandomPreset(0.0f);   -avoid this here; causes some DX9 stuff to happen.

    return true;
}

//----------------------------------------------------------------------


//----------------------------------------------------------------------

float SquishToCenter(float x, float fExp)
{
    if (x > 0.5f)
        return powf(x*2-1, fExp)*0.5f + 0.5f;

    return (1-powf(1-x*2, fExp))*0.5f;
}

int GetNearestPow2Size(int w, int h)
{
	float fExp = logf( max(w,h)*0.75f + 0.25f*min(w,h) ) / logf(2.0f);
    float bias = 0.55f;
    if (fExp + bias >= 11.0f)   // ..don't jump to 2048x2048 quite as readily
        bias = 0.5f;
	int   nExp = (int)(fExp + bias);
	int log2size = (int)powf(2.0f, (float)nExp);
    return log2size;
}

int CPlugin::AllocateMyDX9Stuff()
{
    // (...aka OnUserResizeWindow) 
    // (...aka OnToggleFullscreen)
    
    // Allocate and initialize all your DX9 and D3DX stuff here: textures, 
    //   surfaces, vertex/index buffers, D3DX fonts, and so on.  
    // If anything fails here, return FALSE to safely exit the plugin,
    //   but only after displaying a messagebox giving the user some information
    //   about what went wrong.  If the error is NON-CRITICAL, you don't *have*
    //   to return; just make sure that the rest of the code will be still safely 
    //   run (albeit with degraded features).  
    // If you run out of video memory, you might want to show a short messagebox
    //   saying what failed to allocate and that the reason is a lack of video
    //   memory, and then call SuggestHowToFreeSomeMem(), which will show them
    //   a *second* messagebox that (intelligently) suggests how they can free up 
    //   some video memory.
    // Don't forget to account for each object you create/allocate here by cleaning
    //   it up in CleanUpMyDX9Stuff!
    // IMPORTANT:
    //   Note that the code here isn't just run at program startup!
    //   When the user toggles between fullscreen and windowed modes
    //   or resizes the window, the base class calls this function before 
    //   destroying & recreating the plugin window and DirectX object, and then
    //   calls AllocateMyDX9Stuff afterwards, to get your plugin running again.

//    wchar_t buf[32768], title[64];

    m_nFramesSinceResize = 0;

    int nNewCanvasStretch = (m_nCanvasStretch == 0) ? 100 : m_nCanvasStretch;

    DWORD PSVersion = GetCaps()->PixelShaderVersion & 0xFFFF;  // 0x0300, etc.
    if (PSVersion >= 0x0300) 
        m_nMaxPSVersion_DX9 = MD2_PS_3_0;
		else if (PSVersion > 0x0200) 
        m_nMaxPSVersion_DX9 = MD2_PS_2_X;
    else if (PSVersion >= 0x0200) 
        m_nMaxPSVersion_DX9 = MD2_PS_2_0;
    else 
        m_nMaxPSVersion_DX9 = MD2_PS_NONE;

    if (m_nMaxPSVersion_ConfigPanel == -1)
        m_nMaxPSVersion = m_nMaxPSVersion_DX9;
    else
    {
        // to still limit their choice by what HW reports:
        m_nMaxPSVersion = m_nMaxPSVersion_DX9;//min(m_nMaxPSVersion_DX9, m_nMaxPSVersion_ConfigPanel);// BRENT - Cuases it to shoot back to unconfig

        // to allow them to override:
//        m_nMaxPSVersion = m_nMaxPSVersion_ConfigPanel; // BRENT - Cuases it to shoot back to unconfig
    }

//    const char* szGPU = GetDriverDescription();
 
    // GREY LIST - slow ps_2_0 cards
    // In Canvas Stretch==Auto mode, for these cards, if they (claim to) run ps_2_0, 
    //   we run at half-res (cuz they're slow).
    // THE GENERAL GUIDELINE HERE:
    //   It should be at least as fast as a GeForce FX 5700 or my GeForce 6200 (TC)
    //   if it's to run without stretch.
/*
	if (m_nCanvasStretch==0)// && m_nMaxPSVersion_DX9 > 0)
    {
        // put cards on this list if you see them successfully run ps_2_0 (using override) 
        // and they run well at a low resolution (512x512 or less).
        if (
            strstr(szGPU, "GeForce 4"       ) ||    // probably not even ps_2_0
            strstr(szGPU, "GeForce FX 52"   ) ||    // chip's computer (FX 5200) - does do ps_2_0, but slow
            strstr(szGPU, "GeForce FX 53"   ) ||    
            strstr(szGPU, "GeForce FX 54"   ) ||    
            strstr(szGPU, "GeForce FX 55"   ) ||   //GeForce FX 5600 is 13 GB/s - 2.5x as fast as my 6200! 
            strstr(szGPU, "GeForce FX 56"   ) ||  
             //...GeForce FX 5700 and up, we let those run at full-res on ps_2_0...
            strstr(szGPU, "GeForce FX 56"   ) ||  
            strstr(szGPU, "GeForce FX 56"   ) ||  
            strstr(szGPU, "SiS 300/305/630/540/730") ||    // mom's computer - just slow.
            strstr(szGPU, "Radeon 8"   ) ||    // no shader model 2.
            strstr(szGPU, "Radeon 90"  ) ||    // from Valve.  no shader model 2.
            strstr(szGPU, "Radeon 91"  ) ||    // no shader model 2.
            strstr(szGPU, "Radeon 92"  ) ||    // from Valve.  no shader model 2.
            strstr(szGPU, "Radeon 93"  ) ||    // no shader model 2.
            strstr(szGPU, "Radeon 94"  ) ||    // no shader model 2.  
            // guessing that 9500+ are ok - they're all ps_2_0 and the 9600 is like an FX 5900.  
            strstr(szGPU, "Radeon 9550") ||  // *maybe* - kiv - super budget R200 chip.  def. ps_2_0 but possibly very slow.  
            strstr(szGPU, "Radeon X300") ||  // *maybe* - kiv - super budget R200 chip   def. ps_2_0 but possibly very slow.
            0)
        {
            nNewCanvasStretch = 200;        
        }
    }
*/
    // BLACK LIST
    // In Pixel Shaders==Auto mode, for these cards, we avoid ps_2_0 completely.
    // There shouldn't be much on this list... feel free to put anything you KNOW doesn't do ps_2_0 (why not),
    // and to put anything that is slow to begin with, and HAS BUGGY DRIVERS (INTEL).
/* 
	if (m_nMaxPSVersion_ConfigPanel==-1) 
    {
        if (strstr(szGPU, "GeForce2"     ) ||    // from Valve
            strstr(szGPU, "GeForce3"     ) ||    // from Valve
            strstr(szGPU, "GeForce4"     ) ||    // from Valve
            strstr(szGPU, "Radeon 7"     ) ||    // from Valve
            strstr(szGPU, "Radeon 8"     ) ||    
            strstr(szGPU, "SiS 661FX_760_741") ||    // from Valve
                //FOR NOW, FOR THESE, ASSUME INTEL EITHER DOESN'T DO PS_2_0, 
                //OR DRIVERS SUCK AND IT WOULDN'T WORK ANYWAY!
                (strstr(szGPU,"Intel") && strstr(szGPU,"945G")) ||
                (strstr(szGPU,"Intel") && strstr(szGPU,"915G")) ||  // ben allison's laptop - snow, freezing when you try ps_2_0
                (strstr(szGPU,"Intel") && strstr(szGPU,"910G")) ||
            (strstr(szGPU,"Intel") && strstr(szGPU,"8291")) ||     // gonna guess that this supports ps_2_0 but is SLOW
            (strstr(szGPU,"Intel") && strstr(szGPU,"8281")) ||     // definitely DOESN'T support pixel shaders
            (strstr(szGPU,"Intel") && strstr(szGPU,"8283")) ||     // definitely DOESN'T support pixel shaders
            (strstr(szGPU,"Intel") && strstr(szGPU,"8284")) ||     // definitely DOESN'T support pixel shaders
            (strstr(szGPU,"Intel") && strstr(szGPU,"8285")) ||     // definitely DOESN'T support pixel shaders
            (strstr(szGPU,"Intel") && strstr(szGPU,"8286")) ||     // definitely DOESN'T support pixel shaders.  Ben Allison's desktop (865) - no image w/ps_2_0.  Plus Nes's desktop - no ps_2_0.
            0) 
        {
            m_nMaxPSVersion = MD2_PS_NONE;
            //if (m_nCanvasStretch==0)
            //    nNewCanvasStretch = 100;        
        }
    }
*/
    // Note: this code used to be in OnResizeGraphicsWindow().

    // SHADERS
    //-------------------------------------

	if (m_nMaxPSVersion > MD2_PS_NONE)
    {
        // Create vertex declarations (since we're not using FVF anymore)
        if (D3D_OK != GetDevice()->CreateVertexDeclaration( g_MyVertDecl, &m_pMyVertDecl )) 
        {
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_CREATE_MY_VERTEX_DECLARATION,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,sizeof(title)), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
        }
        if (D3D_OK != GetDevice()->CreateVertexDeclaration( g_WfVertDecl, &m_pWfVertDecl ))
        {
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_CREATE_WF_VERTEX_DECLARATION,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,sizeof(title)), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
        }
        if (D3D_OK != GetDevice()->CreateVertexDeclaration( g_SpriteVertDecl, &m_pSpriteVertDecl ))
        {
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_CREATE_SPRITE_VERTEX_DECLARATION,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,sizeof(title)), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
		}

		// Load the FALLBACK shaders...
		if (!RecompilePShader(m_szDefaultWarpPShaderText, &m_fallbackShaders_ps.warp, SHADER_WARP, true, 2))
		{
/*			wchar_t szSM[64];
			switch(m_nMaxPSVersion_DX9)
			{
				case MD2_PS_2_0:
				case MD2_PS_2_X:
						WASABI_API_LNGSTRINGW_BUF(IDS_SHADER_MODEL_2,szSM,64); break;
				case MD2_PS_3_0: WASABI_API_LNGSTRINGW_BUF(IDS_SHADER_MODEL_3,szSM,64); break;
				case MD2_PS_4_0: WASABI_API_LNGSTRINGW_BUF(IDS_SHADER_MODEL_4,szSM,64); break;
				default:
					swprintf(szSM, WASABI_API_LNGSTRINGW(IDS_UKNOWN_CASE_X), m_nMaxPSVersion_DX9);
				break;
			}
			if (m_nMaxPSVersion_ConfigPanel >= MD2_PS_NONE && m_nMaxPSVersion_DX9 < m_nMaxPSVersion_ConfigPanel) 
					swprintf(buf, WASABI_API_LNGSTRINGW(IDS_FAILED_TO_COMPILE_PIXEL_SHADERS_USING_X),szSM,PSVersion);
				else
					swprintf(buf, WASABI_API_LNGSTRINGW(IDS_FAILED_TO_COMPILE_PIXEL_SHADERS_HARDWARE_MIS_REPORT),szSM,PSVersion);
				dumpmsg(buf); 
				MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
*/
			return false;
		}
		if (!RecompileVShader(m_szDefaultWarpVShaderText, &m_fallbackShaders_vs.warp, SHADER_WARP, true))
		{
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_FALLBACK_WV_SHADER,buf,sizeof(buf));
//			dumpmsg(buf); 
//			MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
			return false;
		}
		if (!RecompileVShader(m_szDefaultCompVShaderText, &m_fallbackShaders_vs.comp, SHADER_COMP, true))
		{
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_FALLBACK_CV_SHADER,buf,sizeof(buf));
//			dumpmsg(buf);
//			MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
			return false;
		}
		if (!RecompilePShader(m_szDefaultCompPShaderText, &m_fallbackShaders_ps.comp, SHADER_COMP, true, 2))
		{
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_FALLBACK_CP_SHADER,buf,sizeof(buf));
//			dumpmsg(buf); 
//			MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
			return false;
		}

        // Load the BLUR shaders...
        if (!RecompileVShader(m_szBlurVS, &m_BlurShaders[0].vs, SHADER_BLUR, true))
        {
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_BLUR1_VERTEX_SHADER,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
        }

        if (!RecompilePShader(m_szBlurPSX, &m_BlurShaders[0].ps, SHADER_BLUR, true, 2))
        {
//		    WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_BLUR1_PIXEL_SHADER,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
        }
        if (!RecompileVShader(m_szBlurVS, &m_BlurShaders[1].vs, SHADER_BLUR, true))
        {
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_BLUR2_VERTEX_SHADER,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
        }
        if (!RecompilePShader(m_szBlurPSY, &m_BlurShaders[1].ps, SHADER_BLUR, true, 2))
        {
//			WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_COMPILE_BLUR2_PIXEL_SHADER,buf,sizeof(buf));
//		    dumpmsg(buf); 
//		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		    return false;
        }
    }

	// create m_lpVS[2] 
    {
        int log2texsize = GetNearestPow2Size(GetWidth(), GetHeight());

        // auto-guess texsize
	    if (m_bTexSizeWasAutoExact)
		{
            // note: in windowed mode, the winamp window could be weird sizes, 
            //        so the plugin shell now gives us a slightly enlarged size,
            //        which pads it out to the nearest 32x32 block size,
            //        and then on display, it intelligently crops the image.
            //       This is pretty likely to work on non-shitty GPUs.
            //        but some shitty ones will still only do powers of 2!
            //       So if we are running out of video memory here or experience 
            //        other problems, though, we can make our VS's smaller;
            //        which will work, although it will lead to stretching.
			m_nTexSizeX = GetWidth();
			m_nTexSizeY = GetHeight();
		}
        else if (m_bTexSizeWasAutoPow2)
	    {
		    m_nTexSizeX = log2texsize;
			m_nTexSizeY = log2texsize;
	    }

	    // clip texsize by max. from caps
	    if ((DWORD)m_nTexSizeX > GetCaps()->MaxTextureWidth && GetCaps()->MaxTextureWidth>0)
		    m_nTexSizeX = GetCaps()->MaxTextureWidth;
	    if ((DWORD)m_nTexSizeY > GetCaps()->MaxTextureHeight && GetCaps()->MaxTextureHeight>0)
		    m_nTexSizeY = GetCaps()->MaxTextureHeight;

        // apply canvas stretch
        m_nTexSizeX = (m_nTexSizeX * 100)/nNewCanvasStretch;
        m_nTexSizeY = (m_nTexSizeY * 100)/nNewCanvasStretch;

        // re-compute closest power-of-2 size, now that we've factored in the stretching...
        log2texsize = GetNearestPow2Size(m_nTexSizeX, m_nTexSizeY);
        if (m_bTexSizeWasAutoPow2)
	    {
		    m_nTexSizeX = log2texsize;
			m_nTexSizeY = log2texsize;
	    }
	    
        // snap to 16x16 blocks
        m_nTexSizeX = ((m_nTexSizeX+15)/16)*16;
        m_nTexSizeY = ((m_nTexSizeY+15)/16)*16;

		// determine format for VS1/VS2
		D3DFORMAT fmt;
		switch(m_nTexBitsPerCh) {
		  case 5:  fmt = D3DFMT_R5G6B5   ; break;
		  case 8:  fmt = D3DFMT_X8R8G8B8 ; break;
		  case 10: fmt = D3DFMT_A2R10G10B10; break;  // D3DFMT_A2W10V10U10 or D3DFMT_A2R10G10B10 or D3DFMT_A2B10G10R10
		  case 16: fmt = D3DFMT_A16B16G16R16F; break; 
		  case 32: fmt = D3DFMT_A32B32G32R32F; break; //FIXME
		  default: fmt = D3DFMT_X8R8G8B8 ; break;
		}

	    // reallocate
	    bool bSuccess = false;
        DWORD vs_flags = D3DUSAGE_RENDERTARGET;// | D3DUSAGE_AUTOGENMIPMAP;//FIXME! (make automipgen optional)
		bool bRevertedBitDepth = false;

        // Create textures to store previous and current frames.
        // Use the same format as the internal render targets for Resolve compatibility.
        HRESULT hrPrev = D3DXCreateTexture(
            GetDevice(),
            m_nTexSizeX,
            m_nTexSizeY,
            1,
            vs_flags,
            fmt,
            D3DPOOL_DEFAULT,
            &m_lpPrevFrame
        );
        if (FAILED(hrPrev))
        {
            hrPrev = D3DXCreateTexture(
                GetDevice(),
                m_nTexSizeX,
                m_nTexSizeY,
                1,
                vs_flags,
                D3DFMT_X8R8G8B8,
                D3DPOOL_DEFAULT,
                &m_lpPrevFrame
            );
        }

        HRESULT hrCur = D3DXCreateTexture(
            GetDevice(),
            m_nTexSizeX,
            m_nTexSizeY,
            1,
            vs_flags,
            fmt,
            D3DPOOL_DEFAULT,
            &m_lpCurFrame
        );
        if (FAILED(hrCur))
        {
            hrCur = D3DXCreateTexture(
                GetDevice(),
                m_nTexSizeX,
                m_nTexSizeY,
                1,
                vs_flags,
                D3DFMT_X8R8G8B8,
                D3DPOOL_DEFAULT,
                &m_lpCurFrame
            );
        }


		do
	    {
		//    SafeRelease(m_lpVSold[0]);
		//    SafeRelease(m_lpVSold[1]);

		    // create VS1
            bSuccess = (GetDevice()->CreateTexture(m_nTexSizeX, m_nTexSizeY, 1, vs_flags, fmt, D3DPOOL_DEFAULT, &m_lpVSold[0], NULL) == D3D_OK);
			if (!bSuccess) 
			{
				bSuccess = (GetDevice()->CreateTexture(m_nTexSizeX, m_nTexSizeY, 1, vs_flags, /*GetBackBufFormat()*/D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &m_lpVSold[0], NULL) == D3D_OK);
				if (bSuccess)
					fmt = D3DFMT_X8R8G8B8;// GetBackBufFormat(); //BRENT
			}

			// create VS2
			if (bSuccess)
                bSuccess = (GetDevice()->CreateTexture(m_nTexSizeX, m_nTexSizeY, 1, vs_flags, fmt, D3DPOOL_DEFAULT, &m_lpVSold[1], NULL) == D3D_OK);

			if (!bSuccess) 
			{
				if (m_bTexSizeWasAutoExact) 
				{
                    if (m_nTexSizeX > 256 || m_nTexSizeY > 256) 
                    {
                        m_nTexSizeX /= 2;
                        m_nTexSizeY /= 2;
                        m_nTexSizeX = ((m_nTexSizeX+15)/16)*16;
                        m_nTexSizeY = ((m_nTexSizeY+15)/16)*16;
                    }
                    else
                    {
					    m_nTexSizeX = log2texsize;
					    m_nTexSizeY = log2texsize;
                        m_bTexSizeWasAutoExact = false;
					    m_bTexSizeWasAutoPow2 = true;
                    }
                }
				else if (m_bTexSizeWasAutoPow2) 
				{
                    if (m_nTexSizeX > 256) 
                    {
				        m_nTexSizeX /= 2;
				        m_nTexSizeY /= 2;
                    }
                    else
                        break;
				}
			}
	    }
	    while (!bSuccess);// && m_nTexSizeX >= 256 && (m_bTexSizeWasAutoExact || m_bTexSizeWasAutoPow2));

	    if (!bSuccess)
	    {
//            wchar_t buf[2048];
/*			UINT err_id = IDS_COULD_NOT_CREATE_INTERNAL_CANVAS_TEXTURE_NOT_ENOUGH_VID_MEM;
		    if (GetScreenMode() == FULLSCREEN)
				err_id = IDS_COULD_NOT_CREATE_INTERNAL_CANVAS_TEXTURE_SMALLER_DISPLAY;
		    else if (!(m_bTexSizeWasAutoExact || m_bTexSizeWasAutoPow2))
				err_id = IDS_COULD_NOT_CREATE_INTERNAL_CANVAS_TEXTURE_NOT_ENOUGH_VID_MEM_RECOMMENDATION;

			WASABI_API_LNGSTRINGW_BUF(err_id,buf,sizeof(buf));
		    dumpmsg(buf); 
		    MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );*/
		    return false;
	    }
        else 
        {
 //           swprintf(buf, WASABI_API_LNGSTRINGW(IDS_SUCCESSFULLY_CREATED_VS0_VS1), m_nTexSizeX, m_nTexSizeY, GetWidth(), GetHeight());
//            dumpmsg(buf);
        }

        // create blur textures w/same format.  A complete mip chain costs 33% more video mem then 1 full-sized VS.
        #if (NUM_BLUR_TEX>0)
            int w = m_nTexSizeX;
            int h = m_nTexSizeY;
            DWORD blurtex_flags = D3DUSAGE_RENDERTARGET;// | D3DUSAGE_AUTOGENMIPMAP;//FIXME! (make automipgen optional)
            for (int i=0; i<NUM_BLUR_TEX; i++) 
            {
                // main VS = 1024
                // blur0 = 512
                // blur1 = 256  <-  user sees this as "blur1"
                // blur2 = 128
                // blur3 = 128  <-  user sees this as "blur2"
                // blur4 =  64
                // blur5 =  64  <-  user sees this as "blur3"
                if (!(i&1) || (i<2))     
                {
                    w = max(16, w/2);
                    h = max(16, h/2);
                }
                int w2 = ((w+3)/16)*16;
                int h2 = ((h+3)/4)*4;
                bSuccess = (GetDevice()->CreateTexture(w2, h2, 1, blurtex_flags, fmt, D3DPOOL_DEFAULT, &m_lpBlur[i], NULL) == D3D_OK);
                m_nBlurTexW[i] = w2;
                m_nBlurTexH[i] = h2;
                if (!bSuccess) 
                {
                    m_nBlurTexW[i] = 1;
                    m_nBlurTexH[i] = 1;
//			        MessageBoxW(GetPluginWindow(), WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_CREATING_BLUR_TEXTURES,buf,sizeof(buf)),
//							   WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_WARNING,title,sizeof(title)), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
                    break;
                }

                // add it to m_textures[].  
                TexInfo x;  
				swprintf(x.texname, L"blur%d%s", i/2+1, (i%2) ? L"" : L"doNOTuseME");
                x.texptr  = m_lpBlur[i];
                //x.texsize_param = NULL;
                x.w = w2;
                x.h = h2;
                x.d = 1;
                x.bEvictable    = false;
                x.nAge          = m_nPresetsLoadedTotal;
                x.nSizeInBytes  = 0;
                m_textures.push_back(x);
            }
        #endif
    }

    m_fAspectX = (m_nTexSizeY > m_nTexSizeX) ? m_nTexSizeX/(float)m_nTexSizeY : 1.0f;
    m_fAspectY = (m_nTexSizeX > m_nTexSizeY) ? m_nTexSizeY/(float)m_nTexSizeX : 1.0f;
    m_fInvAspectX = 1.0f/m_fAspectX;
    m_fInvAspectY = 1.0f/m_fAspectY;


    // BUILD VERTEX LIST for final composite blit
	//   note the +0.5-texel offset! 
	//   (otherwise, a 1-pixel-wide line of the image would wrap at the top and left edges).
	ZeroMemory(m_comp_verts, sizeof(MYVERTEX)*FCGSX*FCGSY);
	//float fOnePlusInvWidth  = 1.0f + 1.0f/(float)GetWidth();
	//float fOnePlusInvHeight = 1.0f + 1.0f/(float)GetHeight();
    float fHalfTexelW =  0.5f / (float)GetWidth();   // 2.5: 2 pixels bad @ bottom right
    float fHalfTexelH =  0.5f / (float)GetHeight();
    float fDivX = 1.0f / (float)(FCGSX-2);
    float fDivY = 1.0f / (float)(FCGSY-2);
    for (int j=0; j<FCGSY; j++) 
    {
        int j2 = j - j/(FCGSY/2);
        float v = j2*fDivY;
        v = SquishToCenter(v, 3.0f);
        float sy = -((v-fHalfTexelH)*2-1);//fOnePlusInvHeight*v*2-1;
        for (int i=0; i<FCGSX; i++) 
        {
            int i2 = i - i/(FCGSX/2);
            float u = i2*fDivX;
            u = SquishToCenter(u, 3.0f);
            float sx = (u-fHalfTexelW)*2-1;//fOnePlusInvWidth*u*2-1;
            MYVERTEX* p = &m_comp_verts[i + j*FCGSX];
            p->x = sx;
            p->y = sy;
            p->z = 0;
            float rad, ang;
            UvToMathSpace( u, v, &rad, &ang );
                // fix-ups:
               if (i==FCGSX/2-1) {
                   if (j < FCGSY/2-1)
                       ang = 3.1415926535898f*1.5f;
                   else if (j == FCGSY/2-1)
                       ang = 3.1415926535898f*1.25f;
                   else if (j == FCGSY/2)
                       ang = 3.1415926535898f*0.75f;
                   else
                       ang = 3.1415926535898f*0.5f;
               }
               else if (i==FCGSX/2) {
                   if (j < FCGSY/2-1)
                       ang = 3.1415926535898f*1.5f;
                   else if (j == FCGSY/2-1)
                       ang = 3.1415926535898f*1.75f;
                   else if (j == FCGSY/2)
                       ang = 3.1415926535898f*0.25f;
                   else
                       ang = 3.1415926535898f*0.5f;
               }
               else if (j==FCGSY/2-1) {
                   if (i < FCGSX/2-1)
                       ang = 3.1415926535898f*1.0f;
                   else if (i == FCGSX/2-1)
                       ang = 3.1415926535898f*1.25f;
                   else if (i == FCGSX/2)
                       ang = 3.1415926535898f*1.75f;
                   else
                       ang = 3.1415926535898f*2.0f;
               }
               else if (j==FCGSY/2) {
                   if (i < FCGSX/2-1)
                       ang = 3.1415926535898f*1.0f;
                   else if (i == FCGSX/2-1)
                       ang = 3.1415926535898f*0.75f;
                   else if (i == FCGSX/2)
                       ang = 3.1415926535898f*0.25f;
                   else
                       ang = 3.1415926535898f*0.0f;
               }
            p->tu = u;
            p->tv = v;
            //p->tu_orig = u;
            //p->tv_orig = v;
            p->rad = rad;
            p->ang = ang;
            p->Diffuse = 0xFFFFFFFF;
        }
    }

    // build index list for final composite blit - 
    // order should be friendly for interpolation of 'ang' value!
    int* cur_index = &m_comp_indices[0];
    for (int y=0; y<FCGSY-1; y++) 
    {
        if (y==FCGSY/2-1)
            continue;
        for (int x=0; x<FCGSX-1; x++) 
        {
            if (x==FCGSX/2-1)
                continue;
            bool left_half = (x < FCGSX/2);
            bool top_half  = (y < FCGSY/2);
            bool center_4 = ((x==FCGSX/2 || x==FCGSX/2-1) && (y==FCGSY/2 || y==FCGSY/2-1));

            if ( ((int)left_half + (int)top_half + (int)center_4) % 2 ) 
            {
                *(cur_index+0) = (y  )*FCGSX + (x  );
                *(cur_index+1) = (y  )*FCGSX + (x+1);
                *(cur_index+2) = (y+1)*FCGSX + (x+1);
                *(cur_index+3) = (y+1)*FCGSX + (x+1);
                *(cur_index+4) = (y+1)*FCGSX + (x  );
                *(cur_index+5) = (y  )*FCGSX + (x  );
            }
            else 
            {
                *(cur_index+0) = (y+1)*FCGSX + (x  );
                *(cur_index+1) = (y  )*FCGSX + (x  );
                *(cur_index+2) = (y  )*FCGSX + (x+1);
                *(cur_index+3) = (y  )*FCGSX + (x+1);
                *(cur_index+4) = (y+1)*FCGSX + (x+1);
                *(cur_index+5) = (y+1)*FCGSX + (x  );
            }
            cur_index += 6;
        }
    }

	// reallocate the texture for font titles + custom msgs (m_lpDDSTitle)
	{
		m_nTitleTexSizeX = max(m_nTexSizeX, m_nTexSizeY);
		m_nTitleTexSizeY = m_nTitleTexSizeX/4;

		//dumpmsg("Init: [re]allocating title surface");

        // [DEPRECATED as of transition to dx9:] 
		// We could just create one title surface, but this is a problem because many
		// systems can only call DrawText on DDSCAPS_OFFSCREENPLAIN surfaces, and can NOT
		// draw text on a DDSCAPS_TEXTURE surface (it comes out garbled).  
		// So, we create one of each; we draw the text to the DDSCAPS_OFFSCREENPLAIN surface 
		// (m_lpDDSTitle[1]), then we blit that (once) to the DDSCAPS_TEXTURE surface 
		// (m_lpDDSTitle[0]), which can then be drawn onto the screen on polys.

        HRESULT hr;

		do
		{
			hr = GetDevice()->CreateTexture(m_nTitleTexSizeX, m_nTitleTexSizeY,1,D3DUSAGE_RENDERTARGET,D3DFMT_X8R8G8B8,D3DPOOL_DEFAULT, &m_lpDDSTitle, NULL);
			if (hr != D3D_OK)
			{
				if (m_nTitleTexSizeY < m_nTitleTexSizeX)
				{
					m_nTitleTexSizeY *= 2;
				}
				else
				{
					m_nTitleTexSizeX /= 2;
					m_nTitleTexSizeY /= 2;
				}
			}
		}
		while (hr != D3D_OK && m_nTitleTexSizeX > 16);

		if (hr != D3D_OK)
		{
			//dumpmsg("Init: -WARNING-: Title texture could not be created!");
            m_lpDDSTitle = NULL;
            //SafeRelease(m_lpDDSTitle);
			//return true;
		}
		else
		{
			//sprintf(buf, "Init: title texture size is %dx%d (ideal size was %dx%d)", m_nTitleTexSizeX, m_nTitleTexSizeY, m_nTexSize, m_nTexSize/4);
			//dumpmsg(buf);
//			m_supertext.bRedrawSuperText = true;
		}
	}

    // -----------------

    // create 'm_gdi_title_font_doublesize'
/*
    int songtitle_font_size = m_fontinfo[SONGTITLE_FONT].nSize * m_nTitleTexSizeX/256;
    if (songtitle_font_size<6) songtitle_font_size=6;
    if (!(m_gdi_title_font_doublesize = CreateFontW(songtitle_font_size, 0, 0, 0, m_fontinfo[SONGTITLE_FONT].bBold ? 900 : 400, 
		    m_fontinfo[SONGTITLE_FONT].bItalic, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, m_fontinfo[SONGTITLE_FONT].bAntiAliased ? ANTIALIASED_QUALITY : DEFAULT_QUALITY, DEFAULT_PITCH, m_fontinfo[SONGTITLE_FONT].szFace)))
    {
        MessageBoxW(NULL, WASABI_API_LNGSTRINGW(IDS_ERROR_CREATING_DOUBLE_SIZED_GDI_TITLE_FONT),
						 WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,sizeof(title)),
						 MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }

	if (pCreateFontW(	GetDevice(), 
						songtitle_font_size, 
						0, 
						m_fontinfo[SONGTITLE_FONT].bBold ? 900 : 400, 
						1,                       
						m_fontinfo[SONGTITLE_FONT].bItalic, 
						DEFAULT_CHARSET, 
						OUT_DEFAULT_PRECIS, 
						ANTIALIASED_QUALITY,//DEFAULT_QUALITY, 
						DEFAULT_PITCH,
						m_fontinfo[SONGTITLE_FONT].szFace, 
						&m_d3dx_title_font_doublesize
					) != D3D_OK)
    {
        MessageBoxW(GetPluginWindow(), WASABI_API_LNGSTRINGW(IDS_ERROR_CREATING_DOUBLE_SIZED_D3DX_TITLE_FONT),
				   WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,sizeof(title)), MB_OK|MB_SETFOREGROUND|MB_TOPMOST);
        return false;
    }
*/
    // -----------------

    m_texmgr.Init(GetDevice());

	//dumpmsg("Init: mesh allocation");
	m_verts      = new MYVERTEX[(m_nGridX+1)*(m_nGridY+1)];
	m_verts_temp = new MYVERTEX[(m_nGridX+2) * 4];
	m_vertinfo   = new td_vertinfo[(m_nGridX+1)*(m_nGridY+1)];
	m_indices_strip = new int[(m_nGridX+2)*(m_nGridY*2)];
	m_indices_list  = new int[m_nGridX*m_nGridY*6];
	if (!m_verts || !m_vertinfo)
	{
//		swprintf(buf, L"couldn't allocate mesh - out of memory");
//		dumpmsg(buf); 
//		MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		return false;
	}

	int nVert = 0;
	float texel_offset_x = 0.5f / (float)m_nTexSizeX;
	float texel_offset_y = 0.5f / (float)m_nTexSizeY;

	for (int y=0; y<=m_nGridY; y++)
	{
		for (int x=0; x<=m_nGridX; x++)
		{
			// precompute x,y,z
			m_verts[nVert].x = x/(float)m_nGridX*2.0f - 1.0f;
			m_verts[nVert].y = y/(float)m_nGridY*2.0f - 1.0f;
			m_verts[nVert].z = 0.0f;

			// precompute rad, ang, being conscious of aspect ratio
			m_vertinfo[nVert].rad = sqrtf(m_verts[nVert].x*m_verts[nVert].x*m_fAspectX*m_fAspectX + m_verts[nVert].y*m_verts[nVert].y*m_fAspectY*m_fAspectY);
			if (y==m_nGridY/2 && x==m_nGridX/2)
				m_vertinfo[nVert].ang = 0.0f;
			else
				m_vertinfo[nVert].ang = atan2f(m_verts[nVert].y*m_fAspectY, m_verts[nVert].x*m_fAspectX);
            m_vertinfo[nVert].a = 1;
            m_vertinfo[nVert].c = 0;

            m_verts[nVert].rad = m_vertinfo[nVert].rad;
            m_verts[nVert].ang = m_vertinfo[nVert].ang;
            m_verts[nVert].tu_orig =  m_verts[nVert].x*0.5f + 0.5f + texel_offset_x;
            m_verts[nVert].tv_orig = -m_verts[nVert].y*0.5f + 0.5f + texel_offset_y;

			nVert++;
		}
	}
	
    // generate triangle strips for the 4 quadrants.
    // each quadrant has m_nGridY/2 strips.
    // each strip has m_nGridX+2 *points* in it, or m_nGridX/2 polygons.
	int xref, yref;
	int nVert_strip = 0;
	for (int quadrant=0; quadrant<4; quadrant++)
	{
		for (int slice=0; slice < m_nGridY/2; slice++)
		{
			for (int i=0; i < m_nGridX + 2; i++)
			{
				// quadrants:	2 3
				//				0 1
				xref = i/2;
				yref = (i%2) + slice;

				if (quadrant & 1)
					xref = m_nGridX - xref;
				if (quadrant & 2)
					yref = m_nGridY - yref;

                int v = xref + (yref)*(m_nGridX+1);

				m_indices_strip[nVert_strip++] = v;
			}
		}
	}

    // also generate triangle lists for drawing the main warp mesh.
    int nVert_list = 0;
	for (int quadrant=0; quadrant<4; quadrant++)
	{
		for (int slice=0; slice < m_nGridY/2; slice++)
		{
			for (int i=0; i < m_nGridX/2; i++)
			{
				// quadrants:	2 3
				//				0 1
				xref = i;
				yref = slice;

				if (quadrant & 1)
					xref = m_nGridX-1 - xref;
				if (quadrant & 2)
					yref = m_nGridY-1 - yref;

                int v = xref + (yref)*(m_nGridX+1);

                m_indices_list[nVert_list++] = v;
                m_indices_list[nVert_list++] = v           +1;
                m_indices_list[nVert_list++] = v+m_nGridX+1  ;
                m_indices_list[nVert_list++] = v           +1;
                m_indices_list[nVert_list++] = v+m_nGridX+1  ;
                m_indices_list[nVert_list++] = v+m_nGridX+1+1;
			}
		}
	}

    // GENERATED TEXTURES FOR SHADERS
    //-------------------------------------
    if (m_nMaxPSVersion > 0)
    {
        // Generate noise textures
        if (!AddNoiseTex(L"noise_lq",      256, 1)) return false; 
        if (!AddNoiseTex(L"noise_lq_lite",  32, 1)) return false; 
        if (!AddNoiseTex(L"noise_mq",      256, 4)) return false;
        if (!AddNoiseTex(L"noise_hq",      256, 8)) return false; 

        if (!AddNoiseVol(L"noisevol_lq", 32, 1)) return false; 
        if (!AddNoiseVol(L"noisevol_hq", 32, 4)) return false; 
    }

    if (!m_bInitialPresetSelected)
    {
		UpdatePresetList(true); //...just does its initial burst!
        LoadRandomPreset(0.0f);
        m_bInitialPresetSelected = true;
    }
    else
        LoadShaders(&m_shaders, m_pState, false);  // Also force-load the shaders - otherwise they'd only get compiled on a preset switch.

	return true;
}

float fCubicInterpolate(float y0, float y1, float y2, float y3, float t)
{
   float a0,a1,a2,a3,t2;

   t2 = t*t;
   a0 = y3 - y2 - y0 + y1;
   a1 = y0 - y1 - a0;
   a2 = y2 - y0;
   a3 = y1;

   return(a0*t*t2+a1*t2+a2*t+a3);
}

DWORD dwCubicInterpolate(DWORD y0, DWORD y1, DWORD y2, DWORD y3, float t)
{
    // performs cubic interpolation on a D3DCOLOR value.
    DWORD ret = 0;
    DWORD shift = 0;
    for (int i=0; i<4; i++) 
    {
        float f = fCubicInterpolate(  
            ((y0 >> shift) & 0xFF)/255.0f, 
            ((y1 >> shift) & 0xFF)/255.0f, 
            ((y2 >> shift) & 0xFF)/255.0f, 
            ((y3 >> shift) & 0xFF)/255.0f, 
            t 
        );
        if (f<0)
            f = 0;
        if (f>1)
            f = 1;
        ret |= ((DWORD)(f*255)) << shift;
        shift += 8;
    }
    return ret;
}

LPDIRECT3DBASETEXTURE9 g_penisptr;

bool CPlugin::AddNoiseTex(const wchar_t* szTexName, int size, int zoom_factor)
{
    // size = width & height of the texture; 
    // zoom_factor = how zoomed-in the texture features should be.
    //           1 = random noise
    //           2 = smoothed (interp)
    //           4/8/16... = cubic interp.

//    wchar_t buf[2048], title[64];

    // Synthesize noise texture(s)
    LPDIRECT3DTEXTURE9 pNoiseTex = NULL;
    // try twice - once with mips, once without.
    for (int i=0; i<2; i++) 
    {
        if (D3D_OK != GetDevice()->CreateTexture(size, size, i, D3DPOOL_DEFAULT, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pNoiseTex, NULL))
        {
            if (i==1) 
            {
//				WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_CREATE_NOISE_TEXTURE,buf,sizeof(buf));
//		        dumpmsg(buf); 
//		        MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		        return false;
            }
        }
        else
            break;
    }

    D3DLOCKED_RECT r;
    if (D3D_OK != pNoiseTex->LockRect(0, &r, NULL, NULL))
    {
//		WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_LOCK_NOISE_TEXTURE,buf,sizeof(buf));
//		dumpmsg(buf); 
//		MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		return false;
    }

    if (r.Pitch < size*4)
    {
//		WASABI_API_LNGSTRINGW_BUF(IDS_NOISE_TEXTURE_BYTE_LAYOUT_NOT_RECOGNISED,buf,sizeof(buf));
//		dumpmsg(buf); 
//		MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		return false;
    }

    // write to the bits...
    DWORD* dst = (DWORD*)r.pBits;
    int dwords_per_line = r.Pitch / sizeof(DWORD);
    int RANGE = (zoom_factor > 1) ? 216 : 256;
    for (int y=0; y<size; y++) {
        LARGE_INTEGER q;
        QueryPerformanceCounter(&q);
        srand(q.LowPart ^ q.HighPart ^ warand());
        for (int x=0; x<size; x++) {
            dst[x] = (((DWORD)(warand() % RANGE)+RANGE/2) << 24) | 
                     (((DWORD)(warand() % RANGE)+RANGE/2) << 16) | 
                     (((DWORD)(warand() % RANGE)+RANGE/2) <<  8) | 
                     (((DWORD)(warand() % RANGE)+RANGE/2)      ); 
        }
        // swap some pixels randomly, to improve 'randomness'
        for (int x=0; x<size; x++) 
        {
            int x1 = (warand() ^ q.LowPart ) % size;
            int x2 = (warand() ^ q.HighPart) % size;
            DWORD temp = dst[x2];
            dst[x2] = dst[x1];
            dst[x1] = temp;
        }
        dst += dwords_per_line;
    }

    // smoothing
    if (zoom_factor > 1) 
    {
        // first go ACROSS, blending cubically on X, but only on the main lines.
        DWORD* dst = (DWORD*)r.pBits;
        for (int y=0; y<size; y+=zoom_factor)
            for (int x=0; x<size; x++) 
                if (x % zoom_factor)
                {
                    int base_x = (x/zoom_factor)*zoom_factor + size;
                    int base_y = y*dwords_per_line;
                    DWORD y0 = dst[ base_y + ((base_x - zoom_factor  ) % size) ];
                    DWORD y1 = dst[ base_y + ((base_x                ) % size) ];
                    DWORD y2 = dst[ base_y + ((base_x + zoom_factor  ) % size) ];
                    DWORD y3 = dst[ base_y + ((base_x + zoom_factor*2) % size) ];

                    float t = (x % zoom_factor)/(float)zoom_factor;

                    DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);
                    
                    dst[ y*dwords_per_line + x ] = result;        
                }
        
        // next go down, doing cubic interp along Y, on every line.
        for (int x=0; x<size; x++) 
            for (int y=0; y<size; y++)
                if (y % zoom_factor)
                {
                    int base_y = (y/zoom_factor)*zoom_factor + size;
                    DWORD y0 = dst[ ((base_y - zoom_factor  ) % size)*dwords_per_line + x ];
                    DWORD y1 = dst[ ((base_y                ) % size)*dwords_per_line + x ];
                    DWORD y2 = dst[ ((base_y + zoom_factor  ) % size)*dwords_per_line + x ];
                    DWORD y3 = dst[ ((base_y + zoom_factor*2) % size)*dwords_per_line + x ];

                    float t = (y % zoom_factor)/(float)zoom_factor;

                    DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);
                    
                    dst[ y*dwords_per_line + x ] = result;        
                }

    }

    // unlock texture
    pNoiseTex->UnlockRect(0);

	g_penisptr = pNoiseTex;

    // add it to m_textures[].  
    TexInfo x;  
    lstrcpyW(x.texname, szTexName);
    x.texptr  = pNoiseTex;
    //x.texsize_param = NULL;
    x.w = size;
    x.h = size;
    x.d = 1;
    x.bEvictable    = false;
    x.nAge          = m_nPresetsLoadedTotal;
    x.nSizeInBytes  = 0;
    m_textures.push_back(x);

    return true;
}

bool CPlugin::AddNoiseVol(const wchar_t* szTexName, int size, int zoom_factor)
{
    // size = width & height & depth of the texture; 
    // zoom_factor = how zoomed-in the texture features should be.
    //           1 = random noise
    //           2 = smoothed (interp)
    //           4/8/16... = cubic interp.

//    wchar_t buf[2048], title[64];

    // Synthesize noise texture(s)
    LPDIRECT3DVOLUMETEXTURE9 pNoiseTex = NULL;
    // try twice - once with mips, once without.
    // NO, TRY JUST ONCE - DX9 doesn't do auto mipgen w/volume textures.  (Debug runtime complains.)
    for (int i=1; i<2; i++) 
    {
        if (D3D_OK != GetDevice()->CreateVolumeTexture(size, size, size, i, D3DUSAGE_CPU_CACHED_MEMORY , D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pNoiseTex, NULL))
        {
            if (i==1) 
            {
//		        WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_CREATE_3D_NOISE_TEXTURE,buf,sizeof(buf));
//		        dumpmsg(buf); 
//		        MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		        return false;
            }
        }
        else
            break;
    }
    D3DLOCKED_BOX r;
    if (D3D_OK != pNoiseTex->LockBox(0, &r, NULL, NULL))
    {
//		WASABI_API_LNGSTRINGW_BUF(IDS_COULD_NOT_LOCK_3D_NOISE_TEXTURE,buf,sizeof(buf));
//		dumpmsg(buf); 
//		MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		return false;
    }
    if (r.RowPitch < size*4 || r.SlicePitch < size*size*4)
    {
//		WASABI_API_LNGSTRINGW_BUF(IDS_3D_NOISE_TEXTURE_BYTE_LAYOUT_NOT_RECOGNISED,buf,sizeof(buf));
//		dumpmsg(buf); 
//		MessageBoxW(GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
		return false;
    }
    // write to the bits...
    int dwords_per_slice = r.SlicePitch / sizeof(DWORD);
    int dwords_per_line = r.RowPitch / sizeof(DWORD);
    int RANGE = (zoom_factor > 1) ? 216 : 256;
    for (int z=0; z<size; z++) {
        DWORD* dst = (DWORD*)r.pBits + z*dwords_per_slice;
        for (int y=0; y<size; y++) {
            LARGE_INTEGER q;
            QueryPerformanceCounter(&q);
            srand(q.LowPart ^ q.HighPart ^ warand());
            for (int x=0; x<size; x++) {
                dst[x] = (((DWORD)(warand() % RANGE)+RANGE/2) << 24) | 
                         (((DWORD)(warand() % RANGE)+RANGE/2) << 16) | 
                         (((DWORD)(warand() % RANGE)+RANGE/2) <<  8) | 
                         (((DWORD)(warand() % RANGE)+RANGE/2)      ); 
            }
            // swap some pixels randomly, to improve 'randomness'
            for (int x=0; x<size; x++) 
            {
                int x1 = (warand() ^ q.LowPart ) % size;
                int x2 = (warand() ^ q.HighPart) % size;
                DWORD temp = dst[x2];
                dst[x2] = dst[x1];
                dst[x1] = temp;
            }
            dst += dwords_per_line;
        }
    }

    // smoothing
    if (zoom_factor > 1) 
    {
        // first go ACROSS, blending cubically on X, but only on the main lines.
        DWORD* dst = (DWORD*)r.pBits;
        for (int z=0; z<size; z+=zoom_factor)
            for (int y=0; y<size; y+=zoom_factor)
                for (int x=0; x<size; x++) 
                    if (x % zoom_factor)
                    {
                        int base_x = (x/zoom_factor)*zoom_factor + size;
                        int base_y = z*dwords_per_slice + y*dwords_per_line;
                        DWORD y0 = dst[ base_y + ((base_x - zoom_factor  ) % size) ];
                        DWORD y1 = dst[ base_y + ((base_x                ) % size) ];
                        DWORD y2 = dst[ base_y + ((base_x + zoom_factor  ) % size) ];
                        DWORD y3 = dst[ base_y + ((base_x + zoom_factor*2) % size) ];

                        float t = (x % zoom_factor)/(float)zoom_factor;

                        DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);
                    
                        dst[ z*dwords_per_slice + y*dwords_per_line + x ] = result;        
                    }
        
        // next go down, doing cubic interp along Y, on the main slices.
        for (int z=0; z<size; z+=zoom_factor)
            for (int x=0; x<size; x++) 
                for (int y=0; y<size; y++)
                    if (y % zoom_factor)
                    {
                        int base_y = (y/zoom_factor)*zoom_factor + size;
                        int base_z = z*dwords_per_slice;
                        DWORD y0 = dst[ ((base_y - zoom_factor  ) % size)*dwords_per_line + base_z + x ];
                        DWORD y1 = dst[ ((base_y                ) % size)*dwords_per_line + base_z + x ];
                        DWORD y2 = dst[ ((base_y + zoom_factor  ) % size)*dwords_per_line + base_z + x ];
                        DWORD y3 = dst[ ((base_y + zoom_factor*2) % size)*dwords_per_line + base_z + x ];

                        float t = (y % zoom_factor)/(float)zoom_factor;

                        DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);
                    
                        dst[ y*dwords_per_line + base_z + x ] = result;        
                    }

        // next go through, doing cubic interp along Z, everywhere.
        for (int x=0; x<size; x++) 
            for (int y=0; y<size; y++)
                for (int z=0; z<size; z++)
                    if (z % zoom_factor)
                    {
                        int base_y = y*dwords_per_line;
                        int base_z = (z/zoom_factor)*zoom_factor + size;
                        DWORD y0 = dst[ ((base_z - zoom_factor  ) % size)*dwords_per_slice + base_y + x ];
                        DWORD y1 = dst[ ((base_z                ) % size)*dwords_per_slice + base_y + x ];
                        DWORD y2 = dst[ ((base_z + zoom_factor  ) % size)*dwords_per_slice + base_y + x ];
                        DWORD y3 = dst[ ((base_z + zoom_factor*2) % size)*dwords_per_slice + base_y + x ];

                        float t = (z % zoom_factor)/(float)zoom_factor;

                        DWORD result = dwCubicInterpolate(y0, y1, y2, y3, t);
                    
                        dst[ z*dwords_per_slice + base_y + x ] = result;        
                    }

    }

    // unlock texture
    pNoiseTex->UnlockBox(0);

    // add it to m_textures[].  
    TexInfo x;  
    lstrcpyW(x.texname, szTexName);
    x.texptr  = pNoiseTex;
    //x.texsize_param = NULL;
    x.w = size;
    x.h = size;
    x.d = size;
    x.bEvictable    = false;
    x.nAge          = m_nPresetsLoadedTotal;
    x.nSizeInBytes  = 0;
    m_textures.push_back(x);

    return true;
}

void VShaderInfo::Clear() 
{ 
    SafeRelease(ptr); 
    SafeRelease(CT); 
    params.Clear(); 
}
void PShaderInfo::Clear() 
{ 
    SafeRelease(ptr); 
    SafeRelease(CT); 
    params.Clear(); 
}

// global_CShaderParams_master_list: a master list of all CShaderParams classes in existence.
//   ** when we evict a texture, we need to NULL out any texptrs these guys have! **
CShaderParamsList global_CShaderParams_master_list;  

CShaderParams::CShaderParams() {
    global_CShaderParams_master_list.push_back(this);
}

CShaderParams::~CShaderParams() {
    int N = global_CShaderParams_master_list.size();
    for (int i=0; i<N; i++)
        if (global_CShaderParams_master_list[i] == this)
            global_CShaderParams_master_list.eraseAt(i);

    texsize_params.clear();
}

void CShaderParams::OnTextureEvict(LPDIRECT3DBASETEXTURE9 texptr)
{
    for (int i=0; i<sizeof(m_texture_bindings)/sizeof(m_texture_bindings[0]); i++)
        if (m_texture_bindings[i].texptr == texptr)
            m_texture_bindings[i].texptr = NULL;
}

void CShaderParams::Clear() 
{
    // float4 handles:
    rand_frame  = NULL;
    rand_preset = NULL;

    ZeroMemory(rot_mat, sizeof(rot_mat));
    ZeroMemory(const_handles, sizeof(const_handles));
    ZeroMemory(q_const_handles, sizeof(q_const_handles));
    texsize_params.clear();

    // sampler stages for various PS texture bindings:
    for (int i=0; i<sizeof(m_texture_bindings)/sizeof(m_texture_bindings[0]); i++)
    {
        m_texture_bindings[i].texptr = NULL;
        m_texcode[i] = TEX_DISK;
    }
}

bool CPlugin::EvictSomeTexture()
{
    // note: this won't evict a texture whose age is zero,
    //       or whose reported size is zero!

    #if _DEBUG
    {
        int nEvictableFiles = 0;
        int nEvictableBytes = 0;
        int N = m_textures.size();
        for (int i=0; i<N; i++)
            if (m_textures[i].bEvictable && m_textures[i].texptr) 
            {
                nEvictableFiles++;
                nEvictableBytes += m_textures[i].nSizeInBytes;
            }
        char buf[1024];
        sprintf(buf, "evicting at %d textures, %.1f MB\n", nEvictableFiles, nEvictableBytes*0.000001f);
        OutputDebugString(buf);
    }
    #endif

    int N = m_textures.size();
    
    // find age gap
    int newest = 99999999;
    int oldest = 0;
    bool bAtLeastOneFound = false;
    for (int i=0; i<N; i++)                                                            
        if (m_textures[i].bEvictable && m_textures[i].nSizeInBytes>0 && m_textures[i].nAge < m_nPresetsLoadedTotal-1) // note: -1 here keeps images around for the blend-from preset, too...
        {
            newest = min(newest, m_textures[i].nAge);
            oldest = max(oldest, m_textures[i].nAge);
            bAtLeastOneFound = true;
        }
    if (!bAtLeastOneFound)
        return false;
    
    // find the "biggest" texture, but dilate things so that the newest textures
    // are HALF as big as the oldest textures, and thus, less likely to get booted.
    int biggest_bytes = 0;
    int biggest_index = -1;
    for (int i=0; i<N; i++)
        if (m_textures[i].bEvictable && m_textures[i].nSizeInBytes>0 && m_textures[i].nAge < m_nPresetsLoadedTotal-1) // note: -1 here keeps images around for the blend-from preset, too...
        {
            float size_mult = 1.0f + (m_textures[i].nAge - newest)/(float)(oldest-newest);
            int bytes = (int)(m_textures[i].nSizeInBytes * size_mult);
            if (bytes > biggest_bytes)
            {
                biggest_bytes = bytes;
                biggest_index = i;
            }            
        }
    if (biggest_index == -1)
        return false;

    
    // evict that sucker
    assert(m_textures[biggest_index].texptr);

    // notify all CShaderParams classes that we're releasing a bindable texture!!
    N = global_CShaderParams_master_list.size();
    for (int i=0; i<N; i++) 
        global_CShaderParams_master_list[i]->OnTextureEvict( m_textures[biggest_index].texptr );

    // 2. erase the texture itself
    SafeRelease(m_textures[biggest_index].texptr);
    m_textures.eraseAt(biggest_index);

    return true;
}

std::string texture_exts[] = { "jpg", "dds", "png", "tga", "bmp", "dib", };
const wchar_t szExtsWithSlashes[] = L"jpg|png|dds|etc.";
typedef Vector<GString> StringVec;

bool PickRandomTexture(const wchar_t* prefix, wchar_t* szRetTextureFilename)  //should be MAX_PATH chars
{
    static StringVec texfiles;
    static DWORD     texfiles_timestamp = 0;   // update this a max of every ~2 seconds or so

    // if it's been more than a few seconds since the last textures dir scan, redo it.  
    // (..just enough to make sure we don't do it more than once per preset load)
    //DWORD t = timeGetTime(); // in milliseconds
    //if (abs(t - texfiles_timestamp) > 2000)
if (g_pPlugin->m_bNeedRescanTexturesDir)
{
    g_pPlugin->m_bNeedRescanTexturesDir = false;
    texfiles.clear();

    char szMask[MAX_PATH];
    sprintf(szMask, "%stextures\\*.*", /*g_pPlugin->m_szMilkdrop2Path*/ "D:\\"); // Use narrow string

    WIN32_FIND_DATAA ffd = {0}; // Narrow version of WIN32_FIND_DATA
    HANDLE hFindFile = FindFirstFileA(szMask, &ffd); // Explicitly use narrow variant

    if (hFindFile == INVALID_HANDLE_VALUE)
        return false;

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        char* ext = strrchr(ffd.cFileName, '.'); // Narrow char*
        if (!ext)
            continue;

        // Ensure texture_exts uses narrow strings (e.g., "jpg", "png")
        for (int i = 0; i < sizeof(texture_exts)/sizeof(texture_exts[0]); i++)
        {
           // if (_stricmp(texture_exts[i].c_str(), ext + 1) == 0) // Narrow case-insensitive compare
            {
                // Convert narrow filename to wide string before storing
                wchar_t wideName[MAX_PATH];
                MultiByteToWideChar(CP_ACP, 0, ffd.cFileName, -1, wideName, MAX_PATH);
                texfiles.push_back(wideName); // Assuming texfiles stores std::wstring
                break;
            }
        }
    }
    while (FindNextFileA(hFindFile, &ffd)); // Narrow variant

    FindClose(hFindFile);
}

    if (texfiles.size() == 0)
        return false;

    // then randomly pick one
    if (prefix==NULL || prefix[0]==0) 
    {
        // pick randomly from entire list
        int i = warand() % texfiles.size();
        lstrcpyW(szRetTextureFilename, texfiles[i].c_str());
    }
    else
    {
        // only pick from files w/the right prefix
        StringVec temp_list;
        int N = texfiles.size();
        int len = lstrlenW(prefix);
        for (int i=0; i<N; i++) 
            if (!_wcsnicmp(prefix, texfiles[i].c_str(), len))
                temp_list.push_back(texfiles[i]);
        N = temp_list.size();
        if (N==0)
            return false;
        // pick randomly from the subset
        int i = warand() % temp_list.size();
        lstrcpyW(szRetTextureFilename, temp_list[i].c_str());
    }
    return true;
}

void CShaderParams::CacheParams(LPD3DXCONSTANTTABLE pCT, bool bHardErrors) 
{
    Clear();

    if (!pCT)
        return;

    D3DXCONSTANTTABLE_DESC d;
    pCT->GetDesc(&d);

    D3DXCONSTANT_DESC cd;

    #define MAX_RAND_TEX 16
    GString RandTexName[MAX_RAND_TEX];

    // pass 1: find all the samplers (and texture bindings).
    for (UINT i=0; i<d.Constants; i++) 
    {
        D3DXHANDLE h = pCT->GetConstant(NULL, i);
        unsigned int count = 1;
        pCT->GetConstantDesc(h, &cd, &count);

        // cd.Name          = VS_Sampler
        // cd.RegisterSet   = D3DXRS_SAMPLER
        // cd.RegisterIndex = 3
        if (cd.RegisterSet == D3DXRS_SAMPLER && cd.RegisterIndex >= 0 && cd.RegisterIndex < sizeof(m_texture_bindings)/sizeof(m_texture_bindings[0])) 
        {
            assert(m_texture_bindings[cd.RegisterIndex].texptr == NULL);

            // remove "sampler_" prefix to create root file name.  could still have "FW_" prefix or something like that.
            wchar_t szRootName[MAX_PATH];
            if (!strncmp(cd.Name, "sampler_", 8)) 
                lstrcpyW(szRootName, AutoWide(&cd.Name[8]));
            else
                lstrcpyW(szRootName, AutoWide(cd.Name));

            // also peel off "XY_" prefix, if it's there, to specify filtering & wrap mode.
            bool bBilinear = true;
            bool bWrap     = true;
            bool bWrapFilterSpecified = false;
            if (lstrlenW(szRootName) > 3 && szRootName[2]==L'_') 
            {
                wchar_t temp[3];
                temp[0] = szRootName[0];
                temp[1] = szRootName[1];
                temp[2] = 0;
                // convert to uppercase
                if (temp[0] >= L'a' && temp[0] <= L'z')
                    temp[0] -= L'a' - L'A';
                if (temp[1] >= L'a' && temp[1] <= L'z')
                    temp[1] -= L'a' - L'A';

                if      (!wcscmp(temp, L"FW")) { bWrapFilterSpecified = true; bBilinear = true;  bWrap = true; }
                else if (!wcscmp(temp, L"FC")) { bWrapFilterSpecified = true; bBilinear = true;  bWrap = false; }
                else if (!wcscmp(temp, L"PW")) { bWrapFilterSpecified = true; bBilinear = false; bWrap = true; }
                else if (!wcscmp(temp, L"PC")) { bWrapFilterSpecified = true; bBilinear = false; bWrap = false; }
                // also allow reverses:
                else if (!wcscmp(temp, L"WF")) { bWrapFilterSpecified = true; bBilinear = true;  bWrap = true; }
                else if (!wcscmp(temp, L"CF")) { bWrapFilterSpecified = true; bBilinear = true;  bWrap = false; }
                else if (!wcscmp(temp, L"WP")) { bWrapFilterSpecified = true; bBilinear = false; bWrap = true; }
                else if (!wcscmp(temp, L"CP")) { bWrapFilterSpecified = true; bBilinear = false; bWrap = false; }

                // peel off the prefix
                int i = 0;
                while (szRootName[i+3]) 
                {
                    szRootName[i] = szRootName[i+3];
                    i++;
                }
                szRootName[i] = 0;
            }
            m_texture_bindings[ cd.RegisterIndex ].bWrap     = bWrap;
            m_texture_bindings[ cd.RegisterIndex ].bBilinear = bBilinear;

            // if <szFileName> is "main", map it to the VS...
            if (!wcscmp(L"main", szRootName))
            {
                m_texture_bindings[ cd.RegisterIndex ].texptr    = NULL;
                m_texcode[ cd.RegisterIndex ] = TEX_VS;
            }
            else if (!wcscmp(L"fc_main", szRootName))
            {
                m_texture_bindings[ cd.RegisterIndex ].texptr    = NULL;
                m_texcode[ cd.RegisterIndex ] = TEX_VS;
            }
            #if (NUM_BLUR_TEX >= 2)
            else if (!wcscmp(L"blur1", szRootName))
            {
                m_texture_bindings[ cd.RegisterIndex ].texptr = g_pPlugin->m_lpBlur[1];
                m_texcode         [ cd.RegisterIndex ]        = TEX_BLUR1;
                if (!bWrapFilterSpecified) { // when sampling blur textures, default is CLAMP
                    m_texture_bindings[ cd.RegisterIndex ].bWrap = false;
                    m_texture_bindings[ cd.RegisterIndex ].bBilinear = true;
                }
            }
            #endif
            #if (NUM_BLUR_TEX >= 4)
                else if (!wcscmp(L"blur2", szRootName))
                {
                    m_texture_bindings[ cd.RegisterIndex ].texptr = g_pPlugin->m_lpBlur[3];
                    m_texcode         [ cd.RegisterIndex ]        = TEX_BLUR2;
                    if (!bWrapFilterSpecified) { // when sampling blur textures, default is CLAMP
                        m_texture_bindings[ cd.RegisterIndex ].bWrap = false;
                        m_texture_bindings[ cd.RegisterIndex ].bBilinear = true;
                    }
                }
            #endif
            #if (NUM_BLUR_TEX >= 6)
                else if (!wcscmp(L"blur3", szRootName))
                {
                    m_texture_bindings[ cd.RegisterIndex ].texptr    = g_pPlugin->m_lpBlur[5];
                    m_texcode         [ cd.RegisterIndex ]        = TEX_BLUR3;
                    if (!bWrapFilterSpecified) { // when sampling blur textures, default is CLAMP
                        m_texture_bindings[ cd.RegisterIndex ].bWrap = false;
                        m_texture_bindings[ cd.RegisterIndex ].bBilinear = true;
                    }
                }
            #endif
            #if (NUM_BLUR_TEX >= 8)
                else if (!wcscmp("blur4", szRootName))
                {
                    m_texture_bindings[ cd.RegisterIndex ].texptr    = g_plugin.m_lpBlur[7];
                    m_texcode         [ cd.RegisterIndex ]        = TEX_BLUR4;
                    if (!bWrapFilterSpecified) { // when sampling blur textures, default is CLAMP
                        m_texture_bindings[ cd.RegisterIndex ].bWrap = false;
                        m_texture_bindings[ cd.RegisterIndex ].bBilinear = true;
                    }
                }
            #endif
            #if (NUM_BLUR_TEX >= 10)
                else if (!wcscmp("blur5", szRootName))
                {
                    m_texture_bindings[ cd.RegisterIndex ].texptr    = g_plugin.m_lpBlur[9];
                    m_texcode         [ cd.RegisterIndex ]        = TEX_BLUR5;
                    if (!bWrapFilterSpecified) { // when sampling blur textures, default is CLAMP
                        m_texture_bindings[ cd.RegisterIndex ].bWrap = false;
                        m_texture_bindings[ cd.RegisterIndex ].bBilinear = true;
                    }
                }
            #endif
            #if (NUM_BLUR_TEX >= 12)
                else if (!wcscmp("blur6", szRootName))
                {
                    m_texture_bindings[ cd.RegisterIndex ].texptr    = g_plugin.m_lpBlur[11];
                    m_texcode         [ cd.RegisterIndex ]        = TEX_BLUR6;
                    if (!bWrapFilterSpecified) { // when sampling blur textures, default is CLAMP
                        m_texture_bindings[ cd.RegisterIndex ].bWrap = false;
                        m_texture_bindings[ cd.RegisterIndex ].bBilinear = true;
                    }
                }
            #endif
            else 
            {
                m_texcode[ cd.RegisterIndex ] = TEX_DISK;

                // check for request for random texture.
                if (!wcsncmp(L"rand", szRootName, 4) && 
                    IsNumericChar(szRootName[4]) && 
                    IsNumericChar(szRootName[5]) && 
                    (szRootName[6]==0 || szRootName[6]=='_') )
                {
                    int rand_slot = -1;
                    
                    // peel off filename prefix ("rand13_smalltiled", for example)
                    wchar_t prefix[MAX_PATH];
                    if (szRootName[6]==L'_')
                        lstrcpyW(prefix, &szRootName[7]);
                    else
                        prefix[0] = 0;
                    szRootName[6] = 0;

                    swscanf(&szRootName[4], L"%d", &rand_slot);
                    if (rand_slot >= 0 && rand_slot <= 15)      // otherwise, not a special filename - ignore it
                    {
                        if (!PickRandomTexture(prefix, szRootName))
                        {
                            if (prefix[0])
                                swprintf(szRootName, L"[rand%02d] %s*", rand_slot, prefix);
                            else
                                swprintf(szRootName, L"[rand%02d] *", rand_slot);
                        }
                        else 
                        {       
                            //chop off extension
                            wchar_t *p = wcsrchr(szRootName, L'.');
                            if (p)
                                *p = 0;
                        }

                        assert(RandTexName[rand_slot].GetLength() == 0);
                        RandTexName[rand_slot] = szRootName; // we'll need to remember this for texsize_ params!
                    }
                }

                // see if <szRootName>.tga or .jpg has already been loaded.
                //   (if so, grab a pointer to it)
                //   (if NOT, create & load it).
                int N = g_pPlugin->m_textures.size();
                for (int n=0; n<N; n++) {
                    if (!wcscmp(g_pPlugin->m_textures[n].texname, szRootName))
                    {
                        // found a match - texture was already loaded
                        m_texture_bindings[ cd.RegisterIndex ].texptr = g_pPlugin->m_textures[n].texptr;
                        // also bump its age down to zero! (for cache mgmt)
                        g_pPlugin->m_textures[n].nAge = g_pPlugin->m_nPresetsLoadedTotal;
						break;
                    }
                }
                // if still not found, load it up / make a new texture
                if (!m_texture_bindings[ cd.RegisterIndex ].texptr)
                {
                    TexInfo x;  
                    wcsncpy(x.texname, szRootName, 254);
                    x.texptr  = NULL;
                    //x.texsize_param = NULL;

                    // check if we need to evict anything from the cache, 
                    // due to our own cache constraints...
                    while (1)
                    {
                        int nTexturesCached = 0;
                        int nBytesCached = 0;
                        int N = g_pPlugin->m_textures.size();

                        for (int i=0; i<N; i++)
                            if (g_pPlugin->m_textures[i].bEvictable && g_pPlugin->m_textures[i].texptr)
                            {
                                nBytesCached += g_pPlugin->m_textures[i].nSizeInBytes;
                                nTexturesCached++;
                            }
                        if ( nTexturesCached < g_pPlugin->m_nMaxImages && 
                             nBytesCached <g_pPlugin->m_nMaxBytes )
                            break;
                        // otherwise, evict now - and loop until we are within the constraints
                        if (!g_pPlugin->EvictSomeTexture())
                            break; // or if there was nothing to evict, just give up
                    }

                    //load the texture
                    char szFilename[MAX_PATH];
                    for (int z=0; z<sizeof(texture_exts)/sizeof(texture_exts[0]); z++) 
                    {
						//sprintf(szFilename, "%stextures\\%s.%s", "D:\\", "mcode1", texture_exts[z].c_str());
                        sprintf(szFilename, "%stextures\\%S.%s", "D:\\", szRootName, texture_exts[z].c_str());

                        if (GetFileAttributes(szFilename) == 0xFFFFFFFF)
                        {
                            // try again, but in presets dir
                            sprintf(szFilename, "%s%s.%s", g_pPlugin->m_szPresetDir, szRootName, texture_exts[z].c_str());

                            if (GetFileAttributes(szFilename) == 0xFFFFFFFF)
								continue;
                        }
                        D3DXIMAGE_INFO desc;

                        // keep trying to load it - if it fails due to memory, evict something and try again.
                        while (1)
                        {

							HRESULT hr = D3DXCreateTextureFromFileExA(
							g_pPlugin->GetDevice(), 
							szFilename,
							D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT, 0, D3DFMT_DXT1,
							D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, &desc, NULL,
							(IDirect3DTexture9**)&x.texptr
							);

                            if (hr==D3DERR_OUTOFVIDEOMEMORY || hr==E_OUTOFMEMORY)
                            {
                                // out of memory - try evicting something old and/or big
                                if ( g_pPlugin->EvictSomeTexture())
                                    continue;
                            }

                            if (hr==ERROR_SUCCESS)
                            {
                                x.w = desc.Width;
                                x.h = desc.Height;
                                x.d = desc.Depth;
                                x.bEvictable    = true;
                                x.nAge          = g_pPlugin->m_nPresetsLoadedTotal;
                                int nPixels = desc.Width*desc.Height*max(1,desc.Depth);
                                int BitsPerPixel = GetDX9TexFormatBitsPerPixel(desc.Format);
                                x.nSizeInBytes  =  nPixels*BitsPerPixel/8 + 16384;  //plus some overhead
                            }
                            break;
                        }
                    }
                
                    if (!x.texptr)
                    {
//		                wchar_t buf[2048], title[64];
//                        swprintf(buf, WASABI_API_LNGSTRINGW(IDS_COULD_NOT_LOAD_TEXTURE_X), szRootName, szExtsWithSlashes);
//		                 g_pPlugin->dumpmsg(buf); 
//                        if (bHardErrors)
//		                    MessageBoxW(g_plugin.GetPluginWindow(), buf, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
//                        else {
//                            g_plugin.AddError(buf, 6.0f, ERR_PRESET, true);
//                        }
                        // Keep caching other samplers; missing disk textures should not
                        // prevent binding of built-in textures like sampler_main.
                        continue;
                    }

					g_pPlugin->m_textures.push_back(x);
                    m_texture_bindings[ cd.RegisterIndex ].texptr    = x.texptr;
                }
            }
        }
    }

    // pass 2: bind all the float4's.  "texsize_XYZ" params will be filled out via knowledge of loaded texture sizes.
    for (UINT i=0; i<d.Constants; i++) 
    {
        D3DXHANDLE h = pCT->GetConstant(NULL, i);
        unsigned int count = 1;
        pCT->GetConstantDesc(h, &cd, &count);

        if (cd.RegisterSet == D3DXRS_FLOAT4)
        {
            if (cd.Class == D3DXPC_MATRIX_COLUMNS) 
            {
                if      (!strcmp(cd.Name, "rot_s1" )) rot_mat[0]  = h;
                else if (!strcmp(cd.Name, "rot_s2" )) rot_mat[1]  = h;
                else if (!strcmp(cd.Name, "rot_s3" )) rot_mat[2]  = h;
                else if (!strcmp(cd.Name, "rot_s4" )) rot_mat[3]  = h;
                else if (!strcmp(cd.Name, "rot_d1" )) rot_mat[4]  = h;
                else if (!strcmp(cd.Name, "rot_d2" )) rot_mat[5]  = h;
                else if (!strcmp(cd.Name, "rot_d3" )) rot_mat[6]  = h;
                else if (!strcmp(cd.Name, "rot_d4" )) rot_mat[7]  = h;
                else if (!strcmp(cd.Name, "rot_f1" )) rot_mat[8]  = h;
                else if (!strcmp(cd.Name, "rot_f2" )) rot_mat[9]  = h;
                else if (!strcmp(cd.Name, "rot_f3" )) rot_mat[10] = h;
                else if (!strcmp(cd.Name, "rot_f4" )) rot_mat[11] = h;
                else if (!strcmp(cd.Name, "rot_vf1")) rot_mat[12] = h;
                else if (!strcmp(cd.Name, "rot_vf2")) rot_mat[13] = h;
                else if (!strcmp(cd.Name, "rot_vf3")) rot_mat[14] = h;
                else if (!strcmp(cd.Name, "rot_vf4")) rot_mat[15] = h;
                else if (!strcmp(cd.Name, "rot_uf1")) rot_mat[16] = h;
                else if (!strcmp(cd.Name, "rot_uf2")) rot_mat[17] = h;
                else if (!strcmp(cd.Name, "rot_uf3")) rot_mat[18] = h;
                else if (!strcmp(cd.Name, "rot_uf4")) rot_mat[19] = h;
                else if (!strcmp(cd.Name, "rot_rand1")) rot_mat[20] = h;
                else if (!strcmp(cd.Name, "rot_rand2")) rot_mat[21] = h;
                else if (!strcmp(cd.Name, "rot_rand3")) rot_mat[22] = h;
                else if (!strcmp(cd.Name, "rot_rand4")) rot_mat[23] = h;
            }
            else if (cd.Class == D3DXPC_VECTOR)
            {
                if      (!strcmp(cd.Name, "rand_frame"))  rand_frame  = h;
                else if (!strcmp(cd.Name, "rand_preset")) rand_preset = h;
                else if (!strncmp(cd.Name, "texsize_", 8)) 
                {
                    // remove "texsize_" prefix to find root file name.
                    wchar_t szRootName[MAX_PATH];
                    if (!strncmp(cd.Name, "texsize_", 8)) 
                        lstrcpyW(szRootName, AutoWide(&cd.Name[8]));
                    else
                        lstrcpyW(szRootName, AutoWide(cd.Name));

                    // check for request for random texture.
                    // it should be a previously-seen random index - just fetch/reuse the name.
                    if (!wcsncmp(L"rand", szRootName, 4) && 
                        IsNumericChar(szRootName[4]) && 
                        IsNumericChar(szRootName[5]) && 
                        (szRootName[6]==0 || szRootName[6]==L'_') )
                    {
                        int rand_slot = -1;

                        // ditch filename prefix ("rand13_smalltiled", for example)
                        // and just go by the slot
                        if (szRootName[6]==L'_')
                            szRootName[6] = 0;

                        swscanf(&szRootName[4], L"%d", &rand_slot);
                        if (rand_slot >= 0 && rand_slot <= 15)      // otherwise, not a special filename - ignore it
                            if (RandTexName[rand_slot].GetLength() > 0)
                                lstrcpyW(szRootName, RandTexName[rand_slot].c_str());
                    }

                    // see if <szRootName>.tga or .jpg has already been loaded.
                    bool bTexFound = false;
                    int N =  g_pPlugin->m_textures.size();
                    for (int n=0; n<N; n++) {
                        if (!wcscmp( g_pPlugin->m_textures[n].texname, szRootName))
                        {
                            // found a match - texture was loaded
                            TexSizeParamInfo y;
//                            y.texname       = szRootName; //for debugging
                            y.texsize_param = h;
                            y.w             =  g_pPlugin->m_textures[n].w;
                            y.h             =  g_pPlugin->m_textures[n].h;
                            texsize_params.push_back(y);
                            
                            bTexFound = true;
                            break;
                        }
                    }

                    if (!bTexFound)
                    {
						OutputDebugString("Fuck");
 //                       wchar_t buf[1024];
//			            swprintf(buf, WASABI_API_LNGSTRINGW(IDS_UNABLE_TO_RESOLVE_TEXSIZE_FOR_A_TEXTURE_NOT_IN_USE), cd.Name);
 //                       g_plugin.AddError(buf, 6.0f, ERR_PRESET, true);
                    }
                }
                else if (cd.Name[0] == '_' && cd.Name[1] == 'c') 
                {
                    int z;
                    if (sscanf(&cd.Name[2], "%d", &z)==1) 
                        if (z >= 0 && z < sizeof(const_handles)/sizeof(const_handles[0]))
                            const_handles[z] = h;
                }
                else if (cd.Name[0] == '_' && cd.Name[1] == 'q') 
                {
                    int z = cd.Name[2] - 'a';
                    if (z >= 0 && z < sizeof(q_const_handles)/sizeof(q_const_handles[0]))
                        q_const_handles[z] = h;
                }
            }
        }
    }
}

//----------------------------------------------------------------------

bool CPlugin::RecompileVShader(const char* szShadersText, VShaderInfo *si, int shaderType, bool bHardErrors)
{
    SafeRelease(si->ptr);
    ZeroMemory(si, sizeof(VShaderInfo));    
   
    // LOAD SHADER
    if (!LoadShaderFromMemory( szShadersText, "VS", "vs_1_1", &si->CT, (void**)&si->ptr, shaderType, bHardErrors && (GetScreenMode()==WINDOWED)))
        return false;

    // Track down texture & float4 param bindings for this shader.  
    // Also loads any textures that need loaded.
    si->params.CacheParams(si->CT, bHardErrors);

    return true;
}

bool CPlugin::RecompilePShader(const char* szShadersText, PShaderInfo *si, int shaderType, bool bHardErrors, int PSVersion)
{
    assert(m_nMaxPSVersion > 0);

    SafeRelease(si->ptr);
    ZeroMemory(si, sizeof(PShaderInfo));    
   
    // LOAD SHADER
    // note: ps_1_4 required for dependent texture lookups.
    //       ps_2_0 required for tex2Dbias.
		char ver[16];
		lstrcpy(ver, "ps_0_0");
		switch(PSVersion) {
		case MD2_PS_NONE: 
			// Even though the PRESET doesn't use shaders, if MilkDrop is running where it CAN do shaders,
			//   we run all the old presets through (shader) emulation.
			// This way, during a MilkDrop session, we are always calling either WarpedBlit() or WarpedBlit_NoPixelShaders(),
			//   and blending always works.
			lstrcpy(ver, "ps_2_0"); 
			break;  
		case MD2_PS_2_0: lstrcpy(ver, "ps_2_0"); break;
		case MD2_PS_2_X: lstrcpy(ver, "ps_2_a"); break; // we'll try ps_2_a first, LoadShaderFromMemory will try ps_2_b if compilation fails
		case MD2_PS_3_0: lstrcpy(ver, "ps_3_0"); break;
		case MD2_PS_4_0: lstrcpy(ver, "ps_4_0"); break;
		default: assert(0); break;
		}

    if (!LoadShaderFromMemory( szShadersText, "PS", ver, &si->CT, (void**)&si->ptr, shaderType, bHardErrors && (GetScreenMode()==WINDOWED))) 
        return false;

    // Track down texture & float4 param bindings for this shader.  
    // Also loads any textures that need loaded.
    si->params.CacheParams(si->CT, bHardErrors);

    return true;
}

bool CPlugin::LoadShaders(PShaderSet* sh, CState* pState, bool bTick)
{
    if (m_nMaxPSVersion <= 0)
        return true;
    
    // load one of the pixel shaders
    if (!sh->warp.ptr && pState->m_nWarpPSVersion > 0)
    {
        int warpPS = min(pState->m_nWarpPSVersion, m_nMaxPSVersion);
        bool bOK = RecompilePShader(pState->m_szWarpShadersText, &sh->warp, SHADER_WARP, false, warpPS);
        if (!bOK) 
        {
            // switch to fallback shader
            m_fallbackShaders_ps.warp.ptr->AddRef();
            m_fallbackShaders_ps.warp.CT->AddRef();
            memcpy(&sh->warp, &m_fallbackShaders_ps.warp, sizeof(PShaderInfo));
            // cancel any slow-preset-load
            //m_nLoadingPreset = 1000;
        }

        if (bTick)
            return true;
    }

    if (!sh->comp.ptr && pState->m_nCompPSVersion > 0)
    {
        int compPS = min(pState->m_nCompPSVersion, m_nMaxPSVersion);
        bool bOK = RecompilePShader(pState->m_szCompShadersText, &sh->comp, SHADER_COMP, false, compPS);
        if (!bOK)
        {
            // switch to fallback shader
            m_fallbackShaders_ps.comp.ptr->AddRef();
            m_fallbackShaders_ps.comp.CT->AddRef();
            memcpy(&sh->comp, &m_fallbackShaders_ps.comp, sizeof(PShaderInfo));
            // cancel any slow-preset-load
            //m_nLoadingPreset = 1000;
        }
    }

    return true;
}

//----------------------------------------------------------------------

bool CPlugin::LoadShaderFromMemory( const char* szOrigShaderText, char* szFn, char* szProfile, 
                                    LPD3DXCONSTANTTABLE* ppConstTable, void** ppShader, int shaderType, bool bHardErrors )
{
    const char szWarpDefines[] = "#define rad _rad_ang.x\n"
                                 "#define ang _rad_ang.y\n"
                                 "#define uv _uv.xy\n"
                                 "#define uv_orig _uv.zw\n";
    const char szCompDefines[] = "#define rad _rad_ang.x\n"
                                 "#define ang _rad_ang.y\n"
                                 "#define uv _uv.xy\n"
                                 "#define uv_orig _uv.xy\n" //[sic]
                                 "#define hue_shader _vDiffuse.xyz\n";
    const char szWarpParams[] = "float4 _vDiffuse : COLOR, float4 _uv : TEXCOORD0, float2 _rad_ang : TEXCOORD1, out float4 _return_value : COLOR0";
    const char szCompParams[] = "float4 _vDiffuse : COLOR, float2 _uv : TEXCOORD0, float2 _rad_ang : TEXCOORD1, out float4 _return_value : COLOR0";
    const char szFirstLine[]  = "    float3 ret = 0;";
    const char szLastLine[]   = "    _return_value = float4(ret.xyz, _vDiffuse.w);";

    char szWhichShader[64];
    switch(shaderType)
    {
    case SHADER_WARP:  lstrcpy(szWhichShader, "warp"); break;
    case SHADER_COMP:  lstrcpy(szWhichShader, "composite"); break;
    case SHADER_BLUR:  lstrcpy(szWhichShader, "blur"); break;
    case SHADER_OTHER: lstrcpy(szWhichShader, "(other)"); break;
    default:           lstrcpy(szWhichShader, "(unknown)"); break;
    }

    LPD3DXBUFFER pShaderByteCode;
//    wchar_t title[64];
    
    *ppShader = NULL;
    *ppConstTable = NULL;

    char szShaderText[92800];
    char temp[92800];
    int writePos = 0;

    // paste the universal #include
    lstrcpy(&szShaderText[writePos], m_szShaderIncludeText);  // first, paste in the contents of 'inputs.fx' before the actual shader text.  Has 13's and 10's.
    writePos += m_nShaderIncludeTextLen;

    // paste in any custom #defines for this shader type
    if (shaderType == SHADER_WARP && szProfile[0]=='p') 
    {
        lstrcpy(&szShaderText[writePos], szWarpDefines);
        writePos += lstrlen(szWarpDefines);
    }
    else if (shaderType == SHADER_COMP && szProfile[0]=='p')
    {
        lstrcpy(&szShaderText[writePos], szCompDefines);
        writePos += lstrlen(szCompDefines);
    }

    // paste in the shader itself - converting LCC's to 13+10's.
    // avoid lstrcpy b/c it might not handle the linefeed stuff...?
    int shaderStartPos = writePos;
    {
        const char *s = szOrigShaderText;
        char *d = &szShaderText[writePos];
        while (*s)
        {
            if (*s == LINEFEED_CONTROL_CHAR)
            {
                *d++ = 13; writePos++;
                *d++ = 10; writePos++;
            }
            else
            {
                *d++ = *s; writePos++;
            }
            s++;
        }
        *d = 0; writePos++;
    }

    // strip out all comments - but cheat a little - start at the shader test.
    // (the include file was already stripped of comments)
    StripComments(&szShaderText[shaderStartPos]);

    /*{
        char* p = szShaderText;
        while (*p)
        {
            char buf[32];
            buf[0] = *p;
            buf[1] = 0;
            OutputDebugString(buf);
            if ((rand() % 9) == 0)
                Sleep(1);
            p++;
        }
        OutputDebugString("\n");
    }/**/

    //note: only do this stuff if type is WARP or COMP shader... not for blur, etc!
    //FIXME - hints on the inputs / output / samplers etc.
    //   can go in the menu header, NOT the preset!  =)
    //then update presets
    //  -> be sure to update the presets on disk AND THE DEFAULT SHADERS (for loading MD1 presets)
    //FIXME - then update auth. guide w/new examples,
    //   and a list of the invisible inputs (and one output) to each shader!
    //   warp: float2 uv, float2 uv_orig, rad, ang
    //   comp: float2 uv, rad, ang, float3 hue_shader
    // test all this string code in Debug mode - make sure nothing bad is happening

    /*
    1. paste warp or comp #defines
    2. search for "void" + whitespace + szFn + [whitespace] + '(' 
    3. insert params
    4. search for [whitespace] + ')'.
    5. search for final '}' (strrchr)
    6. back up one char, insert the Last Line, and add '}' and that's it.
    */
    if ((shaderType == SHADER_WARP || shaderType == SHADER_COMP) && szProfile[0]=='p')
    {
        char* p = &szShaderText[shaderStartPos];
        
        // seek to 'shader_body' and replace it with spaces
        while (*p && strncmp(p, "shader_body", 11))
            p++;
        if (p) 
        {
            for (int i=0; i<11; i++)
                *p++ = ' ';
        }

        if (p)
        {
            // insert "void PS(...params...)\n"
            lstrcpy(temp, p);
            const char *params = (shaderType==SHADER_WARP) ? szWarpParams : szCompParams;
            sprintf(p, "void %s( %s )\n", szFn, params);
            p += lstrlen(p);
            lstrcpy(p, temp);

            // find the starting curly brace
            p = strchr(p, '{');
            if (p)
            {
                // skip over it
                p++;
                // then insert "float3 ret = 0;"
                lstrcpy(temp, p);
                sprintf(p, "%s\n", szFirstLine);
                p += lstrlen(p);
                lstrcpy(p, temp);

                // find the ending curly brace
                p = strrchr(p, '}');
                // add the last line - "    _return_value = float4(ret.xyz, _vDiffuse.w);"
                if (p)
                    sprintf(p, " %s\n}\n", szLastLine);
            }
        }

        if (!p)
        {
//			wchar_t temp[512];
 //           swprintf(temp, WASABI_API_LNGSTRINGW(IDS_ERROR_PARSING_X_X_SHADER), szProfile, szWhichShader);
//		    dumpmsg(temp);
 //           AddError(temp, 8.0f, ERR_PRESET, true);
		    return false;
        }
    }
    
    // now really try to compile it.

    int len = lstrlen(szShaderText);

    bool failed=false;
    if (D3D_OK != D3DXCompileShader(
        szShaderText,
        len,
        NULL,//CONST D3DXMACRO* pDefines,
        NULL,//LPD3DXINCLUDE pInclude,
        szFn,
        szProfile,
        D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY  ,
        &pShaderByteCode,
				&m_pShaderCompileErrors,
				ppConstTable
				)) 
		{
			failed=true;
		}

		// before we totally fail, let's try using ps_2_b instead of ps_2_a
		if (failed && !strcmp(szProfile, "ps_2_a"))
		{
			SafeRelease(m_pShaderCompileErrors);
			if (D3D_OK == D3DXCompileShader(szShaderText, len, NULL, NULL, szFn,
				"ps_2_b", m_dwShaderFlags, &pShaderByteCode, &m_pShaderCompileErrors, ppConstTable))
			{
				failed=false;
			}
		}

		if (failed)
		{
/*			wchar_t temp[1024];
			swprintf(temp, WASABI_API_LNGSTRINGW(IDS_ERROR_COMPILING_X_X_SHADER), szProfile, szWhichShader);
			if (m_pShaderCompileErrors && m_pShaderCompileErrors->GetBufferSize() < sizeof(temp) - 256) 
			{
				lstrcatW(temp, L"\n\n");
				lstrcatW(temp, AutoWide((char*)m_pShaderCompileErrors->GetBufferPointer()));
			}
			SafeRelease(m_pShaderCompileErrors);
			dumpmsg(temp);
			if (bHardErrors)
				MessageBoxW(GetPluginWindow(), temp, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
			else {
				AddError(temp, 8.0f, ERR_PRESET, true);
			}*/
			return false;
		}
        else
        {
        }

    HRESULT hr = 1;
    if (szProfile[0] == 'v') 
    {
        hr = GetDevice()->CreateVertexShader((const unsigned long *)(pShaderByteCode->GetBufferPointer()), (IDirect3DVertexShader9**)ppShader);
    }
    else if (szProfile[0] == 'p') 
    {
        hr = GetDevice()->CreatePixelShader((const unsigned long *)(pShaderByteCode->GetBufferPointer()), (IDirect3DPixelShader9**)ppShader);
    }

    if (hr != D3D_OK)
    {
/*		wchar_t temp[512];
        WASABI_API_LNGSTRINGW_BUF(IDS_ERROR_CREATING_SHADER,temp,sizeof(temp));
		dumpmsg(temp); 
        if (bHardErrors)
		    MessageBoxW(GetPluginWindow(), temp, WASABI_API_LNGSTRINGW_BUF(IDS_MILKDROP_ERROR,title,64), MB_OK|MB_SETFOREGROUND|MB_TOPMOST );
        else {
            AddError(temp, 6.0f, ERR_PRESET, true);
        }*/
		return false;
    }

    pShaderByteCode->Release();

    return true;
}

//----------------------------------------------------------------------

void CPlugin::CleanUpMyDX9Stuff(int final_cleanup)
{
    // Clean up all your DX9 and D3DX textures, fonts, buffers, etc. here.
    // EVERYTHING CREATED IN ALLOCATEMYDX9STUFF() SHOULD BE CLEANED UP HERE.
    // The input parameter, 'final_cleanup', will be 0 if this is 
    //   a routine cleanup (part of a window resize or switch between
    //   fullscr/windowed modes), or 1 if this is the final cleanup
    //   and the plugin is exiting.  Note that even if it is a routine
    //   cleanup, *you still have to release ALL your DirectX stuff,
    //   because the DirectX device is being destroyed and recreated!*
    // Also set all the pointers back to NULL;
    //   this is important because if we go to reallocate the DX9
    //   stuff later, and something fails, then CleanUp will get called,
    //   but it will then be trying to clean up invalid pointers.)
    // The SafeRelease() and SafeDelete() macros make your code prettier;
    //   they are defined here in utility.h as follows:
    //       #define SafeRelease(x) if (x) {x->Release(); x=NULL;}
    //       #define SafeDelete(x)  if (x) {delete x; x=NULL;} 
    // IMPORTANT:
    //   This function ISN'T only called when the plugin exits!
    //   It is also called whenever the user toggles between fullscreen and 
    //   windowed modes, or resizes the window.  Basically, on these events, 
    //   the base class calls CleanUpMyDX9Stuff before Reset()ing the DirectX 
    //   device, and then calls AllocateMyDX9Stuff afterwards.



    // One funky thing here: if we're switching between fullscreen and windowed,
    //  or doing any other thing that causes all this stuff to get reloaded in a second,
    //  then if we were blending 2 presets, jump fully to the new preset.
    // Otherwise the old preset wouldn't get all reloaded, and it app would crash
    //  when trying to use its stuff.
    if (m_nLoadingPreset != 0) {
        // finish up the pre-load & start the official blend
        m_nLoadingPreset = 8;
        LoadPresetTick();        
    }
    // just force this:
    m_pState->m_bBlending = false;



    for (size_t i=0; i<m_textures.size(); i++) 
        if (m_textures[i].texptr)
        {
            // notify all CShaderParams classes that we're releasing a bindable texture!!
            size_t N = global_CShaderParams_master_list.size();
            for (size_t j=0; j<N; j++) 
                global_CShaderParams_master_list[j]->OnTextureEvict( m_textures[i].texptr );

            SafeRelease(m_textures[i].texptr);
        }
    m_textures.clear();

    // DON'T RELEASE blur textures - they were already released because they're in m_textures[].
    #if (NUM_BLUR_TEX>0)
        for (int i=0; i<NUM_BLUR_TEX; i++)
            m_lpBlur[i] = NULL;//SafeRelease(m_lpBlur[i]);
    #endif

    // NOTE: not necessary; shell does this for us.
    /*if (GetDevice())
    {
        GetDevice()->SetTexture(0, NULL);
        GetDevice()->SetTexture(1, NULL);
    }*/

    SafeRelease(m_pSpriteVertDecl);
    SafeRelease(m_pWfVertDecl);
    SafeRelease(m_pMyVertDecl);

    m_shaders.comp.Clear();
    m_shaders.warp.Clear();
    m_OldShaders.comp.Clear();
    m_OldShaders.warp.Clear();
    m_NewShaders.comp.Clear();
    m_NewShaders.warp.Clear();
    m_fallbackShaders_vs.comp.Clear();
    m_fallbackShaders_ps.comp.Clear();
    m_fallbackShaders_vs.warp.Clear();
    m_fallbackShaders_ps.warp.Clear();
    m_BlurShaders[0].vs.Clear();
    m_BlurShaders[0].ps.Clear();
    m_BlurShaders[1].vs.Clear();
    m_BlurShaders[1].ps.Clear();

    SafeRelease( m_pShaderCompileErrors );

    SafeRelease(m_lpPrevFrame);
    SafeRelease(m_lpCurFrame);

    // 2. release stuff
  //  SafeRelease(m_lpVSold[0]);
   // SafeRelease(m_lpVSold[1]);
    m_texmgr.Finish();

	if (m_verts != NULL)
	{
		delete[] m_verts;
		m_verts = NULL;
	}

	if (m_verts_temp != NULL)
	{
		delete[] m_verts_temp;
		m_verts_temp = NULL;
	}

	if (m_vertinfo != NULL)
	{
		delete[] m_vertinfo;
		m_vertinfo = NULL;
	}

	if (m_indices_list != NULL)
	{
		delete[] m_indices_list;
		m_indices_list = NULL;
	}

    if (m_indices_strip != NULL)
	{
		delete[] m_indices_strip;
		m_indices_strip = NULL;
	}

    ClearErrors();

    // This setting is closely tied to the modern skin "random" button.
    // The "random" state should be preserved from session to session.
    // It's pretty safe to do, because the Scroll Lock key is hard to
    //   accidentally click... :)
}

//----------------------------------------------------------------------

void CPlugin::MyRenderFn(int redraw)
{
    // Render a frame of animation here.  
    // This function is called each frame just AFTER BeginScene().
    // For timing information, call 'GetTime()' and 'GetFps()'.
    // The usual formula is like this (but doesn't have to be):
    //   1. take care of timing/other paperwork/etc. for new frame
    //   2. clear the background
    //   3. get ready for 3D drawing
    //   4. draw your 3D stuff
    //   5. call PrepareFor2DDrawing()
    //   6. draw your 2D stuff (overtop of your 3D scene)
    // If the 'redraw' flag is 1, you should try to redraw
    //   the last frame; GetTime, GetFps, and GetFrame should
    //   all return the same values as they did on the last 
    //   call to MyRenderFn().  Otherwise, the redraw flag will
    //   be zero, and you can draw a new frame.  The flag is
    //   used to force the desktop to repaint itself when 
    //   running in desktop mode and Winamp is paused or stopped.

    //   1. take care of timing/other paperwork/etc. for new frame
    if (!redraw)
    {
        float dt = GetTime() - m_prev_time;
        m_prev_time = GetTime(); // note: m_prev_time is not for general use!
        m_bPresetLockedByCode = (m_UI_mode != UI_REGULAR);
        if (m_bPresetLockedByUser || m_bPresetLockedByCode)
        {
            // to freeze time (at current preset time value) when menus are up or Scroll Lock is on:
		    //m_fPresetStartTime += dt;
		    //m_fNextPresetTime += dt;
            // OR, to freeze time @ [preset] zero, so that when you exit menus,
            //   you don't run the risk of it changing the preset on you right away:
            m_fPresetStartTime = GetTime();
        	m_fNextPresetTime = -1.0f;		// flags UpdateTime() to recompute this.
        }

        //if (!m_bPresetListReady)
        //    UpdatePresetList(true);//UpdatePresetRatings(); // read in a few each frame, til they're all in
    }

    // 2. check for lost or gained kb focus:
    // (note: can't use wm_setfocus or wm_killfocus because they don't work w/embedwnd)
    if (GetFrame()==0)
    {
        // NOTE: we skip this if we've already gotten a WM_COMMAND/ID_VIS_RANDOM message
        //       from the skin - if that happened, we're running windowed with a fancy
        //       skin with a 'rand' button.



        // make sure the 'random' button on the skin shows the right thing:
        // NEVERMIND - if it's a fancy skin, it'll send us WM_COMMAND/ID_VIS_RANDOM
        //   and we'll match the skin's Random button state.
        //SendMessage(GetWinampWindow(),WM_WA_IPC,m_bMilkdropScrollLockState, IPC_CB_VISRANDOM);
    }
    else
    {



        //HWND t1 = GetFocus();
        //HWND t2 = GetPluginWindow();
        //HWND t3 = GetParent(t2);
        

        {
            //m_bMilkdropScrollLockState = GetKeyState(VK_SCROLL) & 1;
//            SetScrollLock(m_bOrigScrollLockState, m_bPreventScollLockHandling);
        }

        {
//            m_bOrigScrollLockState = GetKeyState(VK_SCROLL) & 1;
//            SetScrollLock(m_bPresetLockedByUser, m_bPreventScollLockHandling);
        }
    }
/*
    if (!redraw)
    {
        GetWinampSongTitle(GetWinampWindow(), m_szSongTitle, sizeof(m_szSongTitle)-1);
        if (wcscmp(m_szSongTitle, m_szSongTitlePrev))
        {
            lstrcpynW(m_szSongTitlePrev, m_szSongTitle, 512);
            if (m_bSongTitleAnims)
                LaunchSongTitleAnim();
        }
    }
*/
    // 2. Clear the background:
    //DWORD clear_color = (m_fog_enabled) ? FOG_COLOR : 0xFF000000;
    GetDevice()->Clear(0, 0, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xFF000000, 1.0f, 0);

    // 5. switch to 2D drawing mode.  2D coord system:
    //         +--------+ Y=-1
    //         |        |
    //         | screen |             Z=0: front of scene
    //         |        |             Z=1: back of scene
    //         +--------+ Y=1
    //       X=-1      X=1
    PrepareFor2DDrawing(GetDevice());

    if (!redraw)
        DoCustomSoundAnalysis();    // emulates old pre-vms milkdrop sound analysis

    RenderFrame(redraw);  // see milkdropfs.cpp

    if (!redraw)
    {
        m_nFramesSinceResize++;
        if (m_nLoadingPreset > 0) 
        {
            LoadPresetTick();
        }
    }

}

//----------------------------------------------------------------------

//----------------------------------------------------------------------



//----------------------------------------------------------------------



//----------------------------------------------------------------------


//----------------------------------------------------------------------


//----------------------------------------------------------------------



void CPlugin::PrevPreset(float fBlendTime)
{
}

void CPlugin::NextPreset(float fBlendTime)  // if not retracing our former steps, it will choose a random one.
{
    LoadRandomPreset(fBlendTime);
}

void CPlugin::LoadRandomPreset(float fBlendTime)
{
	UpdatePresetList();
	
	// make sure file list is ok
	if (m_nPresets - m_nDirs == 0)
	{
		if (m_nPresets - m_nDirs == 0)
		{
			// note: this error message is repeated in milkdropfs.cpp in DrawText()
//			wchar_t buf[1024];
//            swprintf(buf, WASABI_API_LNGSTRINGW(IDS_ERROR_NO_PRESET_FILE_FOUND_IN_X_MILK), m_szPresetDir);
//            AddError(buf, 6.0f, ERR_MISC, true);

            // also bring up the dir. navigation menu...
/*			if (m_UI_mode == UI_REGULAR || m_UI_mode == UI_MENU)
		    {
			    m_UI_mode = UI_LOAD;
			    m_bUserPagedUp = false;
			    m_bUserPagedDown = false;
            }*/
			return;
		}
	}

    bool bHistoryEmpty = (m_presetHistoryFwdFence==m_presetHistoryBackFence);

    // if we have history to march back forward through, do that first
    if (!m_bSequentialPresetOrder)
    {
        int next = (m_presetHistoryPos+1) % PRESET_HIST_LEN;
        if (next != m_presetHistoryFwdFence && !bHistoryEmpty)
        {
            m_presetHistoryPos = next;
            LoadPreset( m_presetHistory[m_presetHistoryPos].c_str(), fBlendTime);
            return;
        }
    }

	// --TEMPORARY--
	// this comes in handy if you want to mass-modify a batch of presets;
	// just automatically tweak values in Import, then they immediately get exported to a .MILK in a new dir.
	/*
	for (int i=0; i<m_nPresets; i++)
	{
		char szPresetFile[512];
		lstrcpy(szPresetFile, m_szPresetDir);	// note: m_szPresetDir always ends with '\'
		lstrcat(szPresetFile, m_pPresetAddr[i]);
		//CState newstate;
		m_state2.Import(szPresetFile, GetTime());

		lstrcpy(szPresetFile, "c:\\t7\\");
		lstrcat(szPresetFile, m_pPresetAddr[i]);
		m_state2.Export(szPresetFile);
	}
	*/
	// --[END]TEMPORARY--

	if (m_bSequentialPresetOrder)
	{
		m_nCurrentPreset++;
		if (m_nCurrentPreset < m_nDirs || m_nCurrentPreset >= m_nPresets)
			m_nCurrentPreset = m_nDirs;
	}
	else
	{
		// pick a random file
		if (!m_bEnableRating || (m_presets[m_nPresets - 1].fRatingCum < 0.1f))// || (m_nRatingReadProgress < m_nPresets))
		{
			m_nCurrentPreset = m_nDirs + (warand() % (m_nPresets - m_nDirs));
		}
		else
		{
			float cdf_pos = (warand() % 14345)/14345.0f*m_presets[m_nPresets - 1].fRatingCum;

			if (cdf_pos < m_presets[m_nDirs].fRatingCum)
			{
				m_nCurrentPreset = m_nDirs;
			}
			else
			{
				int lo = m_nDirs;
				int hi = m_nPresets;
				while (lo + 1 < hi)
				{
					int mid = (lo+hi)/2;
					if (m_presets[mid].fRatingCum > cdf_pos)
						hi = mid;
					else
						lo = mid;
				}
				m_nCurrentPreset = hi;
			}
		}
	}

	// m_pPresetAddr[m_nCurrentPreset] points to the preset file to load (w/o the path);
	// first prepend the path, then load section [preset00] within that file
	char szFile[MAX_PATH] = {0};
	strcpy(szFile, m_szPresetDir);	// note: m_szPresetDir always ends with '\'
	strcat(szFile, m_presets[m_nCurrentPreset].szFilename.c_str());

    if (!bHistoryEmpty)
        m_presetHistoryPos = (m_presetHistoryPos+1) % PRESET_HIST_LEN;

	LoadPreset(szFile, fBlendTime);
}

void CPlugin::RandomizeBlendPattern()
{
    if (!m_vertinfo)
        return;

    // note: we now avoid constant uniform blend b/c it's half-speed for shader blending. 
    //       (both old & new shaders would have to run on every pixel...)
    int mixtype = 1 + (warand()%3);//warand()%4;

    if (mixtype==0)
    {
        // constant, uniform blend
        int nVert = 0;
	    for (int y=0; y<=m_nGridY; y++)
	    {
		    for (int x=0; x<=m_nGridX; x++)
		    {
                m_vertinfo[nVert].a = 1;
                m_vertinfo[nVert].c = 0;
			    nVert++;
            }
        }
    }
    else if (mixtype==1)
    {
        // directional wipe
        float ang = FRAND*6.28f;
        float vx = cosf(ang);
        float vy = sinf(ang);
        float band = 0.1f + 0.2f*FRAND; // 0.2 is good
        float inv_band = 1.0f/band;
    
        int nVert = 0;
	    for (int y=0; y<=m_nGridY; y++)
	    {
            float fy = (y/(float)m_nGridY)*m_fAspectY;
		    for (int x=0; x<=m_nGridX; x++)
		    {
                float fx = (x/(float)m_nGridX)*m_fAspectX;

                // at t==0, mix rangse from -10..0
                // at t==1, mix ranges from   1..11

                float t = (fx-0.5f)*vx + (fy-0.5f)*vy + 0.5f;
                t = (t-0.5f)/sqrtf(2.0f) + 0.5f;

                m_vertinfo[nVert].a = inv_band * (1 + band);
                m_vertinfo[nVert].c = -inv_band + inv_band*t;//(x/(float)m_nGridX - 0.5f)/band;
			    nVert++;
		    }
	    }
    }
    else if (mixtype==2)
    {
        // plasma transition
        float band = 0.12f + 0.13f*FRAND;//0.02f + 0.18f*FRAND;
        float inv_band = 1.0f/band;

        // first generate plasma array of height values
        m_vertinfo[                               0].c = FRAND;
        m_vertinfo[                        m_nGridX].c = FRAND;
        m_vertinfo[m_nGridY*(m_nGridX+1)           ].c = FRAND;
        m_vertinfo[m_nGridY*(m_nGridX+1) + m_nGridX].c = FRAND;
        GenPlasma(0, m_nGridX, 0, m_nGridY, 0.25f);

        // then find min,max so we can normalize to [0..1] range and then to the proper 'constant offset' range.
        float minc = m_vertinfo[0].c;
        float maxc = m_vertinfo[0].c;
        int x,y,nVert;
    
        nVert = 0;
	    for (y=0; y<=m_nGridY; y++)
	    {
		    for (x=0; x<=m_nGridX; x++)
            {
                if (minc > m_vertinfo[nVert].c)
                    minc = m_vertinfo[nVert].c;
                if (maxc < m_vertinfo[nVert].c)
                    maxc = m_vertinfo[nVert].c;
			    nVert++;
		    }
	    }

        float mult = 1.0f/(maxc-minc);
        nVert = 0;
	    for (y=0; y<=m_nGridY; y++)
	    {
		    for (x=0; x<=m_nGridX; x++)
            {
                float t = (m_vertinfo[nVert].c - minc)*mult;
                m_vertinfo[nVert].a = inv_band * (1 + band);
                m_vertinfo[nVert].c = -inv_band + inv_band*t;
                nVert++;
            }
        }
    }
    else if (mixtype==3)
    {
        // radial blend
        float band = 0.02f + 0.14f*FRAND + 0.34f*FRAND;
        float inv_band = 1.0f/band;
        float dir = (float)((warand()%2)*2 - 1);      // 1=outside-in, -1=inside-out

        int nVert = 0;
	    for (int y=0; y<=m_nGridY; y++)
	    {
            float dy = (y/(float)m_nGridY - 0.5f)*m_fAspectY;
		    for (int x=0; x<=m_nGridX; x++)
		    {
                float dx = (x/(float)m_nGridX - 0.5f)*m_fAspectX;
                float t = sqrtf(dx*dx + dy*dy)*1.41421f;
                if (dir==-1)
                    t = 1-t;

                m_vertinfo[nVert].a = inv_band * (1 + band);
                m_vertinfo[nVert].c = -inv_band + inv_band*t;
			    nVert++;
            }
        }
    }
}

void CPlugin::GenPlasma(int x0, int x1, int y0, int y1, float dt)
{
    int midx = (x0+x1)/2;
    int midy = (y0+y1)/2;
    float t00 = m_vertinfo[y0*(m_nGridX+1) + x0].c;
    float t01 = m_vertinfo[y0*(m_nGridX+1) + x1].c;
    float t10 = m_vertinfo[y1*(m_nGridX+1) + x0].c;
    float t11 = m_vertinfo[y1*(m_nGridX+1) + x1].c;

    if (y1-y0 >= 2)
    {
        if (x0==0)
            m_vertinfo[midy*(m_nGridX+1) + x0].c = 0.5f*(t00 + t10) + (FRAND*2-1)*dt*m_fAspectY;
        m_vertinfo[midy*(m_nGridX+1) + x1].c = 0.5f*(t01 + t11) + (FRAND*2-1)*dt*m_fAspectY;
    }
    if (x1-x0 >= 2)
    {
        if (y0==0)
            m_vertinfo[y0*(m_nGridX+1) + midx].c = 0.5f*(t00 + t01) + (FRAND*2-1)*dt*m_fAspectX;
        m_vertinfo[y1*(m_nGridX+1) + midx].c = 0.5f*(t10 + t11) + (FRAND*2-1)*dt*m_fAspectX;
    }

    if (y1-y0 >= 2 && x1-x0 >= 2)
    {
        // do midpoint & recurse:
        t00 = m_vertinfo[midy*(m_nGridX+1) + x0].c;
        t01 = m_vertinfo[midy*(m_nGridX+1) + x1].c;
        t10 = m_vertinfo[y0*(m_nGridX+1) + midx].c;
        t11 = m_vertinfo[y1*(m_nGridX+1) + midx].c;
        m_vertinfo[midy*(m_nGridX+1) + midx].c = 0.25f*(t10 + t11 + t00 + t01) + (FRAND*2-1)*dt;

        GenPlasma(x0, midx, y0, midy, dt*0.5f);
        GenPlasma(midx, x1, y0, midy, dt*0.5f);
        GenPlasma(x0, midx, midy, y1, dt*0.5f);
        GenPlasma(midx, x1, midy, y1, dt*0.5f);
    }
}

void CPlugin::LoadPreset(std::string szPresetFilename, float fBlendTime)
{
	std::string temp = szPresetFilename;

    // clear old error msg...
    if (m_nFramesSinceResize > 4)
    	ClearErrors(ERR_PRESET);     

    // make sure preset still exists.  (might not if they are using the "back"/fwd buttons 
    //  in RANDOM preset order and a file was renamed or deleted!)
	//if (GetFileAttributes(szPresetFilename.c_str()) == 0xFFFFFFFF)
    {
#ifndef _XBOX
		const wchar_t *p = wcsrchr(szPresetFilename, L'\\');
        p = (p) ? p+1 : szPresetFilename;
        wchar_t buf[1024];
		swprintf(buf, WASABI_API_LNGSTRINGW(IDS_ERROR_PRESET_NOT_FOUND_X), p);
        AddError(buf, 6.0f, ERR_PRESET, true);
#endif
      ///  return;
    }
/*
    if ( !m_bSequentialPresetOrder )
    {
        // save preset in the history.  keep in mind - maybe we are searching back through it already!
        if ( m_presetHistoryFwdFence == m_presetHistoryPos )
        {
            // we're at the forward frontier; add to history
            m_presetHistory[m_presetHistoryPos] = szPresetFilename;
            m_presetHistoryFwdFence = (m_presetHistoryFwdFence+1) % PRESET_HIST_LEN;

            // don't let the two fences touch
            if (m_presetHistoryBackFence == m_presetHistoryFwdFence)
                m_presetHistoryBackFence = (m_presetHistoryBackFence+1) % PRESET_HIST_LEN;
        }
        else 
        {
            // we're retracing our steps, either forward or backward...

        }
    }
*/
    // if no preset was valid before, make sure there is no blend, because there is nothing valid to blend from.
    if (!wcscmp(m_pState->m_szDesc, INVALID_PRESET_DESC))
        fBlendTime = 0;

    if (fBlendTime == 0)
    {
        // do it all NOW!
/*	    if (szPresetFilename != m_szCurrentPresetFile) //[sic]
		    strcpy(m_szCurrentPresetFile, szPresetFilename.c_str());
	    
	    CState *temp = m_pState;
	    m_pState = m_pOldState;
	    m_pOldState = temp;
   */  
        DWORD ApplyFlags = STATE_ALL;
        ApplyFlags ^= (m_bWarpShaderLock ? STATE_WARP : 0);
        ApplyFlags ^= (m_bCompShaderLock ? STATE_COMP : 0);

        m_pState->Import(temp, GetTime(), m_pOldState, ApplyFlags);

	    if (fBlendTime >= 0.001f) 
        {
            RandomizeBlendPattern();
		    m_pState->StartBlendFrom(m_pOldState, GetTime(), fBlendTime);
        }

	    m_fPresetStartTime = GetTime();
	    m_fNextPresetTime = -1.0f;		// flags UpdateTime() to recompute this

        // release stuff from m_OldShaders, then move m_shaders to m_OldShaders, then load the new shaders.
        SafeRelease( m_OldShaders.comp.ptr );
        SafeRelease( m_OldShaders.warp.ptr );
        SafeRelease( m_OldShaders.comp.CT );
        SafeRelease( m_OldShaders.warp.CT );
        m_OldShaders = m_shaders;
        ZeroMemory(&m_shaders, sizeof(PShaderSet));
    
        LoadShaders(&m_shaders, m_pState, false);

       OnFinishedLoadingPreset();
    }
    else 
    {
        // set ourselves up to load the preset (and esp. compile shaders) a little bit at a time
        SafeRelease( m_NewShaders.comp.ptr );
        SafeRelease( m_NewShaders.warp.ptr );
        ZeroMemory(&m_NewShaders, sizeof(PShaderSet));

        DWORD ApplyFlags = STATE_ALL;
        ApplyFlags ^= (m_bWarpShaderLock ? STATE_WARP : 0);
        ApplyFlags ^= (m_bCompShaderLock ? STATE_COMP : 0);
	    
        m_pNewState->Import(szPresetFilename, GetTime(), m_pOldState, ApplyFlags);

        m_nLoadingPreset = 1;   // this will cause LoadPresetTick() to get called over the next few frames...

        m_fLoadingPresetBlendTime = fBlendTime;
//        lstrcpyW(m_szLoadingPreset, szPresetFilename);
    }
}

void CPlugin::OnFinishedLoadingPreset()
{
    // note: only used this if you loaded the preset *intact* (or mostly intact)

//    SetMenusForPresetVersion( m_pState->m_nWarpPSVersion, m_pState->m_nCompPSVersion );
    m_nPresetsLoadedTotal++; //only increment this on COMPLETION of the load.
    
    for (int mash=0; mash<MASH_SLOTS; mash++)
        m_nMashPreset[mash] = m_nCurrentPreset;
}

void CPlugin::LoadPresetTick()
{
    if (m_nLoadingPreset == 2 || m_nLoadingPreset == 5)
    {
        // just loads one shader (warp or comp) then returns.
        LoadShaders(&m_NewShaders, m_pNewState, true);
    }
    else if (m_nLoadingPreset == 8)
    {
        // finished loading the shaders - apply the preset!
    //    lstrcpyW(m_szCurrentPresetFile, m_szLoadingPreset);
        m_szLoadingPreset[0] = 0;
	    
	    CState *temp = m_pState;
	    m_pState = m_pOldState;
	    m_pOldState = temp;

	    temp = m_pState;
	    m_pState = m_pNewState;
	    m_pNewState = temp;

        RandomizeBlendPattern();

		m_pState->StartBlendFrom(m_pOldState, GetTime(), m_fLoadingPresetBlendTime);

	    m_fPresetStartTime = GetTime();
	    m_fNextPresetTime = -1.0f;		// flags UpdateTime() to recompute this

        // release stuff from m_OldShaders, then move m_shaders to m_OldShaders, then load the new shaders.
        SafeRelease( m_OldShaders.comp.ptr );
        SafeRelease( m_OldShaders.warp.ptr );
        m_OldShaders = m_shaders;
        m_shaders = m_NewShaders;
        ZeroMemory(&m_NewShaders, sizeof(PShaderSet));

        // end slow-preset-load mode
        m_nLoadingPreset = 0;

        OnFinishedLoadingPreset();
    }

    if (m_nLoadingPreset > 0)
        m_nLoadingPreset++;
}

char* NextLine(char* p)
{
    // p points to the beginning of a line
    // we'll return a pointer to the first char of the next line
    // if we hit a NULL char before that, we'll return NULL.
    if (!p)
        return NULL;

    char* s = p;
    while (*s != '\r' && *s != '\n' && *s != 0)
        s++;

    while (*s == '\r' || *s == '\n')
        s++;

    if (*s==0)
        return NULL;

    return s;
}

float GetPrivateProfileFloat(char *szSectionName, char *szKeyName, float fDefault, char *szIniFile) //BRENT - TODO
{
//    char string[64];
    char szDefault[64];
    float ret = fDefault;

    sprintf(szDefault, "%f", fDefault);

//    if (GetPrivateProfileString(szSectionName, szKeyName, szDefault, string, 64, szIniFile) > 0)
    {
//        sscanf(string, "%f", &ret);
    }

    return ret;
}

void CPlugin::MergeSortPresets(int left, int right)
{
	// note: left..right range is inclusive
	int nItems = right-left+1;

	if (nItems > 2)
	{
		// recurse to sort 2 halves (but don't actually recurse on a half if it only has 1 element)
		int mid = (left+right)/2;
		/*if (mid   != left) */ MergeSortPresets(left, mid);
		/*if (mid+1 != right)*/ MergeSortPresets(mid+1, right);
				
		// then merge results
		int a = left;
		int b = mid + 1;
		while (a <= mid && b <= right)
		{
			bool bSwap;

			// merge the sorted arrays; give preference to strings that start with a '*' character
			int nSpecial = 0;
			if (m_presets[a].szFilename.c_str()[0] == '*') nSpecial++;
			if (m_presets[b].szFilename.c_str()[0] == '*') nSpecial++;

			if (nSpecial == 1)
			{
				bSwap = (m_presets[b].szFilename.c_str()[0] == '*');
			}
			else
			{
				bSwap = (mystrcmpiW(m_presets[a].szFilename.c_str(), m_presets[b].szFilename.c_str()) > 0);
			}

			if (bSwap)
			{
				PresetInfo temp = m_presets[b];
				for (int k=b; k>a; k--)
					m_presets[k] = m_presets[k-1];
				m_presets[a] = temp;
				mid++;
				b++;
			}
			a++;
		}
	}
	else if (nItems == 2)
	{
		// sort 2 items; give preference to 'special' strings that start with a '*' character
		int nSpecial = 0;
		if (m_presets[left].szFilename.c_str()[0] == '*') nSpecial++;
		if (m_presets[right].szFilename.c_str()[0] == '*') nSpecial++;

		if (nSpecial == 1)
		{
			if (m_presets[right].szFilename.c_str()[0] == '*')
			{
                PresetInfo temp = m_presets[left];
				m_presets[left] = m_presets[right];
				m_presets[right] = temp;
			}
		}
		else if (mystrcmpiW(m_presets[left].szFilename.c_str(), m_presets[right].szFilename.c_str()) > 0)
		{
            PresetInfo temp = m_presets[left];
			m_presets[left] = m_presets[right];
			m_presets[right] = temp;
		}
	}
}

void CPlugin::UpdatePresetList(bool bBackground, bool bForce, bool bTryReselectCurrentPreset)
{
    WIN32_FIND_DATA fd;
    ZeroMemory(&fd, sizeof(fd));
    HANDLE h = INVALID_HANDLE_VALUE;

	bool bRetrying = false;

retry:

	// make sure the path exists; if not, go to winamp plugins dir
	if (GetFileAttributes(m_szPresetDir) == -1)
    {
//		FindValidPresetDir();
    }

    // if Mask (dir) changed, do a full re-scan;
    // if not, just finish our old scan.
    char szMask[MAX_PATH];
	sprintf(szMask, "%s*.*", m_szPresetDir);  // cuz dirnames could have extensions, etc.
	
    if (bForce || !m_szUpdatePresetMask[0] || strcmp(szMask, m_szUpdatePresetMask))
    {
        // if old dir was "" or the dir changed, reset our search
        if (h != INVALID_HANDLE_VALUE)
            FindClose(h);
        
		h = INVALID_HANDLE_VALUE;
		
        m_bPresetListReady = false;
        strcpy(m_szUpdatePresetMask, szMask);
        ZeroMemory(&fd, sizeof(fd));

        m_nPresets = 0;
	    m_nDirs    = 0;
        m_presets.clear();

	    // find first .MILK file
	    //if( (hFile = _findfirst(szMask, &c_file )) != -1L )		// note: returns filename -without- path
	    if( (h = FindFirstFile(m_szUpdatePresetMask, &fd )) == INVALID_HANDLE_VALUE )		// note: returns filename -without- path
        {
            // --> revert back to plugins dir
//			wchar_t buf[1024];
//			_swprintf(buf, "Unable to load preset:"), m_szPresetDir);
//			AddError(buf, 4.0f, ERR_MISC, true);

//			FindValidPresetDir();

            bRetrying = true;
            goto retry;
        }
//		AddError(WASABI_API_LNGSTRINGW(IDS_SCANNING_PRESETS), 8.0f, ERR_SCANNING_PRESETS, false);
	}

    if (m_bPresetListReady)
        return;

    int  nMaxPSVersion = m_nMaxPSVersion;
    char szPresetDir[MAX_PATH];
    lstrcpy(szPresetDir, m_szPresetDir);

    PresetList temp_presets;
    int temp_nDirs = 0;
    int temp_nPresets = 0;

    // scan for the desired # of presets, this call...
    while (h != INVALID_HANDLE_VALUE)
    {
		bool bSkip = false;
        bool bIsDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        float fRating = 0;

		char szFilename[512];
		strcpy(szFilename, fd.cFileName);

		if (bIsDir)
		{
			// skip "." directory
			if (strcmp(fd.cFileName, ".")==0)// || lstrlen(ffd.cFileName) < 1)
				bSkip = true;
			else
				sprintf(szFilename, "*%s", fd.cFileName);
		}
		else
		{
			// skip normal files not ending in ".milk"
			int len = strlen(fd.cFileName);

			if (len < 5 || strcmp(fd.cFileName + len - 5, ".milk") != 0)
				bSkip = true;					

            // if it is .milk, make sure we know how to run its pixel shaders -
            // otherwise we don't want to show it in the preset list!

            if (!bSkip) 
            {
                // If the first line of the file is not "MILKDROP_PRESET_VERSION XXX",
                //   then it's a MilkDrop 1 era preset, so it is definitely runnable. (no shaders)
                // Otherwise, check for the value "PSVERSION".  It will be 0, 2, or 3.
                //   If missing, assume it is 2.

				char szFullPath[MAX_PATH];
                sprintf(szFullPath, "%s%s", szPresetDir, fd.cFileName);
                FILE* f = fopen(szFullPath, "r");

                if (!f)
                    bSkip = true;
                else
				{
                    #define PRESET_HEADER_SCAN_BYTES 160
                    char szLine[PRESET_HEADER_SCAN_BYTES];
                    char *p = szLine;

                    int bytes_to_read = sizeof(szLine)-1;
                    int count = fread(szLine, bytes_to_read, 1, f);

                    if (count < 1)
                    {
                        fseek(f, SEEK_SET, 0);
                        count = fread(szLine, 1, bytes_to_read, f);
                        szLine[ count ] = 0;
                    }
                    else
                        szLine[bytes_to_read-1] = 0;

                    bool bScanForPreset00AndRating = false;
                    bool bRatingKnown = false;

                    // try to read the PSVERSION and the fRating= value.
                    // most presets (unless hand-edited) will have these right at the top.
                    // if not, [at least for fRating] use GetPrivateProfileFloat to search whole file.
                    // read line 1

                    //p = NextLine(p);//fgets(p, sizeof(p)-1, f);
                    if (!strncmp(p, "MILKDROP_PRESET_VERSION", 23)) 
                    {
                        p = NextLine(p);//fgets(p, sizeof(p)-1, f);
                        int ps_version = 2;

                        if (p && !strncmp(p, "PSVERSION", 9)) 
                        {
                            sscanf(&p[10], "%d", &ps_version);

                            if (ps_version > nMaxPSVersion)
                                bSkip = true;
                            else
                            {
                                p = NextLine(p);//fgets(p, sizeof(p)-1, f);
                                bScanForPreset00AndRating = true;
                            }
                        }
                    }
                    else 
                    {
                        // otherwise it's a MilkDrop 1 preset - we can run it.
                        bScanForPreset00AndRating = true;
                    }

                    // scan up to 10 more lines in the file, looking for [preset00] and fRating=...
                    // (this is WAY faster than GetPrivateProfileFloat, when it works!)

                    int reps = (bScanForPreset00AndRating) ? 10 : 0;

                    for (int z=0; z<reps; z++)
                    {
                        if (p && !strncmp(p, "[preset00]", 10)) 
                        {
                            p = NextLine(p);

                            if (p && !strncmp(p, "fRating=", 8)) 
                            {
                                sscanf(&p[8], "%f", &fRating);//_sscanf_l(&p[8], "%f", _create_locale(LC_NUMERIC, "C"), &fRating);
                                bRatingKnown = true;
                                break;
                            }
                        }
                        p = NextLine(p);
                    }

                    fclose(f);

                    if (!bRatingKnown)
		                fRating = GetPrivateProfileFloat("preset00", "fRating", 3.0f, szFullPath);

                    fRating = max(0.0f, min(5.0f, fRating));
                }
            }
		}

		if (!bSkip)
		{
            float fPrevPresetRatingCum = 0;

            if (temp_nPresets > 0)
                fPrevPresetRatingCum += temp_presets[temp_nPresets-1].fRatingCum;

            PresetInfo x;
            x.szFilename  = szFilename;
            x.fRatingThis = fRating;
            x.fRatingCum  = fPrevPresetRatingCum + fRating;

            temp_presets.push_back(x);

			temp_nPresets++;

			if (bIsDir) 
				temp_nDirs++;
        }

    	if (!FindNextFile(h, &fd))
        {
        	FindClose(h);
            h = INVALID_HANDLE_VALUE;

            break;
        }

        // every so often, add some presets...
        #define PRESET_UPDATE_INTERVAL 64

        if (temp_nPresets == 30 || ((temp_nPresets % PRESET_UPDATE_INTERVAL)==0))
        {
            //g_plugin.m_presets  = temp_presets;

            for (int i=m_nPresets; i<temp_nPresets; i++)
                m_presets.push_back(temp_presets[i]);

            m_nPresets = temp_nPresets;
            m_nDirs    = temp_nDirs;
        }
    }

    for (int i=m_nPresets; i<temp_nPresets; i++)
		m_presets.push_back(temp_presets[i]);

	m_nPresets = temp_nPresets;
    m_nDirs    = temp_nDirs;
    m_bPresetListReady = true;

    if (m_bPresetListReady && m_nPresets == 0)
    {
        // no presets OR directories found - weird - but it happens.
        // --> revert back to plugins dir

//		wchar_t buf[1024];
//		swprintf(buf, WASABI_API_LNGSTRINGW(IDS_ERROR_NO_PRESET_FILES_OR_DIRS_FOUND_IN_X), g_plugin.m_szPresetDir);
//		g_plugin.AddError(buf, 4.0f, ERR_MISC, true);

        if (bRetrying)
        {
            return;
        }

//		FindValidPresetDir();

        bRetrying = true;
        goto retry;
    }

    if (m_bPresetListReady)
    {
    	MergeSortPresets(0, m_nPresets-1);

        // update cumulative ratings, since order changed...
        m_presets[0].fRatingCum = m_presets[0].fRatingThis;
        
//		for (int i=0; i < m_nPresets; i++)
//            m_presets[i].fRatingCum = m_presets[i-1].fRatingCum + m_presets[i].fRatingThis; //BRENT FIXME

        // clear the "scanning presets" msg
//		ClearErrors(ERR_SCANNING_PRESETS);

	    // finally, try to re-select the most recently-used preset in the list
	    m_nPresetListCurPos = 0;

        if (bTryReselectCurrentPreset)
        {
/*	        if (m_szCurrentPresetFile[0])
	        {
		        // try to automatically seek to the last preset loaded
                char *p = strrchr(m_szCurrentPresetFile, '\\');

                p = (p) ? (p+1) : m_szCurrentPresetFile;
		        
				for (int i=m_nDirs; i<m_nPresets; i++)
				{
                    if (strcmp(p, m_presets[i].szFilename.c_str())==0)
					{
				        m_nPresetListCurPos = i; 
                        break;
                    }
				}
	        }*/
        }
    }
}

void CPlugin::KillSprite(int iSlot)
{
	m_texmgr.KillTex(iSlot);
}

void CPlugin::DoCustomSoundAnalysis()
{
    memcpy(mysound.fWave[0], m_sound.fWaveform[0], sizeof(float)*576);
    memcpy(mysound.fWave[1], m_sound.fWaveform[1], sizeof(float)*576);

    // do our own [UN-NORMALIZED] fft
	float fWaveLeft[576];
	for (int i=0; i<576; i++) 
        fWaveLeft[i] = m_sound.fWaveform[0][i];

	memset(mysound.fSpecLeft, 0, sizeof(float)*MY_FFT_SAMPLES);

	myfft.time_to_frequency_domain(fWaveLeft, mysound.fSpecLeft);
	//for (i=0; i<MY_FFT_SAMPLES; i++) fSpecLeft[i] = sqrtf(fSpecLeft[i]*fSpecLeft[i] + fSpecTemp[i]*fSpecTemp[i]);

	// sum spectrum up into 3 bands
	for (int i=0; i<3; i++)
	{
		// note: only look at bottom half of spectrum!  (hence divide by 6 instead of 3)
		int start = MY_FFT_SAMPLES*i/6;
		int end   = MY_FFT_SAMPLES*(i+1)/6;
		int j;

		mysound.imm[i] = 0;

		for (j=start; j<end; j++)
			mysound.imm[i] += mysound.fSpecLeft[j];
	}

	// do temporal blending to create attenuated and super-attenuated versions
	for (int i=0; i<3; i++)
	{
        float rate;

		if (mysound.imm[i] > mysound.avg[i])
			rate = 0.2f;
		else
			rate = 0.5f;
        rate = AdjustRateToFPS(rate, 30.0f, GetFps());
        mysound.avg[i] = mysound.avg[i]*rate + mysound.imm[i]*(1-rate);

		if (GetFrame() < 50)
			rate = 0.9f;
		else
			rate = 0.992f;
        rate = AdjustRateToFPS(rate, 30.0f, GetFps());
        mysound.long_avg[i] = mysound.long_avg[i]*rate + mysound.imm[i]*(1-rate);


		// also get bass/mid/treble levels *relative to the past*
		if (fabsf(mysound.long_avg[i]) < 0.001f)
			mysound.imm_rel[i] = 1.0f;
		else
			mysound.imm_rel[i]  = mysound.imm[i] / mysound.long_avg[i];

		if (fabsf(mysound.long_avg[i]) < 0.001f)
			mysound.avg_rel[i]  = 1.0f;
		else
			mysound.avg_rel[i]  = mysound.avg[i] / mysound.long_avg[i];
	}
}

void CPlugin::GenWarpPShaderText(char *szShaderText, float decay, bool bWrap)
{
    // find the pixel shader body and replace it with custom code.

    lstrcpy(szShaderText, m_szDefaultWarpPShaderText);
    char LF = LINEFEED_CONTROL_CHAR;
    char *p = strrchr( szShaderText, '{' );
    if (!p) 
        return;
    p++;
    p += sprintf(p, "%c", 1);

    p += sprintf(p, "    // sample previous frame%c", LF);
    p += sprintf(p, "    ret = tex2D( sampler%s_main, uv ).xyz;%c", bWrap ? L"" : L"_fc", LF);
    p += sprintf(p, "    %c", LF);
    p += sprintf(p, "    // darken (decay) over time%c", LF);
    p += sprintf(p, "    ret *= %.2f; //or try: ret -= 0.004;%c", decay, LF);
    //p += sprintf(p, "    %c", LF);
    //p += sprintf(p, "    ret.w = vDiffuse.w; // pass alpha along - req'd for preset blending%c", LF);
    p += sprintf(p, "}%c", LF);
}

void CPlugin::GenCompPShaderText(char *szShaderText, float brightness, float ve_alpha, float ve_zoom, int ve_orient, float hue_shader, bool bBrighten, bool bDarken, bool bSolarize, bool bInvert)
{
    // find the pixel shader body and replace it with custom code.

    lstrcpy(szShaderText, m_szDefaultCompPShaderText);
    char LF = LINEFEED_CONTROL_CHAR;
    char *p = strrchr( szShaderText, '{' );
    if (!p) 
        return;
    p++;
    p += sprintf(p, "%c", 1);

    if (ve_alpha > 0.001f) 
    {
        int orient_x = (ve_orient %  2) ? -1 : 1;
        int orient_y = (ve_orient >= 2) ? -1 : 1;
        p += sprintf(p, "    float2 uv_echo = (uv - 0.5)*%.3f*float2(%d,%d) + 0.5;%c", 1.0f/ve_zoom, orient_x, orient_y, LF);
        p += sprintf(p, "    ret = lerp( tex2D(sampler_main, uv).xyz, %c", LF);
        p += sprintf(p, "                tex2D(sampler_main, uv_echo).xyz, %c", LF);
        p += sprintf(p, "                %.2f %c", ve_alpha, LF);
        p += sprintf(p, "              ); //video echo%c", LF);
        p += sprintf(p, "    ret *= %.2f; //gamma%c", brightness, LF);
    }
    else 
    {
        p += sprintf(p, "    ret = tex2D(sampler_main, uv).xyz;%c", LF);
        p += sprintf(p, "    ret *= %.2f; //gamma%c", brightness, LF);
    }
    if (hue_shader >= 1.0f)
        p += sprintf(p, "    ret *= hue_shader; //old hue shader effect%c", LF);
    else if (hue_shader > 0.001f)
        p += sprintf(p, "    ret *= %.2f + %.2f*hue_shader; //old hue shader effect%c", 1-hue_shader, hue_shader, LF);

    if (bBrighten)
        p += sprintf(p, "    ret = sqrt(ret); //brighten%c", LF);
    if (bDarken)
        p += sprintf(p, "    ret *= ret; //darken%c", LF);
    if (bSolarize)
        p += sprintf(p, "    ret = ret*(1-ret)*4; //solarize%c", LF);
    if (bInvert)
        p += sprintf(p, "    ret = 1 - ret; //invert%c", LF);
    //p += sprintf(p, "    ret.w = vDiffuse.w; // pass alpha along - req'd for preset blending%c", LF);
    p += sprintf(p, "}%c", LF);
}