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
};

#endif //CVISUALISATION_MILKDROP2_H