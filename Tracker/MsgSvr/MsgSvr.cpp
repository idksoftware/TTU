/***************************************************

****************************************************
**
**	Filename	: CIDKApplication.cpp
**	Author		: I.Ferguson
**	Version		: v1.0
**	Date		: 12-05-99
**
****************************************************
**
**	Classes:
**
**	CDPLRowCtrl implementation
**
****************************************************/

/** Copyright 1998 IDK Software Ltd **/

#include "stdafx.h"
#include "MsgSvr.h"
#include <direct.h>
#include "CIDKArgumentList.h"
#include "CIDKEnvirArguments.h"
#include "CIDKStringFields.h"
#include "CIDKEErrorLog.h"
#include "CIDKFileFind.h"
#include "CIDKFile.h"
#include "IDKMachine.h"
#include "CIDKThreads.h"
#include "ConfigFile.h"
#include "MsgSvrFile.h"
#include "TCPServer.h"

#include <time.h>
#include <stdio.h>
#include <io.h>





#define ARGTOK_CONNECT_STR			ARGSTOK_BASE + 1
#define ARGTOK_STREAMNAME			ARGTOK_CONNECT_STR + 1		// Stream name stat, acc etc.
#define ARGTOK_TRACE_ON				ARGTOK_STREAMNAME + 1	// Trace on/off
#define ARGTOK_INIT_ONLY			ARGTOK_TRACE_ON + 1	// Initalise only
#define ARGTOK_LOG_LEVEL			ARGTOK_INIT_ONLY + 1	// Log warning level
#define ARGTOK_APPPATH				ARGTOK_LOG_LEVEL + 1	// IDK_HOME path
#define ARGTOK_ECHO_ON				ARGTOK_APPPATH + 1		// Echo error logs to screen
#define ARGTOK_LOGSIZE				ARGTOK_ECHO_ON + 1				// Log file max lines.
#define ARGTOK_LOGPERIOD			ARGTOK_LOGSIZE + 1				// Archive period.
#define ARGTOK_LOGPATH				ARGTOK_LOGPERIOD + 1			// Log file path.
#define ARGTOK_SENT2			ARGTOK_LOGPATH + 1
#define ARGTOK_IP_ADDR				ARGTOK_SENT2 + 1	
#define ARGTOK_COMMAND				ARGTOK_IP_ADDR + 1		// Command to be sent

#define LOG_FILE	"/logs"
#define APP_PATH	"/bin"
#define CFG_PATH	"/config"
//#define STREAM_PATH	"/ddf"

// GPSManager [$CNT,"TRIPLOGGER","Start"]


CMsgSvrApplicationError CMsgSvrApp::MSGSVRAPP_ERROR("001");

