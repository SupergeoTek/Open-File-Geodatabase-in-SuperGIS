// GDBWorkspace.cpp : CGDBWorkspace 的實作

#include "stdafx.h"
#include "GDBWorkspace.h"
#include "GDBFeatureClass.h"

EncodingConv g_ec;
BOOL IsFieldExist(std::vector<CString> *FNs, LPCTSTR IndFld)
{
	long Cnt = (long)FNs->size();
	for (long i = 0; i < Cnt; i++)
	{
		CString pFld = FNs->at(i);
		if (pFld.CompareNoCase(IndFld) == 0)
			return TRUE;
	}
	return FALSE;
}

CString ConfirmFieldName(std::vector<CString> *FNs, LPCTSTR sFld, long nStartIndex)
{
	long Idx = nStartIndex;
	CString IndFld = sFld;
	if (nStartIndex > 0)
		IndFld.Format(_T("%s_%d"), sFld, Idx);
	while (IsFieldExist(FNs, IndFld))
		IndFld.Format(_T("%s_%d"), sFld, ++Idx);
	return IndFld;
}


FileGDBAPI::FieldType SGOFieldType2GDBType(SGCore::SGOFieldType stype, int len, int nprec)
{
	switch (stype)
	{
	case SGCore::SGO_FT_SmallInt:
	case SGCore::SGO_FT_Boolean:
		return FileGDBAPI::fieldTypeSmallInteger;
	case SGCore::SGO_FT_Integer:
		return FileGDBAPI::fieldTypeInteger;
	case SGCore::SGO_FT_Single:
		return FileGDBAPI::fieldTypeSingle;
	case SGCore::SGO_FT_Double:
		return FileGDBAPI::fieldTypeDouble;
	case SGCore::SGO_FT_Currency:
	case SGCore::SGO_FT_Decimal:
	{
		if (nprec == 0)
		{
			if (len<=5)
				return FileGDBAPI::fieldTypeSmallInteger;
			return FileGDBAPI::fieldTypeInteger;
		}
		if (nprec>=4)
			return FileGDBAPI::fieldTypeDouble;
		else
			return FileGDBAPI::fieldTypeSingle;
	}
	case SGCore::SGO_FT_Date:
		return FileGDBAPI::fieldTypeDate;
	case SGCore::SGO_FT_IDispatch:
	case SGCore::SGO_FT_Variant:
	case SGCore::SGO_FT_IUnknown:
	case SGCore::SGO_FT_Binary:
		return FileGDBAPI::fieldTypeBlob;
	}
	if (stype & SGCore::SGO_FT_Array)
		return FileGDBAPI::fieldTypeBlob;

	return FileGDBAPI::fieldTypeString;
}

int Field2FieldDef(SGCore::IFieldPtr pFld, FileGDBAPI::FieldDef& fld)
{
	if (pFld == NULL)
		return -1;

	std::wstring wname;
	FileGDBAPI::FieldType ftype = FileGDBAPI::fieldTypeInteger;
	int nlen;
	fld.SetIsNullable(true);

	SGCore::IField2Ptr pFld2 = pFld;
	wname = pFld->GetName();
	nlen = pFld->GetLength();
	ftype = SGOFieldType2GDBType(pFld->GetType(), nlen, ((pFld2) ? pFld2->GetPrecision():0));
	fld.SetName(wname);
	fld.SetAlias(wname);
	fld.SetType(ftype);
	if (ftype == FileGDBAPI::fieldTypeSmallInteger)
		nlen = 2;
	else if (ftype == FileGDBAPI::fieldTypeInteger || ftype == FileGDBAPI::fieldTypeSingle)
		nlen = 4;
	else if (ftype == FileGDBAPI::fieldTypeDouble)
		nlen = 8;
	else if (ftype != FileGDBAPI::fieldTypeString)
		nlen = 0;

	fld.SetLength(nlen);
	return ftype;
}

// CGDBWorkspace
CGDBWorkspace::CGDBWorkspace() : m_FeatureEditable(true), m_pGeoDatabase(NULL), m_pColl(NULL),
	PersistXML::CPersistXMLImpl(0x02000000, &CLSID_GDBWorkspace)
{
}

