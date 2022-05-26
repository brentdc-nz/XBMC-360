#ifndef H_IWINDOWMANAGERCALLBACK
#define H_IWINDOWMANAGERCALLBACK

class IWindowManagerCallback
{
public:
	IWindowManagerCallback(void){};
	virtual ~IWindowManagerCallback(void){};

	virtual void FrameMove() = 0;	
	virtual void Render() = 0;
	virtual void Process() = 0;
};

#endif //H_IWINDOWMANAGERCALLBACK