CMsgSvrApplicationError::CMsgSvrApplicationError(CIDKStr szCode) : CIDKEErrorPackage(szCode),
	APPLICATION_START
		(szCode,  1, "Starting %s Version 2.1 for IISVR",		CIDKEError::IDK_SEVERITY_GENERAL),
	APPLICATION_END
		(szCode,  2, "Ending %s for '%s'",					CIDKEError::IDK_SEVERITY_GENERAL),
	APPLICATION_FAIL
		(szCode,  3, "%s failed for '%s'",					CIDKEError::IDK_SEVERITY_GENERAL),
	APPLICATION_TIME
		(szCode,  4, "%s process took %s h %s m %s s",		CIDKEError::IDK_SEVERITY_MESSAGE),	
	NO_IDK_HOME_ENV
		(szCode,  5, "No IDK_HOME environment variable",	CIDKEError::IDK_SEVERITY_ERROR),
	DIAG_LOGSIZE
		(szCode, 6, "Using log file size %s",				CIDKEError::IDK_SEVERITY_MESSAGE),
	DIAG_LOGPERIOD
		(szCode, 7, "Using log period %s days",				CIDKEError::IDK_SEVERITY_MESSAGE),
	LOG_PATH
		(szCode, 8, "Log path: %s",							CIDKEError::IDK_SEVERITY_MESSAGE),
	IPADDRESS
		(szCode, 9, "Destination IP Address: %s",			CIDKEError::IDK_SEVERITY_MESSAGE),
	COMMAND
		(szCode, 10, "Command to be sent: %s",				CIDKEError::IDK_SEVERITY_MESSAGE),

	NUM_ACTIVE_CONNECTIONS
		(szCode,  1, "Max number of active connections '%s'",
															CIDKEError::IDK_SEVERITY_MESSAGE),
	NUM_CONNECTIONS
		(szCode,  2, "Max number of connections '%s'",		CIDKEError::IDK_SEVERITY_MESSAGE),
	NUM_INPUT_QUEUE
		(szCode,  3, "Max number on the Input Queue %s",	CIDKEError::IDK_SEVERITY_MESSAGE),
	PORT_NUM
		(szCode,  4, "Port number %s",						CIDKEError::IDK_SEVERITY_MESSAGE),
	NUM_RETRIES
		(szCode, 5, "Max number of retries is %s",			CIDKEError::IDK_SEVERITY_MESSAGE),
	CONNECTION_TIMEOUT
		(szCode, 6, "Connection timeout is %s sec",			CIDKEError::IDK_SEVERITY_MESSAGE),
	IDLE_TIMEOUT
		(szCode, 7, "Idle timeout is %s sec",				CIDKEError::IDK_SEVERITY_MESSAGE),
	FILTER_ON
		(szCode, 8, "Filter slot %s msg %s comms %s",		CIDKEError::IDK_SEVERITY_MESSAGE),
	FILTER_DEFAULT_PROT0COL
		(szCode, 9, "Filter default protocol %s",			CIDKEError::IDK_SEVERITY_MESSAGE),
	OBS_INITIAISING
		(szCode, 9, "Outbound Server Initalising",			CIDKEError::IDK_SEVERITY_MESSAGE),
	OBS_RUNNING
		(szCode, 9, "Outbound Server Started",				CIDKEError::IDK_SEVERITY_MESSAGE)
{}

CIDKStr CMsgSvrApp::s_IDK_HOME;
CIDKStr CMsgSvrApp::s_LogFilePath;


class CIISVRArgumentList : public CIDKArgumentList
{
public:
	CIISVRArgumentList();
};

CIISVRArgumentList::CIISVRArgumentList()
{

	if (!AddOption(ARGTOK_TRACE_ON, ARG_TRACE_ON,
							"Trace on/off.", false, 0, false))
	{
		return;
	}

	if (!AddOption(ARGTOK_INIT_ONLY, ARG_INIT_ONLY,
							"Only runs through the initialization phase of the application.", false, 0))
	{
		return;
	}

	if (!AddArgument(ARGTOK_LOG_LEVEL, ARG_LOG_LEVEL,
							"The log warning level (default = MESSAGE).", "DETAIL", 0))
	{
		return;
	}

	if (!AddOption(ARGTOK_ECHO_ON, ARG_ECHO_ON,
							"Echoes error logs to screen.", false, 0, false))
	{
		return;
	}
	
	if (!AddArgument(ARGTOK_APPPATH, 0, 0, "", "IDK_HOME", true))
	{
		return;
	}

	if (!AddArgument(ARGTOK_LOGSIZE, 0, 0, "1000", "LOG_SIZE"))
	{
		return;
	}

	if (!AddArgument(ARGTOK_LOGPERIOD, 0, 0, "30", "LOG_PERIOD"))
	{
		return;
	}

	if (!AddArgument(ARGTOK_IP_ADDR, ARG_IP_ADDR "IP Address of the vehicle to send the command", false, 0, false))
	{
		return;
	}
 
	if (!AddParameter(ARGTOK_SENT2, " to send the command","", 1, "", true))
	{
		return;
	}

	if (!AddParameter(ARGTOK_COMMAND, "Command to be sent","", 1, "", true))
	{
		return;
	}

	// Note there is no default as the default is $PPDM_HOME/logs 
#ifdef LINUX
	if (!AddArgument(ARGTOK_LOGPATH, 0, 0, "/var/idk/logs", "IDK_LOGS"))
#else
	if (!AddArgument(ARGTOK_LOGPATH, 0, 0, "", "IDK_LOGS"))
#endif
	{
		return;
	}

	SetOk();
}

