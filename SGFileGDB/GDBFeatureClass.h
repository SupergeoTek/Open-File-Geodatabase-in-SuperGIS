// GDBFeatureClass.h : CGDBFeatureClass 的宣告

#pragma once
#include "resource.h"       // 主要符號



#include "SGFileGDB_i.h"
#include "../GeoDataset.h"
#include "GDBWorkspace.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE 平台上未正確支援單一執行緒 COM 物件，例如 Windows Mobile 平台沒有包含完整的 DCOM 支援。請定義 _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA 來強制 ATL 支援建立單一執行緒 COM 物件的實作，以及允許使用其單一執行緒 COM 物件實作。您的 rgs 檔中的執行緒模型已設定為 'Free'，因為這是非 DCOM Windows CE 平台中唯一支援的執行緒模型。"
#endif

using namespace ATL;

class CGDBFeature;

// CGDBFeatureClass

class ATL_NO_VTABLE CGDBFeatureClass :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CGDBFeatureClass, &CLSID_GDBFeatureClass>,
	public IDispatchImpl<IGDBFeatureClass, &IID_IGDBFeatureClass, &LIBID_SGFileGDB, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IDispatchImpl<SGCore::IFeatureClassEdit, &SGCore::IID_IFeatureClassEdit, &SGCore::LIBID_SGCore>,
	public SGCore::ITableEdit,
	public SGCore::IEditTask,
	public SGCore::IFieldsAlter,
	public SGCore::IAttachment,
	public CGeoDatasetImpl,
	public SGCore::IGDBClass,
	public SGCore::IFeatureClass2,	//k
	public PersistXML::CPersistXMLImpl
{
public:
	friend CGDBFeature;
	CGDBFeatureClass();
	virtual ~CGDBFeatureClass();

	static HRESULT WINAPI InterfaceFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw);


DECLARE_REGISTRY_RESOURCEID(IDR_GDBFEATURECLASS)


BEGIN_COM_MAP(CGDBFeatureClass)
	COM_INTERFACE_ENTRY(IGDBFeatureClass)
	COM_INTERFACE_ENTRY2(IDispatch, IGDBFeatureClass)
	COM_INTERFACE_ENTRY(SGCore::IFeatureClass)
	COM_INTERFACE_ENTRY(SGCore::IGeoDataset)
	COM_INTERFACE_ENTRY(SGCore::IGeoDataset2)
	COM_INTERFACE_ENTRY(SGCore::ITable)
	COM_INTERFACE_ENTRY_FUNC(SGCore::IID_IFeatureClassEdit, 0, InterfaceFunc)
	COM_INTERFACE_ENTRY_FUNC(SGCore::IID_ITableEdit, 1, InterfaceFunc)
	COM_INTERFACE_ENTRY_FUNC(SGCore::IID_IEditTask, 2, InterfaceFunc)
	COM_INTERFACE_ENTRY(SGCore::IFieldsAlter)
	COM_INTERFACE_ENTRY(SGCore::IAttachment)
	COM_INTERFACE_ENTRY(IPersist)
	COM_INTERFACE_ENTRY(SGGML::IPersistXML)
	COM_INTERFACE_ENTRY(SGCore::IGDBClass)
	COM_INTERFACE_ENTRY(SGCore::IFeatureClass2)		//k
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
	CGDBWorkspace* m_pWorkspace;
	FileGDBAPI::Table* m_pTable;
	FileGDBAPI::EnumRows* m_pRows;
	FileGDBAPI::Row* m_pCurRow;
	CFields* m_Fields;
	CArray<FileGDBAPI::FieldType> m_ftypes;
	CArray<std::wstring> m_fnames;
	//CArray<int> m_fOrder;	m_Fields->m_index[]
	//int m_IdentIndex;
	//int m_GeomIndex;
	CString m_TableName;
	CString m_oidField;
	CString m_GeometryField;
	SGCore::SGOGeometryType m_GeometryType;
	int m_gtype;
	bool m_bZ, m_bM;
	int m_srid;
	SGSFCOMS::IGeometryFactoryPtr m_GeoFac;
	int m_maxfid;
	SGCore::SGOEditTaskStatus m_EditStatus;
	long m_editRef;
	long m_curFi;
	long m_curFid;

	long m_rowN;
	long m_RowIndex;
	long m_UpdateCount;
	SGCore::IMappedFIDEditPtr m_fids2;		//k
	long m_lockkey;

	long m_nRecDirty;
	long m_nGeomDirty;
	long m_nValueDirty;	//value有更新
	FileGDBAPI::ShapeBuffer* m_ftShape;
	FileGDBAPI::ByteArray m_ftByteArray;

	virtual void LoadSaveXMLExt(DWORD version,
		SGGML::IXMLArchiveElement *pSource = NULL,
		BOOL bLoad = TRUE);

	void closeGDBObject();
	bool m_bBulk;
	void StartBulkLoad();
	void EndBulkLoad();
	int ResetFieldsQuery(bool bOpenTable=false);


	SGSFCOMS::IGeometryPtr CreateGeometry(VARIANT *var, VARIANT* retWKB = NULL);
	void UpdateEnvelope(SGSFCOMS::IEnvelope *pEnv);
	long FindFieldIndex(VARIANT *pField);
	//SGCore::IFieldPtr GetField(long nIndex);
	void DeleteKey(long key);		//k
	void AddKey(long key, VARIANT fid);
	void UpdateKeyBoundary(long key, SGCore::tagSGDRect* pRect);

	void ResetMappedFID();
	void SetRecDirty() { m_nRecDirty++; };
	void SetGeomDirty() { m_nGeomDirty++; };
	void SetValueDirty() { m_nValueDirty++; };
	bool GetFeatureEditable();
	int ResetReading();
	int FetchRowFeature(long& row, long fid = -1, BOOL bSub = FALSE);
	int GetRow(FileGDBAPI::EnumRows& enumRows, FileGDBAPI::Row& row, int nFID);
	int VariantToGDBRowValue(FileGDBAPI::Row& row, int index, VARIANT* pVal);

	long InsertFeature(CArray<VARIANT>& vals, SGCore::tagSGDRect* pRect = NULL);
	long DeleteFeature(long nFID);
	long UpdateFeature(long nFID, CArray<VARIANT>& vals, CArray<int>& fldSets, SGCore::tagSGDRect* pRect = NULL);

