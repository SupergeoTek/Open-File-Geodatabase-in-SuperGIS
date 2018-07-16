// dllmain.cpp : DllMain ªº¹ê§@¡C

#include "stdafx.h"
#include "resource.h"
#include "SGFileGDB_i.h"
#include "dllmain.h"
#include "xdlldata.h"
#include "../xml.h"

_variant_t _varConfiguration;

SGGML::IXMLArchivePtr GetAppDoc()
{
	SGGML::IXMLArchivePtr _AppDoc = NewDocument();
	if (LoadDocument(_AppDoc, _varConfiguration))
		return _AppDoc;
	return NULL;
}



CSGFileGDBModule _AtlModule;

class CSGFileGDBApp : public CWinApp
{
public:

// ÂÐ¼g
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(CSGFileGDBApp, CWinApp)
END_MESSAGE_MAP()

CSGFileGDBApp theApp;

BOOL CSGFileGDBApp::InitInstance()
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(m_hInstance, DLL_PROCESS_ATTACH, NULL))
		return FALSE;
#endif
	return CWinApp::InitInstance();
}

int CSGFileGDBApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}
