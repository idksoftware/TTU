#include "StdAfx.h"
#include "MsgSvrFile.h"

#include "SCCS.h"
static char RCSString[] = "$Id: GGASentence.cpp,v 1.26 2003/08/07 11:31:53 iainf Exp $";
SCCS_CLASS(LocationGeo);

CTruckInfoList *CTruckFile::m_pList;

CTruckFileError CTruckFile::TRUCKFILE_ERROR("002");



CTruckFileError::CTruckFileError(CIDKStr szCode) : CIDKEErrorPackage(szCode),
	OPENED_LOCATION_FILE
		(szCode,  1, "Opened Location file '%s'",	CIDKEError::IDK_SEVERITY_DEBUG),
	FAILED_OPENING_LOCATION_FILE
		(szCode,  2, "failed Location file '%s'", CIDKEError::IDK_SEVERITY_ERROR),
	FOUND_LOCATIONS
		(szCode,  3, "Found Locations list", CIDKEError::IDK_SEVERITY_DEBUG),
	FOUND_LOCATION
		(szCode,  4, "Location %s Found", CIDKEError::IDK_SEVERITY_DEBUG),	
	FOUND_ITEM
		(szCode,  5, "Location Parm %s = %s", CIDKEError::IDK_SEVERITY_DEBUG),
	FAILED_LOCATION
		(szCode,  6, "Failed Location %s", CIDKEError::IDK_SEVERITY_ERROR),
	READ_LOCATION_FILE
		(szCode, 7, "Read Location file %s successfully",	CIDKEError::IDK_SEVERITY_DEBUG),
	FOUND_GPSLOCATION
		(szCode, 8, "Reading GPS Location infomation",	CIDKEError::IDK_SEVERITY_DEBUG),
	FAILED_READING_LOCATION_FILE
		(szCode, 9, "Failed reading Location file %s",	CIDKEError::IDK_SEVERITY_ERROR)

{}

CTruckFile::CTruckFile()
{
	m_pList = new CTruckInfoList;
	m_eError = ErrOk;
}

CTruckFile::~CTruckFile()
{
	delete m_pList;
}

bool CTruckFile::Read(const char *szConfigPath, const char *szConfigFile)
{

	CIDKInitFile* pFile = new CIDKInitFile;

	pFile->AddDelimiter(0xA9);
	pFile->AddDelimiter('=');
	pFile->AddLiteral('\'');
	pFile->AddWhitespace('\t');
	pFile->AddWhitespace(' ');
	pFile->AddRemark('#');
	pFile->AddLiteral('"');

	CIDKStr l_ConfigString; // = "\"";
	l_ConfigString += szConfigPath;
	l_ConfigString += "\\";
	l_ConfigString += "config";
	l_ConfigString += "\\";
	l_ConfigString += szConfigFile;
	l_ConfigString += ".cfg";
	// l_ConfigString += "\"";
	if (!(pFile->Open(l_ConfigString)))

	{
		LogMessage(TRUCKFILE_ERROR.FAILED_OPENING_LOCATION_FILE, l_ConfigString);
		delete pFile;
		return false;
	}
	LogMessage(TRUCKFILE_ERROR.OPENED_LOCATION_FILE, l_ConfigString);
//CEventClient l_EventClient;
//	l_EventClient.EventReport(1, 1, 1, "", "", "");

	/*
	 *   scan file for keywords
	 *
	 *   sections must be unique and, for clarity, their order of
     *   appearance should mirror the sequence of operations. these
	 *   obligations are enforced.
	 */

	IDK_INITFILE_RETURN	eRetValue = IDK_INITFILE_OK;

	CIDKStringFields fields;

	CIDKStr szKeyword1;
	CIDKStr szKeyword2;


	while ((eRetValue = pFile->ReadLine(fields)) == IDK_INITFILE_OK)
	{

		szKeyword1 = fields.GetField(0);
		szKeyword2 = fields.GetField(1);

		if (szKeyword1 == "TRUCKS")
		{
			LogMessage(TRUCKFILE_ERROR.FOUND_LOCATIONS);
			break;
		}
	}
	while ((eRetValue = pFile->ReadLine(fields)) == IDK_INITFILE_OK)
	{

		szKeyword1 = fields.GetField(0);
		if (szKeyword1 == "[")
		{
			break;
		}

	}
	while ((eRetValue = pFile->ReadLine(fields)) == IDK_INITFILE_OK)
	{

		CIDKStr szKeyword1 = fields.GetField(0);
		CIDKStr szKeyword2 = fields.GetField(1);

	

		if (szKeyword1 == "TRUCKID")
		{
			LogMessage(TRUCKFILE_ERROR.FOUND_LOCATION, szKeyword2);
			if (!ReadTruckInfo(*pFile, szKeyword2))
			{
				return false;
			}
			continue;
		}
		if (szKeyword1 == "]")
		{
			break;
		}
		return false;
	}


	if (eRetValue != IDK_INITFILE_OK)
	{
		LogMessage(TRUCKFILE_ERROR.FAILED_READING_LOCATION_FILE, l_ConfigString);
		delete pFile;
		return false;
	}

	delete pFile;

	LogMessage(TRUCKFILE_ERROR.READ_LOCATION_FILE, l_ConfigString);
	Write(szConfigPath,"Test");
	return true;
}

