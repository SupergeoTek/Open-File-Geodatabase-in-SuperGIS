// GDBFeatureClass.cpp : CGDBFeatureClass 的實作

#include "stdafx.h"
#include "GDBFeatureClass.h"
#include "GDBGeometry.h"
#include "../TProcess.h"

using namespace FileGDBAPI;

extern EncodingConv g_ec;
CString ConfirmFieldName(std::vector<CString> *FNs, LPCTSTR sFld, long nStartIndex);
int Field2FieldDef(SGCore::IFieldPtr pFld, FileGDBAPI::FieldDef& fld);
CString GDBFieldType2String(int ftype)
{
	if (ftype == FileGDBAPI::fieldTypeSmallInteger)
		return L"esriFieldTypeSmallInteger";
	if (ftype == FileGDBAPI::fieldTypeInteger)
		return L"esriFieldTypeInteger";
	if (ftype == FileGDBAPI::fieldTypeSingle)
		return L"esriFieldTypeSingle";

	if (ftype == FileGDBAPI::fieldTypeDouble)
		return L"esriFieldTypeDouble";

	if (ftype == FileGDBAPI::fieldTypeDate)
		return L"esriFieldTypeDate";
	if (ftype == FileGDBAPI::fieldTypeBlob)
		return L"esriFieldTypeBlob";
	if (ftype == fieldTypeOID)
		return L"esriFieldTypeOID";

	return L"esriFieldTypeString";
}


// CGDBFeatureClass
HRESULT WINAPI CGDBFeatureClass::InterfaceFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
	if (pv == NULL)
		return E_NOINTERFACE;

	CGDBFeatureClass* pCls = (CGDBFeatureClass*)pv;
	if (pCls == NULL)
		return E_NOINTERFACE;
	if (pCls->m_Fields == NULL) // || pCls->m_pRecordset == NULL)
		return E_NOINTERFACE;
	if (pCls->m_Fields->m_IdentIndex < 0)
		return E_NOINTERFACE;

	if (dw == 0)
		*ppv = (SGCore::IFeatureClassEdit*)pCls;
	else if (dw == 1)
		*ppv = (SGCore::ITableEdit*)pCls;
	else if (dw == 2)
		*ppv = (SGCore::IEditTask*)pCls;
	else if (dw == 3)
		*ppv = (SGCore::IFieldsAlter*)pCls;
	pCls->AddRef();
	return S_OK;
}

CGDBFeatureClass::CGDBFeatureClass() :m_pWorkspace(NULL), m_pTable(NULL), m_pRows(NULL), m_pCurRow(NULL),
		PersistXML::CPersistXMLImpl(0x02000000, &CLSID_GDBFeatureClass)
{
	m_Fields = NULL;
	m_GeometryType = SGCore::SGO_GT_Unknown;
	m_gtype = 0;
	m_bZ = false;
	m_bM = false;
	m_srid = -1;

	m_GeoFac.CreateInstance(SGSFCOMS::CLSID_GeometryFactory);
	m_maxfid = 1;
	m_EditStatus = SGCore::SGO_ETS_Default;
	m_editRef=0;

	m_rowN=0;

	m_nRecDirty=0;
	m_nGeomDirty=0;
	m_nValueDirty=0;	//value有更新
	m_bBulk = false;
	m_ftShape = NULL;
}

CGDBFeatureClass::~CGDBFeatureClass()
{
	closeGDBObject();
}

void CGDBFeatureClass::closeGDBObject()
{
	EndBulkLoad();
	ResetReading();
	//m_pTable->LoadOnlyMode(true);
	//m_pTable->SetWriteLock();
	if (m_Fields)
	{
		m_Fields->Release();
		m_Fields = NULL;
	}
	m_fnames.RemoveAll();
	m_ftypes.RemoveAll();
	//m_fOrder.RemoveAll();
	if (m_pTable)
	{
		//m_pTable->LoadOnlyMode(false);
		//m_pTable->FreeWriteLock();
		if (m_pWorkspace && m_pWorkspace->getGeoDatabase())
			m_pWorkspace->getGeoDatabase()->CloseTable(*m_pTable);
		delete m_pTable;
	}
	m_pTable = NULL;
	m_SpatialReference = NULL;
	m_TableName = L"";
	m_oidField = L"";
	m_GeometryField=L"";
	//m_IdentIndex = -1;
	//m_GeomIndex = -1;
	m_GeometryType = SGCore::SGO_GT_Unknown;
	m_gtype = 0;
	m_bZ = false;
	m_bM = false;
	m_srid = -1;
	if (m_pWorkspace)
		m_pWorkspace->Release();
	m_pWorkspace = NULL;
	m_fids2 = NULL;
	if (m_ftShape)
		delete m_ftShape;
	m_ftShape = NULL;
}

void CGDBFeatureClass::StartBulkLoad()
{
	if (!m_pTable || m_bBulk)
		return;

	m_bBulk = true;
	m_pTable->LoadOnlyMode(true);
	m_pTable->SetWriteLock();
}
void CGDBFeatureClass::EndBulkLoad()
{
	if (!m_pTable || !m_bBulk)
		return;

	m_bBulk = false;
	m_pTable->LoadOnlyMode(false);
	m_pTable->FreeWriteLock();
}


void CGDBFeatureClass::LoadSaveXMLExt(DWORD version,
	SGGML::IXMLArchiveElement *pSource, BOOL bLoad)
{
	if (bLoad)
	{
		CGDBWorkspace *sqlws = new CComObject<CGDBWorkspace>;
		sqlws->AddRef();
		sqlws->LoadSaveXMLExt(version, FindSingleNode(pSource, L"Workspace"), bLoad);

		_bstr_t TableName = GetNodeTextB(FindSingleNode(pSource, L"TableName"));
		_bstr_t GField = GetNodeTextB(FindSingleNode(pSource, L"GeometryField"));
		SGCore::SGOGeometryType GType = (SGCore::SGOGeometryType)GetInteger(GetNodeText(FindSingleNode(pSource, L"GeometryType")), SGCore::SGO_GT_Unknown);

		if (sqlws->IsConnected())
			OpenTable(sqlws, TableName, GField, GType);
		sqlws->Release();

		SGGML::IXMLArchiveElementPtr pElem = FindSingleNode(pSource, "AlternateSpatialReference");
		if (pElem)
		{
			SGSFCOMS::ISpatialReferenceFactoryPtr pSRF(SGSFCOMS::CLSID_SpatialReferenceFactory);
			m_SpatialReference = pSRF->CreateFromWKT(_bstr_t(GetNodeTextB(pElem)));
			m_bSpatialReferenceModified = true;
		}
	}
	else
	{
		if (m_pWorkspace)
			m_pWorkspace->LoadSaveXMLExt(version, CreateElement(pSource, L"Workspace"), bLoad);
		SetNodeText(CreateElement(pSource, L"TableName"), _bstr_t(m_TableName));
		SetNodeText(CreateElement(pSource, L"GeometryField"), _bstr_t(m_GeometryField));
		SetNodeText(CreateElement(pSource, L"GeometryType"), _bstr_t((long)m_GeometryType));

		if (m_bSpatialReferenceModified)
		{
			SGGML::IXMLArchiveElementPtr pElem = CreateElement(pSource, L"AlternateSpatialReference");
			if (m_SpatialReference)
				SetNodeText(pElem, m_SpatialReference->WellKnownText);
		}
	}
}

SGSFCOMS::IGeometryPtr CGDBFeatureClass::CreateGeometry(VARIANT *var, VARIANT* retWKB)
{
	SGSFCOMS::IGeometryPtr pGeom;
	if (V_VT(var) == VT_BSTR)
	{
		pGeom = m_GeoFac->CreateFromWKT(V_BSTR(var), m_SpatialReference);
		if (retWKB)
		{
			SGSFCOMS::IWksPtr pWks = pGeom;
			if (pWks)
			{
				pWks->raw_ExportToWKB(retWKB);
				return NULL;
			}
		}
		return pGeom;
	}
	if (retWKB)
	{
		VariantCopy(retWKB, var);
		return NULL;
	}
	m_GeoFac->raw_CreateFromWKB(*var, m_SpatialReference, &pGeom);
	return pGeom;
}

void CGDBFeatureClass::UpdateEnvelope(SGSFCOMS::IEnvelope *pEnv)
{
	if (pEnv == NULL)
		return;

	m_dMinX = (_isnan(m_dMinX) || m_dMinX > pEnv->minX) ? pEnv->minX : m_dMinX;
	m_dMaxX = (_isnan(m_dMaxX) || m_dMaxX < pEnv->maxX) ? pEnv->maxX : m_dMaxX;
	m_dMinY = (_isnan(m_dMinY) || m_dMinY > pEnv->minY) ? pEnv->minY : m_dMinY;
	m_dMaxY = (_isnan(m_dMaxY) || m_dMaxY < pEnv->maxY) ? pEnv->maxY : m_dMaxY;
	m_UpdateCount++;

	if (m_UpdateCount >= 10000)
	{
		m_UpdateCount = 0;
		//m_pWorkspace->UpdateLayer(m_TTableName, m_dMinX, m_dMaxX, m_dMinY, m_dMaxY);
	}
}