bool CIISVRCmdLine::Process(int argc, char *argv[])
{
	CIDKApplication *l_pApp = GetApp();

	if (!l_pApp)
	// Command line arguments installed
	{
		return false;
	}
	//
	// Initalise the Command line arguments
	//
	switch (ProcessCommands(argc, argv))
		{
		case CIDKArgumentList::DO_ERROR:
			// Do help if switched on
			if (!OnError())
			{
				return false;
			}
			return false; // Failed to Initalise

		case CIDKArgumentList::DO_HELP:
			if (!OnHelp())
			{
				return false;
			}
			return false;

		case CIDKArgumentList::DO_COPYRIGHT:
			if (!OnCopyRight())
			{
				return false;
			}
			return false;

		case CIDKArgumentList::DO_USING:
			if (!OnUsing())
			{
				return false;
			}
			return 0;

		case CIDKArgumentList::DO_PROCESS_NORMALY:
			break; // continue on to the next item.
		}
	return true;
}

const char *szAppName = "MSGSVR";

//////////////////////////////////////////////////////////////////////////////////
//
// Help amd Copy Right Notice
//
const char *szHelpPage =	"Usage:\n"
							"\n"
							"%s [arguments...]\n"
							"\n";
			
const char *szCopyRightNotice = "IDK Software Ltd(R) %s Version 1.3\n"
								"Copyright (C) IDK Software Ltd\n"
								"All rights reserved.";

CMsgSvrApp::CMsgSvrApp()
: CIDKApplication(szHelpPage, szCopyRightNotice, szAppName, "1.1"),
	CMsgSvrApplicationEObject(szAppName)
{

	m_ConfigFile = 0;

}

CMsgSvrApp::~CMsgSvrApp()
{

	if (m_ConfigFile)
	{
		delete m_ConfigFile;
	}
	if (m_TCPServer)
	{
		delete m_TCPServer;
	}
}