bool CTruckFile::ReadTruckInfo(CIDKInitFile &rFile, const char *szID)
{
	IDK_INITFILE_RETURN	eRetValue = IDK_INITFILE_OK;

	CIDKStringFields fields;

	CTruckInfo *l_TruckInfo = new CTruckInfo(szID);

	while ((eRetValue = rFile.ReadLine(fields)) == IDK_INITFILE_OK)
	{
		CIDKStr szKeyword = fields.GetField(0);
		if (!strcmp(szKeyword, "["))
		{
			break;
		}
	}

	while ((eRetValue = rFile.ReadLine(fields)) == IDK_INITFILE_OK)
	{
		CIDKStr szKeyword = fields.GetField(0);
		
		if (szKeyword == "IPADDRESS")
		{

			LogMessage(TRUCKFILE_ERROR.FOUND_ITEM, szKeyword, fields.GetField(1));
			l_TruckInfo->m_IPAddr = fields.GetField(1);
	
			continue;
		}

		if (szKeyword == "SYNCPERIOD")
		{

			LogMessage(TRUCKFILE_ERROR.FOUND_ITEM, szKeyword, fields.GetField(1));
			l_TruckInfo->m_SyncPeriod = atoi(fields.GetField(1));
			continue;
		}

		if (!strcmp(szKeyword, "]"))
		{

			m_pList->Insert(l_TruckInfo);
			break;
		}

	}
	if (eRetValue == IDK_INITFILE_EOF)
	{
		LogMessage(TRUCKFILE_ERROR.FAILED_LOCATION);
		delete l_TruckInfo;
		return false;
	}

	return true;

}


