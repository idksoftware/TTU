#include "stdafx.h"
#include "TCPServer.h"
#include "WriteChild.h"
#include "CIDKThreads.h"
#include "CIDKMutex.h"
#include "IDKTCPMessenger.h"
#include "IDKUDPMessager.h"

class CWriteChildContainer;
class CThreadContainer
{
	CIDKQueue<CWriteThread *> m_FreeList;
	CIDKLinkList<CWriteThread *> m_ActiveList;
	int m_iNumThreads;
	CWriteChildContainer *m_pWriteChildContainer;
	int m_WaitingTimeOut;
	int m_ConnectTimeOut;
public:
	CThreadContainer(int iMax, CWriteChildContainer *pWriteChildContainer);
	bool IsFree()
	{
		return m_FreeList.GetSize();
	}

	void SetWaitingTimeOut(int iTimeOut)
	{
		m_WaitingTimeOut = iTimeOut;
	}

	void SetConnectTimeOut(int iTimeOut)
	{
		m_ConnectTimeOut = iTimeOut;
	}

	bool Process();
	void OnIdle();
};


class CMessageContainer;
class CWriteChildContainerError : public CIDKEErrorPackage
{
public:

	CWriteChildContainerError(CIDKStr szCode);

	CIDKEError INITALISEDOK;
	CIDKEError FREEING;
	CIDKEError QUEUEING_OK;
	CIDKEError QUEUEING_FAILED;
	CIDKEError QUEUEING_OUTOFSLOTS;
	CIDKEError QUEUEING_EXISTINGIP;
	CIDKEError ALLOC_EMPTY;
	CIDKEError ALLOC_FAILED;
	CIDKEError ALLOC_OK;
};

class CWriteChildContainer : public CIDKEObject
{
	CIDKQueue<CWriteChild *> m_FreeList;
	CIDKQueue<CWriteChild *> m_Queue;
	int m_iMaxQueue;
	CMessageContainer *m_pMessageContainer;

protected:
	static CWriteChildContainerError WRITECHILDCONTAINER_ERROR;

public:
	CWriteChildContainer(int iMax, int iPort);
	void Init(CMessageContainer *pMessageContainer)
	{
		m_pMessageContainer = pMessageContainer;
	}
	CWriteChild *Find(unsigned long ipAddr);
	void Free(CWriteChild *pPackage);
	bool Queue();
	CWriteChild *Alloc();
};

class CMessageContainerError : public CIDKEErrorPackage
{
public:

	CMessageContainerError(CIDKStr szCode);

	CIDKEError INITALISEDOK;
	CIDKEError FREEING;
	CIDKEError QUEUEING_OK;
	CIDKEError QUEUEING_FAILED;
	CIDKEError QUEUEING_OUTOFSLOTS;
	CIDKEError QUEUEING_EXISTINGIP;
	CIDKEError ALLOC_EMPTY;
	CIDKEError ALLOC_FAILED;
	CIDKEError ALLOC_OK;
};

class CMessageContainer : public CIDKEObject 
{
	CIDKQueue<CMessagePackage *> m_PackageQueue;
	CIDKQueue<CMessagePackage *> m_PackageFreeList;
	CIDKQueue<CMessagePackage *> m_InUseList;
	CWriteChildContainer *m_pWriteChildContainer;
	int m_iMaxMessages;
protected:
	static CMessageContainerError MESSAGECONTAINER_ERROR;
public:

	CMessageContainer(int iMax, CWriteChildContainer *pWriteChildContainer);
	void Free(CMessagePackage *pPackage);
	bool Queue(unsigned char *Buffer, int iSize, unsigned long lIPAddr, CTCPServerFilter::EType iType);
	CMessagePackage *Alloc();
	bool IsEmpty();
};

CMessageContainerError CMessageContainer::MESSAGECONTAINER_ERROR("005");

