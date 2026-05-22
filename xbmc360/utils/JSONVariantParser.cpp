/*
 *      Copyright (C) 2005-2010 Team XBMC
 *      http://www.xbmc.org
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
 */

#include "JSONVariantParser.h"
#include <stdlib.h>
#include <string.h>
#include <sstream>

// ===== Parser =====

CVariant CJSONVariantParser::Parse(const std::string &json)
{
	return Parse(json.c_str(), (unsigned int)json.length());
}

CVariant CJSONVariantParser::Parse(const char *json, unsigned int length)
{
	if (!json || length == 0)
		return CVariant();

	const char *p = json;
	const char *end = json + length;
	return ParseValue(p, end);
}

void CJSONVariantParser::SkipWhitespace(const char *&p, const char *end)
{
	while (p < end && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n'))
		p++;
}

CVariant CJSONVariantParser::ParseValue(const char *&p, const char *end)
{
	SkipWhitespace(p, end);
	if (p >= end)
		return CVariant();

	switch (*p)
	{
		case '{': return ParseObject(p, end);
		case '[': return ParseArray(p, end);
		case '"': { std::string s = ParseString(p, end); return CVariant(s); }
		case 't':
			if (p + 4 <= end && strncmp(p, "true", 4) == 0) { p += 4; return CVariant(true); }
			return CVariant();
		case 'f':
			if (p + 5 <= end && strncmp(p, "false", 5) == 0) { p += 5; return CVariant(false); }
			return CVariant();
		case 'n':
			if (p + 4 <= end && strncmp(p, "null", 4) == 0) { p += 4; return CVariant(); }
			return CVariant();
		default:
			if (*p == '-' || (*p >= '0' && *p <= '9'))
				return ParseNumber(p, end);
			return CVariant();
	}
}

CVariant CJSONVariantParser::ParseObject(const char *&p, const char *end)
{
	CVariant obj(CVariant::VariantTypeObject);
	p++; // skip '{'
	SkipWhitespace(p, end);

	if (p < end && *p == '}') { p++; return obj; }

	while (p < end)
	{
		SkipWhitespace(p, end);
		if (p >= end || *p != '"') break;

		std::string key = ParseString(p, end);

		SkipWhitespace(p, end);
		if (p >= end || *p != ':') break;
		p++; // skip ':'

		CVariant value = ParseValue(p, end);
		obj[key] = value;

		SkipWhitespace(p, end);
		if (p >= end) break;
		if (*p == '}') { p++; return obj; }
		if (*p == ',') { p++; continue; }
		break;
	}

	return obj;
}

CVariant CJSONVariantParser::ParseArray(const char *&p, const char *end)
{
	CVariant arr(CVariant::VariantTypeArray);
	p++; // skip '['
	SkipWhitespace(p, end);

	if (p < end && *p == ']') { p++; return arr; }

	while (p < end)
	{
		CVariant value = ParseValue(p, end);
		arr.append(value);

		SkipWhitespace(p, end);
		if (p >= end) break;
		if (*p == ']') { p++; return arr; }
		if (*p == ',') { p++; continue; }
		break;
	}

	return arr;
}

std::string CJSONVariantParser::ParseString(const char *&p, const char *end)
{
	if (p >= end || *p != '"')
		return "";

	p++; // skip opening quote
	std::string result;
	result.reserve(32);

	while (p < end && *p != '"')
	{
		if (*p == '\\')
		{
			p++;
			if (p >= end) break;
			switch (*p)
			{
				case '"':  result += '"'; break;
				case '\\': result += '\\'; break;
				case '/':  result += '/'; break;
				case 'b':  result += '\b'; break;
				case 'f':  result += '\f'; break;
				case 'n':  result += '\n'; break;
				case 'r':  result += '\r'; break;
				case 't':  result += '\t'; break;
				case 'u':
					// Skip unicode escape (4 hex digits)
					p++;
					for (int i = 0; i < 4 && p < end; i++) p++;
					p--; // will be incremented below
					result += '?'; // placeholder
					break;
				default: result += *p; break;
			}
		}
		else
		{
			result += *p;
		}
		p++;
	}

	if (p < end && *p == '"')
		p++; // skip closing quote

	return result;
}

CVariant CJSONVariantParser::ParseNumber(const char *&p, const char *end)
{
	const char *start = p;
	bool isFloat = false;

	if (*p == '-') p++;
	while (p < end && *p >= '0' && *p <= '9') p++;
	if (p < end && *p == '.')
	{
		isFloat = true;
		p++;
		while (p < end && *p >= '0' && *p <= '9') p++;
	}
	if (p < end && (*p == 'e' || *p == 'E'))
	{
		isFloat = true;
		p++;
		if (p < end && (*p == '+' || *p == '-')) p++;
		while (p < end && *p >= '0' && *p <= '9') p++;
	}

	std::string numStr(start, p);
	if (isFloat)
		return CVariant(atof(numStr.c_str()));
	else
	{
		int64_t val = _atoi64(numStr.c_str());
		if (val >= 0)
			return CVariant((int)val);
		else
			return CVariant((int)val);
	}
}

// ===== Writer =====

std::string CJSONVariantWriter::Write(const CVariant &value, bool compact)
{
	std::string output;
	output.reserve(256);
	WriteValue(value, output, compact, 0);
	return output;
}

void CJSONVariantWriter::WriteString(const std::string &str, std::string &output)
{
	output += '"';
	for (size_t i = 0; i < str.length(); i++)
	{
		char c = str[i];
		switch (c)
		{
			case '"':  output += "\\\""; break;
			case '\\': output += "\\\\"; break;
			case '\b': output += "\\b"; break;
			case '\f': output += "\\f"; break;
			case '\n': output += "\\n"; break;
			case '\r': output += "\\r"; break;
			case '\t': output += "\\t"; break;
			default:
				if ((unsigned char)c < 0x20)
				{
					char buf[8];
					sprintf(buf, "\\u%04x", (unsigned char)c);
					output += buf;
				}
				else
					output += c;
				break;
		}
	}
	output += '"';
}

void CJSONVariantWriter::WriteValue(const CVariant &value, std::string &output, bool compact, int indent)
{
	switch (value.type())
	{
		case CVariant::VariantTypeNull:
		case CVariant::VariantTypeConstNull:
			output += "null";
			break;

		case CVariant::VariantTypeBoolean:
			output += value.asBoolean() ? "true" : "false";
			break;

		case CVariant::VariantTypeInteger:
		{
			char buf[32];
			sprintf(buf, "%lld", (long long)value.asInteger());
			output += buf;
			break;
		}

		case CVariant::VariantTypeUnsignedInteger:
		{
			char buf[32];
			sprintf(buf, "%llu", (unsigned long long)value.asUnsignedInteger());
			output += buf;
			break;
		}

		case CVariant::VariantTypeDouble:
		{
			char buf[64];
			sprintf(buf, "%g", value.asDouble());
			output += buf;
			break;
		}

		case CVariant::VariantTypeString:
			WriteString(value.asString(), output);
			break;

		case CVariant::VariantTypeArray:
		{
			output += '[';
			for (unsigned int i = 0; i < value.size(); i++)
			{
				if (i > 0) output += ',';
				if (!compact) output += ' ';
				WriteValue(value[i], output, compact, indent + 1);
			}
			output += ']';
			break;
		}

		case CVariant::VariantTypeObject:
		{
			output += '{';
			bool first = true;
			for (CVariant::const_iterator_map it = value.begin_map(); it != value.end_map(); ++it)
			{
				if (!first) output += ',';
				if (!compact) output += ' ';
				first = false;
				WriteString(it->first, output);
				output += ':';
				WriteValue(it->second, output, compact, indent + 1);
			}
			output += '}';
			break;
		}

		default:
			output += "null";
			break;
	}
}
