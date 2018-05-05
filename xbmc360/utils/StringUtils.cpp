#include "StringUtils.h"
#include <vector>

using namespace std;

// Converts string to wide
void CStringUtils::StringtoWString(CStdString strText, std::wstring &strResult)
{
	wchar_t* wtext = new wchar_t[strText.size()+1];
	mbstowcs(wtext, strText.c_str(), strlen(strText.c_str())+1);
	strResult = wtext;
	delete wtext;
}

// Splits the string input into pieces delimited by delimiter.
// if 2 delimiters are in a row, it will include the empty string between them.
// added MaxStrings parameter to restrict the number of returned substrings (like perl and python)
int CStringUtils::SplitString(const CStdString& input, const CStdString& delimiter, CStdStringArray &results, unsigned int iMaxStrings /* = 0 */)
{
	int iPos = -1;
	int newPos = -1;
	int sizeS2 = delimiter.GetLength();
	int isize = input.GetLength();

	results.clear();

	vector<unsigned int> positions;

	newPos = input.Find (delimiter, 0);

	if ( newPos < 0 )
	{
		results.push_back(input);
		return 1;
	}

	while ( newPos > iPos )
	{
		positions.push_back(newPos);
		iPos = newPos;
		newPos = input.Find (delimiter, iPos + sizeS2);
	}

	// numFound is the number of delimeters which is one less
	// than the number of substrings
	unsigned int numFound = positions.size();
	if (iMaxStrings > 0 && numFound >= iMaxStrings)
		numFound = iMaxStrings - 1;

	for ( unsigned int i = 0; i <= numFound; i++ )
	{
		CStdString s;
		if ( i == 0 )
		{
			if ( i == numFound )
				s = input;
			else
				s = input.Mid( i, positions[i] );
		}
		else
		{
			int offset = positions[i - 1] + sizeS2;
			if ( offset < isize )
			{
				if ( i == numFound )
					s = input.Mid(offset);
			    else if ( i > 0 )
					 s = input.Mid( positions[i - 1] + sizeS2,
                         positions[i] - positions[i - 1] - sizeS2 );
			}	
		}
		results.push_back(s);
	}
	// return the number of substrings
	return results.size();
}

// assumes it is called from after the first open bracket is found
int CStringUtils::FindEndBracket(const CStdString &str, char opener, char closer, int startPos)
{
	int blocks = 1;
	for (unsigned int i = startPos; i < str.size(); i++)
	{
		if (str[i] == opener)
			blocks++;
		else if (str[i] == closer)
		{
			blocks--;
			if (!blocks)
				return i;
		}
	}
	return (int)CStdString::npos;
}

bool CStringUtils::IsNaturalNumber(const CStdString& str)
{
	if (0 == (int)str.size())
		return false;

	for (int i = 0; i < (int)str.size(); i++)
	{
		if ((str[i] < '0') || (str[i] > '9')) return false;
	}
	return true;
}

CStdString CStringUtils::SecondsToTimeString(long lSeconds, TIME_FORMAT format)
{
	int hh = lSeconds / 3600;
	lSeconds = lSeconds % 3600;
	int mm = lSeconds / 60;
	int ss = lSeconds % 60;

	if (format == TIME_FORMAT_GUESS)
		format = (hh >= 1) ? TIME_FORMAT_HH_MM_SS : TIME_FORMAT_MM_SS;
	
	CStdString strHMS;
	
	if (format & TIME_FORMAT_HH)
		strHMS.AppendFormat("%02.2i", hh);
	else if (format & TIME_FORMAT_H)
		strHMS.AppendFormat("%i", hh);
	
	if (format & TIME_FORMAT_MM)
		strHMS.AppendFormat(strHMS.IsEmpty() ? "%02.2i" : ":%02.2i", mm);
	
	if (format & TIME_FORMAT_SS)
		strHMS.AppendFormat(strHMS.IsEmpty() ? "%02.2i" : ":%02.2i", ss);
	
	return strHMS;
}