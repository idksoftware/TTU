#pragma once

#include "CIDKStr.h"
#include "CIDKEError.h"
#include "CIDKEErrorLog.h"
#include "CIDKEObject.h"
#include "CIDKLinkList.h"
#include "CIDKQueue.h"
#include "CIDKDate.h"

#define MAX_FILTER_SIZE 10


class CTCPServerFilter
{
public:
	typedef enum {
		Unknown,
		TCP,
		UDP
	} EType;
private:
	bool _inuse;
	int _msg;
	EType _type;

public:

	CTCPServerFilter()
	{
		_inuse = false;
	}

	void Set(int msg, EType type)
	{
		_msg = msg;
		_type = type;
		_inuse = true;
	}

	void Clear()
	{
		_inuse = false;
	}

	bool IsInuse()
	{
		return _inuse;
	}

	int GetMsg()
	{
		return _msg;
	}
	
	EType GetType()
	{
		return _type;
	}

};



class CTCPServerConfig
{
public:
	int _numActiveConnections;
	int _numConnections;
	int _numInputQueue;
	int _portNumOutBound;
	int _numRetries;
	int _connectionTimeOut;
	int _idleTimeOut;
	bool _Logging;
	CTCPServerFilter _Filter[MAX_FILTER_SIZE];
	CTCPServerFilter::EType _DefaultType;
};

class CTCPMsgStatus
{
public:
	typedef enum {
		Waiting,	// Waiting for a thread
		Pending,	// Got a free thread sending messages 
		Complete,	// Sent messages so complete However waiting 
					// for new messages (this will not be used,
					// but this state may exist waiting for the idle
					// task to remove it from the thread
		Empty,		// Free to be used
		Error		// An Error has acurred therefore abort the message conversation
	} EState;
private:

	EState m_eState;
	time_t m_TimeStamp;
	unsigned long m_IPAddres;
	int m_MsgType;
//	clock_t m_ConnectTime;
	clock_t m_TransferTime;
	int m_NumOfRetrys;
	int m_LastError;
	char *m_LogMessage;

public:
	CTCPMsgStatus()
	{
		m_LogMessage = 0;
	};

	EState GetState()
	{
		return m_eState;
	}

	time_t GetTimeStamp()
	{
		return m_TimeStamp;
	}

	unsigned long GetIPAddres()
	{
		return m_IPAddres;
	}

	int GetMsgType()
	{
		return m_MsgType;
	}

/*
	long GetConnectTime()
	{
		return m_ConnectTime;
	}
*/
	long GetTransferTime()
	{
		return m_TransferTime;
	}

	int GetNumOfRetrys()
	{
		return m_NumOfRetrys;
	}

	int SetNumOfRetrys(int i)
	{
		return m_NumOfRetrys = i;
	}

	int GetLastError()
	{
		return m_LastError;
	}

	void Clear()
	{
		m_eState = EState::Empty;
		m_TimeStamp = 0;
		m_IPAddres = 0;
		m_MsgType = -1;
//		m_ConnectTime = 0;
		m_TransferTime = 0;
		m_NumOfRetrys = -1;
		m_LastError = 0;
	}

	void Start(unsigned long szIPAddr)
	{
		m_eState = EState::Waiting;
		m_TimeStamp = time(0);
		m_IPAddres = szIPAddr;
		m_MsgType = -1;
//		m_ConnectTime = 0;
		m_TransferTime = 0;
		m_NumOfRetrys = -1;
		m_LastError = 0;
	}

	void StartTransmitTime()
	{
		m_TransferTime = clock();
	}

	void EndTransmitTime()
	{
		clock_t l_Start = m_TransferTime;
		clock_t l_End = clock();
		m_TransferTime = l_End - l_Start;
	}

	const char *LogMessage();
	const char *StateString();
};

class CMessagePackage
{
	unsigned char *m_Buffer;
	int m_Size;
	unsigned long m_IPAddr;	
	CTCPMsgStatus m_TCPMsgStatus;
	CTCPServerFilter::EType m_CommsType;
public:
	CMessagePackage(unsigned char *Buffer, int iSize, unsigned long szIPAddr)
	{
		
		m_Buffer = new unsigned char[iSize];
		m_Size = iSize;
		m_IPAddr = szIPAddr;
	}

