#include "stdafx.h"
#include "Extension.h"
#include "PersistXMLImpl.h"

SGGML::IXMLArchivePtr GetAppDoc();
CExtensionContainerImpl::CExtensionContainerImpl() : m_Persisting(FALSE)
{
	CWrapImpl *pWrap = new CComObject<CExtensionContainerImpl::CWrapImpl>();
	pWrap->m_Parent = this;
	m_Wrap = pWrap;
}

CExtensionContainerImpl::~CExtensionContainerImpl()
{
	CWrapImpl *pWrap = static_cast<CWrapImpl*>((SGCore::IExtensionContainer*)m_Wrap);
	if (pWrap != NULL)
		pWrap->m_Parent = NULL;
	m_Wrap = NULL;
}

void CExtensionContainerImpl::LoadDefaultExtension(WCHAR *tagName)
{
	SGGML::IXMLArchiveElementPtr xmlEle = GetRootElement(GetAppDoc());
	SGGML::IXMLArchiveElementPtr pElem = FindSingleNode(xmlEle, tagName);
	if (pElem == NULL)
		return;

	SGGML::IXMLArchiveElementListPtr pNodes = FindNodeList(pElem, L"Extension");
	long cnt = GetNodeCount(pNodes);
	for (int i=0;i<cnt;i++)
	{
		SGCore::IExtensionPtr pExt = PersistXML::ImportImplement(GetNode(pNodes, i));
		if (pExt)
			raw_AddExtension(pExt);
	}
}

void CExtensionContainerImpl::ClearExtension()
{
	std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin();
	std::vector<SGCore::IExtensionPtr>::iterator pEItr = m_Extensions.end();
	for (; pItr != pEItr ; pItr++)
	{
		(*pItr)->Stop();
		(*pItr)->FinalExtension((SGCore::IExtensionContainer *)m_Wrap);
	}
	m_Extensions.clear();
}

STDMETHODIMP CExtensionContainerImpl::get_Extension(long Index, SGCore::IExtension **pVal)
{
	m_Extensions[Index]->QueryInterface(SGCore::IID_IExtension, (void **)pVal);
	return S_OK;
}

STDMETHODIMP CExtensionContainerImpl::putref_Extension(long Index, SGCore::IExtension *newVal)
{
	std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin() + Index;
	if (*pItr)
	{
		(*pItr)->Stop();
		(*pItr)->FinalExtension((SGCore::IExtensionContainer *)m_Wrap);
	}
	if (newVal->InitialExtension((SGCore::IExtensionContainer *)m_Wrap))
	{
		newVal->Start();
		(*pItr) = newVal;
	}
	else
		m_Extensions.erase(pItr);

	return S_OK;
}

STDMETHODIMP CExtensionContainerImpl::get_ExtensionCount(long *pVal)
{
	*pVal = (long)m_Extensions.size();
	return S_OK;
}

STDMETHODIMP CExtensionContainerImpl::raw_AddExtension(SGCore::IExtension *Extension)
{
	if (Extension != NULL)
	{
		if (Extension->InitialExtension((SGCore::IExtensionContainer *)m_Wrap))
		{
			Extension->Start();
			m_Extensions.push_back(Extension);
		}
	}
	return S_OK;
}

STDMETHODIMP CExtensionContainerImpl::raw_RemoveExtension(long Index)
{
	std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin() + Index;
	if (*pItr)
	{
		(*pItr)->Stop();
		(*pItr)->FinalExtension((SGCore::IExtensionContainer *)m_Wrap);
	}
	m_Extensions.erase(pItr);
	return S_OK;
}

STDMETHODIMP CExtensionContainerImpl::raw_QueryExtension(GUID *riid, LPVOID *ppvObj)
{
	*ppvObj = NULL;
	std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin();
	std::vector<SGCore::IExtensionPtr>::iterator pEItr = m_Extensions.end();
	for (; pItr != pEItr ; pItr++)
	{
		if ((*pItr)->QueryInterface(*riid, ppvObj) == S_OK)
			return S_OK;
	}
	return S_FALSE;
}

BOOL GetCLSID(IUnknown *pUnk, CLSID *pclsid)
{
	IPersistPtr pPst = pUnk;
	if (pPst == NULL)
		return FALSE;
	pPst->GetClassID(pclsid);
	return TRUE;
}

void CExtensionContainerImpl::LoadSaveXML(DWORD version, 
	SGGML::IXMLArchiveElement *pSource, BOOL bLoad)
{
	if (bLoad)
	{
		SGGML::IXMLArchiveElementListPtr pExts = FindNodeList(FindSingleNode(pSource, L"Extensions"), L"Extension");
		long Cnt = GetNodeCount(pExts);
		for (long i = 0 ; i < Cnt ; i++)
		{
			SGCore::IExtensionPtr pExt = PersistXML::ImportImplement(GetNode(pExts, i));
			CLSID clsid;
			if (!GetCLSID(pExt, &clsid))
				continue;


			BOOL bExist = FALSE;
			std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin();
			std::vector<SGCore::IExtensionPtr>::iterator pEItr = m_Extensions.end();
			for (; pItr != pEItr ; pItr++)
			{
				CLSID eclsid;
				if (GetCLSID((*pItr), &eclsid))
				{
					if (InlineIsEqualGUID(clsid, eclsid))
					{
						(*pItr)->Stop();
						(*pItr)->FinalExtension((SGCore::IExtensionContainer *)m_Wrap);
						if (pExt->InitialExtension((SGCore::IExtensionContainer *)m_Wrap))
						{
							pExt->Start();
							(*pItr) = pExt;
						}
						else
							m_Extensions.erase(pItr);
						bExist = TRUE;
						break;
					}
				}
			}
			if (!bExist)
			{
				if (pExt->InitialExtension((SGCore::IExtensionContainer *)m_Wrap))
				{
					pExt->Start();
					m_Extensions.push_back(pExt);
				}
			}
		}
	}
	else
	{
		if (m_Persisting)
			return;
		m_Persisting = TRUE;
		SGGML::IXMLArchiveElementPtr pExts = CreateElement(pSource, L"Extensions");
		std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin();
		std::vector<SGCore::IExtensionPtr>::iterator pEItr = m_Extensions.end();
		for (; pItr != pEItr ; pItr++)
		{
			IPersistPtr pPst = (*pItr);
			if (pPst)
				PersistXML::ExportImplement(CreateElement(pExts, L"Extension"), pPst);
		}
		m_Persisting = FALSE;
	}
}

void CExtensionContainerImpl::GetAdditionalPages(CArray<CLSID, CLSID> *pClsids)
{
	std::vector<SGCore::IExtensionPtr>::iterator pItr = m_Extensions.begin();
	std::vector<SGCore::IExtensionPtr>::iterator pEItr = m_Extensions.end();
	for (; pItr != pEItr ; pItr++)
	{
		ISpecifyPropertyPagesPtr pPages = (*pItr);
		if (pPages && pPages != ISpecifyPropertyPagesPtr(this))
		{
			CAUUID caGUID;
			if (SUCCEEDED(pPages->GetPages(&caGUID)))
			{
				for (unsigned int i = 0 ; i < caGUID.cElems ; i++)
				{
					if (!InlineIsEqualGUID(caGUID.pElems[i], CLSID_NULL))
						pClsids->Add(caGUID.pElems[i]);
				}
				CoTaskMemFree(caGUID.pElems);
			}
		}
	}
}