public:
	BOOL OpenTable(CGDBWorkspace *pWS, BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType);
	SGCore::SGOGeometryType GetGeometryType() {	return m_GeometryType;	};

	// SGCore::ITable
	STDMETHOD(get_RowCount)(long *pVal);
	STDMETHOD(get_Row)(long Index, SGCore::ITableRow **pRow);
	STDMETHOD(raw_Search)(SGCore::IFilter *Filter, SGCore::ITableCursor **pCursor);
	// SGCore::ITableEdit
	STDMETHOD(raw_CreateRow)(SGCore::ITableRowEdit **Row);

	// IFeatureClass
	STDMETHOD(raw_CreateFeatureDraw)(SGCore::IFeatureDraw **pVal);
	STDMETHOD(get_FeatureType)(SGCore::SGOGeometryType *pVal);
	STDMETHOD(get_Fields)(SGCore::IFields **pVal);
	STDMETHOD(raw_Search)(SGCore::IFeatureFilter* Filter, SGCore::IFeatureCursor **FeatureCursor);

	// IFeatureClassEdit
	STDMETHOD(raw_CreateFeature)(SGCore::IFeatureEdit **Feature);
	// IEditTask
	STDMETHOD(raw_BeginEdit)(VARIANT_BOOL *pVal);
	STDMETHOD(raw_EndEdit)();
	STDMETHOD(get_Status)(SGCore::SGOEditTaskStatus *pVal);

	STDMETHOD(raw_AddField)(SGCore::IField* newField);
	STDMETHOD(raw_DropField)(VARIANT Field);
	STDMETHOD(raw_ModifyField)(VARIANT Field, SGCore::IField* newField);

	// IAttachment
	STDMETHOD(get_Attachment)(BSTR Name, VARIANT* pVal);
	STDMETHOD(put_Attachment)(BSTR Name, VARIANT newVal);

	// IGDBClass
	STDMETHOD(raw_Connect)(BSTR ConnectionString, VARIANT_BOOL *Result);
	STDMETHOD(get_ConnectionString)(BSTR *pVal);
	STDMETHOD(get_ContentType)(BSTR *pVal);
	STDMETHOD(get_IsValid)(VARIANT_BOOL *Result);

	//IFeatureClass2		//k
	STDMETHOD(get_MappedFID)(SGCore::IMappedFID** ppVal);
};


