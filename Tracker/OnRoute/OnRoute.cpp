/***************************************************
**
**    III                DDD  KKK
**    III                DDD  KKK
**                       DDD  KKK
**    III   DDDDDDDDDDD  DDD  KKK            KKK
**    III  DDD           DDD  KKK            KKK
**    III  DDD           DDD  KKK           KKK
**    III  DDD           DDD  KKK        KKKKKK
**    III  DDD           DDD  KKK   KKKKKKKKK
**    III  DDD           DDD  KKK        KKKKKK
**    III  DDD           DDD  KKK           KKK
**    III  DDD           DDD  KKK            KKK
**    III   DDDDDDDDDDDDDDDD  KKK            KKK
**
**
**     SSS         FF
**    S           F   T
**     SSS   OO   FF  TTT W   W  AAA  R RR   EEE
**        S O  O  F   T   W W W  AAAA RR  R EEEEE
**    S   S O  O  F   T   W W W A   A R     E
**     SSS   OO  FFF   TT  W W   AAAA R      EEE
**
**
****************************************************
**
**	Filename	: CIDKApplication.h
**	Author		: I.Ferguson
**	Version		: v1.0
**	Date		: 12-05-99
**
****************************************************
**
**	Classes:
**
**	
**
****************************************************/

/** Copyright 1998 IDK Software Ltd **/
// GPSManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "OnRoute.h"
#include "OnRouteTaskFile.h"
#include "OnRouteMain.h"
#include "CIDKThreads.h"
#include "CIDKDate.h"
#include "TaskID.h"
#include "IDKMessage.h"
#include "MessengerConnection.h"

static char RCSString[] = "$Revision: 1.3 $""$Date: 2003/09/09 00:00:05 $"__FILE__;

const char *szAppName = ONROUTE_NAME;

COnRouteError COnRoute::OnRouteYSTEM_ERROR("005");

COnRouteError::COnRouteError(CIDKStr szCode) : CIDKEErrorPackage(szCode),
	INIT_FAIL
		(szCode, 8, "Process Failed: %s",			CIDKEError::IDK_SEVERITY_MESSAGE)
{}

COnRoute::COnRoute()
: CIDKTask(szAppName, ONROUTE_TASK)
{
	m_pOnRouteTaskFile = 0;	
	m_iUpTime = 20;		// 
	m_iHeartRate = 10;
	m_iRebootTime = 0;	// sec from midnight
	m_LastPollTime = 0;
}

COnRoute::~COnRoute()
{
}

bool COnRoute::InitInstance()
{
	bool ret = CIDKTask::InitInstance();
	m_pOnRouteTaskFile = new COnRouteTaskFile(this);

	if (!m_pOnRouteTaskFile->Read(GetIDKHome(),GetApplicationName()))
	{
		return false;
	}

	if (!CIDKTask::InitInstance())
	{
		return false;
	}

	m_pMainTask = new COnRouteMain(this);
	m_pMainTask->Connect();

/*
	if (!InsertTask(new CDrvTerminalTask(this)))
	{
		return false;
	}
*/
	return ret;
}

int COnRoute::Run()
{
	int retval;

//	printf("Got Here");
//	if(!AuxConnect())
//	{
//		return false;
//	}
	CMessengerConnection::EStatus l_Status = m_pMainTask->Listen();
	return true;
}
		
bool COnRoute::PollLoop()
{
	while ( m_bPollTaskLooping )
	{
		IDKSleep(1);
	}
	return(0);
}



COnRoute theApp;