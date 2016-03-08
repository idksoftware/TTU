#include "StdAfx.h"
#include "OnRouteTaskFile.h"
#include "OnRoute.h"
	
COnRouteTaskFileError COnRouteTaskFile::OnRouteTASKFILE_ERROR("006");

COnRouteTaskFileError::COnRouteTaskFileError(CIDKStr szCode) : CIDKEErrorPackage(szCode),
	OPENED_FILE
		(szCode,  1, "Opened config file '%s'",			CIDKEError::IDK_SEVERITY_DEBUG),
	FAILED_OPENING_FILE
		(szCode,  2, "failed opening config file '%s'", CIDKEError::IDK_SEVERITY_ERROR),
	FOUND_OPTIONS
		(szCode,  3, "Found Options",					CIDKEError::IDK_SEVERITY_DEBUG),
	FOUND_ITEM
		(szCode,  4, "Item %s = %s", CIDKEError::IDK_SEVERITY_DEBUG),
	READ_FILE
		(szCode, 5, "Config file %s read successfully",	CIDKEError::IDK_SEVERITY_DEBUG),
	FAILED_READING_FILE
		(szCode, 6, "Failed reading config file %s",	CIDKEError::IDK_SEVERITY_ERROR)
{}



bool COnRouteTaskFile::Read(const char *szConfigPath, const char *szConfigFile)
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
		LogMessage(OnRouteTASKFILE_ERROR.FAILED_OPENING_FILE, l_ConfigString);
		delete pFile;
		return false;
	}
	LogMessage(OnRouteTASKFILE_ERROR.OPENED_FILE, l_ConfigString);
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

		if (szKeyword1 == "OPTIONS")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_OPTIONS);
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
	m_pOnRouteSystem->_defaultExitAddOn = 10;
	m_pOnRouteSystem->_defaultArrivingAddOn = 20;
	m_pOnRouteSystem->_veryCloseAddOn = 0;

	m_pOnRouteSystem->_defaultGeoFence = (double)20/(double)1851.85;
	m_pOnRouteSystem->_defaultPassAngle = 100;	//100 degrees
	m_pOnRouteSystem->_smoothingMethod = "NONE";
	m_pOnRouteSystem->_coverageLossThreshold = 0;

	while ((eRetValue = pFile->ReadLine(fields)) == IDK_INITFILE_OK)
	{

		CIDKStr szKeyword1 = fields.GetField(0);
		CIDKStr szKeyword2 = fields.GetField(1);

	

		if (szKeyword1 == "EXITADDON")
		{
			
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_defaultExitAddOn = atoi(fields.GetField(1));
			continue;
		}

		if (szKeyword1 == "ARRIVINGADDON")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_defaultArrivingAddOn = atoi(fields.GetField(1));
			continue;
		}

		if (szKeyword1 == "VERYCLOSEADDON")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_veryCloseAddOn = atoi(fields.GetField(1));
			continue;
		}
	
		if (szKeyword1 == "DEFAULTGEOFENCE")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_defaultGeoFence = (atof(fields.GetField(1)))/1851.85;
			continue;
		}
	
		if (szKeyword1 == "DEFAULTGEOFENCE")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_defaultGeoFence = (atof(fields.GetField(1)))/1851.85;
			continue;
		}

		if (szKeyword1 == "DEFAULTPASSANGLE")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_defaultPassAngle = (atoi(fields.GetField(1)))/1851.85;
			continue;
		}
	
		if (szKeyword1 == "SMOOTHINGMETHOD")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_smoothingMethod = fields.GetField(1);
			continue;
		}
	
	

//	if ( (gSelfConfig->SCgetTaskParam("ACTIVEBUSROUTE", "atStopLookAhead", sTmp)) != CM_ERROR)
//		_atStopLookAhead = atoi(sTmp);
//	else
//		_atStopLookAhead = 0;

		if (szKeyword1 == "SMOOTHINGMETHOD")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_coverageLossThreshold = atoi(fields.GetField(1));
			continue;
		}

		if (szKeyword1 == "COVERAGELOSSTHESHOLD")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->_coverageLossThreshold = atoi(fields.GetField(1));
			continue;
		}

		if (szKeyword1 == "INACTIVITYTIMEOUT")
		{
			LogMessage(OnRouteTASKFILE_ERROR.FOUND_ITEM, szKeyword1, fields.GetField(1) );
			m_pOnRouteSystem->m_InactivityTimeout = atoi(fields.GetField(1));
			continue;
		}
	
	
// Get Inactivity Parameters
		/*
		if (szKeyword == "INACTIVITYACTION")
		{
			m_pOnRouteSystem->m_InactivityTimeout = atoi(fields.GetField(1));
			continue;
		}

	if ( (gSelfConfig->SCgetTaskParam(_taskName,  "inactivityAction", 
					sTmp )) != CM_ERROR)
	{
		if (strcmp(sTmp, "reboot") == 0)
			m_pOnRouteSystem->_inactivityAction = INACTIVE_REBOOT;
		else
		{
			if (strcmp(sTmp, "restart") == 0)
				_inactivityAction = INACTIVE_RESTART;
		}
	}

	if ( (gSelfConfig->SCgetTaskParam(_taskName,  "inactivityReport", 
					sTmp )) == CM_ERROR)
		m_pOnRouteSystem->_inactivityReport = false;
	else
		_inactivityReport = atoi(sTmp);
		*/

		if (szKeyword1 == "]")
		{
			break;
		}
		return false;
	}


	if (eRetValue != IDK_INITFILE_OK)
	{
		LogMessage(OnRouteTASKFILE_ERROR.FAILED_READING_FILE, l_ConfigString);
		delete pFile;
		return false;
	}
	m_pOnRouteSystem->_defaultExitAddOn = m_pOnRouteSystem->_defaultExitAddOn / 1851.85;
	m_pOnRouteSystem->_defaultArrivingAddOn = m_pOnRouteSystem->_defaultArrivingAddOn / 1851.85;
	m_pOnRouteSystem->_veryCloseAddOn = m_pOnRouteSystem->_veryCloseAddOn / 1851.85;
	delete pFile;

	LogMessage(OnRouteTASKFILE_ERROR.READ_FILE, l_ConfigString);

	return true;
}
