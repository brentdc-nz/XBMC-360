#ifndef H_CPODOCUMENT
#define H_CPODOCUMENT

#include <string>
#include <vector>
#include <stdint.h>

enum
{
	ID_FOUND = 0, // We have an entry with a numeric (previously XML) identification number
	MSGID_FOUND = 1, // We have a classic gettext entry with textual msgid. No numeric ID
	MSGID_PLURAL_FOUND = 2 // We have a classic gettext entry with textual msgid in plural form
};

enum Boolean
{
	ISSOURCELANG=true
};

// Struct to hold current position and text of the string field in the main PO entry
struct CStrEntry
{
	size_t Pos;
	std::string Str;
};

// Struct to collect all important data of the current processed entry
struct CPOEntry
{
	int Type;
	uint32_t xID;
	size_t xIDPos;
	std::string Content;
	CStrEntry msgCtxt;
	CStrEntry msgID;
	CStrEntry msgStr;
	std::vector<CStrEntry> msgStrPlural;
};

class CPODocument
{
public:
	CPODocument();
	~CPODocument();

	// Tries to load a PO file into a temporary memory buffer.
	// It also tries to parse the header of the PO file.
	// param pofilename filename of the PO file to load.
	// return true if the load was successful, unless return false
	bool LoadFile(const std::string &pofilename);

	// Fast jumps to the next entry in PO buffer.
	// Finds next entry started with "#: id:" or msgctx or msgid.
	// to be as fast as possible this does not even get the id number
	// just the type of the entry found. GetEntryID() has to be called
	// for getting the id. After that ParseEntry() needs a call for
	// actually getting the msg strings. The reason for this is to
	// have calls and checks as fast as possible generally and specially
	// for parsing weather tokens and to parse only the needed strings from
	// the fallback language (missing from the gui language translation)
	// return true if there was an entry found, false if reached the end of buffer
	bool GetNextEntry();

	// Gets the type of entry found with GetNextEntry.
	// the type of entry: ID_FOUND || MSGID_FOUND || MSGID_PLURAL_FOUND
	int GetEntryType() const {return m_Entry.Type;}

	// Parses the numeric ID from current entry.
	// This function can only be called right after GetNextEntry()
	// to make sure that we have a valid entry detected.
	// return parsed ID number
	uint32_t GetEntryID() const {return m_Entry.xID;}

	// Parses current entry.
	// Reads msgid, msgstr, msgstr[x], msgctxt strings.
	// Note that this function also back-converts the c++ style escape sequences.
	// The function only parses the needed strings, considering if it is a source language file.
	// bisSourceLang if we parse a source English file
	void ParseEntry(bool bisSourceLang);

	// Gets the msgctxt string previously parsed by ParseEntry().
	// return string* containing the msgctxt string, unescaped and linked together.
	const std::string& GetMsgctxt() const {return m_Entry.msgCtxt.Str;}

	// Gets the msgid string previously parsed by ParseEntry().
	// return string* containing the msgid string, unescaped and linked together.
	const std::string& GetMsgid() const {return m_Entry.msgID.Str;}

	// Gets the msgstr string previously parsed by ParseEntry().
	// return string* containing the msgstr string, unescaped and linked together.
	const std::string& GetMsgstr() const {return m_Entry.msgStr.Str;}

	// Gets the msgstr[x] string previously parsed by ParseEntry().
	// plural the number of plural-form expected to get (0-6).
	// string* containing the msgstr string, unescaped and linked together.
	const std::string& GetPlurMsgstr (size_t plural) const;

protected:
	// Converts c++ style char escape sequences back to char.
	// Supports: \a \v \n \t \r \" \0 \f \? \' \\
	// strInput string contains the string to be unescaped.
	// return unescaped string.
	std::string UnescapeString(const std::string &strInput);

	// Finds the position of line, starting with a given string in current entry.
	// This function can only be called after GetNextEntry()
	// strToFind a string what we look for, at beginning of the lines.
	// FoundPos will get the position where we found the line starting with the string.
	// return false if no line like that can be found in the entry (m_Entry)
	bool FindLineStart(const std::string &strToFind, size_t &FoundPos);

	// Reads, and links together the quoted strings found with ParseEntry().
	// This function can only be called after GetNextEntry() called.
	// strEntry.Str a string where we get the appended string lines.
	// strEntry.Pos the position in m_Entry.Content to start reading the string.
	void GetString(CStrEntry &strEntry);

	// Parses the numeric id and checks if it is valid.
	// This function can only be called after GetNextEntry()
	// It checks m_Entry.Content at position m_Entry.xIDPos for the numeric id.
	// The converted ID number goes into m_Entry.xID for public read out.
	// return false, if parse and convert of the id number was unsuccessful.
	bool ParseNumID();

	// If we have Windows or Mac line-end chars in PO file, convert them to Unix LFs
	void ConvertLineEnds(const std::string &filename);

	// Temporary string buffer to read file in
	std::string m_strBuffer;

	// Size of the string buffer
	size_t m_POfilelength;

	// Current cursor position in m_strBuffer
	size_t m_CursorPos;

	// The next PO entry position in m_strBuffer
	size_t m_nextEntryPos;

	// Variable to hold all data of currently processed entry
	CPOEntry m_Entry;
};

#endif //H_CPODOCUMENT