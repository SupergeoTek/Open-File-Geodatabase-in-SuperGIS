// GDBWorkspace.h : CGDBWorkspace 的宣告

#pragma once
#include "resource.h"       // 主要符號



#include "SGFileGDB_i.h"
#include "filegdbapi.h"
#include "../PersistXMLImpl.h"
#include "../GeoDataset.h"
#include "../GDBError.h"
#include "../EncodingConv.h"
#include "NameCollection.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台上未正確支援單一執行緒 COM 物件，例如 Windows Mobile 平台沒有包含完整的 DCOM 支援。請定義 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 來強制 ATL 支援建立單一執行緒 COM 物件的實作，以及允許使用其單一執行緒 COM 物件實作。您的 rgs 檔中的執行緒模型已設定為 'Free'，因為這是非 DCOM Windows CE 平台中唯一支援的執行緒模型。"
#endif

using namespace ATL;


// CGDBWorkspace

class ATL_NO_VTABLE CGDBWorkspace :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGDBWorkspace, &CLSID_GDBWorkspace>,
	public IDispatchImpl<IGDBWorkspace, &IID_IGDBWorkspace, &LIBID_SGFileGDB, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public SGCore::IDBWorkspace4,
	public PersistXML::CPersistXMLImpl
{
public:
	CGDBWorkspace();
	virtual ~CGDBWorkspace();

DECLARE_REGISTRY_RESOURCEID(IDR_GDBWORKSPACE)


BEGIN_COM_MAP(CGDBWorkspace)
	COM_INTERFACE_ENTRY(IGDBWorkspace)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IDBWorkspace)
	COM_INTERFACE_ENTRY(IDBWorkspace2)
	COM_INTERFACE_ENTRY(IDBWorkspace3)
	COM_INTERFACE_ENTRY(IDBWorkspace4)
	COM_INTERFACE_ENTRY(IPersist)
	COM_INTERFACE_ENTRY(SGGML::IPersistXML)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

protected:
	FileGDBAPI::Geodatabase* m_pGeoDatabase;
	CNameCollection *m_pColl;
	CString m_filePath;
	bool m_FeatureEditable;

	void closeDB();

public:
	virtual void LoadSaveXMLExt(DWORD version, SGGML::IXMLArchiveElement *pSource = NULL, BOOL bLoad = TRUE);

	BOOL IsConnected();
	FileGDBAPI::Geodatabase* getGeoDatabase() { return m_pGeoDatabase;  };
	CString getFilePath() { return m_filePath; };

	__declspec(property(get = GetFeatureEditable)) bool FeatureEditable;
	bool GetFeatureEditable() { return m_FeatureEditable; }
	CFields* CreateField(LPCWSTR TableName, LPCWSTR GeomField, BOOL bIdent = TRUE);

	STDMETHOD(Open)(BSTR FilePath, VARIANT_BOOL *Result);
	STDMETHOD(Create)(BSTR FilePath, VARIANT_BOOL *Result);

	// IDBWorkspace
	STDMETHOD(raw_SchemaNames)(SGCore::INameCollection **Collection);
	STDMETHOD(raw_TableNames)(BSTR Schema, SGCore::INameCollection **Collection);
	STDMETHOD(raw_FieldGeometryType)(BSTR Table, BSTR Field, SGCore::SGOGeometryType *Type);
	STDMETHOD(raw_OpenTable)(BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType, SGCore::IFeatureClass **pFeatureClass);
	STDMETHOD(raw_CreateTable)(BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType, SGCore::IFields *Fields, SGCore::IFeatureClass **pFeatureClass);

	STDMETHOD(get_Name)(BSTR *pVal);
	STDMETHOD(get_Authority)(BSTR *pVal);
	STDMETHOD(get_IsValid)(VARIANT_BOOL *pVal);

	STDMETHOD(raw_TableFields)(BSTR TableName, SGCore::IFields **Fields);
	STDMETHOD(raw_DeleteTable)(BSTR TableName);

	STDMETHOD(raw_ExecuteNonQueryCommand)(BSTR Command);
	STDMETHOD(raw_ExecuteScalarCommand)(BSTR Command, VARIANT *result);
	STDMETHOD(raw_ExecuteTableCommand)(BSTR Command, SGCore::ITable **result);

	// IDBWorkspace3
	STDMETHOD(raw_CreateParcelInformation)(VARIANT_BOOL *Result);
	STDMETHOD(raw_OpenParcel)(BSTR Name, IUnknown **pUnk);
	STDMETHOD(raw_CreateParcel)(BSTR Name, IUnknown **pUnk);
	STDMETHOD(raw_DeleteParcel)(BSTR Name);
	STDMETHOD(raw_ParcelNames)(BSTR Schema, SGCore::INameCollection **Collection);

	// IDBWorkspace4
	STDMETHOD(raw_CreateTable2)(BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType, SGCore::IFields *Fields, IUnknown *SpatialReference, SGCore::IFeatureClass **pFeatureClass);
};

OBJECT_ENTRY_AUTO(__uuidof(GDBWorkspace), CGDBWorkspace)
