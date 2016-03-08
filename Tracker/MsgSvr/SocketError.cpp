#include "stdafx.h"
#include "SocketError.h"

CSocketError CSocketError::SocketErrorList[] = {
CSocketError(WSAEINTR, "Interrupted function call"), 
CSocketError(WSAEACCES, "Permission denied."), 
CSocketError(WSAEFAULT, "Bad address."), 
CSocketError(WSAEINVAL, "Invalid argument."), 
CSocketError(WSAEMFILE, "Too many open files."), 
CSocketError(WSAEWOULDBLOCK, "Resource temporarily unavailable."), 
CSocketError(WSAEINPROGRESS, "Operation now in progress."), 
CSocketError(WSAEALREADY, "Operation already in progress."), 
CSocketError(WSAENOTSOCK, "Socket operation on nonsocket."), 
CSocketError(WSAEDESTADDRREQ, "Destination address required."), 
CSocketError(WSAEMSGSIZE, "Message too long."), 
CSocketError(WSAEPROTOTYPE, "Protocol wrong type for socket."),  
CSocketError(WSAENOPROTOOPT, "Bad protocol option."), 
CSocketError(WSAEPROTONOSUPPORT, "Protocol not supported."), 
CSocketError(WSAESOCKTNOSUPPORT, "Socket type not supported."), 
CSocketError(WSAEOPNOTSUPP, "Operation not supported."), 
CSocketError(WSAEPFNOSUPPORT, "Protocol family not supported."), 
CSocketError(WSAEAFNOSUPPORT, "Address family not supported by protocol family."), 
CSocketError(WSAEADDRINUSE, "Address already in use."), 
CSocketError(WSAEADDRNOTAVAIL, "Cannot assign requested address."), 
CSocketError(WSAENETDOWN, "Network is down."), 
CSocketError(WSAENETUNREACH, "Network is unreachable."), 
CSocketError(WSAENETRESET, "Network dropped connection on reset."), 
CSocketError(WSAECONNABORTED, "Software caused connection abort."), 
CSocketError(WSAECONNRESET, "Connection reset by peer."), 
CSocketError(WSAENOBUFS, "No buffer space available."), 
CSocketError(WSAEISCONN, "Socket is already connected."), 
CSocketError(WSAENOTCONN, "Socket is not connected."), 
CSocketError(WSAESHUTDOWN, "Cannot send after socket shutdown."), 
CSocketError(WSAETIMEDOUT, "Connection timed out."), 
CSocketError(WSAECONNREFUSED, "Connection refused." ), 
CSocketError(WSAEHOSTDOWN, "Host is down."), 
CSocketError(WSAEHOSTUNREACH, "No route to host."), 
CSocketError(WSAEPROCLIM, "Too many processes."), 
CSocketError(WSASYSNOTREADY, "Network subsystem is unavailable."), 
CSocketError(WSAVERNOTSUPPORTED, "Winsock.dll version out of range."), 
CSocketError(WSANOTINITIALISED, "Successful WSAStartup not yet performed."), 
CSocketError(WSAEDISCON, "Graceful shutdown in progress."), 
CSocketError(WSATYPE_NOT_FOUND, "Class type not found."), 
CSocketError(WSAHOST_NOT_FOUND, "Host not found."), 
CSocketError(WSATRY_AGAIN, "Nonauthoritative host not found."), 
CSocketError(WSANO_RECOVERY, "This is a nonrecoverable error."), 
CSocketError(WSANO_DATA, "Valid name, no data record of requested type."), 
CSocketError(WSA_INVALID_HANDLE, "Specified event object handle is invalid."),  
CSocketError(WSA_INVALID_PARAMETER, "One or more parameters are invalid."), 
CSocketError(WSA_IO_INCOMPLETE, "Overlapped I/O event object not in signaled state."), 
CSocketError(WSA_IO_PENDING, "OS dependent Overlapped operations will complete later."), 
CSocketError(WSA_NOT_ENOUGH_MEMORY, "Insufficient memory available."), 
CSocketError(WSA_OPERATION_ABORTED, "Overlapped operation aborted."), 
//CSocketError(WSAINVALIDPROCTABLE, "Invalid procedure table from service provider."), 
//CSocketError(WSAINVALIDPROVIDER, "Invalid service provider version number."), 
//CSocketError(WSAPROVIDERFAILEDINIT, "Unable to initialize a service provider."), 
CSocketError(WSASYSCALLFAILURE, "System call failure.")
};

const char *CSocketError::ErrorString(int iCode)
{
	int c = sizeof(SocketErrorList)/sizeof(CSocketError);

	for (int i = 0; i < c; i++ )
	{
		if (SocketErrorList[i].m_iCode == iCode)
		{
			return SocketErrorList[i].m_szText;
		}
	}
	return 0;
}