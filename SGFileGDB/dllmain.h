// dllmain.h : 模組類別的宣告。

class CSGFileGDBModule : public ATL::CAtlDllModuleT< CSGFileGDBModule >
{
public :
	DECLARE_LIBID(LIBID_SGFileGDB)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_SGFILEGDB, "{1CD0E49E-740D-40FE-B702-40BD658A4EE2}")
};

extern class CSGFileGDBModule _AtlModule;
