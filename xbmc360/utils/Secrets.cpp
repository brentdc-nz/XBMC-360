#include "Secrets.h"

static const char HIDDEN[] = "*****";

// Names that a key or password follows, in lower case
static const char* KEYS[] = { "api_key=", "apikey=", "access_token=", "token=", "password=" };

void CSecrets::Hide(CStdString& strText)
{
	// scheme://user:password@server/path: who's signed in comes before the
	// server, up to the last @ before the path, and their password after
	// the first : in that
	int iPos = 0;
	while ((iPos = strText.Find("://", iPos)) >= 0)
	{
		iPos += 3;

		int iAt = -1;
		int iEnd = iPos;
		while (iEnd < (int)strText.size() && strText[iEnd] != '/' && strText[iEnd] != '"' && !isspace((unsigned char)strText[iEnd]))
		{
			if (strText[iEnd] == '@')
				iAt = iEnd;
			iEnd++;
		}

		int iColon = strText.Find(':', iPos);
		if (iAt < 0 || iColon < 0 || iColon > iAt)
			continue;

		strText = strText.Left(iColon + 1) + HIDDEN + strText.Mid(iAt);
		iPos = iColon + 1 + sizeof(HIDDEN) - 1;
	}

	CStdString strLower = strText;
	strLower.ToLower();

	// The same, encoded inside another address: zip://smb%3a%2f%2fuser%3apassword%40server%2fgame.zip/game.nes is
	// how a zip on a share is opened
	iPos = 0;
	while ((iPos = strLower.Find("%3a%2f%2f", iPos)) >= 0)
	{
		iPos += 9;

		int iAt = -1;
		int iEnd = iPos;
		while (iEnd < (int)strLower.size() && strLower[iEnd] != '/' && strLower[iEnd] != '"' &&
			!isspace((unsigned char)strLower[iEnd]) && strLower.Mid(iEnd, 3) != "%2f")
		{
			if (strLower.Mid(iEnd, 3) == "%40")
				iAt = iEnd;
			iEnd++;
		}

		int iColon = strLower.Find("%3a", iPos);
		if (iAt < 0 || iColon < 0 || iColon > iAt)
			continue;

		strText = strText.Left(iColon + 3) + HIDDEN + strText.Mid(iAt);
		strLower = strLower.Left(iColon + 3) + HIDDEN + strLower.Mid(iAt);
		iPos = iColon + 3 + sizeof(HIDDEN) - 1;
	}

	if (strText.Find('=') < 0)
		return;

	// name=value in a URL's query, or name="value" in a header

	for (unsigned int i = 0; i < sizeof(KEYS) / sizeof(KEYS[0]); i++)
	{
		int iLength = strlen(KEYS[i]);
		int iFrom = 0;
		while ((iFrom = strLower.Find(KEYS[i], iFrom)) >= 0)
		{
			// The whole name, not the end of a longer one
			if (iFrom > 0 && isalnum((unsigned char)strLower[iFrom - 1]))
			{
				iFrom += iLength;
				continue;
			}

			int iValue = iFrom + iLength;
			bool bQuoted = iValue < (int)strText.size() && strText[iValue] == '"';
			if (bQuoted)
				iValue++;

			int iValueEnd = iValue;
			while (iValueEnd < (int)strText.size() && strText[iValueEnd] != '&' && strText[iValueEnd] != '"' &&
				(bQuoted || (strText[iValueEnd] != ',' && !isspace((unsigned char)strText[iValueEnd]))))
				iValueEnd++;

			if (iValueEnd > iValue)
			{
				strText = strText.Left(iValue) + HIDDEN + strText.Mid(iValueEnd);
				strLower = strLower.Left(iValue) + HIDDEN + strLower.Mid(iValueEnd);
			}

			iFrom = iValue + sizeof(HIDDEN) - 1;
		}
	}
}
