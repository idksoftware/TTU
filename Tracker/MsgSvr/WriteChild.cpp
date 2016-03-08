#include "stdafx.h"
#include "WriteChild.h"
#include "CIDKMutex.h"
#include "TCPServer.h"
#include "MessengerTask.h"

CWriteChild::CWriteChild(CMessageWriter *pMessageWriter, int iPortNumber)
{
	m_State = CWriteChild::Empty;
	m_pMessageWriter = pMessageWriter;
	m_iPortNumber = iPortNumber;
}

/*
bool CWriteChild::Send(CMessagePackage *pPackage)
{
	switch(m_State)
//	case 
//	if (m_PackageQueue.GetSize() )
	m_pPackage = pPackage;
	m_State = CWriteChild::Waiting;
	return true;
}
*/

CWriteChild::~CWriteChild()
{
	
}

//
// Note this send function will send all the messages queued on this IP address
//

bool CWriteChild::Send()
{
	if (!m_PackageQueue.GetSize())
	{
		// No messages to send;
		m_State = CWriteChild::Complete;
	}
	char l_szHostName[16];
	in_addr l_sAddr;
	l_sAddr.s_addr = m_IPAddr;

	strcpy(l_szHostName, inet_ntoa(l_sAddr));

	int res = m_pMessageWriter->Connect(0, m_iPortNumber, l_szHostName);
	if (res < 0)
	{
		return res;
	}

	do {
		CIDKCriticalSection l_cs;
		l_cs.Enter();
		CMessagePackage *l_pPackage;
		m_State = CWriteChild::Pending;
		m_PackageQueue.RemoveItem(l_pPackage);
		
		if (!l_pPackage)
		{
			m_State = CWriteChild::Complete;
			l_cs.Leave();
			return true;
		}
		SetTime();
		CTCPMsgStatus l_MsgStatus = l_pPackage->GetMsgStatus();
		l_MsgStatus.StartTransmitTime();
		int l_Retrys = 3;  // This needs to be set as a config parameter 
		for (int i = 0; i < l_Retrys; i++)
		{
			res = m_pMessageWriter->Write((char *)l_pPackage->GetBuffer(), l_pPackage->GetSize());
			if (res == 0)
			{
				l_MsgStatus.SetNumOfRetrys(i);
				break;
			}
		}
		l_MsgStatus.EndTransmitTime();
		//taskWriteGlobal("UNITOUTBOUND", (char *)l_pPackage->GetBuffer(), l_pPackage->GetSize());
		l_pPackage->Empty();
		l_cs.Leave();
	} while (m_PackageQueue.GetSize());
	m_State = CWriteChild::Complete;
	res = m_pMessageWriter->Close();
	return true;
}

void CWriteChild::SetTime()
{
	m_Time = time(0);
}

void CWriteChild::Queue(CMessagePackage *pPackage)
{
	CIDKCriticalSection l_cs;
	l_cs.Enter();
	m_PackageQueue.AddItem(pPackage);
	l_cs.Leave();
}

