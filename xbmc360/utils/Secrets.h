#ifndef H_CSECRETS
#define H_CSECRETS

#include "utils\StdString.h"

// Passwords and keys, kept out of what's written down, such as the log
class CSecrets
{
public:
	// Puts ***** for a URL's password (smb://user:password@server/...), also
	// when it's encoded in another URL (zip://smb%3a%2f%2fuser%3apassword%40...),
	// and for keys given in a URL or a header (api_key=..., Token="...")
	static void Hide(CStdString& strText);
};

#endif //H_CSECRETS