CGDBWorkspace::~CGDBWorkspace()
{
	closeDB();
}

void CGDBWorkspace::LoadSaveXMLExt(DWORD version, SGGML::IXMLArchiveElement *pSource, BOOL bLoad)
{
	if (bLoad)
	{
		m_filePath = GetNodeText(FindSingleNode(pSource, L"Connection"));	//#5126
		//Connect();
	}
	else
	{
		SetNodeText(CreateElement(pSource, L"Connection"), _bstr_t(m_filePath));
		SetNodeText(CreateElement(pSource, L"GDB"), _bstr_t("FileGDB"));
	}
}

BOOL CGDBWorkspace::IsConnected()
{
	return (m_pGeoDatabase!=NULL);
}

void CGDBWorkspace::closeDB()
{
	if (m_pColl)
		m_pColl->Release();

	m_pColl = NULL;
	if (m_pGeoDatabase)
	{
		FileGDBAPI::CloseGeodatabase(*m_pGeoDatabase);
		delete m_pGeoDatabase;
	}
	m_pGeoDatabase = NULL;
}

STDMETHODIMP CGDBWorkspace::Open(BSTR FilePath, VARIANT_BOOL *Result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Result = VARIANT_FALSE;
	closeDB();
	m_filePath = FilePath;
	int nLen=m_filePath.GetLength();
	if (!(nLen >= 4 && m_filePath.Right(4).CompareNoCase(L".gdb") == 0))
		return S_FALSE;
	struct _stat64i32 st;
	if (_tstat(m_filePath.GetString(), &st) != 0 || !(st.st_mode & S_IFDIR))
		return S_FALSE;

	std::wstring wbuf = m_filePath;
	FileGDBAPI::Geodatabase* pGeoDatabase = new FileGDBAPI::Geodatabase;
	fgdbError hr= FileGDBAPI::OpenGeodatabase(wbuf, *pGeoDatabase);
	if (hr == S_OK)
	{
		*Result = VARIANT_TRUE;
		m_pGeoDatabase = pGeoDatabase;
		return S_OK;
	}
	TRACE("open err=%d(%x)\n", hr, hr);
	FileGDBAPI::CloseGeodatabase(*pGeoDatabase);
	delete pGeoDatabase;
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::Create(BSTR FilePath, VARIANT_BOOL *Result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*Result = VARIANT_FALSE;
	closeDB();
	m_filePath = FilePath;
	struct _stat64i32 st;
	if (_tstat(m_filePath.GetString(), &st)== 0)
		return S_FALSE;

	std::wstring wbuf = m_filePath;
	FileGDBAPI::Geodatabase* pGeoDatabase = new FileGDBAPI::Geodatabase;
	fgdbError hr = FileGDBAPI::CreateGeodatabase(wbuf, *pGeoDatabase);
	if (hr == S_OK)
	{
		*Result = VARIANT_TRUE;
		m_pGeoDatabase = pGeoDatabase;
		return S_OK;
	}
	TRACE("create err=%d(%x)\n", hr, hr);
	FileGDBAPI::CloseGeodatabase(*pGeoDatabase);
	delete pGeoDatabase;
	return S_FALSE;

}

STDMETHODIMP CGDBWorkspace::raw_SchemaNames(SGCore::INameCollection **Collection)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*Collection = NULL;
	if (!IsConnected())
		return S_FALSE;

	std::vector<std::wstring> tables;
	//std::vector<std::wstring> fs;
	//std::vector<std::wstring> fds;
	int n=0;
	if (m_pGeoDatabase->GetChildDatasets(L"\\", L"Table", tables) == S_OK)
		n = tables.size();
	tables.clear();
	if (m_pGeoDatabase->GetChildDatasets(L"\\", L"Feature Class", tables) == S_OK)
		n += tables.size();

	int m = 0;
	tables.clear();
	if (m_pGeoDatabase->GetChildDatasets(L"\\", L"Feature Dataset", tables) == S_OK)
		m = tables.size();

	if (m > 0 || n > 0)
	{
		CNameCollection *pColl = new CComObject<CNameCollection>;
		pColl->AddRef();
		if (n>0)
			pColl->Add(_T("\\"));
		if (m > 0)
		{
			for (int i = 0; i < m; i++)
				pColl->Add(tables[i].c_str());
		}
		pColl->QueryInterface(SGCore::IID_INameCollection, (void **)Collection);
		pColl->Release();
	}
	return S_OK;
}

