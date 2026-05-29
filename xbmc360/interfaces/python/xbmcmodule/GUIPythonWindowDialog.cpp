/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIPythonWindowDialog.h"
#include "utils/log.h"

// TODO: Full implementation requires CGUIWindowManager, CApplicationMessenger

CGUIPythonWindowDialog::CGUIPythonWindowDialog(int id)
: CGUIPythonWindow(id)
{
	m_bRunning = false;
	m_loadOnDemand = false;
}

CGUIPythonWindowDialog::~CGUIPythonWindowDialog(void)
{
}

void CGUIPythonWindowDialog::Show(bool show /* = true */)
{
	// TODO: Send TMSG_GUI_PYTHON_DIALOG via ApplicationMessenger
	Show_Internal(show);
}

void CGUIPythonWindowDialog::Show_Internal(bool show /* = true */)
{
	if (show)
	{
		// TODO: g_windowManager.RouteToWindow(this);
		// TODO: Send GUI_MSG_WINDOW_INIT
		m_bRunning = true;
	}
	else
	{
		// TODO: Send GUI_MSG_WINDOW_DEINIT
		// TODO: g_windowManager.RemoveDialog(GetID());
		m_bRunning = false;
	}
}
