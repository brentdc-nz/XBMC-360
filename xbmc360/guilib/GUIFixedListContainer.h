#ifndef GUILIB_CGUIFIXEDLISTCONTAINER_H
#define GUILIB_CGUIFIXEDLISTCONTAINER_H

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

#include "GUIBaseContainer.h"

class CGUIFixedListContainer : public CGUIBaseContainer
{
public:
	CGUIFixedListContainer(int parentID, int controlID, float posX, float posY, float width, float height, ORIENTATION orientation, int scrollTime, int preloadItems, int fixedPosition, int cursorRange);
	virtual ~CGUIFixedListContainer(void);
	virtual CGUIFixedListContainer *Clone() const { return new CGUIFixedListContainer(*this); };

	virtual bool OnAction(const CAction &action);

protected:
	virtual void Scroll(int amount);
	virtual bool MoveDown(bool wrapAround);
	virtual bool MoveUp(bool wrapAround);
	virtual void ValidateOffset();
	virtual bool SelectItemFromPoint(const CPoint &point);
	virtual int GetCursorFromPoint(const CPoint &point, CPoint *itemPoint = NULL) const;
	virtual void SelectItem(int item);
	virtual bool HasNextPage() const;
	virtual bool HasPreviousPage() const;
	virtual int GetCurrentPage() const;

private:
	void GetCursorRange(int &minCursor, int &maxCursor) const;

	int m_fixedCursor;
	int m_cursorRange;
};

#endif //GUILIB_CGUIFIXEDLISTCONTAINER_H
