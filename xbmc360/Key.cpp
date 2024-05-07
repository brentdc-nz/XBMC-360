#include "guilib\include.h"
#include "guilib\Key.h"

CKey::CKey(void)
{
	m_buttonCode = KEY_INVALID;
	m_leftTrigger = 0;
	m_rightTrigger = 0;
	m_leftThumbX = 0.0f;
	m_leftThumbY = 0.0f;
	m_rightThumbX = 0.0f;
	m_rightThumbY = 0.0f;
	m_repeat = 0.0f;
	m_held = 0;
}

CKey::CKey(uint32_t buttonCode, uint8_t leftTrigger, uint8_t rightTrigger, float leftThumbX, float leftThumbY, float rightThumbX, float rightThumbY, float repeat)
{
	m_leftTrigger = leftTrigger;
	m_rightTrigger = rightTrigger;
	m_leftThumbX = leftThumbX;
	m_leftThumbY = leftThumbY;
	m_rightThumbX = rightThumbX;
	m_rightThumbY = rightThumbY;
	m_buttonCode = buttonCode;
	m_repeat = repeat;
	m_held = 0;
}

CKey::~CKey(void)
{
}

CKey::CKey(const CKey& key)
{
	*this = key;
}

uint32_t CKey::GetButtonCode() const // for backwards compatibility only
{
	return m_buttonCode;
}

const CKey& CKey::operator=(const CKey& key)
{
	if (&key == this) return * this;

	m_leftTrigger = key.m_leftTrigger;
	m_rightTrigger = key.m_rightTrigger;
	m_buttonCode = key.m_buttonCode;
	m_leftThumbX = key.m_leftThumbX;
	m_leftThumbY = key.m_leftThumbY;
	m_rightThumbX = key.m_rightThumbX;
	m_rightThumbY = key.m_rightThumbY;
	m_repeat = key.m_repeat;
	m_held = key.m_held;

	return *this;
}

BYTE CKey::GetLeftTrigger() const
{
	return m_leftTrigger;
}

BYTE CKey::GetRightTrigger() const
{
	return m_rightTrigger;
}

float CKey::GetLeftThumbX() const
{
	return m_leftThumbX;
}

float CKey::GetLeftThumbY() const
{
	return m_leftThumbY;
}

float CKey::GetRightThumbX() const
{
	return m_rightThumbX;
}

float CKey::GetRightThumbY() const
{
	return m_rightThumbY;
}

bool CKey::IsAnalogButton() const
{
	if ((GetButtonCode() > 261 && GetButtonCode() < 270) || (GetButtonCode() > 279 && GetButtonCode() < 284))
		return true;

	return false;
}

float CKey::GetRepeat() const
{
	return m_repeat;
}

void CKey::SetHeld(unsigned int held)
{
	m_held = held;
}

unsigned int CKey::GetHeld() const
{
	return m_held;
}

CAction::CAction(int actionID, float amount1 /* = 1.0f */, float amount2 /* = 0.0f */, const CStdString &name /* = "" */, unsigned int holdTime /*= 0*/)
{
	m_id = actionID;
	m_amount[0] = amount1;
	m_amount[1] = amount2;
	
	for(unsigned int i = 2; i < max_amounts; i++)
		m_amount[i] = 0;  
	
	m_name = name;
	m_repeat = 0;
	m_buttonCode = 0;
	m_unicode = 0;
	m_holdTime = holdTime;
}

CAction::CAction(int actionID, unsigned int state, float posX, float posY, float offsetX, float offsetY)
{
	m_id = actionID;
	m_amount[0] = posX;
	m_amount[1] = posY;
	m_amount[2] = offsetX;
	m_amount[3] = offsetY;
	
	for(unsigned int i = 4; i < max_amounts; i++)
		m_amount[i] = 0;  
	
	m_repeat = 0;
	m_buttonCode = 0;
	m_unicode = 0;
	m_holdTime = state;
}

CAction::CAction(int actionID, wchar_t unicode)
{
	m_id = actionID;
	
	for(unsigned int i = 0; i < max_amounts; i++)
		m_amount[i] = 0;  

	m_repeat = 0;
	m_buttonCode = 0;
	m_unicode = unicode;
	m_holdTime = 0;
}

CAction::CAction(int actionID, const CStdString &name, const CKey &key)
{
	m_id = actionID;
	m_name = name;
	m_amount[0] = 1; // Digital button (could change this for repeat acceleration)
	
	for(unsigned int i = 1; i < max_amounts; i++)
		m_amount[i] = 0;
	
	m_repeat = key.GetRepeat();
	m_buttonCode = key.GetButtonCode();
	m_unicode = 0;
	m_holdTime = key.GetHeld();
	
	// Get the action amounts of the analog buttons
	if (key.GetButtonCode() == KEY_BUTTON_LEFT_ANALOG_TRIGGER)
		m_amount[0] = (float)key.GetLeftTrigger() / 255.0f;
	else if (key.GetButtonCode() == KEY_BUTTON_RIGHT_ANALOG_TRIGGER)
		m_amount[0] = (float)key.GetRightTrigger() / 255.0f;
	else if (key.GetButtonCode() == KEY_BUTTON_LEFT_THUMB_STICK)
	{
		m_amount[0] = key.GetLeftThumbX();
		m_amount[1] = key.GetLeftThumbY();
	}
	else if (key.GetButtonCode() == KEY_BUTTON_RIGHT_THUMB_STICK)
	{
		m_amount[0] = key.GetRightThumbX();
		m_amount[1] = key.GetRightThumbY();
	}
	else if (key.GetButtonCode() == KEY_BUTTON_LEFT_THUMB_STICK_UP)
		m_amount[0] = key.GetLeftThumbY();
	else if (key.GetButtonCode() == KEY_BUTTON_LEFT_THUMB_STICK_DOWN)
		m_amount[0] = -key.GetLeftThumbY();
	else if (key.GetButtonCode() == KEY_BUTTON_LEFT_THUMB_STICK_LEFT)
		m_amount[0] = -key.GetLeftThumbX();
	else if (key.GetButtonCode() == KEY_BUTTON_LEFT_THUMB_STICK_RIGHT)
		m_amount[0] = key.GetLeftThumbX();
	else if (key.GetButtonCode() == KEY_BUTTON_RIGHT_THUMB_STICK_UP)
		m_amount[0] = key.GetRightThumbY();
	else if (key.GetButtonCode() == KEY_BUTTON_RIGHT_THUMB_STICK_DOWN)
		m_amount[0] = -key.GetRightThumbY();
	else if (key.GetButtonCode() == KEY_BUTTON_RIGHT_THUMB_STICK_LEFT)
		m_amount[0] = -key.GetRightThumbX();
	else if (key.GetButtonCode() == KEY_BUTTON_RIGHT_THUMB_STICK_RIGHT)
		m_amount[0] = key.GetRightThumbX();
}