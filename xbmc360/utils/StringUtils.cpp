#include "StringUtils.h"
#include <vector>

using namespace std;

// empty string for use in returns by ref
const CStdString CStringUtils::EmptyString = "";

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

void CStringUtils::StringSplit(CStdString str, CStdString delim, std::vector<CStdString>* results, bool bShowEmptyEntries)
{
	unsigned int cutAt;
	
	while((cutAt = str.find_first_of(delim)) != str.npos)
	{
		if(!bShowEmptyEntries)
		{
			if(cutAt > 0)
				results->push_back(str.substr(0,cutAt));
		}
		else
			results->push_back(str.substr(0, cutAt));

		str = str.substr(cutAt+1);
	}

	if(str.length() > 0)
		results->push_back(str);
}

vector<string> CStringUtils::Split(const CStdString& input, const CStdString& delimiter, unsigned int iMaxStrings /* = 0 */)
{
	CStdStringArray result;
	SplitString(input, delimiter, result, iMaxStrings);

	vector<string> strArray;
	for (unsigned int index = 0; index < result.size(); index++)
		strArray.push_back(result.at(index));

	return strArray;
}

// Assumes it is called from after the first open bracket is found
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

bool CStringUtils::IsInteger(const CStdString& str)
{
	if (str.size() > 0 && str[0] == '-')
		return IsNaturalNumber(str.Mid(1));
	else
		return IsNaturalNumber(str);
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

long CStringUtils::TimeStringToSeconds(const CStdString &timeString)
{  
	if(timeString.Right(4).Equals(" min"))
	{
		// This is imdb format of "XXX min"
		return 60 * atoi(timeString.c_str());
	}
	else
	{
		CStdStringArray secs;
		CStringUtils::SplitString(timeString, ":", secs);
		
		int timeInSecs = 0;
		for (unsigned int i = 0; i < secs.size(); i++)
		{
			timeInSecs *= 60;
			timeInSecs += atoi(secs[i]);
		}
		return timeInSecs;
	}
}

CStdString CStringUtils::ReplaceAllA(CStdString s, CStdString sub, CStdString other)
{
	size_t b = 0;
	for(;;)
	{
		b = s.find(sub, b);
		if (b == s.npos) break;
		s.replace(b, sub.size(), other);
		b += other.size();
	}
	return s;
}

CStdString CStringUtils::sprintfa(const char *format, ...)
{
	char temp[16384];

	va_list ap;
	va_start (ap, format);
	vsprintf_s (temp, 16384, format, ap);
	va_end (ap);

	return temp;
}

CStdString CStringUtils::MakeLowercase(CStdString strTmp)
{
	struct lowercase_func
	{
		void operator()(std::string::value_type& v) { v = (char)tolower(v); }
	};

	for_each(strTmp.begin(), strTmp.end(), lowercase_func());
	return strTmp;
}

size_t CStringUtils::FindWords(const char *str, const char *wordLowerCase)
{
	// NOTE: This assumes word is lowercase!
	unsigned char *s = (unsigned char *)str;
	do
	{
		// Start with a compare
		unsigned char *c = s;
		unsigned char *w = (unsigned char *)wordLowerCase;
		bool same = true;
		
		while (same && *c && *w)
		{
			unsigned char lc = *c++;

			if (lc >= 'A' && lc <= 'Z')
				lc += 'a'-'A';

			if (lc != *w++) // Different
				same = false;
		}

		if (same && *w == 0)  // Only the same if word has been exhausted
			return (const char *)s - str;

		// Otherwise, find a space and skip to the end of the whitespace
		while (*s && *s != ' ') s++;
		while (*s && *s == ' ') s++;

		// and repeat until we're done
	} while (*s);

	return CStdString::npos;
}

void CStringUtils::WordToDigits(CStdString &word) 
{
	static const char word_to_letter[] = "22233344455566677778889999";
	word.ToLower();
	
	for (unsigned int i = 0; i < word.size(); ++i)
	{
		// NB: This assumes ascii, which probably needs extending at some  point.
		char letter = word[i];
		if ((letter >= 'a' && letter <= 'z')) // assume contiguous letter range
		{  
			word[i] = word_to_letter[letter-'a'];
		}
		else if (letter < '0' || letter > '9') // We want to keep 0-9!
		{  
			word[i] = ' ';  // replace everything else with a space
		}
	}
}