CMessageContainerError::CMessageContainerError(CIDKStr szCode) : CIDKEErrorPackage(szCode),

	INITALISEDOK
		(szCode,  1, "Initalised message container with %s free message slots",		CIDKEError::IDK_SEVERITY_MESSAGE),
	FREEING
		(szCode,  2, "Freeing message slot",										CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_OK
		(szCode,  3, "Queued message sucessfully",									CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_FAILED
		(szCode,  4, "Failed to queue message",										CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_OUTOFSLOTS
		(szCode,  5, "Failed to queue message, out of message slots",				CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_EXISTINGIP
		(szCode,  6, "Queued message sucessfully using existing IP address list",	CIDKEError::IDK_SEVERITY_MESSAGE),
	ALLOC_EMPTY
		(szCode,  7, "Alloc out of message slots",									CIDKEError::IDK_SEVERITY_MESSAGE),
	ALLOC_FAILED
		(szCode,  8, "Alloc message failed",										CIDKEError::IDK_SEVERITY_MESSAGE),
	ALLOC_OK
		(szCode,  9, "Alloc message sucessfull",									CIDKEError::IDK_SEVERITY_MESSAGE)
{}

CMessageContainer::CMessageContainer(int iMax, CWriteChildContainer *pWriteChildContainer)
{
	m_iMaxMessages = iMax;
	for (int i = 0; i < iMax; i++ )
	{
		CMessagePackage *l_Tmp = new CMessagePackage;
		m_PackageFreeList.AddItem(l_Tmp);
	}
	m_pWriteChildContainer = pWriteChildContainer;
	LogMessage(MESSAGECONTAINER_ERROR.INITALISEDOK,iMax);
}

void CMessageContainer::Free(CMessagePackage *pPackage)
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	m_PackageFreeList.AddItem(pPackage);
	LogMessage(MESSAGECONTAINER_ERROR.FREEING);
	l_cs.Leave();
	
}
bool CMessageContainer::Queue(unsigned char *Buffer, int iSize, unsigned long lIPAddr, CTCPServerFilter::EType iType)
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	if (m_PackageFreeList.GetSize() == 0)
	{
		LogMessage(MESSAGECONTAINER_ERROR.QUEUEING_OUTOFSLOTS);
		return false;
	}
	
	CMessagePackage *l_pPackage = 0;
	if (!m_PackageFreeList.RemoveItem(l_pPackage))
	{
		// empty
		LogMessage(MESSAGECONTAINER_ERROR.QUEUEING_FAILED);
		return false;
	}
	l_pPackage->Set(Buffer, iSize, lIPAddr, iType);

	CWriteChild *l_pWriteChild = 0;

	if ((l_pWriteChild = m_pWriteChildContainer->Find(lIPAddr)) != 0)
	{
		l_pWriteChild->Queue(l_pPackage);
		LogMessage(MESSAGECONTAINER_ERROR.QUEUEING_EXISTINGIP);
	}
	else
	{
		m_PackageQueue.AddItem(l_pPackage);
		LogMessage(MESSAGECONTAINER_ERROR.QUEUEING_OK);
	}
	l_cs.Leave();
	return true;
}

CMessagePackage *CMessageContainer::Alloc()
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	if (m_PackageQueue.GetSize() == 0)
	{
		LogMessage(MESSAGECONTAINER_ERROR.ALLOC_EMPTY);
		return 0;
	}
	CMessagePackage* pPackage;
	if (!m_PackageQueue.RemoveItem(pPackage))
	{
		LogMessage(MESSAGECONTAINER_ERROR.ALLOC_FAILED);
		return 0;
	}
	m_InUseList.AddItem(pPackage);
	l_cs.Leave();
	LogMessage(MESSAGECONTAINER_ERROR.ALLOC_OK);
	return pPackage;
}

bool CMessageContainer::IsEmpty()
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	return m_PackageQueue.IsEmpty();
	l_cs.Leave();
}

/*
	CIDKEError QUEUEING_OK;
	CIDKEError QUEUEING_FAILED;
	CIDKEError QUEUEING_OUTOFSLOTS;
	CIDKEError QUEUEING_EXISTINGIP;
	CIDKEError ALLOC_EMPTY;
	CIDKEError ALLOC_FAILED;
	CIDKEError ALLOC_OK;
*/

