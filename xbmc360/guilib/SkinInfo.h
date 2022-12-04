#ifndef H_CGUISKININFO
#define H_CGUISKININFO

#include "GraphicContext.h" // needed for the RESOLUTION members
#include "GUIIncludes.h"    // needed for the GUIInclude member

#include "utils\StdString.h"
#include "tinyxml\tinyxml.h"

#define CREDIT_LINE_LENGTH 50

class CSkinInfo
{
public:
	class CStartupWindow
	{
		public:
		CStartupWindow(int id, const CStdString &name)
		{
			m_id = id; m_name = name;
		};

		int m_id;
		CStdString m_name;
	};

	CSkinInfo();
	~CSkinInfo();

	void Load(const CStdString& skinDir, bool loadIncludes = true);
	bool ResolveConstant(const CStdString &constant, float &value) const;
	bool ResolveConstant(const CStdString &constant, unsigned int &value) const;

	// Brief Get the full path to the specified file in the skin
	// We search for XML files in the skin folder that best matches the current resolution
	CStdString GetSkinPath(const CStdString& file, RESOLUTION *res = NULL, const CStdString& baseDir = "") const;

	CStdString GetBaseDir() const;

	double GetVersion() const { return m_Version; };

	// Given a resolution, retrieve the corresponding directory name
	// param res RESOLUTION to translate
	// return directory name for res
	CStdString GetDirFromRes(RESOLUTION res) const;

	inline float GetSkinZoom() const { return m_skinzoom; };

	inline float GetLegacy() { return m_bLegacy; };

	float GetEffectsSlowdown() const { return m_effectsSlowDown; };

	// Get the id of the window the user wants to start in after any skin animation
	// id of the start window
	int GetStartWindow() const;

	void ResolveIncludes(TiXmlElement *node, const CStdString &type = "");

	const INFO::CSkinVariableString* CreateSkinVariable(const CStdString& name, int context);

protected:
	// Brief grab a resolution tag from an XML node
	// param node XML node to look for the given tag
	// param tag name of the tag to look for
	// param res resolution to return
	// return true if we find a valid XML node containing a valid resolution, false otherwise
	bool GetResolution(const TiXmlNode *node, const char *tag, RESOLUTION &res) const;

	void LoadIncludes();


	wchar_t credits[6][CREDIT_LINE_LENGTH];  // Credits info
	int m_iNumCreditLines;  // Number of credit lines
	RESOLUTION m_DefaultResolution; // Default resolution for the skin in 4:3 modes
	RESOLUTION m_DefaultResolutionWide; // Default resolution for the skin in 16:9 modes
	CStdString m_strBaseDir;
	double m_Version;

	float m_effectsSlowDown;
	CGUIIncludes m_includes;

	std::vector<CStartupWindow> m_startupWindows;
	bool m_onlyAnimateToHome;

	float m_skinzoom;

	bool m_bLegacy;
};

extern CSkinInfo g_SkinInfo;

#endif //H_CGUISKININFO