class CGDBFeature :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<SGCore::IFeatureEdit, &SGCore::IID_IFeatureEdit, &SGCore::LIBID_SGCore>,
	public SGCore::ITableRowEdit
{
public:
	CGDBFeature(void);
	virtual ~CGDBFeature(void);

	BEGIN_COM_MAP(CGDBFeature)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(SGCore::IFeature)
		COM_INTERFACE_ENTRY(SGCore::IFeatureEdit)
		COM_INTERFACE_ENTRY(SGCore::ITableRow)
		COM_INTERFACE_ENTRY(SGCore::ITableRowEdit)
	END_COM_MAP()

protected:
	CGDBFeatureClass *m_pClass;
	BOOL m_bCreate;
	long m_fid;
	long m_RowIndex;
	//OGRFeature* m_poFeature;

	//SGSFCOMS::IGeometryPtr m_pGeometry;
	SGCore::tagSGDRect m_rect;
	SGSFCOMS::IEnvelopePtr m_Envelope;
	long m_GeometryFieldFormat;
	long m_nGeomDirty;
	VARIANT* m_pVarGeom;
	//VARIANT m_varGeom;
	CArray<VARIANT> m_varVals;
	CArray<VARIANT> m_varOrgVals;
	CArray<int> m_fset;

	void ClearValues();

public:
	BOOL InitialFeature(CGDBFeatureClass *pClass, BOOL isNew = FALSE);
	BOOL SetupRecord(long RowIndex);
	int RecordToFeature(void* hResult, int offi, int RowIndex);

private:
	//void SetupEnvelope();

public:
	// IFeature
	STDMETHOD(get_Value)(/*[in]*/ VARIANT Field, /*[out, retval]*/ VARIANT *value);
	STDMETHOD(put_Value)(/*[in]*/ VARIANT Field, /*[in]*/ VARIANT value);
	STDMETHOD(get_Fields)(SGCore::IFields **pVal);
	STDMETHOD(get_Geometry)(SGSFCOMS::IGeometry **pVal);
	STDMETHOD(put_Geometry)(SGSFCOMS::IGeometry *newVal);
	STDMETHOD(get_Envelope)(SGSFCOMS::IEnvelope **pVal);
	STDMETHOD(get_GeometryWithWKB)(VARIANT* wkb);
	STDMETHOD(get_Identifier)(long* id);

	// IFeatureEdit
	STDMETHOD(raw_Delete)();
	STDMETHOD(raw_Update)();
	STDMETHOD(raw_CancelUpdate)();

	// ITableRow
	STDMETHOD(get_Identifier)(VARIANT* id);
};

class CGDBFeatureCursor :
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<SGCore::IFeatureCursor2, &SGCore::IID_IFeatureCursor2, &SGCore::LIBID_SGCore>,
	public SGCore::ITableCursor
{
	typedef IDispatchImpl<SGCore::IFeatureCursor2, &SGCore::IID_IFeatureCursor2, &SGCore::LIBID_SGCore> IFeatureCursor2Impl;
public:
	CGDBFeatureCursor(void);
	virtual ~CGDBFeatureCursor(void);

	BOOL InitialCursor(CGDBFeatureClass *pClass, SGCore::IFilter *pTFilter, SGCore::IFeatureFilter *pFFilter);

	BEGIN_COM_MAP(CGDBFeatureCursor)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(SGCore::IFeatureCursor)
		COM_INTERFACE_ENTRY(SGCore::IFeatureCursor2)
		COM_INTERFACE_ENTRY(SGCore::ITableCursor)
	END_COM_MAP()

protected:
	CGDBFeatureClass *m_pClass;
	SGCore::IFeatureFilterPtr m_pFFilter;
	SGCore::IFilterPtr m_pTFilter;

	long m_CurrentIndex;
	long m_TableRowCnt;
	long m_cursori;
	DWORD m_t1;
	//int m_curi;

	CComObject< CGDBFeature > *m_pFeature;
private:
	SGCore::SGOGeometryType m_GeomType;

private:
	BOOL ValidRecord(SGSFCOMS::IGeometry *pGeom);
	bool ReadRecord(long Current, CGDBFeature *pFeature);
	long NextRecord(long Current, long Step, CGDBFeature *pFeature);
	long NextIndex(long Current, long Step, CGDBFeature *pFeature);
	//bool ReadRecord(long Current);
	//long NextRecord(long Current, long Step);
	//long NextIndex(long Current, long Step);

public:
	STDMETHOD(raw_NextFeature)(SGCore::IFeature **Feature);
	STDMETHOD(get_FeatureCount)(long * pVal);
	STDMETHOD(raw_Seek)(long Offset, SGCore::SGOSeekOrigin Origin, SGCore::IFeature ** Feature);
	// SGCore::ITableCursor
	STDMETHOD(raw_NextRow)(SGCore::ITableRow **pRow);
	STDMETHOD(get_RowCount)(long *pVal);
	STDMETHOD(raw_Seek)(long Offset, SGCore::SGOSeekOrigin Origin, SGCore::ITableRow **pRow);
};

OBJECT_ENTRY_AUTO(__uuidof(GDBFeatureClass), CGDBFeatureClass)
