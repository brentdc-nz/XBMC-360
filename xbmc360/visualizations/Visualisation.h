#ifndef CVISUALISATIONBASE_H
#define CVISUALISATIONBASE_H

#include <xtl.h>
#include <string>

class CVisualisation
{
public:
	enum VIS_ACTION { VIS_ACTION_NONE = 0,
	                  VIS_ACTION_NEXT_PRESET,
	                  VIS_ACTION_PREV_PRESET,
	                  VIS_ACTION_LOAD_PRESET,
	                  VIS_ACTION_RANDOM_PRESET,
	                  VIS_ACTION_LOCK_PRESET,
	                  VIS_ACTION_RATE_PRESET_PLUS,
	                  VIS_ACTION_RATE_PRESET_MINUS,
	                  VIS_ACTION_UPDATE_ALBUMART,
	                  VIS_ACTION_UPDATE_TRACK
	};

	CVisualisation(const std::string strVisualisationName);
	virtual ~CVisualisation();

	virtual void Create(LPDIRECT3DDEVICE9 pd3dDevice, int iPosX, int iPosY, int iWidth, int iHeight) = 0;
	virtual void AudioData(const short* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength) = 0;
	virtual void Render() = 0;
	virtual void Stop() = 0;
	virtual bool OnAction(VIS_ACTION action, void *param = NULL) { return false; }
	virtual void GetPresets(char ***pPresets, int *currentPreset, int *numPresets, bool *locked) {}
	virtual void GetCurrentPreset(char **pPreset, bool *locked) {}
	virtual bool IsLocked() { return false; }
	virtual char *GetPreset() { return NULL; }

protected:
	std::string m_strVisualisationName;
};

#endif //CVISUALISATIONBASE_H