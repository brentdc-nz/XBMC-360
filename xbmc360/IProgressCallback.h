#ifndef IPROGRESSCALLBACK_H
#define IPROGRESSCALLBACK_H

class IProgressCallback
{
public:
	virtual ~IProgressCallback() {}
	virtual void SetProgressMax(int max) = 0;
	virtual void SetProgressAdvance(int nSteps=1) = 0;
	virtual bool Abort() = 0;
};

#endif //IPROGRESSCALLBACK_H