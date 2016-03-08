#pragma once


#include "CIDKStr.h"
#include "CIDKEError.h"
#include "CIDKEErrorLog.h"
#include "CIDKCommandLine.h"
#include "CIDKEObject.h"

#include <process.h>
#include <time.h>

class CMsgSvrAppOutOfMemoryHandler;

#define ARG_TRACE_ON		"t"	// Trace on/off
#define ARG_INIT_ONLY		"i"	// Initalise only
#define ARG_LOG_LEVEL		"l"	// Log warning level
#define ARG_ECHO_ON			"v"	// Echo error logs to screen turned off
#define ARG_IP_ADDR			"n"	// Echo error logs to screen turned off

class CMsgSvrApplicationError : public CIDKEErrorPackage
{
public:

	CMsgSvrApplicationError(CIDKStr szCode);

	CIDKEError APPLICATION_START;
	CIDKEError APPLICATION_END;
	CIDKEError APPLICATION_FAIL;
	CIDKEError APPLICATION_TIME;
	CIDKEError NO_IDK_HOME_ENV;
	CIDKEError DIAG_LOGSIZE;
	CIDKEError DIAG_LOGPERIOD;
	CIDKEError LOG_PATH;
	CIDKEError IPADDRESS;
	CIDKEError COMMAND;

	CIDKEError NUM_ACTIVE_CONNECTIONS;
	CIDKEError NUM_CONNECTIONS;
	CIDKEError NUM_INPUT_QUEUE;
	CIDKEError PORT_NUM;
	CIDKEError NUM_RETRIES;
	CIDKEError CONNECTION_TIMEOUT;
	CIDKEError IDLE_TIMEOUT;
	CIDKEError FILTER_ON;
	CIDKEError FILTER_DEFAULT_PROT0COL;
	CIDKEError OBS_INITIAISING;
	CIDKEError OBS_RUNNING;
};

class CMsgSvrApplicationLogMessage : public CIDKEErrorPackage
{
public:

	CMsgSvrApplicationLogMessage(CIDKStr szCode);

	CIDKEError END_OF_DAY;
	CIDKEError END_OF_QUOTA;
	CIDKEError SUMMARY;
};

class CMsgSvrApplicationLog : public CIDKEErrorLog
{
	CIDKStr m_AppName;
public:

	CMsgSvrApplicationLog(const char* szAppName);

	~CMsgSvrApplicationLog();

	void SetLogStream(CIDKStr str)		{SetStreamName(str); }
	void SetLogPath(CIDKStr dir)			{m_szLogPath = dir; }
	void SetLogLineLimit (int n)
	{
		if (n > 100)	m_nLineLimit = n;
	}

	void SetArchivePeriod (int days)
	{
		if (days > 0)	m_nCutOffDays = (long) days;
	}

	bool StartLogging(CIDKEError::SEVERITY eSeverity, bool bTrace, bool bEcho);

	virtual bool WriteMessage(CIDKEError& error, CIDKEObject& object, CIDKStringFields* pArguments);
	virtual void Close();

protected:

	
	static CMsgSvrApplicationLogMessage CIISVRLOGGER_MESSAGE;

	int LoggingDay();

	const char* NextLogFileName();
	void NewLogFile();
	bool OpenLog();

	CIDKStr m_szLogPath;	// logging directory

	int	m_nCurrentDay;	// day of current log file
	int	m_nLineCount;	// line number
	int	m_nLineLimit;	// maximum before starting a new log
	long m_nCutOffDays;	// length of archiving period for logs

	long* m_pMessageCounts;	// totals for each message type
	CIDKStr** m_pMessages;	// message severity strings

	int m_nSequenceNumber;	// for log files on the same day

	CIDKStr m_szLogPattern;	// wild card match for log files
};

class CMsgSvrApplicationEObject : public CIDKEObject
{
protected:

	CMsgSvrApplicationEObject(const char* szAppName, const CIDKEStream* pStream = 0) :
	CIDKEObject(pStream)
	{
		if (m_perrorLog)	// replace it!
		{
			delete m_perrorLog;
		}
		m_perrorLog = new CMsgSvrApplicationLog(szAppName);
	}

	virtual ~CMsgSvrApplicationEObject()
	{
		delete m_perrorLog;
	}

	bool StartLog(CIDKEError::SEVERITY eSeverity, bool bTrace, bool bEcho)
	{
		return ((CMsgSvrApplicationLog*) m_perrorLog)->
					StartLogging(eSeverity, bTrace, bEcho);
	}

	CMsgSvrApplicationLog* GetLogObject()
	{
		return (CMsgSvrApplicationLog*) m_perrorLog;
	}
};
//
//	-v -t -l DETAIL
//
 
class CIISVRCmdLine : public CIDKCommandLine
{
public:
	CIISVRCmdLine()
		: CIDKCommandLine()
	{
	};

	bool Process(int argc, char *argv[]);
};

class CConfigFile;
class CTruckFile;
class CTCPServer;
class CTCPServerFilter;

class CMsgSvrApp : public CIDKApplication, public CMsgSvrApplicationEObject
{
public:
	typedef enum {
		V_Not_Found,
		V_Found,
		V_Lost,
		V_Synced
	} EState;

private:

	EState m_State;

	friend CConfigFile;

	// The timer used to calculate how long the process took
	time_t m_timer;
	// The log file severity token
	CIDKStr m_szSeverity;

	// The log file trace flag
	bool m_bIsTraceOn;

	// The log file echo to screen flag
	bool m_bIsEchoOn;

	CTCPServer *m_TCPServer;

	// IP Address that the command is to go
	CIDKStr m_IPAddress;

	// Command to be send
	CIDKStr m_Command;

	int		m_TimeOut;
	CIDKStr	m_DefaultBinPath;
	CIDKStr	m_DefaultConfigPath;
	CIDKStr	m_DefaultLogsPath;
	int		m_DelayTime;
	int		m_LoopLimit;
	int		m_LogTimeInterval;
	CIDKStr	m_DebufLogFilePath;
	CIDKStr	m_DefaultHistoryPath;


	time_t	_timeOfLastMessage;
	//STATUS taskLoop();

	int _loopTaskLooping;
	int _loopInterval;


	// DSDB home directory
	static CIDKStr s_IDK_HOME;

	static CIDKStr s_LogFilePath;

	bool SetLogArguments();

protected:
	static CMsgSvrApplicationError MSGSVRAPP_ERROR;
	virtual bool Initalise(int argc, char *argv[]);

	void StartTimer();
	long GetElapsedTime();
	
	int ReadConfigFile();

	CIDKStr GetSeverity()
	{
		return m_szSeverity;
	}

	CTCPServerFilter *m_pMessageFilter;

	CConfigFile *m_ConfigFile;
	CTruckFile *m_TruckFile;
public:
	CMsgSvrApp();
	~CMsgSvrApp();

	bool InitInstance();
	int Run();

};

