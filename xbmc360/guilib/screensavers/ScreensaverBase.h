#ifndef H_GUILIB_CSCREENSAVERBASE
#define H_GUILIB_CSCREENSAVERBASE

class CSceensaverBase
{
public:
	CSceensaverBase::CSceensaverBase() {};
	virtual CSceensaverBase::~CSceensaverBase() {};

	virtual bool Initialize() = 0;
	virtual void Render() = 0;
	virtual bool Close() = 0;
};

#endif //H_GUILIB_CSCREENSAVERBASE