long CGDBFeatureClass::FindFieldIndex(VARIANT *pField)
{
	if (m_Fields != NULL)
		return m_Fields->FindFieldIndex(pField);
	return -1;
}

void CGDBFeatureClass::DeleteKey(long key)
{
	if (m_fids2)
	{
		VARIANT_BOOL bRet = VARIANT_FALSE;
		m_fids2->raw_UnLockKey(m_lockkey, &bRet);
		if (!bRet)	//若已被它方鎖定, 用此解鎖
		{
			m_fids2->raw_LockKey(m_lockkey, &bRet);
			m_fids2->raw_UnLockKey(m_lockkey, &bRet);
		}
		m_fids2->raw_Remove(key, &bRet);
		//if (bRet)
		//	m_fids2->raw_UpdateKeyOneByOne(key+1,-1, -1);	//是否要更動之後的key
		m_fids2->raw_LockKey(m_lockkey, &bRet);
	}
}
void CGDBFeatureClass::AddKey(long key, VARIANT fid)
{
	if (m_fids2)
	{
		if (key<0)
			key = m_fids2->GetCount(); //  m_pDataTable->Count-1;	//已update後, 是否這個也會更新? 若不會就不-1
		if (key >= 0)
		{
			VARIANT_BOOL bRet = VARIANT_FALSE;
			m_fids2->raw_UnLockKey(m_lockkey, &bRet);
			if (!bRet)	//若已被它方鎖定, 用此解鎖
			{
				m_fids2->raw_LockKey(m_lockkey, &bRet);
				m_fids2->raw_UnLockKey(m_lockkey, &bRet);
			}
			m_fids2->raw_Add(key, fid, NULL, &bRet);
			m_fids2->raw_LockKey(m_lockkey, &bRet);
		}
	}
}
void CGDBFeatureClass::UpdateKeyBoundary(long key, SGCore::tagSGDRect* pRect)
{
	if (m_fids2)
	{
		VARIANT_BOOL bRet = VARIANT_FALSE;
		m_fids2->raw_UnLockKey(m_lockkey, &bRet);
		if (!bRet)	//若已被它方鎖定, 用此解鎖
		{
			m_fids2->raw_LockKey(m_lockkey, &bRet);
			m_fids2->raw_UnLockKey(m_lockkey, &bRet);
		}
		m_fids2->SetBoundary(key, pRect);	//k
		m_fids2->raw_LockKey(m_lockkey, &bRet);
	}
}

bool CGDBFeatureClass::GetFeatureEditable()
{
	if (m_pWorkspace != NULL)
	{
		if (m_pWorkspace->FeatureEditable && m_Fields && m_pTable)		//ken , rem not editable yet.
		{
			bool b=false;
			m_pTable->IsEditable(b);
			return (m_Fields->m_IdentIndex >= 0 && b);
		}
	}
	return false;
}

void CGDBFeatureClass::ResetMappedFID()
{
	m_rowN = 0;
	int n = 0;
	long fidType = 0; //0:none, 1:long, 16:string
	{
		if (m_pTable)
			m_pTable->GetRowCount(n);

		m_maxfid = 1;
		m_rowN = n;
		if (n <= 0 || m_pTable==NULL)
		{
			m_fids2 = NULL;
			return;
		}
		m_maxfid = n + 1;

		if (m_fids2 == NULL)
			m_fids2.CreateInstance(SGDataAccess::CLSID_SGMappedFID);
		SGCore::IMappedFIDEditPtr fidse = m_fids2;
		if (m_lockkey == 0)
			m_lockkey = (long)(((double)rand() / RAND_MAX)*(INT_MAX - 99) + 99); //123987;	//可用rand取一個存起來,這樣每一featureclass都有各自的lockkey

		if (!fidse->UnLockKey(m_lockkey))	//若已被它方鎖定, 用此解鎖
		{
			fidse->LockKey(m_lockkey);
			fidse->UnLockKey(m_lockkey);
		}
		fidse->raw_Clear();
		fidse->put_Size(n);
		VARIANT_BOOL bRet = VARIANT_FALSE;

		VARIANT retFid;
		retFid.vt = VT_I4;

		fidType = (m_Fields->m_IdentIndex>=0) ? 1 : 0; //0:none, 1:long, 16:string
		SGCore::tagSGDRect fRect;
		memset(&fRect, 0, sizeof(SGCore::tagSGDRect));
		
		std::wstring wstrSubfields;
		if (m_Fields->m_IdentIndex >= 0)
			wstrSubfields = m_oidField;
		if (m_Fields->m_GeomIndex >= 0)
			wstrSubfields += L"," + m_GeometryField;

		int i = 0;
		EnumRows rows;
		fgdbError hr = m_pTable->Search(wstrSubfields, L"", true, rows);
		if (hr == S_OK)
		{
			FileGDBAPI::Row row;
			int32 oid;
			ShapeBuffer shapebuf;
			WKS::tagWKSEnvelope bound;
			memset(&bound, 0, sizeof(WKS::tagWKSEnvelope));
			VARIANT retVar;
			retVar.vt = VT_EMPTY;
			while (rows.Next(row) == S_OK)
			{
				row.GetOID(oid);
				retFid.lVal = oid;
				if (row.GetGeometry(shapebuf) == S_OK)
				{
					ShapeType stype;
					shapebuf.GetShapeType(stype);
					//shapebuf.HasZs(stype);
					//shapebuf.HasMs(stype);
					int bufLen = shapebuf.inUseLength;
					BYTE* pBuf = shapebuf.shapeBuffer;  //wkb? OGRCreateFromShapeBin()
					if (bufLen >= 4)
					{
						int nSHPType = pBuf[0];
						retVar = GDBGeometryToGeometry(pBuf, bufLen, &bound);
						if (retVar.vt==VT_EMPTY)
							memset(&fRect, 0, sizeof(SGCore::tagSGDRect));
						else
						{
							fRect.left = bound.xMin;
							fRect.right = bound.xMax;
							fRect.bottom = bound.yMin;
							fRect.top = bound.yMax;
							//TRACE("%d=%.4f %.4f, %.4f %.4f\n", i, bound.xMin, bound.yMax, bound.xMax, bound.yMin);
						}
						//VariantClear(&retVar);
					}
				}
				fidse->raw_Add(i, retFid, &fRect, &bRet);
				i++;
			}
			rows.Close();
		}
		if (m_rowN != i)
		{
			TRACE("*** resetmap count err=%d,%d\n", m_rowN, i);
		}
		m_rowN = i;

		fidse->SetFIDType(fidType);
		fidse->LockKey(m_lockkey);
		//m_curFi=-1;
		if (m_rowN>0 && m_Fields->m_GeomIndex>=0)
		{
			fidse->get_Boundary(-9999, &fRect);
			m_dMinX = fRect.left;
			m_dMaxX = fRect.right;
			m_dMinY = fRect.bottom;
			m_dMaxY = fRect.top;
			TRACE("resetmfid=%.4f %.4f, %.4f %.4f\n", m_dMinX, m_dMaxY, m_dMaxX, m_dMinY);
		}
	}
}

int CGDBFeatureClass::ResetReading()
{
	m_curFi = -1;
	m_curFid = -1;
	if (m_pCurRow)
		delete m_pCurRow;
	m_pCurRow = NULL;
	if (m_pRows)
	{
		m_pRows->Close();
		delete m_pRows;
	}
	m_pRows = NULL;
	return 0;
}

//傳回的是result offset, 另外傳入的row,若fid不符會跟著異動
int CGDBFeatureClass::FetchRowFeature(long& row, long fid, BOOL bSub)
{
	bool bStart = false;
	if ((m_curFi < 0) || (fid<0 && row < m_curFi))
	{
		ResetReading();
		std::wstring wstrSubfields = L"*";
		EnumRows* pRows = new EnumRows();
		if (m_pTable->Search(wstrSubfields, L"", true, *pRows) == S_OK)
		{
			m_pRows = pRows;
			m_pCurRow = new FileGDBAPI::Row();
		}
		else
			return -1;

		bStart = true;
	}
	if (row == m_curFi)
	{
		if (fid<0)
			return m_curFi;
		if (m_curFid == fid)
			return m_curFi;
	}
	while (row > m_curFi)
	{
		if (m_pRows->Next(*m_pCurRow) != S_OK)
		{
			m_curFi = -1;
			break;
		}
		m_curFi++;
		if (fid >= 0)
		{
			if (m_Fields->m_IdentIndex >= 0)
			{
				int32 oid;
				m_pCurRow->GetOID(oid);
				m_curFid = oid;
			}
			else
				m_curFid = m_curFi;

			if (fid == m_curFid)
			{
				row = m_curFi;
				return m_curFi;
			}
		}
	}
	m_curFi = -1; //最後找無
	return (bStart) ? m_curFi : FetchRowFeature(row, fid, TRUE);
}

int CGDBFeatureClass::GetRow(FileGDBAPI::EnumRows& enumRows, FileGDBAPI::Row& row, int nFID)
{
	if (m_pTable == NULL || nFID<0)
		return -1;

	CString buf;
	buf.Format(L"%s=%d", m_oidField, nFID);
	std::wstring szwh = buf.GetString();
	if (m_pTable->Search(L"*", szwh, true, enumRows) != S_OK)
		return -2;

	if (enumRows.Next(row) != S_OK)
		return -3;

	return 0;
}