STDMETHODIMP CGDBWorkspace::raw_TableNames(BSTR Schema, SGCore::INameCollection **Collection)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*Collection = NULL;
	if (!IsConnected())
		return S_FALSE;
	
	bool broot = false;
	std::wstring wschema = Schema;
	if (wschema.empty())
	{
		wschema = L"\\";
		broot = true;
	}
	else
	{
		if (wschema[0] != L'\\')
			wschema.insert(wschema.begin(), L'\\');
		else if (wschema.compare(L"\\") == 0)
			broot = true;
	}
	if (broot && m_pColl)
	{
		m_pColl->QueryInterface(SGCore::IID_INameCollection, (void **)Collection);
		return S_OK;
	}

	CNameCollection *pColl = new CComObject<CNameCollection>;
	pColl->AddRef();
	std::vector<std::wstring> tables;
	int n;
	if (m_pGeoDatabase->GetChildDatasets(wschema, L"Table", tables) == S_OK)
	{
		n = tables.size();
		for (int i = 0; i < n; i++)
			pColl->Add(tables[i].c_str());
	}
	tables.clear();
	if (m_pGeoDatabase->GetChildDatasets(wschema, L"Feature Class", tables) == S_OK)
	{
		n = tables.size();
		for (int i = 0; i < n; i++)
			pColl->Add(tables[i].c_str());
	}
	tables.clear();
	if (broot)
	{
		if (m_pGeoDatabase->GetChildDatasets(wschema, L"Feature Dataset", tables) == S_OK)
		{
			std::vector<std::wstring> tables2;
			int m = tables.size();
			for (int k = 0; k < m; k++)
			{
				tables2.clear();
				if (m_pGeoDatabase->GetChildDatasets(tables[k], L"Table", tables2) == S_OK)
				{
					n = tables2.size();
					for (int i = 0; i < n; i++)
						pColl->Add(tables2[i].c_str());
				}
				tables2.clear();
				if (m_pGeoDatabase->GetChildDatasets(tables[k], L"Feature Class", tables2) == S_OK)
				{
					n = tables2.size();
					for (int i = 0; i < n; i++)
						pColl->Add(tables2[i].c_str());
				}
			}
		}
	}
	pColl->QueryInterface(SGCore::IID_INameCollection, (void **)Collection);
	if (broot)
		m_pColl = pColl;
	else
		pColl->Release();

	return S_OK;
}

STDMETHODIMP CGDBWorkspace::raw_FieldGeometryType(BSTR Table, BSTR Field, SGCore::SGOGeometryType *Type)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	*Type = SGCore::SGO_GT_Unknown;
	if (!IsConnected())
		return S_FALSE;

	std::wstring wstrTablePath = Table;
	if (wstrTablePath[0] != L'\\')
		wstrTablePath.insert(wstrTablePath.begin(), L'\\');
	FileGDBAPI::Table table;
	if (m_pGeoDatabase->OpenTable(wstrTablePath, table) == S_OK)
	{
		std::vector<FileGDBAPI::FieldDef> fieldDefs;
		if (table.GetFields(fieldDefs) == S_OK)
		{
			FileGDBAPI::FieldDef* pFd;
			FileGDBAPI::FieldType ftype;
			int n = fieldDefs.size();
			for (int i = 0; i < n; i++)
			{
				pFd = &(fieldDefs.at(i));
				pFd->GetType(ftype);
				if (ftype == FileGDBAPI::fieldTypeGeometry)
				{
					FileGDBAPI::GeometryDef gd;
					if (pFd->GetGeometryDef(gd) == S_OK)
					{
						FileGDBAPI::GeometryType gtype= FileGDBAPI::geometryNull; //1,2(mp),3,4,9(mpatch)
						gd.GetGeometryType(gtype);
						if (gtype == FileGDBAPI::geometryPoint || gtype == FileGDBAPI::geometryMultipoint)
							*Type = SGCore::SGO_GT_Point;
						else if (gtype == FileGDBAPI::geometryPolyline)
							*Type = SGCore::SGO_GT_LineString;
						else if (gtype == FileGDBAPI::geometryPolygon || gtype == FileGDBAPI::geometryMultiPatch)
							*Type = SGCore::SGO_GT_Polygon;

						return S_OK;
					}
				}
			}
		}
	}
	return S_OK;
}


