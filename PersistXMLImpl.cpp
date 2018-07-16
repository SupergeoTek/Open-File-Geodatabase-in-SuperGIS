#include "stdafx.h"
#include "PersistXMLImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace PersistXML
{

void ExportImplement(SGGML::IXMLArchiveElement *pArchive, 
					 IUnknown* pUnk, 
					 BOOL XMLPriority)
{
//	if (pArchive)
//		pArchive->raw_ExportComponent(pUnk, XMLPriority);
//	return;

//	IDispatchPtr pDisp;
//	pArchive->get_DOMElement(&pDisp);
	MSXML2::IXMLDOMElementPtr pSource = pArchive;
	if (pArchive==NULL)
		return;

	IPersistPtr pPers = pUnk;
	if (pPers!=NULL)
	{
		CLSID clsid;
		pPers->GetClassID(&clsid);
		LPOLESTR str;
		if (ProgIDFromCLSID(clsid, &str) != S_OK)
			StringFromCLSID(clsid, &str);
		AddAttribute(pSource, CComBSTR("clsid"), CComBSTR(str));
		CoTaskMemFree(str);
		SGGML::IPersistXMLPtr pXml = pUnk;
		if (XMLPriority && pXml!=NULL)
			pXml->SaveXML(pArchive);
		else
		{
			IStreamPtr pStream;
			CreateStreamOnHGlobal(NULL, TRUE, &pStream);
			IPersistStreamPtr pPStream = pUnk;
			if (pPStream!=NULL && pStream!=NULL)
			{
				HRESULT hr = pPStream->Save(pStream, TRUE);
				SetNodeText(pArchive, EncodeStream(pStream));
				AddAttribute(pArchive, L"PersistStream", L"True");
			}
			else if (!XMLPriority && pXml!=NULL)
				pXml->raw_SaveXML(pArchive);
		}
	}
	return;
}

IUnknownPtr ImportImplement(SGGML::IXMLArchiveElement *pArchive)
{
//	if (pArchive)
//	{
//		IUnknownPtr pUnk;
//		pArchive->raw_ImportComponent(&pUnk);
//		return pUnk;
//	}
//	return NULL;

//	IDispatchPtr pDisp;
//	pArchive->get_DOMElement(&pDisp);
	MSXML2::IXMLDOMElementPtr pNode = pArchive;
	if (pNode)
	{
		BOOL ps = GetBoolean(GetNodeAttribute(pArchive, L"PersistStream"), FALSE);
		CString strclsid = GetNodeAttribute(pNode, CComBSTR("clsid"));
		if (strclsid.IsEmpty())
			return NULL;
		IUnknownPtr pImpl;
		pImpl.CreateInstance((LPTSTR)(LPCTSTR)strclsid);
		if (!ps)
		{
			SGGML::IPersistXMLPtr pXml = pImpl;
			if (pXml)
				pXml->raw_LoadXML(pNode);
		}
		else
		{
			IPersistStreamPtr pPStream = pImpl;
			if (pPStream!=NULL)
			{
				IStreamPtr pStream = DecodeStream(GetNodeTextB(pNode));
				pPStream->Load(pStream);
			}
		}
		return pImpl;
	}
	return NULL;

}

CVersion::CVersion() 
{
	m_Value.m_Value = 0;
}

CVersion::CVersion(LPCTSTR string)
{
	this->operator =(string);
}

void CVersion::operator= (LPCTSTR string)
{
	char *pbuf = (char *)string;
	m_Value.m_Struct.m_Num1 = (BYTE)strtol(pbuf, &pbuf, 10);
	if (*pbuf == '.') pbuf++;
	m_Value.m_Struct.m_Num2 = (BYTE)strtol(pbuf, &pbuf, 10);
	if (*pbuf == '.') pbuf++;
	m_Value.m_Struct.m_Num3 = (BYTE)strtol(pbuf, &pbuf, 10);
	if (*pbuf == '.') pbuf++;
	m_Value.m_Struct.m_Num4 = (BYTE)strtol(pbuf, &pbuf, 10);
}

void CVersion::operator= (DWORD value)
{
	m_Value.m_Value = value;
}

CVersion::operator CComBSTR()
{
	CString strTemp;
	strTemp.Format(TEXT("%d.%d.%d.%d"), 
		m_Value.m_Struct.m_Num1, 
		m_Value.m_Struct.m_Num2, 
		m_Value.m_Struct.m_Num3, 
		m_Value.m_Struct.m_Num4);
	return CComBSTR(strTemp);
}

CVersion::operator DWORD()
{
	return m_Value.m_Value;
}

CPersistXMLBaseImpl::CPersistXMLBaseImpl(DWORD version)
{
	m_Version = version;
	m_bPersistDirty = FALSE;
	CoCreateGuid(&m_ObjectID);
}

void CPersistXMLBaseImpl::SetPersistDirty(BOOL b)
{
	m_bPersistDirty = b;
}

// IPersistXML
STDMETHODIMP CPersistXMLBaseImpl::raw_IsDirty(VARIANT_BOOL *Result)
{
	if (*Result = m_bPersistDirty)
		return S_OK;
	return S_FALSE;
}

STDMETHODIMP CPersistXMLBaseImpl::raw_LoadXML(IDispatch *varArchive)
{
	if (varArchive)
	{
		m_Version = GetNodeAttribute(SGGML::IXMLArchiveElementPtr(varArchive), L"version");
		LoadSaveXMLExt(m_Version, SGGML::IXMLArchiveElementPtr(varArchive));
	}
	return S_OK;
}

STDMETHODIMP CPersistXMLBaseImpl::raw_SaveXML(IDispatch *varArchive)
{
	if (varArchive)
	{
		if (m_Version!=0)
			AddAttribute(SGGML::IXMLArchiveElementPtr(varArchive), CComBSTR("version"), CComBSTR(m_Version));
		LoadSaveXMLExt(m_Version, SGGML::IXMLArchiveElementPtr(varArchive), FALSE);
	}
	return S_OK;
}

CPersistXMLImpl::CPersistXMLImpl(DWORD version,const CLSID *pclsid) :
	CPersistXMLBaseImpl(version)
{
	m_clsid = *pclsid;
}

STDMETHODIMP CPersistXMLImpl::GetClassID(struct _GUID * guid)
{
	*guid = m_clsid;
	return S_OK;
}

CPersistXMLStreamImpl::CPersistXMLStreamImpl(DWORD version,const CLSID *pclsid) :
	CPersistXMLBaseImpl(version)
{
	m_clsid = *pclsid;
	m_bPersistDirty = FALSE;
}

void CPersistXMLStreamImpl::SetPersistDirty(BOOL b)
{
	m_bPersistDirty = b;
	CPersistXMLBaseImpl::SetPersistDirty(b);
}

STDMETHODIMP CPersistXMLStreamImpl::GetClassID(struct _GUID * guid)
{
	*guid = m_clsid;
	return S_OK;
}

STDMETHODIMP CPersistXMLStreamImpl::IsDirty(void)
{
	if (m_bPersistDirty)
		return S_OK;
	return S_FALSE;
}

}
