#pragma once
#include "IdkMainTask.h"

class COnRouteMain :
	public CIDKMainTask
{
	bool ProcessGPSPosition(CIDKMessage *pHost);
public:
	COnRouteMain(CIDKTask *pTask);
	~COnRouteMain();
	
	bool OnMessage(CIDKMessage *pHost);
};
