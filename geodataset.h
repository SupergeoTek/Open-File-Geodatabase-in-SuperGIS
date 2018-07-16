#ifndef _GEODATASET_H_
#define _GEODATASET_H_

#include "InterfaceImpl.h"
#include <vector>
struct DRECT;

struct WORLDFILEDESC
{
	double XResolution;
	double XRotate;
	double YRotate;
	double YResolution;
	double Left;
	double Top;
};

CString EncodeGeoTransformString(SGSFCOMS::IGeographicTransform *GeoTrans);
SGSFCOMS::IGeographicTransformPtr ParseGeoTransformString(CString strTrans);
CString ReadProjFile(LPCTSTR strFileName);
SGSFCOMS::ISpatialReferencePtr GetProjFile(LPCTSTR strFileName);
void SetProjFile(LPCTSTR strFileName, SGSFCOMS::ISpatialReferencePtr pRef);
BOOL GetWordFile(CString strFileName, WORLDFILEDESC *wfdesc);
void SplitPath(LPCTSTR PathName, CString &strPath, CString &strName, CString &strExt);
CString SeperatePath(LPCTSTR Path, LPCTSTR WorkPath);
CString CombinePath(LPCTSTR Path, LPCTSTR WorkPath = NULL);

class CGeoDatasetImpl :
	public IDispatchImpl<SGCore::IGeoDataset2, &SGCore::IID_IGeoDataset2>
{
public:
	CGeoDatasetImpl();
protected:
	static void ProjectBoundary(SGSFCOMS::IEnvelope* pEnv, SGSFCOMS::ISpatialReference *SpatialRef, SGSFCOMS::ISpatialTransformPtr &SpatialTransform);
	void SetOSR(SGSFCOMS::ISpatialReference *SpatialRef)
	{
		m_OriginSpatialReference = SpatialRef;
		m_bSpatialReferenceModified = false;
		m_SpatialReference = SpatialRef;
	}
private:
	SGSFCOMS::IGeographicTransformPtr m_GeographicTransform;

	SGSFCOMS::ISpatialReferencePtr m_OriginSpatialReference;
protected:
	bool	m_bSpatialReferenceModified;
	SGSFCOMS::ISpatialReferencePtr m_SpatialReference;
	double m_dMinX;
	double m_dMaxX;
	double m_dMinY;
	double m_dMaxY;
public:
	STDMETHOD(get_SpatialReference)(SGSFCOMS::ISpatialReference **pVal);
	IMPL_GETPTR_PROPERTY(SGSFCOMS::IGeographicTransform, GeographicTransform)
	IMPL_PUTPTRREF_PROPERTY(SGSFCOMS::IGeographicTransform, GeographicTransform)
	STDMETHOD(get_Extent)(SGSFCOMS::ISpatialReference *SpatialRef, SGSFCOMS::IEnvelope **pVal);

	STDMETHOD(raw_ModifySpatialReference)(SGSFCOMS::ISpatialReference *SpatialRef);
};

//class CFileDatasetImpl : public IFileDataset
//{
//public:
//// IFileDataset
//	STDMETHOD(raw_OpenFile)(BSTR PathName, VARIANT_BOOL *Result);
//	IMPL_GETBSTR_PROPERTY(FilePathName)
//	STDMETHOD(get_ContentType)(BSTR *pVal);
//};

class CFileClassImpl : public SGCore::IFileClass, public SGCore::IAttachment
{
protected:
	_bstr_t m_FilePath;

public:
	STDMETHOD(get_FilePathName)(BSTR *pVal);
	STDMETHOD(get_Attachment)(BSTR Name, VARIANT* pVal);
	STDMETHOD(put_Attachment)(BSTR Name, VARIANT newVal);
};


class CFields :
	public IDispatchImpl<SGCore::IFields, &SGCore::IID_IFields, &SGCore::LIBID_SGCore>,
	public CComObjectRoot
{
public:
	CFields();
	virtual ~CFields();
	CString UniqueFieldName(LPCTSTR strConst, long nStartIndex = 1);

	long FindFieldIndex(LPCWSTR strName);
	long FindFieldIndex(VARIANT *pField);
	SGCore::IFieldPtr GetField(long nIndex);
	BOOL FieldExists(LPCWSTR strName);

	void SetSourceCount(long Cnt);
	void AddField(long nIndex, SGCore::IField2 *pFld);
	void SetIdentity(long nIndex, SGCore::IField2 *pFld);
	void SetGeomIndex(long nIndex, LPCTSTR geomName = NULL);

	BEGIN_COM_MAP(CFields)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(SGCore::IFields)
	END_COM_MAP()

public:
	SGCore::IFieldPtr m_IndexField;
	std::vector<SGCore::IField2Ptr> m_Fields;

	long m_SourceCount;
	long *m_Indexs;
	long m_IdentIndex;
	bool m_bRowid;
	long m_GeomIndex;
	CString m_geomName;

	// IFields
public:
	STDMETHOD(get_Field)(long Index, SGCore::IField **pVal);
	STDMETHOD(get_FieldCount)(long *pVal);
	STDMETHOD(raw_Find)(BSTR Name, long *Index);
};

#endif //_GEODATASET_H_