bool CMsgSvrApp::Initalise(int argc, char *argv[])
{
	InstallArgumentList(new CIISVRArgumentList);
	CIDKEnvironmentArguments l_ea;
	if (!l_ea.Process())
	{
		return false;
	}
	CIISVRCmdLine l_cl;
	if (!l_cl.Process(argc, argv))
	{
		return false;
	}
		
	CIDKArgumentList *l_ArgumentList = GetArgumentList();
	if (!l_ArgumentList)
	{
		return false;
	}
	//
	// Set the if the echo is set on.
	// line.
	//
	CIDKArgument::EReturnCode rc;

	if ((rc = l_ArgumentList->GetFlagValue(ARGTOK_ECHO_ON)) == CIDKArgument::RC_ERROR)
	{
		// ERROR
		return false;
	}
	
	m_bIsEchoOn = (rc == CIDKArgument::RC_FOUND)?true:false;

	GetLogObject()->SetEcho(m_bIsEchoOn);


	//
	// Set IDKDB HOME from the path.
	//
	s_IDK_HOME = l_ArgumentList->GetValue(ARGTOK_APPPATH);
	if (s_IDK_HOME.IsEmpty())
	{
		// The Error is printed since the init log cannot be
		// opened until IDKDB is found..
		PrintCopyRightNotice();
		printf("\n\n%s\n",(char*)(MSGSVRAPP_ERROR.NO_IDK_HOME_ENV.GetErrorMessage()));
		return false;
	}

	// check for a change of log file path
	PrintCopyRightNotice();

	s_LogFilePath = l_ArgumentList->GetValue(ARGTOK_LOGPATH);
	if (s_LogFilePath.IsEmpty())
	{
		s_LogFilePath = s_IDK_HOME;
		s_LogFilePath += LOG_FILE;
	}

	// Init and open the log file with sensible values

	GetLogObject()->SetLogPath(s_LogFilePath);

	if (!StartLog(CIDKEError::IDK_SEVERITY_MESSAGE, false, true))
	{	
		return false;
	}

	SetCfgFilePath(l_ArgumentList->GetValue(ARGTOK_APPPATH));
	SetCfgFilename(CFG_PATH);

	//
	// Set the if the log level is set on.
	// line.
	//
	m_szSeverity = l_ArgumentList->GetValue(ARGTOK_LOG_LEVEL);

	if (m_szSeverity.IsEmpty())
	{
		m_szSeverity = "MESSAGE";
	}

	//
	// Set the if the Trace is eset on.
	// line.
	//
	if ((rc = l_ArgumentList->GetFlagValue(ARGTOK_TRACE_ON)) == CIDKArgument::RC_ERROR)
	{
		// ERROR
		return false;
	}
	m_bIsTraceOn = (rc == CIDKArgument::RC_FOUND)?true:false;
	
	// Convert from command line string to enumerated
	// type for log file severity.  By Default SEVERITY = MESSAGE

	CIDKEError::SEVERITY eSeverity = CIDKEError::IDK_SEVERITY_MESSAGE;

	if (GetSeverity() == "DETAIL")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_DETAIL;
	}
	else if (GetSeverity() == "DEBUG")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_DEBUG;
	}
	else if (GetSeverity() == "WARNING")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_WARNING;
	}
	else if (GetSeverity() == "MESSAGE")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_MESSAGE;
	}
	else if (GetSeverity() == "ERROR")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_ERROR;
	}
	else if (GetSeverity() == "FATAL")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_FATAL;
	}
	else if (GetSeverity() == "GENERAL")
	{
		eSeverity = CIDKEError::IDK_SEVERITY_GENERAL;
	}
	else if (GetSeverity() == "DUMP")
	{
		// Dump out all the errors
		CIDKStr szPath = s_IDK_HOME;
		szPath += DIRECTORY_SLASH;
		szPath += "logs";

		DumpErrorMessages(szPath);

		// Not strictly an error, but this prevents the rest
		// of the app from running
		return false;
	}
	else
	{
		// Severity level not recognised
		PrintHelp();
		return false;
	}

// TCP Server code 

	GetLogObject()->SetSeverityLevel(eSeverity);

int retval;
	char l_Buffer[500];

	LogMessage(MSGSVRAPP_ERROR.OBS_INITIAISING);
//	retval = MessengerTask::taskInitialize();

	// Get parameter indicating which task to send the outbound data to.
	// For example we masy want to send the message to a message 
	// reformatter before passing to OUTSIDEWORLD