/*
bool CLocationGeoFile::ReadLatLongInfo(CIDKInitFile &rFile, CLatLong *pLatLong)
{
	IDK_INITFILE_RETURN	eRetValue = IDK_INITFILE_OK;

	CIDKStringFields fields;

	while ((eRetValue = rFile.ReadLine(fields)) == IDK_INITFILE_OK)
	{
		CIDKStr szKeyword = fields.GetField(0);
		if (!strcmp(szKeyword, "["))
		{
			break;
		}
	}
	int l_Hours;
	int l_Min;
	double l_Sec;
	while ((eRetValue = rFile.ReadLine(fields)) == IDK_INITFILE_OK)
	{
		CIDKStr szKeyword = fields.GetField(0);

		if (szKeyword == "LATITUDE")
		{
			LogMessage(LOCATIONGEOZONEFILE_ERROR.FOUND_ITEM, szKeyword, fields.GetField(1));
		
			Deg2Num(fields.GetField(1), l_Hours,l_Min,l_Sec);
			pLatLong->Latitude.m_Latitude = CLongitude::DegMinSec(l_Hours,l_Min,l_Sec,'?');
			continue;
		}
		if (szKeyword == "NORTHING")
		{
			LogMessage(LOCATIONGEOZONEFILE_ERROR.FOUND_ITEM, szKeyword, fields.GetField(1));
			pLatLong->Latitude.m_Northing = (*(fields.GetField(1))== 'N')?CNMEA0183::North:CNMEA0183::South;
			continue;
		}
		if (szKeyword == "LONGITUDE")
		{
			LogMessage(LOCATIONGEOZONEFILE_ERROR.FOUND_ITEM, szKeyword, fields.GetField(1));
			Deg2Num(fields.GetField(1), l_Hours,l_Min,l_Sec);
			pLatLong->Longitude.m_Longitude = CLongitude::DegMinSec(l_Hours,l_Min,l_Sec,'?');
			continue;
		}
		if (szKeyword == "EASTING")
		{
			LogMessage(LOCATIONGEOZONEFILE_ERROR.FOUND_ITEM, szKeyword, fields.GetField(1));
			pLatLong->Longitude.m_Easting = (*(fields.GetField(1))=='E')?CNMEA0183::East:CNMEA0183::West;
			continue;
		}
		if (!strcmp(szKeyword, "]"))
		{

			//m_pList->Insert(pLatLong);
			break;
		}

	}
	if (eRetValue == IDK_INITFILE_EOF)
	{
		LogMessage(LOCATIONGEOZONEFILE_ERROR.FAILED_LOCATION);
		//delete pLatLong;
		return false;
	}

	return true;

}
*/
bool CTruckFile::Write(const char *szConfigPath, const char *szConfigFile)
{

	CIDKStr l_ConfigString; // = "\"";
	l_ConfigString += szConfigPath;
	l_ConfigString += "\\";
	l_ConfigString += "config";
	l_ConfigString += "\\";
	l_ConfigString += szConfigFile;

	FILE *fp = fopen(l_ConfigString, "w");

	if (!fp)
	{
		return false;
	}
	
	fprintf(fp, "#\n# Truck data file \n#\n\n");

//	CXLDate l_Now;
//	l_Now.Now();
//	fprintf(fp, "DATE=%s[\n",(const char *)l_Now.Print());

	fprintf(fp, "TRUCKS\n[\n");
	
	m_pList->Rewind();
	int i = 0;

	do {
		
		CTruckInfo *l_TrackInfo = m_pList->Get();
		fprintf(fp, "\tTRUCK ID = %d\n\t[\n",l_TrackInfo->m_ID);
/*
		fprintf(fp, "\t\tLOCATION NAME = \"%s\"\n",(char *)l_LocationGeoInfo->m_Name);
		fprintf(fp, "\t\tGPS LOCATION\n\t\t[\n");
		fprintf(fp, "\t\t\tLATITUDE  = %f\n",l_LocationGeoInfo->m_Position.Latitude.m_Latitude );
		fprintf(fp, "\t\t\tNORTHING  = %s\n",(l_LocationGeoInfo->m_Position.Latitude.m_Northing == CNMEA0183::North)?"N":"S");
		fprintf(fp, "\t\t\tLONGITUDE  = %f\n",l_LocationGeoInfo->m_Position.Longitude.m_Longitude );
		fprintf(fp, "\t\t\tEASTING  = %s\n",(l_LocationGeoInfo->m_Position.Longitude.m_Easting == CNMEA0183::East)?"E":"W");	
*/	
	fprintf(fp, "\t\t]\n");
		fprintf(fp, "\t]\n");
	} while (m_pList->Next());

	fprintf(fp, "]\n");
	fclose(fp);

	return true;
}


