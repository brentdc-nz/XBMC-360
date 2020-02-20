#ifndef CNTPCLIENT_H
#define CNTPCLIENT_H

#include "Thread.h"

class CNTPClient : public CThread
{
public:
	CNTPClient() {};
	~CNTPClient() {};

	void SyncTime();

private:
	virtual void Process();
	void DoTimeSync();
};

#endif //CNTPCLIENT_H