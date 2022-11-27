#include "XBApplicationEx.h"
#include "utils\Log.h"
#include "guilib\GraphicContext.h"

// Deadzone for the gamepad inputs
SHORT GAMEPAD::LEFT_THUMB_DEADZONE  = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
SHORT GAMEPAD::RIGHT_THUMB_DEADZONE = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;

CXBApplicationEX::CXBApplicationEX()
{
	m_pD3D = NULL;
	m_pd3dDevice = NULL;
	
	// Set up the presentation parameters for a double-buffered
	// 32-bit display using depth-stencil. Override these parameters in
	// your derived class as your app requires.
	ZeroMemory( &m_d3dpp, sizeof(m_d3dpp) );
	XVIDEO_MODE VideoMode;
	XGetVideoMode( &VideoMode );
	m_d3dpp.BackBufferWidth        = min( VideoMode.dwDisplayWidth, 1280 );
	m_d3dpp.BackBufferHeight       = min( VideoMode.dwDisplayHeight, 720 );
	m_d3dpp.BackBufferFormat       = D3DFMT_LIN_X8R8G8B8;
	m_d3dpp.BackBufferCount        = 1;
	m_d3dpp.EnableAutoDepthStencil = FALSE;
	m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	m_d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_DEFAULT;

	m_bStop = false;
}

CXBApplicationEX::~CXBApplicationEX()
{
	m_bStop = false;
}

bool CXBApplicationEX::Create()
{
	// Initialize core peripheral port support.
	ZeroMemory( m_Gamepads,        sizeof(m_Gamepads) );
	ZeroMemory( &m_DefaultGamepad, sizeof(m_DefaultGamepad) );

	if (!Initialize())
	{
		CLog::Log(LOGERROR, "XBAppEx: Call to Initialize() failed!" );
		return false;
	}

	return true;
}

int CXBApplicationEX::Run()
{
	CLog::Log(LOGNOTICE, "Running the application..." );

	// Run the game loop, animating and rendering frames
	while(!m_bStop)
	{
		Process();
		FrameMove();
		Render();
	}

	CLog::Log(LOGNOTICE, "Application stopped..." );

	return 0;
}

void CXBApplicationEX::Destroy()
{
	// Perform app-specific cleanup
	Cleanup();

	if(m_pd3dDevice)
	{
		g_graphicsContext.TLock();
		m_pd3dDevice->Release();
		m_pd3dDevice = NULL;
		g_graphicsContext.TUnlock();
	}

	if(m_pD3D)
	{
		g_graphicsContext.TLock();
		m_pD3D->Release();
		m_pD3D = NULL;
		g_graphicsContext.TUnlock();
	}
}

