#pragma once
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

#include "utils\Variant.h"
#include <string>

// Simple JSON parser (recursive descent) - replaces yajl dependency from Eden
class CJSONVariantParser
{
public:
	static CVariant Parse(const char *json, unsigned int length);
	static CVariant Parse(const std::string &json);

private:
	static CVariant ParseValue(const char *&p, const char *end);
	static CVariant ParseObject(const char *&p, const char *end);
	static CVariant ParseArray(const char *&p, const char *end);
	static std::string ParseString(const char *&p, const char *end);
	static CVariant ParseNumber(const char *&p, const char *end);
	static void SkipWhitespace(const char *&p, const char *end);
};

// Simple JSON writer - replaces yajl dependency from Eden
class CJSONVariantWriter
{
public:
	static std::string Write(const CVariant &value, bool compact = true);

private:
	static void WriteValue(const CVariant &value, std::string &output, bool compact, int indent);
	static void WriteString(const std::string &str, std::string &output);
};
