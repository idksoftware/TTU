#pragma once

#include "CIDKStringFields.h"
#include "CIDKInitFile.h"
#include "CIDKLinkList.h"
#include "CIDKStr.h"
#include "CIDKEError.h"
#include "CIDKEObject.h"
#include "CIDKEErrorLog.h"

class COnRouteTaskFileError : public CIDKEErrorPackage
{
public:

	COnRouteTaskFileError(CIDKStr szCode);

	CIDKEError OPENED_FILE;
	CIDKEError FAILED_OPENING_FILE;
	CIDKEError FOUND_OPTIONS;
	CIDKEError FOUND_ITEM;
	CIDKEError READ_FILE;
	CIDKEError FAILED_READING_FILE;
};

class COnRoute;
class COnRouteTaskFile : public CIDKEObject
{
public:
	typedef enum {
		ErrOk,
		ErrReadingConfigFile
	} EError;
private:
	static COnRouteTaskFile *m_This;
	EError m_eError;
	COnRoute *m_pOnRouteSystem;
	
	static COnRouteTaskFileError OnRouteTASKFILE_ERROR;
	

public:
	bool Read(const char *szCommandPath, const char *szCommand);
	EError GetLastError()
	{
		return m_eError;
	}

	COnRouteTaskFile(COnRoute *pOnRouteSystem)
	{
		m_eError = ErrOk;
		m_pOnRouteSystem = pOnRouteSystem;
	}
};