	CMessagePackage()
	{	
		m_Buffer = 0;
		m_Size = 0;
		m_IPAddr = 0;
	}

	~CMessagePackage()
	{
		Empty();	
	}

	void Set(unsigned char *Buffer, int iSize, unsigned long szIPAddr, CTCPServerFilter::EType m_CommsType)
	{
		m_TCPMsgStatus.Start(szIPAddr);
		m_Buffer = new unsigned char[iSize];
		memcpy(m_Buffer,Buffer,iSize);
		m_Size = iSize;
		m_IPAddr = szIPAddr;
	}

	unsigned char *GetBuffer()
	{
		return m_Buffer;
	}
	
	void Empty()
	{
		if (m_Buffer)
		{
			delete [] m_Buffer;
		}
		m_Size = 0;
		m_IPAddr = 0;	
		//m_TCPMsgStatus.Clear();
	}

	int GetSize()
	{
		return m_Size;
	}

	unsigned long  GetIPAddr()
	{
		return m_IPAddr;
	}

	CTCPMsgStatus GetMsgStatus()
	{
		return m_TCPMsgStatus;
	}
};

class CWriteThread;
class CWriteChild;
class CMessageContainer;
class CWriteChildContainer;
class CThreadContainer;
class CIDKApplication;

class CMessageWriter
{
public:
	typedef enum {
		TCP_TYPE,
		UDP_TYPE,
		UNKNOWN_TYPE
	};

	CMessageWriter() {};
	virtual ~CMessageWriter() {};

	virtual int Connect(char *serviceName = 0, int portNumber = 0, char *hostName = 0) = 0;
	virtual int Write(char *write_buf, int writeLen) = 0;
	virtual int Read(char *read_buf) = 0;
	virtual int Close() = 0;
	virtual int IsA() = 0;
};

class CIDKTCPMessenger;
class CTCPMessageWriter : public CMessageWriter 
{
	CIDKTCPMessenger *m_pMessengerTCP;

public:

	CTCPMessageWriter();
	~CTCPMessageWriter();

	int Connect(char *serviceName, int portNumber, char *hostName);
	int Write(char *write_buf, int writeLen);
	int Read(char *read_buf);
	int IsA()
	{
		return CMessageWriter::TCP_TYPE;
	}
	int Close();
};
class CIDKUDPMessager;
class CUDPMessageWriter : public CMessageWriter 
{
	CIDKUDPMessager *m_pMessengerUDP;

public:

	CUDPMessageWriter();
	~CUDPMessageWriter();

	int Connect(char *serviceName, int portNumber, char *hostName);
	int Write(char *write_buf, int writeLen);
	int Read(char *read_buf);
	int IsA()
	{
		return CMessageWriter::TCP_TYPE;
	}
	int Close();
};


class CTCPServerError : public CIDKEErrorPackage
{
public:

	CTCPServerError(CIDKStr szCode);

	CIDKEError SENDING_MESSAGE;
	CIDKEError FAILED2QUEUE;	
	CIDKEError QUEUEDMESSAGE;
};

class CTCPServer : public CIDKEObject
{
	CThreadContainer *m_pThreadContainer;
	CMessageContainer *m_pMessageContainer;
	CWriteChildContainer *m_pWriteChildContainer;

	CIDKLinkList<CWriteThread *> m_WriteThreadActiveList;
	int m_iNumThreads;
	int m_iMaxQueue;
	int m_iMaxMessages;

	int m_WaitingTimeOut;
	int m_ConnectTimeOut;
	bool m_Logging;
	CIDKApplication *m_pTask;
	static CTCPServerError TCPSERVER_ERROR;
public:
	CTCPServer( CIDKApplication *pTask, int iPort, int iNumThreads = 10, int m_iMaxQueue = 100,int iMaxMessages = 200);
	CTCPServer( CIDKApplication *pTask, CTCPServerConfig aConfig);
	~CTCPServer();

	int Send(unsigned char *Buffer, int iSize, const char *szIPAddr, CTCPServerFilter::EType iType);
	void OnIdle();
};
