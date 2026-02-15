#ifndef CVISUALISATIONBASE_H
#define CVISUALISATIONBASE_H

#include <xtl.h>
#include <string>

class CVisualisation
{
public:
	CVisualisation(const std::string strVisualisationName);
	virtual ~CVisualisation();

	virtual void Create(LPDIRECT3DDEVICE9 pd3dDevice, int iPosX, int iPosY, int iWidth, int iHeight) = 0;
	virtual void AudioData(const short* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength) = 0;
	virtual void Render() = 0;
	virtual void Stop() = 0;

protected:
	std::string m_strVisualisationName;
};

#endif //CVISUALISATIONBASE_H