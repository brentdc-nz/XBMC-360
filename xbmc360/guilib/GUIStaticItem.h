#ifndef GUILIB_CGUISTATICITEM_H
#define GUILIB_CGUISTATICITEM_H

#include "GUIInfoTypes.h"
#include "FileItem.h"
#include "GUIAction.h"

class TiXmlElement;

// Wrapper class for a static item in a list container

// A wrapper class for the items in a container specified via the <content>
// flag. Handles constructing items from XML and updating item labels, icons
// and properties.

class CGUIStaticItem : public CFileItem
{
public:
/*
	Construct an item based on an XML description:
	<item>
		<label>$INFO[MusicPlayer.Artist]</label>
		<label2>$INFO[MusicPlayer.Album]</label2>
		<thumb>bar.png</thumb>
		<icon>foo.jpg</icon>
		<onclick>ActivateWindow(Home)</onclick>
	</item>
*/
	// Element XML element to construct from
	// contextWindow window context to use for any info labels
	CGUIStaticItem(const TiXmlElement *element, int contextWindow);
	virtual ~CGUIStaticItem() {};
	virtual CGUIListItem *Clone() const { return new CGUIStaticItem(*this); };

	// Update any infolabels in the items properties
	// Runs through all the items properties, updating any that should be
	// periodically recomputed
	// contextWindow window context to use for any info labels
	void UpdateProperties(int contextWindow);
	const CGUIAction &GetClickActions() const { return m_clickActions; };

private:
	typedef std::vector< std::pair<CGUIInfoLabel, CStdString> > InfoVector;
	InfoVector m_info;
	CGUIAction m_clickActions;
};

#ifdef _HAS_BOOST
typedef boost::shared_ptr<CGUIStaticItem> CGUIStaticItemPtr;
#else
typedef std::shared_ptr<CGUIStaticItem> CGUIStaticItemPtr;
#endif

#endif //GUILIB_CGUISTATICITEM_H