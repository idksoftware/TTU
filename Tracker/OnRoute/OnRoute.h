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
**	Filename	: CIDKTask.h
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

#if !defined(CTRIPLOGGER_H__6AF530E4_F0AE_11D2_807B_00A024DFDF9E__INCLUDED_)
#define CTRIPLOGGER_H__6AF530E4_F0AE_11D2_807B_00A024DFDF9E__INCLUDED_

#include "CIDKTask.h"

class CMessengerConnection;
class COnRouteError : public CIDKEErrorPackage
{
public:

	COnRouteError(CIDKStr szCode);

	CIDKEError INIT_FAIL;
};
class COnRouteTaskFile;

class COnRoute : public CIDKTask
{
public:

private:

	friend COnRouteTaskFile;
	
protected:
	int m_iTimeInterval;
	CMessengerConnection *m_pListen;
	COnRouteTaskFile *m_pOnRouteTaskFile;
	int	m_iUpTime;		// 
	int	m_iHeartRate;
	int m_iRebootTime;	// sec from midnight
	time_t	m_LastPollTime;
	void CheckActivity();
	void CreateDiagnosticsFile();
	int		_wordlen;
	int		_readlen;
	int		_doshift;
	CIDKStr	_checkOutput[200];
	int		_oldTime;
	bool	_defaultMessage;
	double	_defaultGeoFence;
	int		_defaultPassAngle;
	int		_atStopLookAhead;
	CIDKStr	_smoothingMethod;
	bool	_routeStarted;
	int		_readCount;
	double  _defaultExitAddOn;
	double	_defaultArrivingAddOn;
	double	_veryCloseAddOn;
	double	_tightVeryCloseAddOn;
	int		_coverageLossThreshold;
	int		_coverageLossCount;



public:
	COnRoute();
	~COnRoute();

	bool PollLoop();
	bool InitInstance();
	virtual int Run();
	static COnRouteError OnRouteYSTEM_ERROR;
};

#endif


/*
class TaskMonitor : public CMessengerConnection
{
public:
	TaskMonitor(char *, char *, int);
	~TaskMonitor();
	virtual STATUS taskLoop();
	STATUS taskInitialize();
	STATUS processMessage(char *, int);
	STATUS taskPing (CMSG_PINGREQUEST inRequest);
	void createDiagnosticsFile();

private:
	void checkActivity();
	STATUS updateTidyUp();
	void heartBeat();
	checkLinariaTripLogLock();

	int		_testJava;
	time_t	_lastPoll;
	int		_pollDelay;
	int		_diagOnScreen;
	int		_heartRate;

	char	_tlLockFile[50];
	int			_tlLockKeep;
	time_t		_tlLockDetect;
};
*/