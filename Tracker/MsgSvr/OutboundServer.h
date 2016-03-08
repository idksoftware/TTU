/*
	COutboundServer.h
	--------------------

	--------------------------------------------------------------------------
	RCS Information
	--------------------------------------------------------------------------
	$Name:  $
	$Id: OutboundServer.h,v 1.3 2003/11/06 14:58:38 iainf Exp $
	--------------------------------------------------------------------------

	Monitors outbound messages to be sent to the outside world ( radio, network etc)
	and forwards them to their correct destination
*/

class MessengerTask;

/* ----------- Outbound Message Structure ----------------------*/
/* Indicates address and other information about how to deliver */
/* the message */
/*
struct _OutboundMessage
{
	CM_MESSAGETYPE		messageType;
	CM_ADDRESSSPEC		addressSpec;
	int					repeats;
	MM_MESSPTR			messageData[MAX_DATA_MESSAGE_LEN];
};
typedef	_OutboundMessage	*CMSG_UPLOADREQUEST;
*/
class CTCPServer;
class COutboundServer: public MessengerTask
{
	public:
				COutboundServer(char 	*taskName,
							char 	*taskType,
							int		channelNumber);
				~COutboundServer();
		STATUS	taskInitialize();
		STATUS	processMessage(char *inMessage, int messLen);
		time_t	getTimeOfLastMessage() { return _timeOfLastMessage; }
		CTCPServer *m_TCPServer;
	private:
		char	_outboundTask[30];
		time_t	_timeOfLastMessage;
		STATUS taskLoop();

		int _loopTaskLooping;
		int _loopInterval;
};