/*
	if ( (gSelfConfig->SCgetTaskParam(_taskName, "outboundTask", 
				_outboundTask)) == CM_ERROR)
	{
		strcpy(_outboundTask, "OUTBOUNDSERVER");
	}


	//
	//  Configuration
	//
	if ((gSelfConfig->SCgetTaskParam(_taskName, "numActiveConnections", 
				l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_numActiveConnections = DEFAULT_ACTIVE_CNX;
	}

	else
	{
		m_pConfig->_numActiveConnections = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.NUM_ACTIVE_CONNECTIONS, m_pConfig->_numActiveConnections);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "numConnections", 
				l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_numConnections = DEFAULT_CNX;
	}
	else
	{
		m_pConfig->_numConnections = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.NUM_CONNECTIONS, m_pConfig->_numConnections);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "numInputQueue", 
		l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_numInputQueue = DEFAULT_INPUT_QUEUE;
	}
	else
	{
		m_pConfig->_numInputQueue = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.NUM_INPUT_QUEUE, m_pConfig->_numInputQueue);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "outBoundPortNum", 
				l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_portNumOutBound = DEFAULT_OUT_BOUND_PORT_NUM;
	}
	else
	{
		m_pConfig->_portNumOutBound = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.PORT_NUM, m_pConfig->_portNumOutBound);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "numRetries", 
				l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_numRetries = DEFAULT_NUM_RETRIES;
	}
	else
	{
		m_pConfig->_numRetries = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.NUM_RETRIES, l_Buffer);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "connectionTimeout", 
				l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_connectionTimeOut = DEFAULT_CNX_TIMEOUT;
	}
	else
	{
		m_pConfig->_connectionTimeOut = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.CONNECTION_TIMEOUT, m_pConfig->_connectionTimeOut);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "idleTimeout", 
				l_Buffer)) == CM_ERROR)
	{
		m_pConfig->_idleTimeOut = DEFAULT_IDLE_TIMEOUT;
	}
	else
	{
		m_pConfig->_idleTimeOut = atoi(l_Buffer);
	}
	LogMessage(OUTSERVER_ERROR.IDLE_TIMEOUT, m_pConfig->_idleTimeOut);

	if ((gSelfConfig->SCgetTaskParam(_taskName, "Logging", l_Buffer)) == CM_ERROR){
	{
		m_pConfig->_Logging = true;
	}
		if(strcmp(l_Buffer, "Y")==0){
			m_pConfig->_Logging = true;
		}
		else
		{
			m_pConfig->_Logging = false;
		}
	}

	char l_Label[50];
	char l_Tmp[50];
	for (int i = 0; i < MAX_FILTER_SIZE; i++ )
	{
		sprintf(l_Label, "Filter%d", i+1 );
		if ((gSelfConfig->SCgetTaskParam(_taskName, l_Label, 
												l_Buffer)) != CM_ERROR)
		{
			TRACE(l_Buffer); TRACE("\n");
			getToken(l_Buffer, ",", "$1", l_Tmp);
			int l_MsgNo = atoi(l_Tmp);
			getToken(l_Buffer, ",", "$2", l_Tmp);
			CTCPServerFilter::EType l_Type;
			SetType(l_Tmp, l_Type);
			m_pConfig->_Filter[i].Set(l_MsgNo,l_Type);

			LogMessage(OUTSERVER_ERROR.FILTER_ON, i, l_MsgNo, l_Tmp);
		}
	}

	if ((gSelfConfig->SCgetTaskParam(_taskName, "defaultProtocol", 
				l_Buffer)) != CM_ERROR)
	{
		SetType(l_Buffer, m_pConfig->_DefaultType);
	}
	else
	{
		strcpy(l_Buffer, "UDP");
		m_pConfig->_DefaultType = CTCPServerFilter::EType::UDP;
	}
	LogMessage(OUTSERVER_ERROR.FILTER_DEFAULT_PROT0COL, l_Buffer);
#ifdef WIN32
	TRACE("numActiveConnections=%d\n", m_pConfig->_numActiveConnections);
	TRACE("numConnections=%d\n", m_pConfig->_numConnections);
	TRACE("numInputQueue=%d\n", m_pConfig->_numInputQueue);
	TRACE("portNum=%d\n", m_pConfig->_portNumOutBound);
	TRACE("numRetries=%d\n", m_pConfig->_numRetries);
	TRACE("connectionTimeout=%d\n", m_pConfig->_connectionTimeOut);
	TRACE("idleTimeout=%d\n", m_pConfig->_idleTimeOut);
	TRACE("Logging=%s\n", (m_pConfig->_Logging)?"Y":"N");
	for (int i = 0; i < MAX_FILTER_SIZE; i++)
	{
		if (m_pConfig->_Filter[i].IsInuse())
		{
			TRACE("Filter%d= M:%d T:%s\n", i, m_pConfig->_Filter[i].GetMsg(), (m_pConfig->_Filter[i].GetType() == CTCPServerFilter::EType::TCP)?"TCP":"UDP");
		}
		else
		{
			TRACE("Filter%d: Not in use\n");
		}
	}
	TRACE("Filter Default = %s", (m_pConfig->_Filter[i].GetType() == CTCPServerFilter::EType::TCP)?"TCP":"UDP" );
#else
	printf("numActiveConnections=%d\n", m_pConfig->_numActiveConnections);
	printf("numConnections=%d\n", m_pConfig->_numConnections);
	printf("numInputQueue=%d\n", m_pConfig->_numInputQueue);
	printf("portNum=%d\n", m_pConfig->_portNumOutBound);
	printf("numRetries=%d\n", m_pConfig->_numRetries);
	printf("connectionTimeout=%d\n", m_pConfig->_connectionTimeOut);
	printf("idleTimeout=%d\n", m_pConfig->_idleTimeOut);
	printf("Logging=%s\n", (m_pConfig->_Logging)?"Y":"N");
#endif
*/