int CGDBFeatureClass::VariantToGDBRowValue(FileGDBAPI::Row& row, int index, VARIANT* pVal)
{
	int fi = m_Fields->m_Indexs[index];
	if (fi == m_Fields->m_IdentIndex)
	{
	}
	else if (fi == m_Fields->m_GeomIndex)
	{
		m_ftShape->SetEmpty();
		//TRACE("SetEmpty, alln=%d, inuse=%d\n", m_ftShape->allocatedLength, m_ftShape->inUseLength);
		if (pVal->vt == VT_EMPTY || pVal->vt == VT_NULL)
		{
			row.SetNull(fi);
			return 0;
		}
		if ((pVal->vt & VT_ARRAY) != VT_ARRAY || pVal->parray==NULL)
			return -1;

		BYTE* pBuf=NULL;
		int nBuf= pVal->parray->cbElements*pVal->parray->rgsabound[0].cElements;
		SafeArrayAccessData(pVal->parray, (void**)&pBuf);
		if (m_gtype == FileGDBAPI::geometryMultiPatch)
		{
			if (GeometryToGDBMultiPatch(pBuf, nBuf, m_ftShape, m_bZ, m_bM) < 0)
			{
				SafeArrayUnaccessData(pVal->parray);
				return -1;
			}
		}
		else
		{
			if (GeometryToGDB(pBuf, nBuf, m_ftShape, m_bZ, m_bM) < 0)
			{
				SafeArrayUnaccessData(pVal->parray);
				return -1;
			}
		}
		SafeArrayUnaccessData(pVal->parray);
		int hr = row.SetGeometry(*m_ftShape);
		if (hr != S_OK)
			TRACE("err setgeom=%x, alln=%d, inuse=%d\n", hr, m_ftShape->allocatedLength, m_ftShape->inUseLength);
	}
	else
	{
		//std::wstring& wfield = m_fnames.GetAt(fi);
		if (pVal->vt == VT_EMPTY || pVal->vt == VT_NULL)
			row.SetNull(fi);
		else
		{
			FileGDBAPI::FieldType ftype= m_ftypes.GetAt(fi);
			if (ftype == fieldTypeInteger || ftype == fieldTypeSmallInteger)
			{
				int v = VariantToValue<int>(*pVal, 0);
				if (ftype == fieldTypeInteger)
					row.SetInteger(fi, v);
				else
				{
					if (v < -32768)
						v = -32768;
					else if (v > 32767)
						v = 32767;
					row.SetShort(fi, (short)v);
				}
			}
			else if (ftype == fieldTypeDouble)
				row.SetDouble(fi, VariantToValue<double>(*pVal, 0));
			else if (ftype == fieldTypeSingle)
				row.SetFloat(fi, VariantToValue<float>(*pVal, 0));
			else if (ftype == fieldTypeString)
			{
				VARIANT val2;
				val2.vt = VT_EMPTY;
				bool bChange = false;
				std::wstring wval;
				if (pVal->vt != VT_BSTR)
				{
					if (VariantChangeType(&val2, pVal, VARIANT_NOVALUEPROP, VT_BSTR) == S_OK)
					{
						pVal = &val2;
						bChange = true;
					}
				}
				if (pVal->vt == VT_BSTR)
					wval = pVal->bstrVal;

				row.SetString(fi, wval);
				if (bChange)
					VariantClear(pVal);
			}
			else if (ftype == fieldTypeDate)
			{
				tm tval;
				memset(&tval, 0, sizeof(tm));
				COleDateTime tdate(*pVal);
				if (tdate.GetStatus() != 0)
				{
					if (pVal->vt==VT_BSTR)
						tdate.ParseDateTime(CString(pVal->bstrVal));
				}
				if (tdate.GetStatus() == 0)
				{
					tval.tm_year = tdate.GetYear() - 1900;
					tval.tm_mon = tdate.GetMonth() - 1; // OGR months go 1-12, FGDB go 0-11 
					tval.tm_mday = tdate.GetDay();
					tval.tm_hour = tdate.GetHour();
					tval.tm_min = tdate.GetMinute();
					tval.tm_sec = tdate.GetSecond();
					row.SetDate(fi, tval);
				}
			}
			else if (ftype == fieldTypeBlob || ftype == fieldTypeRaster)
			{
				if ((pVal->vt & VT_ARRAY)==VT_ARRAY && pVal->parray)
				{
					int n=pVal->parray->cbElements*pVal->parray->rgsabound[0].cElements;
					m_ftByteArray.Allocate(n);
					BYTE* pBuf;
					SafeArrayAccessData(pVal->parray, (void**)&pBuf);
					memcpy(m_ftByteArray.byteArray, pBuf, n);
					SafeArrayUnaccessData(pVal->parray);
					m_ftByteArray.inUseLength = n;
					row.SetBinary(fi, m_ftByteArray);  //must hold
				}
			}
			else
			{
				VARIANT val2;
				val2.vt = VT_EMPTY;
				bool bChange = false;
				std::wstring wval;
				if (pVal->vt != VT_BSTR)
				{
					if (VariantChangeType(&val2, pVal, VARIANT_NOVALUEPROP, VT_BSTR) == S_OK)
					{
						pVal = &val2;
						bChange = true;
					}
				}
				if (pVal->vt == VT_BSTR)
					wval = pVal->bstrVal;

				if (ftype == fieldTypeGUID || ftype == fieldTypeGlobalID)
				{
					Guid guid;
					guid.FromString(wval);
					row.SetGUID(fi, guid);
				}
				else if (ftype == fieldTypeXML)
				{
					std::string sval = g_ec.ToChar(CP_UTF8, wval.data());
					row.SetXML(fi, sval);
				}
				if (bChange)
					VariantClear(pVal);
			}
		}
	}
	return 0;
}


long CGDBFeatureClass::InsertFeature(CArray<VARIANT>& vals, SGCore::tagSGDRect* pRect)
{
	FileGDBAPI::Row row;
	if (m_pTable==NULL || m_pTable->CreateRowObject(row) != S_OK)
		return -2;

	VARIANT val;
	int fn = m_Fields->GetFieldCount();
	for (int i = 0; i<fn; i++)
	{
		val = vals.GetAt(i);
		if (val.vt == VT_EMPTY || val.vt == VT_NULL)
			continue;

		VariantToGDBRowValue(row, i, &val);
	}
	if (m_Fields->m_GeomIndex>=0)
	{
		VariantToGDBRowValue(row, fn, &(vals.GetAt(fn)));
	}

	long fid = -1;
	if (m_pTable->Insert(row)==S_OK)
	{
		int32 oid = -1;
		if (row.GetOID(oid) == S_OK)
		{
			fid = oid;
			m_maxfid = oid;
		}
		else
		{
			fid = m_maxfid;
			m_maxfid++;
		}
	}
	//TRACE("insert fid=%d\n", fid);
	return fid;
}

long CGDBFeatureClass::UpdateFeature(long nFID, CArray<VARIANT>& vals, CArray<int>& fldSets, SGCore::tagSGDRect* pRect)
{
	if (m_pTable)
	{
		if (m_curFid != nFID)
		{
			long row = -1;
			if (FetchRowFeature(row, nFID) < 0)
				return -1;
		}
		int fn=m_Fields->GetFieldCount();
		for (int i = 0; i < fn; i++)
		{
			if (fldSets[i] == 0)
				continue;
			VariantToGDBRowValue(*m_pCurRow, i, &(vals.GetAt(i)));
		}

		if (m_Fields->m_GeomIndex >= 0)
		{
			if (fldSets[fn] != 0)
			{
				VariantToGDBRowValue(*m_pCurRow, fn, &(vals.GetAt(fn)));
			}
		}
		if (m_pCurRow && m_pTable->Update(*m_pCurRow) == S_OK)
			return 0;
	}
	return -1;
}

long CGDBFeatureClass::DeleteFeature(long nFID)
{
	if (m_pTable)
	{
		if (m_curFid != nFID)
		{
			long row = -1;
			if (FetchRowFeature(row, nFID) < 0)
				return -1;
		}
		if (m_pCurRow && m_pTable->Delete(*m_pCurRow) == S_OK)
		{
			m_curFid = -1;
			return 0;
		}
	}
	return -1;
}

