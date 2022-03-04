#ifndef H_CELD
#define H_CELD

#include <vector>
#include "Utils\StdString.h"
#include "URL.h"

class CEdl
{
public:
	CEdl();
	virtual ~CEdl(void);

	typedef enum 
	{
		CUT = 0,
		MUTE = 1,
		// SCENE = 2,
		COMM_BREAK = 3
	} Action;  

	struct Cut
	{
		int64_t start; // ms
		int64_t end;   // ms
		Action action;
	};

	void Clear();
	bool ReadEditDecisionLists(const CStdString& strMovie, const float fFramesPerSecond, const int iHeight);
	bool InCut(int64_t iSeek, Cut *pCut = NULL);
	bool HasCut();
	bool HasSceneMarker();
	int64_t RemoveCutTime(int64_t iSeek);
	int64_t RestoreCutTime(int64_t iClock);
	CStdString GetInfo();
	static CStdString MillisecondsToTimeString(const int64_t iMilliseconds);

protected:
private:
	int64_t m_iTotalCutTime; // ms
	std::vector<Cut> m_vecCuts;
	std::vector<int64_t> m_vecSceneMarkers;

};

#endif //H_CELD