//	m_pMessageFilter = new CTCPServerFilter(m_pConfig->_Filter, m_pConfig->_DefaultType);

	LogMessage(MSGSVRAPP_ERROR.OBS_RUNNING);

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,1),&wsaData) != 0){
		fprintf(stderr,"WSAStartup failed: %d\n",GetLastError());

	 }

	_timeOfLastMessage = 0;
	_loopInterval = 1;
	int l_iPort = 80;
	m_TCPServer = new CTCPServer(this, l_iPort);
	_loopTaskLooping = true;

// End
	//  Options
	


	
	m_ConfigFile = new CConfigFile(this);
	if (!m_ConfigFile->Read(l_ArgumentList->GetValue(ARGTOK_APPPATH),"Config"))
	{
		return false;
	}

	m_TruckFile = new CTruckFile;
	if (!m_TruckFile->Read(l_ArgumentList->GetValue(ARGTOK_APPPATH),"Truck"))
	{
		return false;
	}
	
	return true;
}




//////////////////////////////////////////////////////////////////////////////////
//
// The Main IDK Application
//

bool CMsgSvrApp::InitInstance()
{

	CIDKArgumentList *l_ArgumentList = GetArgumentList();

	if (!l_ArgumentList)
	{
		return false;
	}
	

	int ilogv = atoi(l_ArgumentList->GetValue(ARGTOK_LOGSIZE));
	if (ilogv < 0)	ilogv = 1000;
	GetLogObject()->SetLogLineLimit(ilogv);
	ilogv = atoi(l_ArgumentList->GetValue(ARGTOK_LOGPERIOD));
	if (ilogv < 0)	ilogv = 30;
	GetLogObject()->SetArchivePeriod(ilogv);

	
	return true;
}




CMsgSvrApp theApp;

/*****************************
	CMsgSvrApplication Logger

*****************************/

CMsgSvrApplicationLogMessage::CMsgSvrApplicationLogMessage(CIDKStr szCode) :
CIDKEErrorPackage(szCode),
	END_OF_DAY
		(szCode,  1, "END OF DAY",						CIDKEError::IDK_SEVERITY_GENERAL),
	END_OF_QUOTA
		(szCode,  2, "LOG FILE SIZE LIMIT REACHED",		CIDKEError::IDK_SEVERITY_GENERAL),
	SUMMARY
		(szCode,  3, "%s %s category messages logged",	CIDKEError::IDK_SEVERITY_GENERAL)
{}


CMsgSvrApplicationLogMessage CMsgSvrApplicationLog::CIISVRLOGGER_MESSAGE("004");


CMsgSvrApplicationLog::CMsgSvrApplicationLog(const char* szAppName) :
m_nSequenceNumber(1),
m_szLogPath("."),	// logging directory
m_nLineLimit(1000),	// maximum before starting a new log
m_nCutOffDays(14L)	// length of archiving period for logs
{
	m_AppName = szAppName;
	// set start logging day
	
	m_nCurrentDay = LoggingDay();

	// start message counts

	m_pMessageCounts = new long [CIDKEError::IDK_SEVERITY_GENERAL + 1];
	m_pMessages = new CIDKStr* [CIDKEError::IDK_SEVERITY_GENERAL + 1];

	for (int i = 0; i<=CIDKEError::IDK_SEVERITY_GENERAL; i++)
	{
		m_pMessageCounts[i] = 0L;
		m_pMessages[i] = 0;
	}
}

CMsgSvrApplicationLog::~CMsgSvrApplicationLog()
{
	Close();
	delete [] m_pMessageCounts;
	delete [] m_pMessages;
}