int CGDBFeatureClass::ResetFieldsQuery(bool bOpenTable)
{
	fgdbError hr = FGDB_E_TABLE_NOT_FOUND;
	if (m_pTable)
	{
		ResetReading();
		if (m_Fields)
		{
			m_Fields->Release();
			m_Fields = NULL;
		}
		m_fnames.RemoveAll();
		m_ftypes.RemoveAll();

		std::vector<FieldDef> fieldDefs;
		hr = m_pTable->GetFields(fieldDefs);
		if (hr == S_OK)
		{
			SGCore::SGOFieldType fldtype;
			FieldDef* pFd;
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
				FieldType ftype;
				pFd = &(fieldDefs.at(i));
				pFd->GetName(fname);
				pFd->GetType(ftype);
				pFd->GetLength(len);
				m_ftypes.Add(ftype);
				m_fnames.Add(fname);
				TRACE("f%d=%s, len=%d, ftype=%d\n", i, g_ec.ToChar(CP_UTF8, fname.c_str()), len, ftype);
				if (ftype == fieldTypeGeometry)
				{
					m_GeometryField = fname.c_str();
					GeometryDef gd;
					if (pFd->GetGeometryDef(gd) == S_OK)
					{
						pFlds->SetGeomIndex(i);
						//m_GeomIndex = i;
						if (bOpenTable)
						{
							FileGDBAPI::GeometryType gtype; //1,2(mp),3,4,9(mpatch)
							FileGDBAPI::SpatialReference sr;
							gd.GetGeometryType(gtype);
							gd.GetHasZ(m_bZ);
							gd.GetHasM(m_bM);
							if (gd.GetSpatialReference(sr) == S_OK)
							{
								sr.GetSpatialReferenceID(m_srid);
								std::wstring szsr;
								if (sr.GetSpatialReferenceText(szsr) == S_OK)
								{
									if (szsr.length() > 3)
									{
										SGSFCOMS::ISpatialReferenceFactoryPtr pSRF(SGSFCOMS::CLSID_SpatialReferenceFactory);
										m_SpatialReference = pSRF->CreateFromWKT(_bstr_t(szsr.c_str()));
										if (m_SpatialReference)
										{
											m_bSpatialReferenceModified = true;
										}
									}
								}
							}
							TRACE("geom=%d, Z=%d,M=%d\n", gtype, m_bZ, m_bM);
							m_gtype = gtype;
							if (gtype == FileGDBAPI::geometryPoint || gtype == FileGDBAPI::geometryMultipoint)
								m_GeometryType = SGCore::SGO_GT_Point;
							else if (gtype == FileGDBAPI::geometryPolyline)
								m_GeometryType = SGCore::SGO_GT_LineString;
							else if (gtype == FileGDBAPI::geometryPolygon || gtype == FileGDBAPI::geometryMultiPatch)
								m_GeometryType = SGCore::SGO_GT_Polygon;

							m_ftShape = new FileGDBAPI::ShapeBuffer();
							//if (gtype == FileGDBAPI::geometryPoint)
							//	m_ftShape = new FileGDBAPI::PointShapeBuffer();
							//else if (gtype == FileGDBAPI::geometryMultipoint)
							//	m_ftShape = new FileGDBAPI::MultiPointShapeBuffer();
							//else if (gtype == FileGDBAPI::geometryMultiPatch)
							//	m_ftShape = new FileGDBAPI::MultiPatchShapeBuffer();
							//else
							//	m_ftShape = new FileGDBAPI::MultiPartShapeBuffer();
						}
					}
				}
				else
				{
					SGCore::IField2Ptr pFld;
					pFld.CreateInstance(SGDataAccess::CLSID_Field);
					pFld->PutName(fname.c_str());
					if (ftype == fieldTypeOID)
					{
						pFlds->SetIdentity(i, pFld);
						//pFIDFld = pFld;
						m_oidField = fname.c_str();
						//m_IdentIndex = i;
						fldtype = SGCore::SGOFieldType(SGCore::SGO_FT_Integer | SGCore::SGO_FT_Identity);
						len = 10;
					}
					else if (ftype == fieldTypeSmallInteger)
					{
						fldtype = SGCore::SGO_FT_SmallInt;
						len = 5;
					}
					else if (ftype == fieldTypeInteger)
					{
						fldtype = SGCore::SGO_FT_Integer;
						len = 9;
					}
					else if (ftype == fieldTypeSingle)
					{
						fldtype = SGCore::SGO_FT_Single;
						len = 15;
						nPrec = 6;
					}
					else if (ftype == fieldTypeDouble)
					{
						fldtype = SGCore::SGO_FT_Double;
						len = 19;
						nPrec = 10;
					}
					else if (ftype == fieldTypeDate)
					{
						fldtype = SGCore::SGO_FT_Date;
						if (len <= 8)
							len = 20;
					}
					else if (ftype == fieldTypeBlob || ftype == fieldTypeRaster)
						fldtype = SGCore::SGO_FT_Binary; // SGO_FT_Binary; // SGCore::SGO_FT_Variant;
														 //fieldTypeGUID, fieldTypeGlobalID                  
					else //if (ftype == fieldTypeString)
					{
						fldtype = SGCore::SGO_FT_BSTR;
						if (len < 1)
							len = (ftype == fieldTypeGUID || ftype == fieldTypeGlobalID) ? 36 : 50;
					}
					//m_fOrder.Add(i);
					pFld->put_Type(fldtype);
					pFld->put_Length(len);
					pFld->put_Precision(nPrec);
					if (ftype != fieldTypeOID)
						pFlds->AddField(i, pFld);
				}
			}
			if (pFlds->m_GeomIndex >= 0)
				pFlds->m_Indexs[pFlds->FieldCount] = pFlds->m_GeomIndex;

			//m_fOrder.Add(m_GeomIndex);
			m_Fields = pFlds;
		}
	}
	return hr;
}


BOOL CGDBFeatureClass::OpenTable(CGDBWorkspace *pWS, BSTR TableName, BSTR GeometryField, SGCore::SGOGeometryType GeometryType)
{
	closeGDBObject();
	if (pWS == NULL)
		return FALSE;
	if (!pWS->IsConnected())
		return FALSE;

	//TRACE("wkbg=%d,pt=%d,line=%d(%d),gon=%d,gc=%d\n", sizeof(WKBGeometry), sizeof(WKBPoint), sizeof(WKBLineString), sizeof(WKS::WKSLinearRing), sizeof(WKBPolygon), sizeof(WKBGeometryCollection));

	m_TableName = TableName;
	std::wstring wzTable= m_TableName;
	if (wzTable[0] != L'\\')
		wzTable.insert(wzTable.begin(), L'\\');
	else
		m_TableName.Delete(0);

	Table* pTable = new Table;
	if (pWS->getGeoDatabase()->OpenTable(wzTable, *pTable) != S_OK)
	{
		delete pTable;
		return FALSE;
	}
	m_pWorkspace = pWS;
	m_pWorkspace->AddRef();
	m_pTable = pTable;

	Envelope bound;
	if (pTable->GetExtent(bound) == S_OK)
	{
		TRACE("ext=%.6f,%.6f %.6f,%.6f\n", bound.xMin, bound.yMax, bound.xMax, bound.yMin);
		m_dMinX = bound.xMin;
		m_dMaxX = bound.xMax;
		m_dMinY = bound.yMin;
		m_dMaxY = bound.yMax;
	}
	fgdbError hr = ResetFieldsQuery(true);
	//if (hr == S_OK)
	{
		m_curFi = -1;
		m_curFid = -1;
		ResetMappedFID();	//k
		m_RowIndex = 0;
		m_nRecDirty = 0;
		m_nGeomDirty = 0;
		m_nValueDirty = 0;
	}
	return TRUE;
}


// SGCore::ITable
STDMETHODIMP CGDBFeatureClass::get_RowCount(long *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	int rn = 0;
	if (m_pTable)
		m_pTable->GetRowCount(rn);

	m_rowN = rn;
	*pVal = m_rowN;
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::get_Row(long Index, SGCore::ITableRow **pRow)
{
	*pRow = NULL;
	if ((Index >= 0) && (Index < m_rowN))
	{
		CComObject< CGDBFeature > *pTRow = new CComObject< CGDBFeature >;
		pTRow->AddRef();
		pTRow->InitialFeature(this);	//ken , TRUE

		pTRow->SetupRecord(Index);
		pTRow->QueryInterface(SGCore::IID_ITableRow, (void **)pRow);
		pTRow->Release();
	}
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::raw_Search(SGCore::IFilter *Filter, SGCore::ITableCursor **pCursor)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pCursor = NULL;
	if (Filter != NULL)
	{
		VARIANT_BOOL result;
		if (FAILED(Filter->raw_Initial(this, &result)) || (result == FALSE))
			return S_OK;
	}
	CComObject<CGDBFeatureCursor> *pTCursor = new CComObject<CGDBFeatureCursor>();
	pTCursor->AddRef();

	if (pTCursor->InitialCursor(this, Filter, NULL))
		pTCursor->QueryInterface(SGCore::IID_ITableCursor, (void **)pCursor);
	pTCursor->Release();
	return S_OK;
}

// SGCore::ITableEdit
STDMETHODIMP CGDBFeatureClass::raw_CreateRow(SGCore::ITableRowEdit **Row)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Row = NULL;
	return S_FALSE;
}


// IFeatureClass
STDMETHODIMP CGDBFeatureClass::raw_CreateFeatureDraw(SGCore::IFeatureDraw **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = NULL;
	return S_FALSE;
}

