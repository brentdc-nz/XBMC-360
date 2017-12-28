//--------------------------------------------------------------------------------------
// XBInput.cpp
//
// Input helper functions for samples
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "XBInput.h"

//--------------------------------------------------------------------------------------
// Globals
//--------------------------------------------------------------------------------------

// Instance of static member variables
GAMEPAD   Input::m_Gamepads[4];

//--------------------------------------------------------------------------------------
// Name: ConvertThumbstickValue()
// Desc: Converts SHORT thumbstick values to FLOAT, while enforcing a deadzone
//--------------------------------------------------------------------------------------
inline FLOAT ConvertThumbstickValue( SHORT sThumbstickValue, SHORT sDeadZone )
{
    if( sThumbstickValue > +sDeadZone )
    {
        return (sThumbstickValue-sDeadZone) / (32767.0f-sDeadZone);
    }
    if( sThumbstickValue < -sDeadZone )
    {
        return (sThumbstickValue+sDeadZone+1.0f) / (32767.0f-sDeadZone);
    }
    return 0.0f;
}


//--------------------------------------------------------------------------------------
// Name: GetInput()
// Desc: Processes input from the gamepads
//--------------------------------------------------------------------------------------
VOID Input::GetInput( GAMEPAD* pGamepads )
{
    // Initial the static data structures
    static BOOL bInitialized = FALSE;
    if( FALSE == bInitialized )
    {
        ZeroMemory( m_Gamepads,        sizeof(m_Gamepads) );
        bInitialized = TRUE;
    }

    // If the user did not specify a list of gamepads, use the global list
    if( NULL == pGamepads )
        pGamepads = m_Gamepads;

    // Loop through all gamepads
    for( DWORD i=0; i < XUSER_MAX_COUNT; i++ )
    {
        // Read the input state
        XINPUT_STATE InputState;
        BOOL bWasConnected = pGamepads[i].bConnected;
        pGamepads[i].bConnected = ( XInputGetState( i, &InputState ) == ERROR_SUCCESS ) ? TRUE : FALSE;

        // Track insertion and removals
        pGamepads[i].bRemoved  = (  bWasConnected && !pGamepads[i].bConnected ) ? TRUE : FALSE;
        pGamepads[i].bInserted = ( !bWasConnected &&  pGamepads[i].bConnected ) ? TRUE : FALSE;

        if( FALSE == pGamepads[i].bConnected )
            continue;

        // Store the capabilities of the device
        if( TRUE == pGamepads[i].bInserted )
        {
            ZeroMemory( &pGamepads[i], sizeof(m_Gamepads[i]) );
            pGamepads[i].bConnected = TRUE;
            pGamepads[i].bInserted  = TRUE;
            XInputGetCapabilities( i, XINPUT_FLAG_GAMEPAD, &pGamepads[i].caps );
        }

        // Copy gamepad to local structure
        memcpy( &pGamepads[i], &InputState.Gamepad, sizeof(XINPUT_GAMEPAD) );

        // Put Xbox device input for the gamepad into our custom format
        pGamepads[i].fX1 = ConvertThumbstickValue( pGamepads[i].sThumbLX, GAMEPAD::LEFT_THUMB_DEADZONE );
        pGamepads[i].fY1 = ConvertThumbstickValue( pGamepads[i].sThumbLY, GAMEPAD::LEFT_THUMB_DEADZONE );
        pGamepads[i].fX2 = ConvertThumbstickValue( pGamepads[i].sThumbRX, GAMEPAD::RIGHT_THUMB_DEADZONE );
        pGamepads[i].fY2 = ConvertThumbstickValue( pGamepads[i].sThumbRY, GAMEPAD::RIGHT_THUMB_DEADZONE );

        // Get the boolean buttons that have been pressed since the last
        // call. Each button is represented by one bit.
        pGamepads[i].wPressedButtons = ( pGamepads[i].wLastButtons ^ pGamepads[i].wButtons ) & pGamepads[i].wButtons;
        pGamepads[i].wLastButtons    = pGamepads[i].wButtons;

        // Figure out if the left trigger has been pressed or released
        BOOL bPressed = ( pGamepads[i].bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

        if( bPressed )
            pGamepads[i].bPressedLeftTrigger = !pGamepads[i].bLastLeftTrigger;
        else
            pGamepads[i].bPressedLeftTrigger = FALSE;

        // Store the state for next time
        pGamepads[i].bLastLeftTrigger = bPressed;

        // Figure out if the right trigger has been pressed or released
        bPressed = ( pGamepads[i].bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD );

        if( bPressed )
            pGamepads[i].bPressedRightTrigger = !pGamepads[i].bLastRightTrigger;
        else
            pGamepads[i].bPressedRightTrigger = FALSE;

        // Store the state for next time
        pGamepads[i].bLastRightTrigger = bPressed;

        // Set the user index for this gamepad
        pGamepads[i].dwUserIndex = i;
    }
}


//--------------------------------------------------------------------------------------
// Name: ClampToShort()
// Desc: Takes an integer input and clamps it to fit in a short, to avoid wrapping.
//--------------------------------------------------------------------------------------
SHORT Input::ClampToShort(INT input)
{
    // Use min/max from windef.h. Define NOMINMAX if you don't want these
    // macros defined.
    input = min(input, SHRT_MAX);
    input = max(input, SHRT_MIN);
    return (SHORT)input;
}