void CXBApplicationEX::ReadInput()
{
	//-----------------------------------------
	// Handle input
	//-----------------------------------------

	// Read the input for all connected gamepads
	//XBInput_GetInput( m_Gamepad );
	Input::GetInput( m_Gamepads );

	// Lump inputs of all connected gamepads into one common structure.
	// This is done so apps that need only one gamepad can function with
	// any gamepad.
	// Sum input across ALL gamepads into one default structure.
	ZeroMemory( &m_DefaultGamepad, sizeof(m_DefaultGamepad) );
	INT  iThumbLX = 0;
	INT  iThumbLY = 0;
	INT  iThumbRX = 0;
	INT  iThumbRY = 0;
	BOOL bActiveThumbs[XUSER_MAX_COUNT]  = { 0 };
	BOOL bActiveButtons[XUSER_MAX_COUNT] = { 0 };

	for( DWORD i=0; i < XUSER_MAX_COUNT; i++ )
	{
		if( m_Gamepads[i].bConnected && ( !0 || ( 0 & ( 1 << i ) ) ) )
		{
			SHORT sThumbLX = 0;
			SHORT sThumbLY = 0;
			SHORT sThumbRX = 0;
			SHORT sThumbRY = 0;

			// Only account for thumbstick info beyond the deadzone
			// Note that this is a simplification of the deadzone. The dead
			// zone is when the thumb stick is at or near its 'home' position.
			// If the thumb stick is pushed all the way to the left then, even
			// though the y-coordinates are still in the dead-zone range, they
			// are actually considered significant. This dead-zone ignoring
			// code will ignore the y-coordinate in a horizontal block all the
			// way across, and will ignore the x-coordinate in a vertical block
			// all the way up/down. This simplification lets us normalize the
			// coordinates into a simple +-1.0 range, but for delicate control
			// (subtle steering when pushing forward) this technique will not
			// be sufficient.
			if( m_Gamepads[i].sThumbLX > GAMEPAD::LEFT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbLX < -GAMEPAD::LEFT_THUMB_DEADZONE )
				sThumbLX = m_Gamepads[i].sThumbLX;
			if( m_Gamepads[i].sThumbLY > GAMEPAD::LEFT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbLY < -GAMEPAD::LEFT_THUMB_DEADZONE )
				sThumbLY = m_Gamepads[i].sThumbLY;
			if( m_Gamepads[i].sThumbRX > GAMEPAD::RIGHT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbRX < -GAMEPAD::RIGHT_THUMB_DEADZONE )
				sThumbRX = m_Gamepads[i].sThumbRX;
			if( m_Gamepads[i].sThumbRY > GAMEPAD::RIGHT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbRY < -GAMEPAD::RIGHT_THUMB_DEADZONE )
				sThumbRY = m_Gamepads[i].sThumbRY;

			// Sum up the raw thumbstick inputs, as long as the thumbstick
			// is outside of the dead-zone. This is different from the
			// summing above where the x and y values are accepted or
			// discarded separately.
			if( m_Gamepads[i].sThumbRX > GAMEPAD::RIGHT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbRX < -GAMEPAD::RIGHT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbRY > GAMEPAD::RIGHT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbRY < -GAMEPAD::RIGHT_THUMB_DEADZONE )
			{
				iThumbRX += m_Gamepads[i].sThumbRX;
				iThumbRY += m_Gamepads[i].sThumbRY;
			}

			if( m_Gamepads[i].sThumbLX > GAMEPAD::LEFT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbLX < -GAMEPAD::LEFT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbLY > GAMEPAD::LEFT_THUMB_DEADZONE ||
				m_Gamepads[i].sThumbLY < -GAMEPAD::LEFT_THUMB_DEADZONE )
			{
				iThumbLX += m_Gamepads[i].sThumbLX;
				iThumbLY += m_Gamepads[i].sThumbLY;
			}

			// Keep track of which gamepads are active
			if( sThumbLX!=0 || sThumbLY!=0 || sThumbRX!=0 || sThumbRY!=0 )
				bActiveThumbs[i] = TRUE;

			m_DefaultGamepad.fX1 += m_Gamepads[i].fX1;
			m_DefaultGamepad.fY1 += m_Gamepads[i].fY1;
			m_DefaultGamepad.fX2 += m_Gamepads[i].fX2;
			m_DefaultGamepad.fY2 += m_Gamepads[i].fY2;

			m_DefaultGamepad.wButtons        |= m_Gamepads[i].wButtons;
			m_DefaultGamepad.wPressedButtons |= m_Gamepads[i].wPressedButtons;
			m_DefaultGamepad.wLastButtons    |= m_Gamepads[i].wLastButtons;

			if( m_Gamepads[i].wButtons != m_Gamepads[i].wLastButtons )
				bActiveButtons[i] = TRUE;

			for( DWORD b=0; b<8; b++ )
			{
				m_DefaultGamepad.bLeftTrigger         |= m_Gamepads[i].bLeftTrigger;
				m_DefaultGamepad.bPressedLeftTrigger  |= m_Gamepads[i].bPressedLeftTrigger;
				m_DefaultGamepad.bLastLeftTrigger     |= m_Gamepads[i].bLastLeftTrigger;

				m_DefaultGamepad.bRightTrigger        |= m_Gamepads[i].bRightTrigger;
				m_DefaultGamepad.bPressedRightTrigger |= m_Gamepads[i].bPressedRightTrigger;
				m_DefaultGamepad.bLastRightTrigger    |= m_Gamepads[i].bLastRightTrigger;

				if( m_Gamepads[i].bLeftTrigger != m_Gamepads[i].bLastLeftTrigger )
					bActiveButtons[i] = TRUE;
				if( m_Gamepads[i].bRightTrigger != m_Gamepads[i].bLastRightTrigger )
					bActiveButtons[i] = TRUE;
			}
		}
	}

	// Clamp summed thumbstick values to proper range
	m_DefaultGamepad.sThumbLX = Input::ClampToShort(iThumbLX);
	m_DefaultGamepad.sThumbLY = Input::ClampToShort(iThumbLY);
	m_DefaultGamepad.sThumbRX = Input::ClampToShort(iThumbRX);
	m_DefaultGamepad.sThumbRY = Input::ClampToShort(iThumbRY);

	// Fill pActiveGamePadsMask
	if ( NULL != NULL )
	{
		DWORD dwActiveGamePadsMask = 0;
		for( DWORD i=0; i<XUSER_MAX_COUNT; i++ )
		{
			if( bActiveButtons[i] || bActiveThumbs[i] || m_Gamepads[i].wPressedButtons )
			{
				dwActiveGamePadsMask |= 1 << i;
			}
		}

		//*pdwActiveGamePadsMask = dwActiveGamePadsMask; // FIXME
	}

	// Assign an active gamepad
	for( DWORD i=0; i<XUSER_MAX_COUNT; i++ )
	{
		if( bActiveButtons[i] )
		{
			m_DefaultGamepad.dwUserIndex = i;
			break;
		}

		if( bActiveThumbs[i] )
		{
			m_DefaultGamepad.dwUserIndex = i;
			break;
		}
	}

	// Handle a convenient quit sequence
	if( ( m_DefaultGamepad.bLeftTrigger  > 128 ) &&
		( m_DefaultGamepad.bRightTrigger > 128 ) &&
		( m_DefaultGamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ) )
	{
		// Quit to dashboard
		XLaunchNewImage( "", 0 );
	}
}