STDMETHODIMP CGDBFeatureClass::get_FeatureType(SGCore::SGOGeometryType *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_GeometryType;
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::get_Fields(SGCore::IFields **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = NULL;
	if (m_Fields)
		m_Fields->QueryInterface(SGCore::IID_IFields, (void**)pVal);
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::raw_Search(SGCore::IFeatureFilter* Filter, SGCore::IFeatureCursor **FeatureCursor)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*FeatureCursor = NULL;
	if (Filter != NULL)
	{
		VARIANT_BOOL result;
		if (FAILED(Filter->raw_Initial(this, &result)) || (result == FALSE))
			return S_OK;
	}
	CComObject<CGDBFeatureCursor> *pCursor = new CComObject<CGDBFeatureCursor>();
	pCursor->AddRef();

	if (pCursor->InitialCursor(this, NULL, Filter))
		pCursor->QueryInterface(SGCore::IID_IFeatureCursor, (void **)FeatureCursor);
	pCursor->Release();
	return S_OK;
}


// IFeatureClassEdit
STDMETHODIMP CGDBFeatureClass::raw_CreateFeature(SGCore::IFeatureEdit **Feature)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Feature = NULL;
	CGDBFeature *pFeature = new CComObject< CGDBFeature >;
	pFeature->AddRef();
	pFeature->InitialFeature(this, TRUE);
	pFeature->SetupRecord(-1);
	pFeature->QueryInterface(SGCore::IID_IFeatureEdit, (void **)Feature);
	pFeature->Release();

	return S_OK;
}

// IEditTask
STDMETHODIMP CGDBFeatureClass::raw_BeginEdit(VARIANT_BOOL *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = VARIANT_FALSE;
	if (!GetFeatureEditable() || m_Fields==NULL)
		return S_FALSE;
	*pVal =(m_Fields->m_IdentIndex>=0) ? VARIANT_TRUE : VARIANT_FALSE;
	m_editRef++;
	m_EditStatus = SGCore::SGO_ETS_Editing;
	StartBulkLoad();
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::raw_EndEdit()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (!GetFeatureEditable())
		return S_FALSE;

	m_editRef--;
	if (m_editRef <= 0)
	{
		EndBulkLoad();
		BOOL bGeom = (m_nGeomDirty>0);
		if (m_nRecDirty>0)
		{
			ResetMappedFID();	//k
		}
		//}
		if (bGeom || m_nRecDirty>0)
		{
			//if (m_pWorkspace != NULL)
			//	m_pWorkspace->UpdateLayer(m_TTableName, m_dMinX, m_dMaxX, m_dMinY, m_dMaxY);
		}
		m_EditStatus = SGCore::SGO_ETS_Default;
		m_nRecDirty = 0;
		m_nGeomDirty = 0;
		m_nValueDirty = 0;
		m_editRef = 0;
	}
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::get_Status(SGCore::SGOEditTaskStatus *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = m_EditStatus;
	return S_FALSE;
}


STDMETHODIMP CGDBFeatureClass::raw_AddField(SGCore::IField* newField)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pTable && newField)
	{
		SGCore::IFieldPtr pFld2 = newField;
		FileGDBAPI::FieldDef fdef;
		int ftype = Field2FieldDef(pFld2, fdef);
		if (ftype >= 0)
		{
			if (ftype == FileGDBAPI::fieldTypeBlob)
			{
				if (!m_GeometryField.IsEmpty() && m_GeometryField.CompareNoCase(pFld2->Name) == 0)
					return S_FALSE;
			}
			std::vector<CString> FNs;
			if (!m_oidField.IsEmpty())
				FNs.push_back(m_oidField);

			if (!m_GeometryField.IsEmpty())
				FNs.push_back(m_GeometryField);

			long Cnt = m_Fields->FieldCount;
			for (long i = 0; i < Cnt; i++)
			{
				SGCore::IFieldPtr pFld = m_Fields->Field[i];
				FNs.push_back((LPCTSTR)pFld->Name);
			}
			_bstr_t orgname = newField->Name;
			CString FN = ConfirmFieldName(&FNs, orgname, 0);
			fdef.SetName(FN.GetBuffer());
			if (m_pTable->AddField(fdef) == S_OK)
			{
				ResetFieldsQuery();
				return S_OK;
			}
		}
	}
	return S_FALSE;
}

STDMETHODIMP CGDBFeatureClass::raw_DropField(VARIANT Field)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pTable)
	{
		long nFld = FindFieldIndex(&Field);
		if (nFld < 0)
			return S_FALSE; //E_INVALIDARG;
		SGCore::IFieldPtr pFld = m_Fields->GetField(nFld);
		CString fldName = pFld->Name;
		if (!m_GeometryField.IsEmpty() && fldName.CompareNoCase(m_GeometryField) == 0)
			return S_FALSE;
		
		if (!m_oidField.IsEmpty() && fldName.CompareNoCase(m_oidField) == 0)
			return S_FALSE;

		std::wstring wname=fldName;
		if (m_pTable->DeleteField(wname) == S_OK)
		{
			ResetFieldsQuery();
			return S_OK;
		}
	}
	return S_FALSE;
}

STDMETHODIMP CGDBFeatureClass::raw_ModifyField(VARIANT Field, SGCore::IField* newField)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_pTable)
	{
		long nFld = FindFieldIndex(&Field);
		if (nFld < 0)
			return S_FALSE; //E_INVALIDARG;

		SGCore::IFieldPtr pFld = m_Fields->GetField(nFld);
		SGCore::IField2Ptr pFld2 = newField;
		if (pFld2->Name != pFld->Name)
		{
			CString fldName = pFld->Name;
			if (!m_GeometryField.IsEmpty() && fldName.CompareNoCase(m_GeometryField) == 0)
				return S_FALSE;
			if (!m_oidField.IsEmpty() && fldName.CompareNoCase(m_oidField) == 0)
				return S_FALSE;

			FileGDBAPI::FieldDef fdef;
			int ftype = Field2FieldDef(pFld2, fdef);
			if (ftype >= 0)
			{
				int nlen=0;
				fdef.GetLength(nlen);
				CString szXml=L"<esri:Field xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:esri=\"http://www.esri.com/schemas/ArcGIS/10.1\" xsi:type=\"esri:Field\">";
				CString lbuf;
				lbuf.Format(L"<Name>%s</Name>", fldName);
				szXml += lbuf;
				lbuf.Format(L"<Type>%s</Type><IsNullable>true</IsNullable>", GDBFieldType2String(ftype));
				szXml += lbuf;
				lbuf.Format(L"<Length>%d</Length><Precision>0</Precision><Scale>0</Scale>", nlen);
				szXml += lbuf;
				lbuf.Format(L"<AliasName>%s</AliasName>", fldName);
				szXml += lbuf;
				lbuf.Format(L"<ModelName>%s</ModelName>", fldName);
				szXml += lbuf;
				szXml += L"</esri:Field>";
				//ConfirmFieldName();
				std::string fdef=g_ec.ToChar(CP_UTF8,lbuf.GetBuffer());
				if (m_pTable->AlterField(fdef) == S_OK)
				{
					ResetFieldsQuery();
					return S_OK;
				}
			}
		}
	}
	return S_FALSE;
}


// IAttachment
STDMETHODIMP CGDBFeatureClass::get_Attachment(BSTR Name, VARIANT* pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_FALSE;
}

STDMETHODIMP CGDBFeatureClass::put_Attachment(BSTR Name, VARIANT newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return S_FALSE;
}