bool CMsgSvrApplicationLog::WriteMessage(CIDKEError& error, CIDKEObject& object, CIDKStringFields* pArguments)
{
	int today = LoggingDay();

	if (!CIDKEErrorLog::WriteMessage(error, object, pArguments))
		return false;

	m_nLineCount++;
	m_pMessageCounts[error.GetSeverity()]++;
	if (m_pMessages[error.GetSeverity()] == 0)
	{
		// save the text

		m_pMessages[error.GetSeverity()] = new CIDKStr(error.GetSeverityString());
	}

	if (m_nCurrentDay != today)
	{
		m_nCurrentDay = today;
		CIDKEErrorLog::WriteMessage(CIISVRLOGGER_MESSAGE.END_OF_DAY, *(CMsgSvrApp*)GetApp(), 0);
		NewLogFile();
	}
	else if (m_nLineCount == m_nLineLimit)
	{
		CIDKEErrorLog::WriteMessage(CIISVRLOGGER_MESSAGE.END_OF_QUOTA, *(CMsgSvrApp*)GetApp(), 0);
		NewLogFile();
	}

	return true;
}

int CMsgSvrApplicationLog::LoggingDay()
{
	// get current day index

	tzset();
	time_t currentTime;
	tm* currentLocalTime;

	time(&currentTime);
	currentLocalTime = localtime(&currentTime);
	return currentLocalTime->tm_mday;
}

const char* CMsgSvrApplicationLog::NextLogFileName()
{
	static char szLogFileName[256];
	char szDate[16];

	tzset();
	time_t now = time(0);

	strftime (szDate, 16, "%Y%m%d", localtime (&now));
	sprintf (szLogFileName, "IDK.%s.%s.%05d.%03d.log",(char *)m_AppName, szDate, getpid(), m_nSequenceNumber++);

	m_nSequenceNumber = m_nSequenceNumber % 100;

	return szLogFileName;
}

bool CMsgSvrApplicationLog::StartLogging(CIDKEError::SEVERITY eSeverity, bool bTrace, bool bEcho)
{
	// initialize the logging parameters and open a log file

	SetSeverityLevel(eSeverity);
	if (bTrace) RowTraceOn(); else RowTraceOff();
	SetEcho(bEcho);

	char szLogFileName[32];
	char szDate[16];
	tzset();
	time_t now = time(0);

	strftime (szDate, 16, "%Y%m%d", localtime (&now));

	// set up the wild card pattern for
	// removal of old logs

	sprintf (szLogFileName, "IDK.%s.*.log",(char*)m_AppName);
	m_szLogPattern = m_szLogPath;
	m_szLogPattern += DIRECTORY_SLASH;
	m_szLogPattern += szLogFileName;

	m_nLineCount = 0;
	for (int i = 0; i<=CIDKEError::IDK_SEVERITY_GENERAL; i++)	m_pMessageCounts[i] = 0L;

	CIDKStr str(m_szLogPath);
	str += DIRECTORY_SLASH;
	str += NextLogFileName();

	return Open("IDK", m_AppName, str);
}

bool CMsgSvrApplicationLog::OpenLog()
{
	m_nLineCount = 0;
	for (int i = 0; i<=CIDKEError::IDK_SEVERITY_GENERAL; i++)	m_pMessageCounts[i] = 0L;

	CIDKStr str(m_szLogPath);
	str += DIRECTORY_SLASH;
	str += NextLogFileName();

	return Open("IDK", GetStreamName(), str);
}

void CMsgSvrApplicationLog::Close()
{
	// dump message counts at end of old log
	/*
	CIDKStringFields args;

	for (int i = 0; i<=CIDKEError::IDK_SEVERITY_GENERAL; i++)
	{
		if (m_pMessageCounts[i] > 0L)
		{
			args.Empty();
			args.AddField(m_pMessageCounts[i]);
			args.AddField(*m_pMessages[i]);
			CIDKEErrorLog::WriteMessage(CIISVRLOGGER_MESSAGE.SUMMARY, *(CMsgSvrApp*)GetApp(),
												&args);
		}
	}
	*/	
	CIDKEErrorLog::Close();
}

