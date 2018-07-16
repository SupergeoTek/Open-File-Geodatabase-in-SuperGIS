#ifndef _PersistXMLImpl_H_
#define _PersistXMLImpl_H_

#import "SGGML\win32\SGGML.tlb" named_guids
#include "XML.h"

namespace PersistXML
{

void ExportImplement(SGGML::IXMLArchiveElement *pArchive, IUnknown* pUnk, BOOL XMLPriority = TRUE);
IUnknownPtr ImportImplement(SGGML::IXMLArchiveElement *pArchive);

class CVersion
{
public:
	CVersion();
	CVersion(LPCTSTR string);
	void operator= (LPCTSTR string);
	void operator= (DWORD value);
	operator CComBSTR();
	operator DWORD();

public:
	union tagVersion
	{
		DWORD m_Value;
		struct tagVersionStruct
		{
			BYTE m_Num4;
			BYTE m_Num3;
			BYTE m_Num2;
			BYTE m_Num1;
		} m_Struct;
	} m_Value;
};

template<const CLSID *pclsid>
class CPersistImpl : public IPersist
{
public:
// IPersist
	STDMETHOD(GetClassID)(struct _GUID * guid)
	{
		*guid = *pclsid;
		return S_OK;
	}
};

class CPersistXMLBaseImpl : 
	public SGGML::IPersistXML,
	public SGGML::IObjectIdentifier
{
public:
	CPersistXMLBaseImpl(DWORD version);

// IPersistXML
	STDMETHOD(raw_IsDirty)(VARIANT_BOOL *Result);
	STDMETHOD(raw_LoadXML)(IDispatch *varArchive);
	STDMETHOD(raw_SaveXML)(IDispatch *varArchive);
// IObjectIdentifier
	STDMETHOD(get_ObjectID)(GUID *pVal) {*pVal = m_ObjectID; return S_OK;}
protected:
	virtual void LoadSaveXMLExt(DWORD version, 
		SGGML::IXMLArchiveElement *pSource = NULL, 
		BOOL bLoad = TRUE) = 0;

	void SetPersistDirty(BOOL b);
	//SGGML::IXMLArchiveElement *m_pArchive;
protected:
	CVersion m_Version;
	BOOL m_bPersistDirty;
	CLSID m_ObjectID;
};

class CPersistXMLImpl : 
	public CPersistXMLBaseImpl,
	public IPersist
{
	//typedef  CPersistXMLImpl PersistXMLImplType;
public:
	CPersistXMLImpl(DWORD version,const CLSID *pclsid);

// IPersist
	STDMETHOD(GetClassID)(struct _GUID * guid);
protected:
	CLSID m_clsid;
};

class CPersistXMLStreamImpl : 
	public CPersistXMLBaseImpl,
	public IPersistStream
{
public:
	CPersistXMLStreamImpl(DWORD version,const CLSID *pclsid);
	void SetPersistDirty(BOOL b);

// IPersist
	STDMETHOD(GetClassID)(struct _GUID * guid);

// IPersistStream
	STDMETHOD(IsDirty)(void);
protected:
	CLSID m_clsid;
	BOOL m_bPersistDirty;
};

};
#endif// _PersistXMLImpl_H_