// IGDBClass
STDMETHODIMP CGDBFeatureClass::raw_Connect(BSTR ConnectionString, VARIANT_BOOL *Result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Result = VARIANT_FALSE;
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::get_ConnectionString(BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString path;
	if (m_pWorkspace)
	{
		path = m_pWorkspace->getFilePath();
		path += L"\\"+ m_TableName;
		*pVal = _bstr_t(path);
	}
	*pVal = (_bstr_t)path;
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::get_ContentType(BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CString type = (m_Fields && m_Fields->m_GeomIndex>=0) ? _T("FileGDB Feature Class") : _T("FileGDB Table");
	if (m_Fields && m_Fields->m_GeomIndex >= 0)
	{
		CString strZM=L"";
		CString buf;
		if (m_bZ)
			strZM = L"Z";
		if (m_bM)
			strZM += L"M";

		if (m_GeometryType == SGCore::SGO_GT_Point)
			buf.Format(_T(" (Point%s)"), strZM);
		else if (m_GeometryType == SGCore::SGO_GT_LineString)
			buf.Format(_T(" (Line%s)"), strZM);
		else if (m_GeometryType == SGCore::SGO_GT_Polygon)
			buf.Format(_T(" (Polygon%s)"), strZM);
		else
			buf = _T(" (Unknown)");

		type += buf;
	}
	*pVal = _bstr_t(type);
	return S_OK;
}

STDMETHODIMP CGDBFeatureClass::get_IsValid(VARIANT_BOOL *Result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*Result = (m_pTable) ? VARIANT_TRUE : VARIANT_FALSE;
	return S_OK;
}


//IFeatureClass2		//k
STDMETHODIMP CGDBFeatureClass::get_MappedFID(SGCore::IMappedFID** ppVal)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (m_fids2)
		m_fids2->QueryInterface(SGCore::IID_IMappedFID, (void**)ppVal);
	else
		*ppVal = NULL;

	return S_OK;
}


/////////////////////////////////////////////////
// CGDBFeature
CGDBFeature::CGDBFeature()
{
	m_pClass = NULL;
	m_bCreate = FALSE;
	m_GeometryFieldFormat = 0;
	m_RowIndex = -1;
	//m_Envelope.CreateInstance(SGSFCOMS::CLSID_Envelope);
	m_nGeomDirty = 0;

	m_fid = -1;
	//m_poFeature=NULL;
	m_pVarGeom = NULL;
	//VariantInit(&m_varGeom);
}

CGDBFeature::~CGDBFeature()
{
	//VariantClear(&m_varGeom);
	ClearValues();
	//if (m_poFeature)
	//	OGRFeature::DestroyFeature( m_poFeature );
	if (m_pClass)
		m_pClass->Release();
	m_pClass = NULL;
}

void CGDBFeature::ClearValues()
{
	int vn = m_varVals.GetCount();
	if (vn>0)
	{
		VARIANT* pData = m_varVals.GetData();
		for (int vi = 0; vi < vn; vi++, pData++)
			VariantClear(pData);
		m_varVals.RemoveAll();
	}
	vn = m_varOrgVals.GetCount();
	if (vn>0)
	{
		VARIANT* pData = m_varOrgVals.GetData();
		for (int vi = 0; vi<vn; vi++, pData++)
			VariantClear(pData);
		m_varOrgVals.RemoveAll();
	}
	m_fset.RemoveAll();
}

BOOL CGDBFeature::InitialFeature(CGDBFeatureClass *pClass, BOOL isNew)
{
	if (pClass == NULL)
		return FALSE;

	m_pClass = pClass;
	m_pClass->AddRef();
	//m_bCreate = isNew;	//ken

	//m_GeometryFieldFormat = m_pClass->GeometryFieldFormat;
	m_nGeomDirty = 0;
	return TRUE;
}

//offi指的是?, rowindex是實際row位
int CGDBFeature::RecordToFeature(void* hResult, int offi, int RowIndex)
{
	Row* pRow = (Row*)hResult;
	//m_RowIndex = RowIndex;
	//m_fid=-1;
	int fid = -1;
	//VariantClear(&m_varGeom);
	m_pVarGeom = NULL;
	ClearValues();
	int iFID = m_pClass->m_Fields->m_IdentIndex;
	int iGeom = m_pClass->m_Fields->m_GeomIndex;
	int fn = m_pClass->m_Fields->GetFieldCount();
	//if (iFID >= 0)
	//	fn++;
	//if (iGeom >= 0)
	//	fn++;

	if (iGeom >= 0)
	{
		m_varVals.SetSize(fn+1);
		m_pVarGeom = &(m_varVals.GetAt(fn));
	}
	else
		m_varVals.SetSize(fn);

	if (offi<0 && RowIndex<0)	//for new feature
	{
		return fid;
	}
	if (hResult == NULL)
		return -1;

	if (iFID>=0)
		pRow->GetOID(fid);

	//if (fid == 3886 || fid == 7330 || fid == 7644)
	//	TRACE("debug fid=%d,%d\n", fid, RowIndex);
	FileGDBAPI::FieldType ftype;
	bool isNull = false;
	VARIANT* pVar;
	int fi = 0;
	for (int i = 0; i < fn; i++)
	{
		fi=m_pClass->m_Fields->m_Indexs[i];
		pVar = &(m_varVals.GetAt(i));
		if (fi == iFID)
		{
			//pRow->GetOID(fid);
			//pVar->vt = VT_I4;
			//pVar->lVal = fid;
			TRACE("RecordToFeature ifid err=%d,%d\n", i, fi);
		}
		else if (fi == iGeom)
		{
			TRACE("RecordToFeature igeom err=%d,%d\n",i, fi);
		}
		else
		{
			//std::wstring& wstrFieldName = m_pClass->m_fnames.GetAt(fi);
			if (pRow->IsNull(fi, isNull) != S_OK || isNull)
				continue;

			ftype=m_pClass->m_ftypes.GetAt(fi);
			if (ftype== fieldTypeString)
			{
				pVar->vt = VT_BSTR;
				std::wstring val;
				pRow->GetString(fi, val);
				if (val.empty())
					pVar->bstrVal = SysAllocString(L"");
				else
					pVar->bstrVal = SysAllocString(val.c_str());
			}
			else if (ftype== fieldTypeInteger)
			{
				int32 val=0;
				if (pRow->GetInteger(fi, val) != S_OK)
				{
					int16 shortval;
					if (pRow->GetShort(fi, shortval)==S_OK)
						val = shortval;
				}
				pVar->vt = VT_I4;
				pVar->lVal = val;
			}
			else if (ftype == fieldTypeSmallInteger)
			{
				int16 shortval=0;
				pRow->GetShort(fi, shortval);
				pVar->vt = VT_I4;
				pVar->lVal = shortval;
			}
			else if (ftype == fieldTypeSingle)
			{
				pVar->vt = VT_R4;
				pRow->GetFloat(fi, pVar->fltVal);
			}
			else if (ftype == fieldTypeDouble)
			{
				pVar->vt = VT_R8;
				pRow->GetDouble(fi, pVar->dblVal);
			}
			else if (ftype== fieldTypeBlob || ftype==fieldTypeRaster)
			{
				ByteArray binaryBuf;
				if (pRow->GetBinary(fi, binaryBuf) == S_OK)
				{
					if (binaryBuf.inUseLength > 0)
					{
						TRACE("  (%d)blob=%d\n", RowIndex, binaryBuf.inUseLength);
						pVar->vt = VT_ARRAY | VT_UI1;
						pVar->parray = SafeArrayCreateVector(VT_UI1, 0, binaryBuf.inUseLength);
						BYTE* pBuf;
						SafeArrayAccessData(pVar->parray, (void**)&pBuf);
						memcpy(pBuf, (BYTE*)binaryBuf.byteArray, binaryBuf.inUseLength);
						SafeArrayUnaccessData(pVar->parray);
					}
				}
			}
			else if (ftype == fieldTypeDate)
			{
				struct tm val;
				if (pRow->GetDate(fi, val) == S_OK)
				{
					pVar->vt = VT_DATE;
					pVar->date = COleDateTime(val.tm_year + 1900, val.tm_mon + 1,
						val.tm_mday, val.tm_hour, val.tm_min, val.tm_sec).m_dt;
				}
			}
			else 
			{
				std::wstring val;
				if (ftype== fieldTypeGUID)
				{
					Guid guid;
					if (pRow->GetGUID(fi, guid) == S_OK)
						guid.ToString(val);
				}
				else if (ftype == fieldTypeGlobalID)
				{
					Guid guid;
					if (pRow->GetGlobalID(guid) == S_OK)
						guid.ToString(val);
				}
				else if (ftype == fieldTypeXML)
				{
					std::string strValue;
					if (pRow->GetXML(fi, strValue) == S_OK)
						val = g_ec.ToWChar(CP_UTF8, strValue.c_str());
				}
				pVar->vt = VT_BSTR;
				if (val.empty())
					pVar->bstrVal = SysAllocString(L"");
				else
					pVar->bstrVal = SysAllocString(val.c_str());
			}
		}
	}
	if (iGeom >= 0)
	{
		//m_pVarGeom = pVar;
		ShapeBuffer shapebuf;
		if (pRow->GetGeometry(shapebuf) == S_OK)
		{
			//ShapeType stype;
			//shapebuf.GetShapeType(stype);
			//shapebuf.HasZs(stype);
			//shapebuf.HasMs(stype);
			BYTE* pBuf = shapebuf.shapeBuffer;  //wkb? OGRCreateFromShapeBin()
			if (shapebuf.inUseLength >= 4)
			{
				//TRACE("gdbgeom=%d,%d\n",offi, RowIndex);
				*m_pVarGeom = GDBGeometryToGeometry(pBuf, shapebuf.inUseLength, NULL);
			}
		}
	}
	return fid;
}

/* must edit
CGDBFeatureCursor::get_FeatureCount
CGDBFeatureCursor::raw_Seek
*/

BOOL CGDBFeature::SetupRecord(long RowIndex)
{
	m_RowIndex = RowIndex;	//注意 -1為new feature
							//m_pDataRow2 = NULL;
							//m_pGeometry = NULL;
	m_Envelope = NULL;
	m_nGeomDirty = 0;

	m_fid = -1;
	if (m_RowIndex >= 0)
	{
		if (m_pClass->m_fids2)
		{
			VARIANT v;
			v.vt = VT_EMPTY;
			m_pClass->m_fids2->get_FIDByKey(m_RowIndex, &v);
			if (v.vt == VT_EMPTY)
				return FALSE;
			m_fid = v.lVal;
		}
		int offi = m_pClass->FetchRowFeature(m_RowIndex, m_fid);
		if (offi >= 0)
		{
			long fid = RecordToFeature(m_pClass->m_pCurRow, offi, m_RowIndex);
			if (fid == m_fid)
				return TRUE;
			TRACE(" dbfid=%d, ", fid);
		}
		TRACE("feature(%d),err fid=%d\n", m_RowIndex, m_fid);
	}
	RecordToFeature(NULL, -1, m_RowIndex);
	return FALSE;
}



STDMETHODIMP CGDBFeature::get_Value(/*[in]*/ VARIANT Field, /*[out, retval]*/ VARIANT *value)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	long nFld = m_pClass->FindFieldIndex(&Field);
	if (nFld < 0)
	{
		if (nFld == -2)
			return get_Identifier(value);
		return S_FALSE; //E_INVALIDARG;
	}
	if (nFld >= 0 && nFld<m_varVals.GetCount())
	{
		VariantCopy(value, &(m_varVals.GetAt(nFld)));
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CGDBFeature::get_Fields(SGCore::IFields **pVal)
{
	return m_pClass->get_Fields(pVal);
}

STDMETHODIMP CGDBFeature::get_Geometry(SGSFCOMS::IGeometry **pVal)
{
	//	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (m_pVarGeom && (m_pVarGeom->vt != VT_EMPTY && m_pVarGeom->vt != VT_NULL))
	{
		SGSFCOMS::IGeometryPtr pGeom = m_pClass->CreateGeometry(m_pVarGeom);
		if (pGeom)
		{
			pGeom->QueryInterface(SGSFCOMS::IID_IGeometry, (void **)pVal);
		}
	}
	return S_OK;
}

STDMETHODIMP CGDBFeature::get_Envelope(SGSFCOMS::IEnvelope **pVal)
{
	//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	*pVal = NULL;
	if (m_pClass->m_GeometryType != SGCore::SGO_GT_Unknown)
	{
		BOOL bGet = FALSE;
		SGCore::tagSGDRect rect;
		memset(&rect, 0, sizeof(SGCore::tagSGDRect));
		if (m_pClass->m_fids2)
			m_pClass->m_fids2->get_Boundary(m_RowIndex, &rect);
		if (rect.left != 0 || rect.right != 0 || rect.top != 0 || rect.bottom != 0)
		{
			bGet = TRUE;
		}
		else
		{
			SGSFCOMS::IGeometryPtr pGeom;
			get_Geometry(&pGeom);
			if (pGeom)
			{
				m_Envelope = pGeom->Envelope();
				if (m_Envelope)
					return m_Envelope->QueryInterface(SGSFCOMS::IID_IEnvelope, (void**)pVal);
			}
		}
		if (bGet)
		{
			if (m_Envelope == NULL)
				m_Envelope.CreateInstance(SGSFCOMS::CLSID_Envelope);

			m_Envelope->put_minX(rect.left);
			m_Envelope->put_maxX(rect.right);
			m_Envelope->put_minY(rect.bottom);
			m_Envelope->put_maxY(rect.top);
			m_Envelope->putref_SpatialReference(m_pClass->SpatialReference);
			m_Envelope->QueryInterface(SGSFCOMS::IID_IEnvelope, (void**)pVal);
		}
	}
	return S_OK;
}

STDMETHODIMP CGDBFeature::get_GeometryWithWKB(VARIANT* wkb)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	VariantClear(wkb);
	if (m_pClass->m_GeometryType != SGCore::SGO_GT_Unknown && m_pVarGeom)
	{
		if (m_pVarGeom->vt != VT_EMPTY && m_pVarGeom->vt != VT_NULL)
		{
			//VariantCopy(wkb, m_pVarGeom);				//#4548
			m_pClass->CreateGeometry(m_pVarGeom, wkb);
			return S_OK;
		}
	}
	return S_FALSE;

	//SGSFCOMS::IWksPtr pWks = Geometry;
	//if (pWks != NULL)
	//	return pWks->raw_ExportToWKB(wkb);
	//return S_FALSE;
}

STDMETHODIMP CGDBFeature::get_Identifier(long* id)
{
	*id = m_fid; //-1;
				 //_variant_t var;
				 //if (get_Identifier(&var) == S_OK)
				 //	*id = VariantToValue(var, -1l);
	return S_OK;
}

STDMETHODIMP CGDBFeature::put_Value(/*[in]*/ VARIANT Field, /*[in]*/ VARIANT value)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		//SGCore::IFieldPtr pFld = m_pClass->FindField(&Field);
		long nFld = m_pClass->FindFieldIndex(&Field);
	if (nFld >= 0 && nFld<m_pClass->m_Fields->GetFieldCount())
	{
		VARIANT* pVar = &(m_varVals[nFld]);
		if (m_RowIndex >= 0)
		{
			if (m_varOrgVals.GetCount() == 0)		//存放原始資料
			{
				m_fset.SetSize(m_varVals.GetCount());
				m_varOrgVals.SetSize(m_varVals.GetCount());
				for (int i = 0; i<m_varOrgVals.GetCount(); i++)
					m_varOrgVals[i].vt = VT_ERROR;
			}
			m_fset[nFld] = 1;
			if (m_varOrgVals[nFld].vt == VT_ERROR)
				VariantCopy(&(m_varOrgVals[nFld]), pVar);
		}
		VariantClear(pVar);
		VariantCopy(pVar, &value);
		return S_OK;
	}
	return S_FALSE;

	return S_OK;
}

