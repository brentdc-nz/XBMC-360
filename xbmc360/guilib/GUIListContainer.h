#ifndef GUILIB_CGUILISTCONTAINER_H
#define GUILIB_CGUILISTCONTAINER_H

#include "GUIBaseContainer.h"

class CGUIListContainer : public CGUIBaseContainer
{
public:
	CGUIListContainer(int parentID, int controlID, float posX, float posY, float width, float height, ORIENTATION orientation, int scrollTime, int preloadItems);
//#ifdef PRE_SKIN_VERSION_9_10_COMPATIBILITY
	CGUIListContainer(int parentID, int controlID, float posX, float posY, float width, float height,
						const CLabelInfo& labelInfo, const CLabelInfo& labelInfo2,
						const CTextureInfo& textureButton, const CTextureInfo& textureButtonFocus,
						float textureHeight, float itemWidth, float itemHeight, float spaceBetweenItems);
//#endif
	virtual ~CGUIListContainer(void);
	virtual CGUIListContainer *Clone() const { return new CGUIListContainer(*this); };

	virtual bool OnAction(const CAction &action);
	virtual bool OnMessage(CGUIMessage& message);

	virtual bool HasNextPage() const;
	virtual bool HasPreviousPage() const;

protected:
	virtual void Scroll(int amount);
	void SetCursor(int cursor);
	virtual bool MoveDown(bool wrapAround);
	virtual bool MoveUp(bool wrapAround);
	virtual void ValidateOffset();
	virtual void SelectItem(int item);
	virtual bool SelectItemFromPoint(const CPoint &point); 
	virtual int GetCursorFromPoint(const CPoint &point, CPoint *itemPoint = NULL) const;
};

#endif //GUILIB_CGUILISTCONTAINER_H