CWriteChildContainerError CWriteChildContainer::WRITECHILDCONTAINER_ERROR("006");

CWriteChildContainerError::CWriteChildContainerError(CIDKStr szCode) : CIDKEErrorPackage(szCode),

	INITALISEDOK
		(szCode,  1, "Initalised message container with %s free message slots",		CIDKEError::IDK_SEVERITY_MESSAGE),
	FREEING
		(szCode,  2, "Freeing message slot",										CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_OK
		(szCode,  3, "Queued message sucessfully",									CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_FAILED
		(szCode,  4, "Failed to queue message",										CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_OUTOFSLOTS
		(szCode,  5, "Failed to queue message, out of message slots",				CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEING_EXISTINGIP
		(szCode,  6, "Queued message sucessfully using existing IP address list",	CIDKEError::IDK_SEVERITY_MESSAGE),
	ALLOC_EMPTY
		(szCode,  7, "Alloc out of message slots",									CIDKEError::IDK_SEVERITY_MESSAGE),
	ALLOC_FAILED
		(szCode,  8, "Alloc message failed",										CIDKEError::IDK_SEVERITY_MESSAGE),
	ALLOC_OK
		(szCode,  9, "Alloc message sucessfull",									CIDKEError::IDK_SEVERITY_MESSAGE)
{}

CWriteChildContainer::CWriteChildContainer(int iMax, int iPort)
{
	m_iMaxQueue = iMax;
	for (int i = 0; i < m_iMaxQueue; i++ )
	{
	//TCPMessageWriter *l_pMW = new CTCPMessageWriter;
		CWriteChild *l_Tmp = new CWriteChild(new CTCPMessageWriter, iPort);
		m_FreeList.AddItem(l_Tmp);
	}
	LogMessage(WRITECHILDCONTAINER_ERROR.INITALISEDOK);
}

CWriteChild *CWriteChildContainer::Find(unsigned long ipAddr)
{
	CWriteChild *l_WriteChild = 0;
	if (!m_Queue.First())
	{
		return 0;
	}
	do {
		if (!m_Queue.Get(l_WriteChild))
		{
			return 0;
		}
		if (l_WriteChild->GetIPAddr() == ipAddr)
		{
			return l_WriteChild;
		}
	} while (m_Queue.Next());
	return 0;
}

void CWriteChildContainer::Free(CWriteChild *pPackage)
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	m_FreeList.AddItem(pPackage);
	LogMessage(WRITECHILDCONTAINER_ERROR.FREEING);
	l_cs.Leave();
	
}

bool CWriteChildContainer::Queue()
{
/*
CIDKEError QUEUEING_OK;
	CIDKEError QUEUEING_FAILED;
	CIDKEError QUEUEING_OUTOFSLOTS;
	CIDKEError QUEUEING_EXISTINGIP;
*/
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	while (1)
	{
		
		// Is there Free CWriteChild avalable
		if (m_FreeList.GetSize() == 0)
		{
			LogMessage(WRITECHILDCONTAINER_ERROR.QUEUEING_OK);
			return true;
		}
		CMessagePackage *pPackage = 0;
		// Is there messages to process
		if (!(pPackage = m_pMessageContainer->Alloc()))
		{
			LogMessage(WRITECHILDCONTAINER_ERROR.QUEUEING_OK);
			return true;
		}
		// Get a free CWriteChild Object of the free list
		CWriteChild *l_pWriteChild = 0;
		if (!m_FreeList.RemoveItem(l_pWriteChild))
		{
			// empty
			LogMessage(WRITECHILDCONTAINER_ERROR.QUEUEING_OUTOFSLOTS);
			return false;
		}
		// Queue the CWriteChild Object on the queue list
		l_pWriteChild->SetIPAddr(pPackage->GetIPAddr());
		l_pWriteChild->Queue(pPackage);
		m_Queue.AddItem(l_pWriteChild);
	}

	l_cs.Leave();
	return true;
}

CWriteChild *CWriteChildContainer::Alloc()
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	if (m_Queue.GetSize() == 0)
	{
		return 0;
	}
	CWriteChild* pWriteChild;
	if (!m_Queue.RemoveItem(pWriteChild))
	{
		return 0;
	}
	l_cs.Leave();
	return pWriteChild;
}

class CWriteThreadFunction : public CIDKThreadFunctor
{
	CWriteChild *m_pWriteChild;
	int Send(unsigned char *Buffer, int iSize);
	CIDKMutex *m_pMutex;
//	CIDKTDKMessenger(char *taskName);
public:
	CWriteThreadFunction()
		: CIDKThreadFunctor()
	{
		m_pWriteChild = 0;
		m_pMutex = 0;
	}

	bool Start()
	{
		m_pMutex = new CIDKMutex;
		m_pMutex->Init(); // Initalise and lock
		
		while(1)
		{
			CIDKWaitForMutex l_WaitForMutex(m_pMutex);
			Sleep(1000);
			if (l_WaitForMutex.Wait())
			{
				if (m_pWriteChild)
				{	

					if (m_pWriteChild->GetState() != CWriteChild::Complete && m_pWriteChild->GetState() != CWriteChild::Error)
					{
						m_pWriteChild->Send();					
					}
					int i = 0;
					i++;
				}
			}
			else
			{
			}
			delete m_pMutex;
			m_pMutex = new CIDKMutex;
			m_pMutex->Init();
		}
		delete m_pMutex;
		return true;
	}
	int Activate(CWriteChild *pWriteChild);

	CWriteChild *GetWriteChild()
	{
		return m_pWriteChild;
	}

	void RemoveChild()
	{
		m_pWriteChild = 0;
	}
};

int CWriteThreadFunction::Activate(CWriteChild *pWriteChild)
{
	m_pWriteChild = pWriteChild;
	m_pMutex->Unlock();
	return true;
}
class CWriteThread : public CIDKThread
{
	CWriteThreadFunction *m_pThreadFunction;
public:
	CWriteThread() :
			CIDKThread(m_pThreadFunction = new CWriteThreadFunction)
	{
	}
	bool Activate(CWriteChild *pWriteChild);

	CWriteChild *GetChild()
	{
		return m_pThreadFunction->GetWriteChild();
	}

	CWriteChild *RemoveChild();
};

bool CWriteThread::Activate(CWriteChild *pWriteChild)
{
	CWriteThreadFunction  *l_pThreadFunction = (CWriteThreadFunction *)GetThreadFunctor();
	l_pThreadFunction->Activate(pWriteChild);
	return true;
}

CWriteChild *CWriteThread::RemoveChild()
{
	CWriteChild *l_pWriteChild;

	CWriteThreadFunction  *l_pThreadFunction = (CWriteThreadFunction *)GetThreadFunctor();
	l_pWriteChild = l_pThreadFunction->GetWriteChild();
	l_pThreadFunction->RemoveChild();
	return l_pWriteChild;
}

CThreadContainer::CThreadContainer(int iMax, CWriteChildContainer *pWriteChildContainer)
{
	m_pWriteChildContainer = pWriteChildContainer;
	m_iNumThreads = iMax;
	for (int i = 0; i < m_iNumThreads; i++ )
	{
		CWriteThread *l_Tmp = new CWriteThread;
		if (!l_Tmp->Start())
		{
			return;
		}
		m_FreeList.AddItem(l_Tmp);
	}
}

bool CThreadContainer::Process()
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	while (1)
	{
		if (m_FreeList.GetSize() == 0)
		{
			return true;
		}
		CWriteChild *l_WriteChild = 0;
		if (!(l_WriteChild = m_pWriteChildContainer->Alloc()))
		{
			return true;
		}
		CWriteThread *l_WriteThread = 0;
		if (!m_FreeList.RemoveItem(l_WriteThread))
		{
			return true;
		}
		m_ActiveList.Insert(l_WriteThread);

//		l_WriteChild->SetTime();

		if (!l_WriteThread->Activate(l_WriteChild))
		{
			return false;
		}
	}
	l_cs.Enter();
	return true;
}
void CThreadContainer::OnIdle()
{
	time_t l_TimeNow = time(0);

	if(m_ActiveList.Count())
	{
		m_ActiveList.Rewind();
		do {
 			CWriteThread  *l_pWriteThread = m_ActiveList.Get();
			if (!l_pWriteThread)
			{
				return;
			}
			CWriteChild *l_pWriteChild = l_pWriteThread->GetChild();
			if (l_pWriteChild)
			{
				CWriteChild::EState l_ChildState = l_pWriteChild->GetState();
				switch(l_ChildState)
				{			
				case CWriteChild::Waiting:
					break;
				case CWriteChild::Pending:
					if (l_TimeNow + m_WaitingTimeOut < l_pWriteChild->GetTime())
					{
						// These 
						CWriteChild *l_WriteChild = l_pWriteThread->RemoveChild();
						l_pWriteChild->SetEmpty();
					}
					break;
				case CWriteChild::Complete:
					if (l_TimeNow + m_ConnectTimeOut < l_pWriteChild->GetTime())
					{
						CWriteChild *l_WriteChild = l_pWriteThread->RemoveChild();
						l_WriteChild->SetEmpty();
					}
					break;
				}
			}
		} while (m_ActiveList.Next());
	}
}

CTCPServerError CTCPServer::TCPSERVER_ERROR("004");

CTCPServerError::CTCPServerError(CIDKStr szCode) : CIDKEErrorPackage(szCode),

	SENDING_MESSAGE
		(szCode,  1, "Sending message of Size %s to IPAddr %s using %s",			CIDKEError::IDK_SEVERITY_MESSAGE),
	FAILED2QUEUE
		(szCode,  2, "Failed to Queue message of Size %s to IPAddr %s using %s",	CIDKEError::IDK_SEVERITY_MESSAGE),
	QUEUEDMESSAGE
		(szCode,  3, "Queued message of Size %s to IPAddr %s using %s",				CIDKEError::IDK_SEVERITY_MESSAGE)
{}



CTCPServer::CTCPServer(CIDKApplication *pTask, int iPort, int iNumThreads, int iMaxQueue, int iMaxMessages)
{

	m_pTask = pTask;
	m_pWriteChildContainer = new CWriteChildContainer(iMaxQueue, iPort);
	m_pMessageContainer = new CMessageContainer(iMaxMessages, m_pWriteChildContainer);
	m_pWriteChildContainer->Init(m_pMessageContainer);
	m_pThreadContainer = new CThreadContainer(iNumThreads, m_pWriteChildContainer);
	m_WaitingTimeOut = 30;
	m_ConnectTimeOut = 15;
}

CTCPServer::CTCPServer(CIDKApplication *pTask, CTCPServerConfig aConfig)
{
/*
int _numActiveConnections;
	int _numConnections;
	int _numInputQueue;
	int _portNumOutBound;
	int _numRetries;
	int _connectionTimeOut;
	int _idleTimeOut;
	bool _Logging;
*/

	m_pTask = pTask;
	m_pWriteChildContainer = new CWriteChildContainer(aConfig._numConnections, aConfig._portNumOutBound);
	m_pMessageContainer = new CMessageContainer(aConfig._numInputQueue, m_pWriteChildContainer);
	m_pWriteChildContainer->Init(m_pMessageContainer);
	m_pThreadContainer = new CThreadContainer(aConfig._numActiveConnections, m_pWriteChildContainer);
	m_WaitingTimeOut = aConfig._idleTimeOut;
	m_ConnectTimeOut = aConfig._connectionTimeOut;
	m_Logging = aConfig._Logging;
}

CTCPServer::~CTCPServer()
{
}

int CTCPServer::Send(unsigned char *Buffer, int iSize, const char *szIPAddr, CTCPServerFilter::EType iType)
{
	unsigned long l_IPAddr = inet_addr(szIPAddr);

	in_addr l_sAddr;
	l_sAddr.s_addr = l_IPAddr;

	char *l_Test = inet_ntoa(l_sAddr);

	CIDKCriticalSection l_cs;
	l_cs.Enter();

	
	LogMessage(TCPSERVER_ERROR.SENDING_MESSAGE, iSize, szIPAddr, (iType == CTCPServerFilter::EType::TCP)?"TCP":"UDP");

	if (!m_pMessageContainer->Queue(Buffer, iSize, l_IPAddr, iType))
	{
		LogMessage(TCPSERVER_ERROR.FAILED2QUEUE, szIPAddr, (iType == CTCPServerFilter::EType::TCP)?"TCP":"UDP");
		return false;
	}
	LogMessage(TCPSERVER_ERROR.QUEUEDMESSAGE, szIPAddr, (iType == CTCPServerFilter::EType::TCP)?"TCP":"UDP");

	if (m_pMessageContainer->IsEmpty())
	{
		// the message is belongs to an exesting conection 
		LogMessage(TCPSERVER_ERROR.QUEUEDMESSAGE, szIPAddr, (iType == CTCPServerFilter::EType::TCP)?"TCP":"UDP");
		return true; 
	}

	if (!m_pWriteChildContainer->Queue())
	{
		return true;
	}

	if (!m_pThreadContainer->Process())
	{
		return true;
	}
	
	l_cs.Leave();
	return 0;
}

void CTCPServer::OnIdle()
{
	time_t l_TimeNow = time(0);
	
	m_pThreadContainer->SetWaitingTimeOut(m_WaitingTimeOut);
	m_pThreadContainer->SetConnectTimeOut(m_ConnectTimeOut);
	m_pThreadContainer->OnIdle();	
}


CTCPMessageWriter::CTCPMessageWriter()
{
	m_pMessengerTCP = new CIDKTCPMessenger("TCPServer",0);
}

CTCPMessageWriter::~CTCPMessageWriter()
{
	delete m_pMessengerTCP;
}

int CTCPMessageWriter::Connect(char *serviceName, int portNumber, char *hostName)
{
	m_pMessengerTCP->setAddress(serviceName, portNumber, hostName);
	return true;
}

int CTCPMessageWriter::Write(char *write_buf, int writeLen)
{
	return m_pMessengerTCP->taskWrite(write_buf, writeLen);
}

int CTCPMessageWriter::Read(char *read_buf)
{
	return m_pMessengerTCP->taskRead(read_buf);
}
int CTCPMessageWriter::Close()
{
	return m_pMessengerTCP->taskClose();
}

CUDPMessageWriter::CUDPMessageWriter()
{
	m_pMessengerUDP = new CIDKUDPMessager("UDPServer",0);
}

CUDPMessageWriter::~CUDPMessageWriter()
{
	delete m_pMessengerUDP;
}

int CUDPMessageWriter::Connect(char *serviceName, int portNumber, char *hostName)
{
	//m_pMessengerUDP->setAddress(serviceName, portNumber, hostName);
	return true;
}

int CUDPMessageWriter::Write(char *write_buf, int writeLen)
{
	return m_pMessengerUDP->Write(write_buf, writeLen);
}

int CUDPMessageWriter::Read(char *read_buf)
{
	return m_pMessengerUDP->Read(read_buf);
}
int CUDPMessageWriter::Close()
{
	return m_pMessengerUDP->Close();
}

const char *CTCPMsgStatus::LogMessage()
{
	char l_Buffer[256];
	if (m_LogMessage)
	{
		delete m_LogMessage;
	}
/*
		EState m_eState;
	time_t m_TimeStamp;
	unsigned long m_IPAddres;
	int m_MsgType;
	clock_t m_ConnectTime;
	clock_t m_TransferTime;
	int m_NumOfRetrys;
	int m_LastError;
*/
	return 0;
}

const char *CTCPMsgStatus::StateString()
{
	switch(m_eState)
	{
	case Waiting:	// Waiting for a thread
		return "Waiting";
	case Pending:	// Got a free thread sending messages
		return "Pending";
	case Complete:	// Sent messages so complete However waiting 
					// for new messages (this will not be used,
					// but this state may exist waiting for the idle
					// task to remove it from the thread
		return "Complete";
	case Empty:		// Free to be used
		return "Empty";
	case Error:		// An Error has acurred therefore abort the message conversation
		return "Error";
	}
	return "Unknown";
}