STDMETHODIMP CGDBFeature::put_Geometry(SGSFCOMS::IGeometry *newVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	SGSFCOMS::IGeometryPtr pGeom = newVal;
	SGSFCOMS::IWksPtr pWks = pGeom;
	if (pWks != NULL)
	{
		//要先轉為layer的SR
		if (m_pClass->SpatialReference && pGeom->SpatialReference && (m_pClass->SpatialReference != pGeom->SpatialReference))
		{
			SGSFCOMS::IGeometryPtr pGeom2;
			pGeom->raw_Project(m_pClass->SpatialReference, &pGeom2);
			pWks = pGeom2;
			if (pWks == NULL)
				pWks = pGeom;
			else
				pGeom = pGeom2;
		}
		m_Envelope = pGeom->Envelope();
		m_Envelope->raw_Extent2D(&m_rect.left, &m_rect.bottom, &m_rect.right, &m_rect.top);

		int nFld = m_pClass->m_Fields->m_GeomIndex;
		if (nFld >= 0 && m_pVarGeom)
		{
			VARIANT var;
			var.vt = VT_EMPTY;
			var.lVal = 0;
			pWks->raw_ExportToWKB(&var);

			//if ((var.vt & VT_ARRAY))
			//{
			//	SAFEARRAY* psaSrc = var.parray;
			//	BYTE* pDataSrc;
			//	::SafeArrayAccessData(psaSrc, (void**)&pDataSrc);
			//	long nLen2 = psaSrc->rgsabound[0].cElements*psaSrc->cbElements;
			//	BYTE* pabyWKB = NULL; // GeometryToDim(pDataSrc, nLen2, 0);
			//	if (nLen2 > 0)
			//	{
			//		SAFEARRAY* psa = SafeArrayCreateVector(VT_UI1, 0, nLen2);
			//		BYTE* pData;
			//		::SafeArrayAccessData(psa, (void**)&pData);
			//		memcpy(pData, pabyWKB, nLen2);
			//		::SafeArrayUnaccessData(psa);
			//		::free(pabyWKB);
			//		VariantClear(&var);
			//		var.parray = psa;
			//		var.vt = VT_ARRAY | VT_UI1;
			//	}
			//	::SafeArrayUnaccessData(psaSrc);
			//}

			VARIANT* pVar = m_pVarGeom; // &(m_varVals[nFld]); 
			if (m_RowIndex >= 0)
			{
				int fn = m_varVals.GetCount() - 1;
				if (m_varOrgVals.GetCount() == 0)		//存放原始資料
				{
					m_fset.SetSize(m_varVals.GetCount());
					m_varOrgVals.SetSize(m_varVals.GetCount());
					for (int i = 0; i<m_varOrgVals.GetCount(); i++)
						m_varOrgVals[i].vt = VT_ERROR;
				}
				m_fset[fn] = 1;
				if (m_varOrgVals[fn].vt == VT_ERROR)
					VariantCopy(&(m_varOrgVals[fn]), pVar);
			}
			VariantClear(pVar);
			*pVar = var;
		}
		m_nGeomDirty++;
	}
	return S_OK;
}

