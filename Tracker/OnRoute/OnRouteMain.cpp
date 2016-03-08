#include "OnRouteMain.h"

COnRouteMain::COnRouteMain(CIDKTask *pTask)
:	CIDKMainTask(pTask)
{
}

COnRouteMain::~COnRouteMain()
{
}
bool COnRouteMain::OnMessage(CIDKMessage *pHost)
{

	switch(pHost->IsA())
	{
	case CMNO_GPSPOS:
		return ProcessGPSPosition(pHost);
//	case CMNO_GPGGA:
//		return Test(pHost);
	}

	return true;
}

bool COnRouteMain::ProcessGPSPosition(CIDKMessage *pHost)
{
	return true;
}