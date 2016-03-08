/*
	COutboundServer.cpp
	----------------------

	--------------------------------------------------------------------------
	RCS Information
	--------------------------------------------------------------------------
	$Name:  $
	$Id: OutboundServer.cpp,v 1.3 2003/11/06 14:58:04 iainf Exp $
	--------------------------------------------------------------------------

	Monitors inbound messages whichhave come from the outside world ( radio, network etc)
	and forwards them to their correct destination

   --------------------------------------------------------------------------
   RCS Information
   --------------------------------------------------------------------------
   $Author: iainf $
   $Date: 2003/11/06 14:58:04 $
   $Name:  $
   $Locker:  $
   $Source: /opt/Centurion/cvsroot/dev/OBU/base/OutboundServer.cpp,v $
   $Revision: 1.3 $
   $State: Exp $
   --------------------------------------------------------------------------

*/

#ifdef WIN32
#include <stdafx.h>
#endif

#ifdef MSCPP
#include "stdafx.h"
#endif

#include <VxWorks.h>
#include "VxWorksWrapper.h"
#include "CenturionMessage.h"
#include "Diagnostics.h"
#include "CenturionTask.h"
#include "MessengerStub.h"
#include "MessengerTask.h"
#include "OutboundServer.h"
#include "LinkedList.h"
#include "SelfConfigurer.h"
#include "TCPServer.h"

extern SelfConfigurer *gSelfConfig;

/* -----------------------------------------------------------------------------
	Constructor
	-----------

	Prepares COutboundServer
   ----------------------------------------------------------------------------- */
COutboundServer::COutboundServer(
		char 	*taskName,
		char 	*taskType,
		int	channelNumber
		) : MessengerTask ( taskName, taskType, channelNumber )
{
	_timeOfLastMessage = 0;
	
	_loopInterval = 1;
	int l_iPort = 80;
	m_TCPServer = new CTCPServer(this, l_iPort);
	_loopTaskLooping = true;
	
}

/* -----------------------------------------------------------------------------
	taskInitialize
	-----------

	Prepares class for initialization
   ----------------------------------------------------------------------------- */
STATUS COutboundServer::taskInitialize()
{
	int retval;

	retval = MessengerTask::taskInitialize();

	// Get parameter indicating which task to send the outbound data to.
	// For example we masy want to send the message to a message 
	// reformatter before passing to OUTSIDEWORLD
	if ( (gSelfConfig->SCgetTaskParam(_taskName, "outboundTask", 
				_outboundTask)) == CM_ERROR)
	{
		strcpy(_outboundTask, "OUTBOUNDSERVER");
	}

	return(retval);
}

/* -----------------------------------------------------------------------------
	Destructor
	-----------

	Prepares UnitInboundMonitor
   ----------------------------------------------------------------------------- */
COutboundServer::~COutboundServer()
{
	delete m_TCPServer;
}

/* -----------------------------------------------------------------------------
	Function: Process Message
	-------------------------

	Processes a message received on the By the Outbound Monitor 
	and writes it to OUTSIDEWORLD
   ----------------------------------------------------------------------------- */
STATUS COutboundServer::processMessage(char *inMessage, int messLen)
{
	int retval=0;
	int ret;

	ret =  MessengerTask::processMessage(inMessage, messLen);
	if ( ret )
	{
		return(retval);
	}

//	retval = taskWrite(_outboundTask, inMessage, messLen);


	CMSG_SENDINSTRUCTIONS_3 msg = (	CMSG_SENDINSTRUCTIONS_3 ) inMessage;
	_timeOfLastMessage = time(NULL);
	
	char *IpAddr = msg->addressSpec;;
	m_TCPServer->Send((unsigned char *)inMessage, messLen, IpAddr);

	return(retval);
}

STATUS COutboundServer::taskLoop()
{
	_loopTaskLooping = true;
#ifdef WIN32
	while ( _loopTaskLooping )
	{
		m_TCPServer->OnIdle();
		taskDelay(_loopInterval);
	}
#endif

	return(0);
}

