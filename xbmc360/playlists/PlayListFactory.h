#pragma once

#include "utils\StdString.h"

class CFileItem;

namespace PLAYLIST
{
	class CPlayList;

	class CPlayListFactory
	{
		public:
		static CPlayList* Create(const CStdString& filename);
		static CPlayList* Create(const CFileItem& item);
		static bool IsPlaylist(const CStdString& filename);
		static bool IsPlaylist(const CFileItem& item);
	};
}
