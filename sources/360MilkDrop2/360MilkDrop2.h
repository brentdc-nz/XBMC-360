#include "visualizations\Visualisation.h"

#ifndef CVISUALISATION_MILKDROP2_H
#define CVISUALISATION_MILKDROP2_H

class CVisualisationMilkDrop2 : public CVisualisation
{
public:
	CVisualisationMilkDrop2(const std::string strVisualisationName);
	~CVisualisationMilkDrop2();

	void Create(LPDIRECT3DDEVICE9 pd3dDevice, int iPosX, int iPosY, int iWidth, int iHeight);
	void AudioData(const short* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength);
	void Render();
	void Stop();
	bool OnAction(VIS_ACTION action, void *param = NULL);
	void GetPresets(char ***pPresets, int *currentPreset, int *numPresets, bool *locked);
	void GetCurrentPreset(char **pPreset, bool *locked);
	bool IsLocked();
	char *GetPreset();

private:
	char **m_presetNames;
	int m_numPresets;
	void BuildPresetNames();
	void FreePresetNames();
};

#endif //CVISUALISATION_MILKDROP2_H