STDMETHODIMP CGDBWorkspace::raw_OpenTable(BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType, SGCore::IFeatureClass **pFeatureClass)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pFeatureClass = NULL;
	CGDBFeatureClass* pFC=new CComObject<CGDBFeatureClass>;
	pFC->AddRef();
	if (pFC->OpenTable(this, TableName, GeometryField, GeometryType))
		pFC->QueryInterface(SGCore::IID_IFeatureClass, (void**)pFeatureClass);

	pFC->Release();
	return S_OK;
}


STDMETHODIMP CGDBWorkspace::raw_CreateTable(BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType, SGCore::IFields *Fields, SGCore::IFeatureClass **pFeatureClass)
{
	return raw_CreateTable2(TableName, GeometryField, GeometryType, Fields, NULL, pFeatureClass);
}


STDMETHODIMP CGDBWorkspace::get_Name(BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_filePath.AllocSysString();
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::get_Authority(BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::get_IsValid(VARIANT_BOOL *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = (IsConnected()) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_FALSE;
}


STDMETHODIMP CGDBWorkspace::raw_TableFields(BSTR TableName, SGCore::IFields **Fields)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Fields = NULL;
	CFields *pFlds = CreateField(TableName, NULL);
	if (pFlds)
	{
		pFlds->QueryInterface(SGCore::IID_IFields, (void **)Fields);
		pFlds->Release();
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_DeleteTable(BSTR TableName)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pGeoDatabase)
	{
		std::wstring path = TableName;
		if (m_pGeoDatabase->Delete(path, L"Table") == S_OK)
		{
			if (m_pColl)
				m_pColl->Release();
			m_pColl = NULL;
			return S_OK;
		}
	}
	return S_FALSE;
}


STDMETHODIMP CGDBWorkspace::raw_ExecuteNonQueryCommand(BSTR Command)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_ExecuteScalarCommand(BSTR Command, VARIANT *result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	VariantInit(result);
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_ExecuteTableCommand(BSTR Command, SGCore::ITable **result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*result = NULL;
	return S_FALSE;
}


// IDBWorkspace3
STDMETHODIMP CGDBWorkspace::raw_CreateParcelInformation(VARIANT_BOOL *Result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Result = VARIANT_FALSE;
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_OpenParcel(BSTR Name, IUnknown **pUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pUnk = NULL;
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_CreateParcel(BSTR Name, IUnknown **pUnk)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pUnk = NULL;
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_DeleteParcel(BSTR Name)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_FALSE;
}

STDMETHODIMP CGDBWorkspace::raw_ParcelNames(BSTR Schema, SGCore::INameCollection **Collection)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Collection = NULL;
	return S_FALSE;
}


// IDBWorkspace4
STDMETHODIMP CGDBWorkspace::raw_CreateTable2(BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType, SGCore::IFields *Fields, IUnknown *SpatialReference, SGCore::IFeatureClass **pFeatureClass)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pFeatureClass = NULL;
	if (!IsConnected())
		return S_FALSE;
	if (!FeatureEditable)
		return S_FALSE;
	if (TableName == NULL)
		return GDB_E_INVALIDTABLENAME;
	if (Fields == NULL)
		return GDB_E_NOCOLUMNFOUND;

	long Cnt = Fields->FieldCount;
	if (Cnt <= 0)
		return GDB_E_NOCOLUMNFOUND;
	
	CString szTable = TableName;
	if (szTable[0] != L'\\')
		szTable.Insert(0, L'\\');

	SGCore::INameCollectionPtr pNC;
	raw_TableNames(L"", &pNC);
	//是否已存在
	if (m_pColl && m_pColl->Find(szTable)>=0)
	{
#ifdef _DEBUG
		CString lbuf;
		int li = 1;
		while (1)
		{
			lbuf.Format(L"%s%d",szTable, li);
			if (m_pColl->Find(lbuf) < 0)
				break;
			li++;
		}
		szTable = lbuf;
		TRACE("new name=%ws\n", szTable);
#else
		return GDB_E_NAMEISALREADYUSED;
#endif
	}

	SGCore::IFieldsPtr pFlds = Fields;
	std::vector<FileGDBAPI::FieldDef> flds;
	FileGDBAPI::FieldDef fld;
	fld.SetName(L"OBJECTID");
	fld.SetAlias(L"OBJECTID");
	fld.SetIsNullable(false);
	fld.SetType(FileGDBAPI::fieldTypeOID);
	fld.SetLength(4);
	flds.push_back(fld);

	CString szGeoFld = GeometryField;
	if (szGeoFld.IsEmpty())
		szGeoFld = L"Shape";

	std::wstring wname;
	int ftype;
	for (int i = 0; i < pFlds->FieldCount; i++)
	{
		ftype= Field2FieldDef(pFlds->GetField(i), fld);
		//if (stype == SGCore::SGO_FT_IDispatch || stype == SGCore::SGO_FT_IUnknown)
		if (ftype== FileGDBAPI::fieldTypeBlob)
		{
			fld.GetName(wname);
			if (_wcsicmp(szGeoFld, wname.c_str()) == 0 || _wcsicmp(L"geometry", wname.c_str()) == 0)
				continue;
		}
		if (ftype>=0)
			flds.push_back(fld);
	}
	if (GeometryType != SGCore::SGO_GT_Unknown)
	{
		FileGDBAPI::GeometryType geotype;
		if (GeometryType == SGCore::SGO_GT_LineString)
			geotype = FileGDBAPI::geometryPolyline;
		else if (GeometryType == SGCore::SGO_GT_Polygon)
			geotype = FileGDBAPI::geometryPolygon;
		else
			geotype = FileGDBAPI::geometryPoint;

		FileGDBAPI::GeometryDef geodef;
		geodef.SetGeometryType(geotype);
		geodef.SetHasZ(false);
		geodef.SetHasM(false);
		SGSFCOMS::ISpatialReferencePtr pSR=SpatialReference;
		if (pSR)
		{
			CString buf = pSR->GetWellKnownText();
			FileGDBAPI::SpatialReference sr;
			sr.SetSpatialReferenceText(buf.GetString());
			buf=pSR->GetAuthority().GetBSTR();
			if (buf.CompareNoCase(L"epsg")==0)
				sr.SetSpatialReferenceID(pSR->GetCode());
			geodef.SetSpatialReference(sr);
		}
		wname = szGeoFld;
		fld.SetName(wname);
		fld.SetAlias(wname);
		fld.SetIsNullable(false);
		fld.SetType(FileGDBAPI::fieldTypeGeometry);
		fld.SetLength(0);
		fld.SetGeometryDef(geodef);
		flds.push_back(fld);
	}
	{
		FileGDBAPI::Table tb;
		std::wstring wtable = szTable;
		if (m_pGeoDatabase->CreateTable(wtable, flds, L"", tb) != S_OK)
			return GDB_E_UNKNOWN;
	}
	CGDBFeatureClass* pFC = new CComObject<CGDBFeatureClass>;
	pFC->AddRef();
	if (pFC->OpenTable(this, _bstr_t(szTable), GeometryField, GeometryType))
	{
		pFC->QueryInterface(SGCore::IID_IFeatureClass, (void**)pFeatureClass);
		if (m_pColl)
			m_pColl->Release();
		m_pColl = NULL;
	}
	pFC->Release();
	return S_OK;
}


CFields* CGDBWorkspace::CreateField(LPCWSTR TableName, LPCWSTR GeomField, BOOL bIdent)
{
	if (m_pGeoDatabase == NULL)
		return NULL;

	std::wstring wzTable = TableName;
	if (wzTable[0] != L'\\')
		wzTable.insert(wzTable.begin(), L'\\');

	FileGDBAPI::Table table;
	if (m_pGeoDatabase->OpenTable(wzTable, table) != S_OK)
	{
		return NULL;
	}
	std::vector<FileGDBAPI::FieldDef> fieldDefs;
	fgdbError hr = table.GetFields(fieldDefs);
	if (hr == S_OK)
	{
		SGCore::SGOFieldType fldtype;
		FileGDBAPI::FieldDef* pFd;
		int fn = fieldDefs.size();
		CFields *pFlds = new CComObject<CFields>();
		pFlds->AddRef();
		pFlds->SetSourceCount(fn + 1);
		//SGCore::IField2Ptr pFIDFld;
		for (int i = 0; i < fn; i++)
		{
			std::wstring fname;
			int len = 0;
			int nPrec = 0;
			FileGDBAPI::FieldType ftype;
			pFd = &(fieldDefs.at(i));
			pFd->GetName(fname);
			pFd->GetType(ftype);
			pFd->GetLength(len);
			if (ftype == FileGDBAPI::fieldTypeGeometry)
			{
				FileGDBAPI::GeometryDef gd;
				if (pFd->GetGeometryDef(gd) == S_OK)
				{
					pFlds->SetGeomIndex(i);
					//m_GeomIndex = i;
				}
			}
			else
			{
				SGCore::IField2Ptr pFld;
				pFld.CreateInstance(SGDataAccess::CLSID_Field);
				pFld->PutName(fname.c_str());
				if (ftype == FileGDBAPI::fieldTypeOID)
				{
					pFlds->SetIdentity(i, pFld);
					//m_oidField = fname.c_str();
					//m_IdentIndex = i;
					fldtype = SGCore::SGOFieldType(SGCore::SGO_FT_Integer | SGCore::SGO_FT_Identity);
					len = 10;
				}
				else if (ftype == FileGDBAPI::fieldTypeSmallInteger)
				{
					fldtype = SGCore::SGO_FT_SmallInt;
					len = 5;
				}
				else if (ftype == FileGDBAPI::fieldTypeInteger)
				{
					fldtype = SGCore::SGO_FT_Integer;
					len = 9;
				}
				else if (ftype == FileGDBAPI::fieldTypeSingle)
				{
					fldtype = SGCore::SGO_FT_Single;
					len = 15;
					nPrec = 6;
				}
				else if (ftype == FileGDBAPI::fieldTypeDouble)
				{
					fldtype = SGCore::SGO_FT_Double;
					len = 19;
					nPrec = 10;
				}
				else if (ftype == FileGDBAPI::fieldTypeDate)
					fldtype = SGCore::SGO_FT_Date;
				else if (ftype == FileGDBAPI::fieldTypeBlob || ftype == FileGDBAPI::fieldTypeRaster)
					fldtype = SGCore::SGO_FT_Binary; // SGO_FT_Binary; // SGCore::SGO_FT_Variant;
													 //fieldTypeGUID, fieldTypeGlobalID                  
				else //if (ftype == fieldTypeString)
				{
					fldtype = SGCore::SGO_FT_BSTR;
					if (len < 1)
						len = (ftype == FileGDBAPI::fieldTypeGUID || ftype == FileGDBAPI::fieldTypeGlobalID) ? 36 : 50;
				}
				//m_fOrder.Add(i);
				pFld->put_Type(fldtype);
				pFld->put_Length(len);
				pFld->put_Precision(nPrec);
				if (ftype != FileGDBAPI::fieldTypeOID)
					pFlds->AddField(i, pFld);
			}
		}
		if (pFlds->m_GeomIndex >= 0)
			pFlds->m_Indexs[pFlds->FieldCount] = pFlds->m_GeomIndex;

		return pFlds;
	}
	return NULL;
}