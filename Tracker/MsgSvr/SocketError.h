
#ifdef WIN32
#include "Winsock2.h"

class CSocketError
{
	int m_iCode;
	const char *m_szText;
	static CSocketError SocketErrorList[];
public:
	CSocketError(int iCode, const char *szText)
	{
		m_iCode = iCode;
		m_szText = szText;
	}

	static const char *ErrorString(int iCode);
	
};

#endif