void CMsgSvrApplicationLog::NewLogFile()
{
	Close();

	// remove files past their deletion date

	CIDKFileFind ffind(m_szLogPattern);
	long mintime;

	ffind.Open();

	if (ffind.GotFile())
	{
		// get the deletion date

		mintime = (time(0) / 86400L) * 86400L - m_nCutOffDays * 86400L;
	}

	while (ffind.GotFile())
	{
		CIDKStr target(ffind.GetFileName());

		if (!target.IsEmpty())	// valid file name
		{
			// get the file status block

			CIDKStr FullFileName = m_szLogPath;
			FullFileName += DIRECTORY_SLASH;
			FullFileName += target;

			CIDKFileStatus fs(FullFileName);

			// keep a record of the oldest log file

			if (fs.GetCreatedTime() < mintime)
				remove(fs.GetFullName());
		}
		ffind.GetNext();
	}

	ffind.Close();

	// now start a new log file

	OpenLog();
}


// Starts the timer
void CMsgSvrApp::StartTimer()
{
	time(&m_timer);
}

// Gets the current elapsed time in seconds
long CMsgSvrApp::GetElapsedTime()
{
	time_t current_time;
	time(&current_time);

	return ((long)current_time - (long)m_timer);
}

//////////////////////////////////////////////////////////////////////////////////
//
// The Main IDK Application
//

bool CMsgSvrApp::SetLogArguments()
{

	// read the Stream name from the command line arguments list
	CIDKArgumentList *l_ArgumentList = GetArgumentList();

	if (!l_ArgumentList)
	{
		return false;
	}

	int ilogv = atoi(l_ArgumentList->GetValue(ARGTOK_LOGSIZE));
	if (ilogv < 0)	ilogv = 1000;
	GetLogObject()->SetLogLineLimit(ilogv);
	ilogv = atoi(l_ArgumentList->GetValue(ARGTOK_LOGPERIOD));
	if (ilogv < 0)	ilogv = 30;
	GetLogObject()->SetArchivePeriod(ilogv);


	// Log the starting message
	LogMessage(MSGSVRAPP_ERROR.APPLICATION_START, GetApplicationName());

  // start the clock ticking...
	StartTimer();
	
	return true;
}


int CMsgSvrApp::ReadConfigFile()
{
	return true;	
}


int CMsgSvrApp::Run()
{

	while (1) {
		printf("Running\n");
		sleep(1);
	}

	// Log the time it took
	long elapsedTime = GetElapsedTime();

	long nHours = elapsedTime / (60 * 60);
	elapsedTime = elapsedTime - (nHours * 60 * 60);
	long nMinutes = elapsedTime / 60;
	long nSeconds = elapsedTime - (nMinutes * 60);

	LogMessage(MSGSVRAPP_ERROR.APPLICATION_TIME, nHours, nMinutes, nSeconds);
	LogMessage(MSGSVRAPP_ERROR.APPLICATION_END, "MSGSVR" );

	return true;
}

/*
//InitInstance
//////////////////////////////////////////////////////////////////////////////////
//
// The Main IDK Application
//

bool CMsgSvrApp::InitInstance()
{

	CIDKArgumentList *l_ArgumentList = GetArgumentList();

	if (!l_ArgumentList)
	{
		return false;
	}
	

	int ilogv = atoi(l_ArgumentList->GetValue(ARGTOK_LOGSIZE));
	if (ilogv < 0)	ilogv = 1000;
	GetLogObject()->SetLogLineLimit(ilogv);
	ilogv = atoi(l_ArgumentList->GetValue(ARGTOK_LOGPERIOD));
	if (ilogv < 0)	ilogv = 30;
	GetLogObject()->SetArchivePeriod(ilogv);

  WSADATA wsaData;
if (WSAStartup(MAKEWORD(2,1),&wsaData) != 0){
	fprintf(stderr,"WSAStartup failed: %d\n",GetLastError());

  }


	return true;
}
*/




