#pragma once

#include <time.h>
#include "CIDKQueue.h"

class CMessagePackage;
class CMessageWriter;
class CWriteChild
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
	CIDKQueue<CMessagePackage *> m_PackageQueue;
	
	CMessageWriter *m_pMessageWriter;

private:
	EState m_State;
	time_t m_Time;
	unsigned long m_IPAddr;   // 255.255.255.255
	int m_iPortNumber;
public:
	CWriteChild(CMessageWriter *pMessageWriter, int iPortNumber);
	~CWriteChild();
	void SetCompleted()	{ m_State = CWriteChild::Complete; };
	void SetPending() { m_State = CWriteChild::Pending; };
	CWriteChild::EState GetState() { return m_State; };

	unsigned long GetIPAddr()
	{
		return m_IPAddr;
	}

	void Queue(CMessagePackage *pPackage);

	bool Send();
	void SetTime();

	time_t GetTime()
	{
		return m_Time;
	}

	void SetEmpty()
	{
	//	return m_Time;
	}

	void SetIPAddr(unsigned long aIPAddr)
	{
		m_IPAddr = aIPAddr;
	}

};
