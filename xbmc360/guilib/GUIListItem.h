#ifndef GUILIB_GUILISTITEM_H
#define GUILIB_GUILISTITEM_H

#include "utils\StdString.h"

#include <map>
#include <string>

// Forward
class CGUIListItemLayout;
class CArchive;
class CVariant;

class CGUIListItem
{
public:
  enum GUIIconOverlay { ICON_OVERLAY_NONE = 0,
						ICON_OVERLAY_RAR,
						ICON_OVERLAY_ZIP,
						ICON_OVERLAY_LOCKED,
						ICON_OVERLAY_HAS_TRAINER,
						ICON_OVERLAY_TRAINED,
						ICON_OVERLAY_UNWATCHED,
						ICON_OVERLAY_WATCHED,
						ICON_OVERLAY_HD};

	CGUIListItem(void);
	CGUIListItem(const CGUIListItem& item);
	CGUIListItem(const CStdString& strLabel);
	virtual ~CGUIListItem(void);
	virtual CGUIListItem *Clone() const { return new CGUIListItem(*this); };

	const CGUIListItem& operator =(const CGUIListItem& item);

	virtual void SetLabel(const CStdString& strLabel);
	const CStdString& GetLabel() const;

	void SetLabel2(const CStdString& strLabel);
	const CStdString& GetLabel2() const;

	void SetIconImage(const CStdString& strIcon);
	const CStdString& GetIconImage() const;

	void SetThumbnailImage(const CStdString& strThumbnail);
	const CStdString& GetThumbnailImage() const;

	void SetOverlayImage(GUIIconOverlay icon, bool bOnOff=false);
	CStdString GetOverlayImage() const;

	void SetSortLabel(const CStdString &label);
	const CStdString &GetSortLabel() const;

	void Select(bool bOnOff);
	bool IsSelected() const;

	bool HasIcon() const;
	bool HasThumbnail() const;
	bool HasOverlay() const;
	virtual bool IsFileItem() const { return false; };

	void SetLayout(CGUIListItemLayout *layout);
	CGUIListItemLayout *GetLayout();

	void SetFocusedLayout(CGUIListItemLayout *layout);
	CGUIListItemLayout *GetFocusedLayout();

	void FreeIcons();
	void FreeMemory(bool immediately = false);
	void SetInvalid();

	bool m_bIsFolder; // Is item a folder or a file

	void SetProperty(const CStdString &strKey, const char *strValue);
	void SetProperty(const CStdString &strKey, const CStdString &strValue);
	void SetProperty(const CStdString &strKey, int nVal);
	void SetProperty(const CStdString &strKey, bool bVal);
	void SetProperty(const CStdString &strKey, double dVal);

	void IncrementProperty(const CStdString &strKey, int nVal);
	void IncrementProperty(const CStdString &strKey, double dVal);

	void ClearProperties();

	void Archive(CArchive& ar);
	void Serialize(CVariant& value);

	bool       HasProperty(const CStdString &strKey) const;
	bool       HasProperties() const { return m_mapProperties.size() > 0; };
	void       ClearProperty(const CStdString &strKey);

	CStdString GetProperty(const CStdString &strKey) const;
	bool       GetPropertyBOOL(const CStdString &strKey) const;
	int        GetPropertyInt(const CStdString &strKey) const;
	double     GetPropertyDouble(const CStdString &strKey) const;

protected:
	CStdString m_strLabel2; // Text of column2
	CStdString m_strThumbnailImage; // Filename of thumbnail
	CStdString m_strIcon; // Filename of icon
	GUIIconOverlay m_overlayIcon; // Type of overlay icon

	CGUIListItemLayout *m_layout;
	CGUIListItemLayout *m_focusedLayout;
	bool m_bSelected; // Item is selected or not

	struct icompare
	{
		bool operator()(const CStdString &s1, const CStdString &s2) const
		{
			return s1.CompareNoCase(s2) < 0;
		}
	};

	typedef std::map<CStdString, CStdString, icompare> PropertyMap;
	PropertyMap m_mapProperties;

private:
	CStdString m_sortLabel; // Text for sorting
	CStdString m_strLabel; // Text of column1
};

#endif //GUILIB_GUILISTITEM_H