STDMETHODIMP CGDBFeature::raw_Delete()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		if (m_pClass == NULL)
			return S_FALSE;
	if (!m_pClass->GetFeatureEditable())
		return S_FALSE;
	{
		if (m_fid >= 0)
		{
			if (m_pClass->DeleteFeature(m_fid) != -1)
			{
				if (m_RowIndex >= 0)	//k
					m_pClass->DeleteKey(m_RowIndex);
				if (!m_bCreate)	//若已新增又刪就不用
					m_pClass->SetRecDirty();

				//glog.WriteLog(L"del feature");
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CGDBFeature::raw_Update()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		if (m_pClass == NULL)
			return S_FALSE;
	if (!m_pClass->GetFeatureEditable())
		return S_FALSE;

	{
		if (m_RowIndex == -1)
		{
			m_fid = m_pClass->InsertFeature(m_varVals, &m_rect);
			if (m_fid<0)
				return S_FALSE;
			m_pClass->SetRecDirty();

		}
		else
		{
			if (m_fset.IsEmpty())
				return S_OK;

			int iRet = m_pClass->UpdateFeature(m_fid, m_varVals, m_fset, &m_rect);
			if (iRet == 0 || iRet == -2)
				return S_OK;
			if (iRet<0)
				return S_FALSE;
		}
	}

	{
		if (m_nGeomDirty>0)
		{
			SGCore::tagSGDRect rect;
			SGSFCOMS::IEnvelopePtr pEnvelope = m_Envelope;
			if (pEnvelope && (m_pClass->SpatialReference && m_Envelope->SpatialReference && (m_pClass->SpatialReference != m_Envelope->SpatialReference)))
			{
				SGSFCOMS::IGeometryPtr pGeom;
				m_Envelope->raw_Project(m_pClass->SpatialReference, &pGeom);
				pEnvelope = pGeom;
				if (pEnvelope == NULL)
					pEnvelope = m_Envelope;
			}
			if (pEnvelope)
				pEnvelope->raw_Extent2D(&rect.left, &rect.bottom, &rect.right, &rect.top);
			else
				memset(&rect, 0, sizeof(SGCore::tagSGDRect));

			m_pClass->SetGeomDirty();
			if (m_pClass->m_fids2 && m_RowIndex != -1)
				m_pClass->UpdateKeyBoundary(m_RowIndex, &rect);
			m_pClass->UpdateEnvelope(pEnvelope);
		}
		m_nGeomDirty = 0;
		m_pClass->SetValueDirty();
	}
	for (int i = 0; i<m_varOrgVals.GetCount(); i++)
	{
		if (m_fset[i] != 0)
		{
			m_fset[i] = 0;
			VariantClear(&(m_varOrgVals[i]));
			VariantCopy(&(m_varOrgVals[i]), &(m_varVals[i]));
		}
	}
	return S_OK;
}

STDMETHODIMP CGDBFeature::raw_CancelUpdate()
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//poDstLayer->RollbackTransaction();
	if (m_fid >= 0)
	{
		//用復原回m_varVals即可
		for (int i = 0; i<m_varOrgVals.GetCount(); i++)
		{
			m_fset[i] = 0;
			VariantClear(&(m_varVals[i]));
			VariantCopy(&(m_varVals[i]), &(m_varOrgVals[i]));
		}
	}
	m_nGeomDirty = 0;
	return S_OK;
}

STDMETHODIMP CGDBFeature::get_Identifier(VARIANT* id)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());

	id->vt = VT_I4;
	id->lVal = m_fid;
	//if (m_pClass->m_fids2)
	//	m_pClass->m_fids2->get_FIDByKey(m_RowIndex, id);
	//else
	//{
	//}
	return S_OK;
}

// CGDBFeatureCursor
CGDBFeatureCursor::CGDBFeatureCursor()
{
	m_pClass = NULL;
	m_pFeature = NULL;
	m_GeomType = SGCore::SGO_GT_Unknown;
	m_CurrentIndex = -1;
	m_TableRowCnt = -1;
	m_cursori = -1;	//記錄目前seek用的(for begin優化)
	m_t1 = GetTickCount();
}

CGDBFeatureCursor::~CGDBFeatureCursor()
{
	if (m_pClass)
	{
		m_pClass->Release();
	}
	m_pClass = NULL;
	if (m_pFeature != NULL)
		m_pFeature->Release();
	m_pFeature = NULL;
}

BOOL CGDBFeatureCursor::InitialCursor(CGDBFeatureClass *pClass, SGCore::IFilter *pTFilter, SGCore::IFeatureFilter *pFFilter)
{
	m_pClass = pClass;
	m_pClass->AddRef();
	m_pTFilter = pTFilter;
	m_pFFilter = pFFilter;
	m_CurrentIndex = -1;
	m_TableRowCnt = -1;
	m_cursori = -1;	//記錄目前seek用的(for begin優化)
	m_GeomType = m_pClass->GetGeometryType();
	return TRUE;
}

BOOL CGDBFeatureCursor::ValidRecord(SGSFCOMS::IGeometry *pGeom)
{
	return TRUE;
}

bool CGDBFeatureCursor::ReadRecord(long Current, CGDBFeature *pFeature)
{
	if (Current < 0)	//判再判原有的 , && Current>=m_rowN
		return false;
	return (pFeature->SetupRecord(Current)?true:false);
}

long CGDBFeatureCursor::NextRecord(long Current, long Step, CGDBFeature *pFeature)
{
	if (Current < 0)	//判再判原有的 , && Current>=m_rowN
		return -1;
	if (!pFeature->SetupRecord(Current))
		return -1;

	return Current;
}

long CGDBFeatureCursor::NextIndex(long Current, long Step, CGDBFeature *pFeature)
{
	Current = NextRecord(Current + Step, Step, pFeature);
	if (Current < 0)
		return -1;
	if (m_pTFilter != NULL)
	{
		VARIANT_BOOL result = FALSE;
		while (SUCCEEDED(m_pTFilter->raw_FilterOut(pFeature, &result)) && !result)
		{
			Current = NextRecord(Current + Step, Step, pFeature);
			if (Current < 0)
				return -1;
		}
	}
	else if (m_pFFilter != NULL)
	{
		VARIANT_BOOL result = FALSE;
		while (SUCCEEDED(m_pFFilter->raw_FilterOut(pFeature, &result)) && !result)
		{
			Current = NextRecord(Current + Step, Step, pFeature);
			if (Current < 0)
				return -1;
		}
	}
	return Current;
}

STDMETHODIMP CGDBFeatureCursor::raw_NextFeature(SGCore::IFeature **Feature)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		return raw_Seek(1, SGCore::SGO_SO_Current, Feature);
}

STDMETHODIMP CGDBFeatureCursor::get_FeatureCount(long * pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		*pVal = 0;
	{
		if (m_TableRowCnt<0)
		{
			m_pClass->get_RowCount(&m_TableRowCnt);
			if (m_pTFilter != NULL || m_pFFilter != NULL)
			{
				CComObject< CGDBFeature > *pFeature = new CComObject< CGDBFeature >;
				pFeature->AddRef();
				if (!pFeature->InitialFeature(m_pClass))
				{
					pFeature->Release();
					return S_OK;
				}
				long Index = NextIndex(-1, 1, pFeature);
				long Count = 0;
				while (Index >= 0)
				{
					Index = NextIndex(Index, 1, pFeature);
					Count++;
				}
				pFeature->Release();
				m_TableRowCnt = Count;
			}
		}
		*pVal = m_TableRowCnt;
	}
	return S_OK;
}

STDMETHODIMP CGDBFeatureCursor::raw_Seek(long Offset, SGCore::SGOSeekOrigin Origin, SGCore::IFeature ** Feature)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
		*Feature = NULL;
	if (m_pFeature != NULL && m_pFeature->m_dwRef > 1)
	{
		m_pFeature->Release();
		m_pFeature = NULL;
	}
	if (m_pFeature == NULL)
	{
		m_pFeature = new CComObject< CGDBFeature >;
		m_pFeature->AddRef();
		m_pFeature->InitialFeature(m_pClass);
	}
	long curi = m_CurrentIndex;
	long Step = (Offset == 0 ? 0 : Offset > 0 ? 1 : -1);
	if (Origin == SGCore::SGO_SO_Begin)
	{
		if (Offset<0)
			return S_FALSE;

		if (m_CurrentIndex<0)
		{
			curi = NextIndex(-1, 1, m_pFeature);
			m_cursori = 0;
		}
		else
		{
			long OffsetOrg = Offset;
			Offset -= m_cursori;
			if (Offset == 0)
			{
				Step = 0;
				curi = NextIndex(m_CurrentIndex, 0, m_pFeature);
			}
			else if (Offset<0)	//從頭來過
			{
				curi = NextIndex(-1, 1, m_pFeature);
				m_cursori = 0;
				Offset = OffsetOrg;
			}
			else
				Step = 1; //Offset > 0 ? 1 : -1;
		}
	}
	else if (Origin == SGCore::SGO_SO_End)
	{
		long cnt;
		m_pClass->get_RowCount(&cnt);
		curi = NextIndex(cnt, -1, m_pFeature);
	}
	else if (Offset == 0)
		curi = NextIndex(m_CurrentIndex, 0, m_pFeature);

	long stepN = 0;
	while (Offset * Step > 0)
	{
		if ((curi = NextIndex(curi, Step, m_pFeature)) < 0)
			break;
		Offset -= Step;
		stepN++;
	}
	HRESULT hr = S_OK;
	if (curi >= 0)
	{
		if (Step>0)
			m_cursori += stepN;
		else
			m_cursori -= stepN;
		m_CurrentIndex = curi;
		//CGDBFeature *pFeature = m_pFeatures[m_CurrentIndex];
		//hr = pFeature->QueryInterface(SGCore::IID_IFeature, (void **)Feature);
		hr = m_pFeature->QueryInterface(SGCore::IID_IFeature, (void **)Feature);
	}
	return hr;
}

STDMETHODIMP CGDBFeatureCursor::raw_NextRow(SGCore::ITableRow **pRow)
{
	return raw_Seek(1, SGCore::SGO_SO_Current, pRow);
}

STDMETHODIMP CGDBFeatureCursor::get_RowCount(long *pVal)
{
	return get_FeatureCount(pVal);
}

STDMETHODIMP CGDBFeatureCursor::raw_Seek(long Offset, SGCore::SGOSeekOrigin Origin, SGCore::ITableRow **pRow)
{
	SGCore::IFeaturePtr pFeature;
	HRESULT hr = raw_Seek(Offset, Origin, &pFeature);
	if (hr == S_OK && pFeature != NULL)
		hr = pFeature->QueryInterface(SGCore::IID_ITableRow, (void **)pRow);
	return hr;
}

