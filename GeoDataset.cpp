#include "stdafx.h"
#include "GeoDataset.h"
//#include "GeoProcess.h"
#include "TProcess.h"
#include <limits>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CGeoDatasetImpl::CGeoDatasetImpl()
{
	m_dMinX = std::numeric_limits<double>::quiet_NaN();
	m_dMaxX = std::numeric_limits<double>::quiet_NaN();
	m_dMinY = std::numeric_limits<double>::quiet_NaN();
	m_dMaxY = std::numeric_limits<double>::quiet_NaN();
	m_bSpatialReferenceModified = false;
}

void CGeoDatasetImpl::ProjectBoundary(SGSFCOMS::IEnvelope* pEnv, SGSFCOMS::ISpatialReference *pTargetSR, SGSFCOMS::ISpatialTransformPtr &SpatialTransform)
{
	SGSFCOMS::ISpatialReferencePtr pSourceSR = pEnv->SpatialReference;
	if (pSourceSR != NULL && pTargetSR != NULL && pSourceSR != pTargetSR)
	{
		//SGSFCOMS::tagWKSPoint pt[4] = {
		//	{ pEnv->minX, pEnv->minY },
		//	{ pEnv->minX, pEnv->maxY },
		//	{ pEnv->maxX, pEnv->minY },
		//	{ pEnv->maxX, pEnv->maxY }
		//};
		if (SpatialTransform == NULL)
			SpatialTransform.CreateInstance(SGSFCOMS::CLSID_SpatialTransform);
		SpatialTransform->putref_SourceSpatialReference(pSourceSR);
		SpatialTransform->putref_TargetSpatialReference(pTargetSR);
		//SpatialTransform->raw_Forward(4, pt);

		//pEnv->put_minX(min(min(min(pt[0].x,pt[1].x),pt[2].x),pt[3].x));
		//pEnv->put_maxX(max(max(max(pt[0].x,pt[1].x),pt[2].x),pt[3].x));
		//pEnv->put_minY(min(min(min(pt[0].y,pt[1].y),pt[2].y),pt[3].y));
		//pEnv->put_maxY(max(max(max(pt[0].y,pt[1].y),pt[2].y),pt[3].y));
		//pEnv->putref_SpatialReference(pTargetSR);
	}
	else if (SpatialTransform != NULL)
		SpatialTransform.Release();

};

STDMETHODIMP CGeoDatasetImpl::get_Extent(SGSFCOMS::ISpatialReference *SpatialRef, SGSFCOMS::IEnvelope **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	if (_isnan(m_dMinX) || _isnan(m_dMaxX) || 
		_isnan(m_dMinY) || _isnan(m_dMaxY))
		return S_OK;
	SGSFCOMS::IEnvelopePtr pEnv(SGSFCOMS::CLSID_Envelope);
	pEnv->put_minX(m_dMinX);
	pEnv->put_maxX(m_dMaxX);
	pEnv->put_minY(m_dMinY);
	pEnv->put_maxY(m_dMaxY);
	pEnv->putref_SpatialReference(GetSpatialReference());
	//ProjectBoundary(pEnv, SpatialRef, m_SpatialTransform);
	SGSFCOMS::IGeometryPtr pGeom = pEnv->Project(SpatialRef);
	pGeom->QueryInterface(SGSFCOMS::IID_IEnvelope, (void **)pVal);

	return S_OK;
}

STDMETHODIMP CGeoDatasetImpl::get_SpatialReference(SGSFCOMS::ISpatialReference **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())

	if (m_bSpatialReferenceModified)
	{
		if (m_SpatialReference)
			m_SpatialReference->QueryInterface(SGSFCOMS::IID_ISpatialReference, (void**)pVal);
	}
	else
	{
		if (m_OriginSpatialReference)
			m_OriginSpatialReference->QueryInterface(SGSFCOMS::IID_ISpatialReference, (void**)pVal);
	}
	return S_OK;
}

STDMETHODIMP CGeoDatasetImpl::raw_ModifySpatialReference(SGSFCOMS::ISpatialReference *SpatialRef)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	m_SpatialReference = SpatialRef;
	m_bSpatialReferenceModified = true;
	return S_OK;
}


