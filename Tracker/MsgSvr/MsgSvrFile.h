#pragma once

#include "CIDKStringFields.h"
#include "CIDKInitFile.h"
#include "CIDKLinkList.h"
#include "CIDKStr.h"
#include "CIDKEError.h"
#include "CIDKEObject.h"
#include "CIDKEErrorLog.h"
#include "CIDKDate.h"

class CIDKTask;
class CGlobalTaskFile;
class CMessengerConnection;

class CTaskList;
class CTruckInfo
{
public:
	typedef enum {
		V_Not_Found,
		V_Found,
		V_Lost,
		V_Synced
	} EState;
	friend CGlobalTaskFile;
	friend CMessengerConnection;
	friend CIDKTask;

	int			m_ID;
	CIDKStr		m_IPAddr;
	long		m_SyncPeriod;
	CIDKDate	m_LastSynced;
	EState		m_State;
public:
	CTruckInfo(const char *szID)
	{	
		m_ID = atoi(szID);
		m_State = V_Not_Found;
	};
};

class CTruckInfoList
{
	CIDKLinkList<CTruckInfo *> m_List;
public:
	CTruckInfoList() {};
	~CTruckInfoList() {};
	bool Insert(CTruckInfo *pTruckInfo)
	{
		return m_List.Insert(pTruckInfo);
	}
	CTruckInfo *FindTask(const char *szName);

	int Count()
	{
		return m_List.Count();
	}
	void Rewind()
	{
		return m_List.Rewind();
	}
	bool Next()
	{
		return m_List.Next();
	}
	CTruckInfo *Get()
	{
		return m_List.Get();
	}


};

class CTruckFileError : public CIDKEErrorPackage
{
public:

	CTruckFileError(CIDKStr szCode);

	CIDKEError OPENED_LOCATION_FILE;
	CIDKEError FAILED_OPENING_LOCATION_FILE;
	CIDKEError FOUND_LOCATIONS;
	CIDKEError FOUND_LOCATION;
	CIDKEError FOUND_ITEM;
	CIDKEError FAILED_LOCATION;
	CIDKEError READ_LOCATION_FILE;
	CIDKEError FAILED_READING_LOCATION_FILE;
	CIDKEError FOUND_GPSLOCATION;
};

class CTruckFile : public CIDKEObject
{
public:
	typedef enum {
		ErrOk,
		ErrReadingConfigFile
	} EError;
private:
	
	bool Read(CIDKInitFile &rFile, const char *szName);
	bool ReadTruckInfo(CIDKInitFile &rFile, const char *szID);
//	bool ReadLatLongInfo(CIDKInitFile &rFile, CLatLong *pLatLong);
	
	EError m_eError;
	static CTruckInfoList *m_pList;
protected:
	static CTruckFileError TRUCKFILE_ERROR;

public:

	CTruckFile();
	~CTruckFile();

	bool Read(const char *szPath, const char *szCommand);
	bool Write(const char *szPath, const char *szFilename);

	EError GetLastError()
	{
		return m_eError;
	}

	int Count()
	{
		return m_pList->Count();
	}

	static CTruckInfoList *GetList()
	{
		return m_pList;
	}
};