CString ReadProjFile(LPCTSTR strFileName)
{
	CFile f;
	if (!f.Open(strFileName, CFile::modeRead | CFile::typeBinary))
		return (LPCTSTR)NULL;
	
	long len = (long)f.GetLength();
	CStringA straTmp;
	char * pChr = straTmp.GetBuffer(len);
	f.Read(pChr, len);
	pChr[len] = 0;
	straTmp.ReleaseBuffer();
	f.Close();

	SGSFCOMS::ISpatialReferenceFactoryPtr pSRF(SGSFCOMS::CLSID_SpatialReferenceFactory);
	return (LPCTSTR)_bstr_t(straTmp);
}
SGSFCOMS::ISpatialReferencePtr GetProjFile(LPCTSTR strFileName)
{
	CString strPrj = ReadProjFile(strFileName);
	if (strPrj.IsEmpty())
		return NULL;
	SGSFCOMS::ISpatialReferenceFactoryPtr pSRF(SGSFCOMS::CLSID_SpatialReferenceFactory);
	return pSRF->CreateFromWKT(_bstr_t(strPrj));
}
void SetProjFile(LPCTSTR strFileName, SGSFCOMS::ISpatialReferencePtr pRef)
{
	if (pRef == NULL)
		return;
	CFile f;
	if (!f.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
		return;
	_bstr_t tmp = pRef->WellKnownText;
	LPCSTR str = (LPCSTR)tmp;
	f.Write(str, (UINT)strlen(str));
	f.Close();
}
//BOOL GetWordFile(CString strFileName, WORLDFILEDESC *wfdesc)
//{
//	if (wfdesc==NULL)
//		return FALSE;
//
//	CString strWorldFile = strFileName;
//	strWorldFile.Delete(strWorldFile.GetLength() - 2);
//	strWorldFile += _T("w");
//	TRACE(_T("wf=%s\n"), strWorldFile);
//
//	CStdioFile f;
//	
//	if (!f.Open(strWorldFile, CFile::modeRead))
//		if (!f.Open(strFileName + _T("w"), CFile::modeRead))
//			return FALSE;
//
//	int nLine = 0; // 表第幾行
//
//	double dXScale = 1;
//	double dYScale = 1;
//	double dLeft = 0;
//	double dTop = 0;
//	double *pValue = (double *)wfdesc;
//
//	//while (fgets(line, 40, file) != NULL)
//	CString strTemp;
//	while (f.ReadString(strTemp))
//	{
//		if (!strTemp.IsEmpty())
//		{
//			pValue[nLine] = _tcstod(strTemp, NULL);
//			nLine++;
//			if (nLine>=6) break;	//ken
//		}
//	}
//
//	f.Close();
//	return TRUE;
//}

void SplitPath(LPCTSTR PathName, CString &strPath, CString &strName, CString &strExt)
{
	TCHAR Drive[_MAX_DRIVE];
	TCHAR Path[_MAX_PATH];
	TCHAR Filename[_MAX_PATH];
	TCHAR Extname[_MAX_PATH];
	_tsplitpath_s(PathName, Drive, _MAX_DRIVE, Path, _MAX_PATH, Filename, _MAX_PATH, Extname, _MAX_PATH);
	strPath = CString(Drive) + CString(Path);
	strName = Filename;
	strExt = Extname;
}

CString SeperatePath(LPCTSTR Path, LPCTSTR WorkPath)
{
	TCHAR ODrive[_MAX_DRIVE];
	TCHAR OPath[_MAX_PATH];
	_tsplitpath_s(Path, ODrive, _MAX_DRIVE, OPath, _MAX_PATH, NULL, 0, NULL, 0);
	if (ODrive[0] == _T('\0') && 
		!(_tcslen(OPath) > 2 && OPath[0] == _T('\\') && OPath[1] == _T('\\')))
		return Path;

	TCHAR TDrive[_MAX_DRIVE];
	_tsplitpath_s(WorkPath, TDrive, _MAX_DRIVE, NULL, 0, NULL, 0, NULL, 0);
	if (_tcsicmp(ODrive, TDrive) != 0)
		return Path;
	
	LPCTSTR pOPath = Path;
	LPCTSTR pTPath = WorkPath;
	while (*pTPath != _T('\0') && *pOPath != _T('\0'))
	{
		LPCTSTR pOdest = _tcschr(pOPath, _T('\\'));
		LPCTSTR pTdest = _tcschr(pTPath, _T('\\'));
		if (pOdest == NULL)
			pOdest = pOPath + _tcslen(pOPath);
		if (pTdest == NULL)
			pTdest = pTPath + _tcslen(pTPath);
		if (pOdest - pOPath != pTdest - pTPath || _tcsnicmp(pOPath, pTPath, pOdest - pOPath) != 0)
			break;
		pOPath = pOdest;
		pTPath = pTdest;
		if (*pOdest == _T('\\')) pOPath++;
		if (*pTPath == _T('\\')) pTPath++;
	}
	if (*pTPath == _T('\0'))
		return pOPath;
	TCHAR DPath[_MAX_PATH];
	TCHAR *pDPath = DPath;
	while (*pTPath != _T('\0'))
	{
		memcpy(pDPath, _T("..\\"), 3 * sizeof(TCHAR));
		pDPath += 3;
		pTPath = _tcschr(pTPath, _T('\\'));
		if (pTPath == NULL)
			break;
		pTPath++;
	}
	_tcscpy_s(pDPath, _MAX_PATH - (pDPath - DPath), pOPath);
	return DPath;
}

CString CombinePath(LPCTSTR Path, LPCTSTR WorkPath)
{
	TCHAR ODrive[_MAX_DRIVE];
	TCHAR OPath[_MAX_PATH];
	_tsplitpath_s(Path, ODrive, _MAX_DRIVE, OPath, _MAX_PATH, NULL, 0, NULL, 0);
	if (ODrive[0] != _T('\0') || 
		(_tcslen(OPath) > 2 && OPath[0] == _T('\\') && OPath[1] == _T('\\')))
		return Path;

	CString TWork = WorkPath;
	if (TWork.IsEmpty())
	{
		TCHAR tchBuffer[MAX_PATH];
		memset(tchBuffer, 0, MAX_PATH);
#ifndef _WIN32_WCE
		DWORD cchCurDir = GetCurrentDirectory(MAX_PATH, tchBuffer);
#else
		DWORD cchCurDir = GetModuleFileName(NULL, tchBuffer, MAX_PATH);
#endif
		tchBuffer[cchCurDir] = _T('\\');
		tchBuffer[cchCurDir + 1] = _T('\0');
		TWork = tchBuffer;
	}
	if (Path[0] == _T('\\'))
	{
		TCHAR TDrive[_MAX_DRIVE];
		_tsplitpath_s(TWork, TDrive, _MAX_DRIVE, NULL, 0, NULL, 0, NULL, 0);
		return CString(TDrive) + Path;
	}

	TCHAR TPath[_MAX_PATH];
	_tcscpy_s(TPath, _MAX_PATH, TWork);
	if (TPath[_tcslen(TPath) - 1] == _T('\\'))
		TPath[_tcslen(TPath) - 1] = _T('\0');

	LPCTSTR pPath = Path;
	while (true)
	{
		if (_tcsncmp(pPath, _T(".\\"), 2) == 0)
			pPath += 2;
		else if (_tcsncmp(pPath, _T("..\\"), 3) == 0)
		{
			TCHAR *pdest = _tcsrchr(TPath, _T('\\'));
			if (pdest) *pdest = _T('\0');
			pPath += 3;
		}
		else
			break;
	}
	TPath[_tcslen(TPath) + 1] = _T('\0');
	TPath[_tcslen(TPath)] = _T('\\');
	
	return CString(TPath) + CString(pPath);
}

CString EncodeGeoTransformString(SGSFCOMS::IGeographicTransform *GeoTrans)
{
	if (GeoTrans==NULL)
		return _T("");

	SGSFCOMS::IParameterInfoPtr paraminfo;
	GeoTrans->get_ParameterInfo(&paraminfo);
	if (paraminfo==NULL)
		return _T("");

	SGSFCOMS::IParameterPtr param;
	double dx,dy,dz,rx,ry,rz,sf;
	paraminfo->get_Parameter(0,&param);
	param->get_Value(&dx);
	paraminfo->get_Parameter(1,&param);
	param->get_Value(&dy);
	paraminfo->get_Parameter(2,&param);
	param->get_Value(&dz);
	paraminfo->get_Parameter(3,&param);
	param->get_Value(&rx);
	paraminfo->get_Parameter(4,&param);
	param->get_Value(&ry);
	paraminfo->get_Parameter(5,&param);
	param->get_Value(&rz);
	paraminfo->get_Parameter(6,&param);
	param->get_Value(&sf);
	CString strTemp;
	strTemp.Format(_T("%.16g,%.16g,%.16g,%.16g,%.16g,%.16g,%.16g"),
		dx,dy,dz,rx,ry,rz,sf);
	return strTemp;
}

SGSFCOMS::IGeographicTransformPtr ParseGeoTransformString(CString strTrans)
{
	if (strTrans.IsEmpty()) return NULL;
	TCHAR *buf = strTrans.GetBuffer(0);
	if (*buf == '\0') return NULL;
	double dx = _tcstod(buf,&buf);
	if (*buf != ',') return NULL;
	double dy = _tcstod(buf+1,&buf);
	if (*buf != ',') return NULL;
	double dz = _tcstod(buf+1,&buf);
	if (*buf != ',') return NULL;
	double rx = _tcstod(buf+1,&buf);
	if (*buf != ',') return NULL;
	double ry = _tcstod(buf+1,&buf);
	if (*buf != ',') return NULL;
	double rz = _tcstod(buf+1,&buf);
	if (*buf != ',') return NULL;
	double sf = _tcstod(buf+1,&buf);
	strTrans.ReleaseBuffer();

	SGSFCOMS::IGeographicTransformPtr GeoTrans;
	GeoTrans.CreateInstance(SGSFCOMS::CLSID_GeographicTransform);
	SGSFCOMS::IParameterInfoPtr paraminfo;
	GeoTrans->get_ParameterInfo(&paraminfo);
	if (paraminfo==NULL)
		return NULL;

	SGSFCOMS::IParameterPtr param;
	ASSERT(SUCCEEDED(param.CreateInstance(SGSFCOMS::CLSID_Parameter)));
	if (param==NULL)
		return NULL;
	param->put_Value(dx);
	paraminfo->put_Parameter(0,param);
	param->put_Value(dy);
	paraminfo->put_Parameter(1,param);
	param->put_Value(dz);
	paraminfo->put_Parameter(2,param);
	param->put_Value(rx);
	paraminfo->put_Parameter(3,param);
	param->put_Value(ry);
	paraminfo->put_Parameter(4,param);
	param->put_Value(rz);
	paraminfo->put_Parameter(5,param);
	param->put_Value(sf);
	paraminfo->put_Parameter(6,param);
	return GeoTrans;
}

STDMETHODIMP CFileClassImpl::get_FilePathName(BSTR *pVal) 
{
	*pVal = m_FilePath.copy();
	return S_OK;
}

STDMETHODIMP CFileClassImpl::get_Attachment(BSTR Name, VARIANT* pVal)
{
	if (_wcsicmp(Name, L"Metadata") == 0)
	{
		CString strTmp = (LPCTSTR)m_FilePath;
		long sidx = strTmp.Find('\\');
		long didx = strTmp.Find('.', sidx);
		strTmp = strTmp.Left(didx) + _T(".xml");

		CFile f;
		if (!f.Open(strTmp, CFile::modeRead))
			return S_FALSE;
		
		ULONG len = (ULONG)f.GetLength();
		SAFEARRAY *sa = SafeArrayCreateVector(VT_UI1, 0, len);
		BYTE *pBuf;
		SafeArrayAccessData(sa, (void **)&pBuf);
		f.Read(pBuf, len);
		SafeArrayUnaccessData(sa);
		f.Close();

		V_VT(pVal) = VT_UI1 | VT_ARRAY;
		V_ARRAY(pVal) = sa;
		return S_OK;
	}
	return S_FALSE;
}

STDMETHODIMP CFileClassImpl::put_Attachment(BSTR Name, VARIANT newVal)
{
	if (V_VT(&newVal) != (VT_UI1 | VT_ARRAY))
		return E_INVALIDARG;

	if (_wcsicmp(Name, L"Metadata") == 0)
	{
		CString strTmp = (LPCTSTR)m_FilePath;
		long sidx = strTmp.Find('\\');
		long didx = strTmp.Find('.', sidx);
		strTmp = strTmp.Left(didx) + _T(".xml");

		CFile f;
		if (!f.Open(strTmp, CFile::modeCreate | CFile::modeWrite))
			return S_FALSE;
		
		SAFEARRAY *sa = V_ARRAY(&newVal);
		ULONG len = sa->rgsabound[0].cElements;

		BYTE *pBuf;
		SafeArrayAccessData(sa, (void**)&pBuf);
		f.Write(pBuf, len);
		SafeArrayUnaccessData(sa);
		f.Close();
		return S_OK;
	}
	return S_FALSE;
}


// CFields
CFields::CFields()
{
	m_SourceCount = 0;
	m_Indexs = NULL;
	m_GeomIndex = -1;
	m_IdentIndex = -1;
	m_bRowid = false;
}

CFields::~CFields()
{
	if (m_Indexs != NULL)
		delete[] m_Indexs;
	m_Indexs = NULL;
}

long CFields::FindFieldIndex(LPCWSTR strName)
{
	if (strName == NULL)
		return -1;
	if (m_IndexField != NULL)
	{
		if (_wcsicmp(m_IndexField->Name, strName) == 0)
			return -2;
	}
	INT_PTR cnt = m_Fields.size();
	for (INT_PTR i = 0; i < cnt; i++)
	{
		if (_wcsicmp(m_Fields[i]->Name, strName) == 0)
			return (long)i;
	}
	return -1;
}

long CFields::FindFieldIndex(VARIANT *pField)
{
	if (pField == NULL)
		return -1;
	switch (V_VT(pField))
	{
	case VT_BYREF | VT_VARIANT:
		return FindFieldIndex((VARIANT *)V_BYREF(pField));
		break;
	case VT_UI1: case VT_UI2: case VT_UI4: case VT_UI8: case VT_UINT:
	case VT_I1: case VT_I2: case VT_I4: case VT_I8: case VT_INT:
		return VariantToValue<long>(pField);
		break;
	case VT_BSTR:
		return FindFieldIndex(_bstr_t(V_BSTR(pField)));
		break;
	case VT_DISPATCH:
	case VT_UNKNOWN:
	{
		SGCore::IFieldPtr pFld = V_UNKNOWN(pField);
		if (pFld != NULL)
			return FindFieldIndex(pFld->Name);
	}
	break;
	}
	return -1;
}

SGCore::IFieldPtr CFields::GetField(long nIndex)
{
	if (nIndex == -2 && m_IndexField != NULL)
		return m_IndexField;
	else if (nIndex >= 0 && nIndex < (long)m_Fields.size())
		return m_Fields[nIndex];
	return NULL;
}

BOOL CFields::FieldExists(LPCWSTR strName)
{
	long idx = FindFieldIndex(strName);
	return (idx >= 0 || idx == -2);
}

CString CFields::UniqueFieldName(LPCTSTR strConst, long nStartIndex)
{
	CString strTmp = strConst;
	long i = nStartIndex;
	while (FieldExists(strTmp))
		strTmp.Format(_T("%s_%d"), strConst, i++);
	return strTmp;
}

void CFields::SetSourceCount(long Cnt)
{
	m_SourceCount = Cnt;
	if (m_Indexs != NULL)
		delete[] m_Indexs;
	m_Indexs = new long[Cnt];
	memset(m_Indexs, 0, Cnt * sizeof(long));
}

void CFields::AddField(long nIndex, SGCore::IField2 *pFld)
{
	m_Indexs[m_Fields.size()] = nIndex;
	m_Fields.push_back(pFld);
}

void CFields::SetIdentity(long nIndex, SGCore::IField2 *pFld)
{
	m_IdentIndex = nIndex;
	m_IndexField = pFld;
}

void CFields::SetGeomIndex(long nIndex, LPCTSTR geomName)
{
	m_GeomIndex = nIndex;
	if (geomName)
		m_geomName = geomName;
}

STDMETHODIMP CFields::get_Field(long Index, SGCore::IField **pVal)
{
	if (Index == -2 && m_IndexField != NULL)
		m_IndexField->QueryInterface(SGCore::IID_IField, (void **)pVal);
	else if (Index >= 0 && Index < (long)m_Fields.size())
		m_Fields[Index]->QueryInterface(SGCore::IID_IField, (void **)pVal);
	else
		return S_FALSE; //E_INVALIDARG;
	return S_OK;
}

STDMETHODIMP CFields::get_FieldCount(long *pVal)
{
	*pVal = (long)m_Fields.size();
	return S_OK;
}

STDMETHODIMP CFields::raw_Find(BSTR Name, long *Index)
{
	*Index = FindFieldIndex(_bstr_t(Name));
	return S_OK;
}
