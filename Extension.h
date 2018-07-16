#ifndef _EXTENSION_H_
#define _EXTENSION_H_

#include <vector>
#include "PersistXMLImpl.h"

class CExtensionContainerImpl : public SGCore::IExtensionContainer
{
public:
	CExtensionContainerImpl();
	virtual ~CExtensionContainerImpl();
	STDMETHOD(get_Extension)(long Index, SGCore::IExtension **pVal);
	STDMETHOD(putref_Extension)(long Index, SGCore::IExtension *newVal);
	STDMETHOD(get_ExtensionCount)(long *pVal);
	STDMETHOD(raw_AddExtension)(SGCore::IExtension *Extension);
	STDMETHOD(raw_RemoveExtension)(long Index);
	STDMETHOD(raw_QueryExtension)(GUID *riid, LPVOID *ppvObj);

protected:
	template <class T>
	static HRESULT WINAPI COMInterfaceEntryFunc(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
	{
		CExtensionContainerImpl* pCtr = (CExtensionContainerImpl*)(T*)pv;
		return pCtr->raw_QueryExtension((GUID*)&riid, ppv);
	}

	class CWrapImpl : public CComObjectRootEx<CComMultiThreadModel>, public SGCore::IExtensionContainer
	{
		friend CExtensionContainerImpl;
	public:
		CWrapImpl() {m_Parent = NULL;};
		virtual ~CWrapImpl() {};

		BEGIN_COM_MAP(CWrapImpl)
			COM_INTERFACE_ENTRY(SGCore::IExtensionContainer)
			COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_Parent)
		END_COM_MAP()

	private:
		SGCore::IExtensionContainer *m_Parent;
	public:
		STDMETHOD(get_Extension)(long Index, SGCore::IExtension **pVal) {return m_Parent->get_Extension(Index, pVal);}
		STDMETHOD(putref_Extension)(long Index, SGCore::IExtension *newVal) {return m_Parent->putref_Extension(Index, newVal);}
		STDMETHOD(get_ExtensionCount)(long *pVal) {return m_Parent->get_ExtensionCount(pVal);}
		STDMETHOD(raw_AddExtension)(SGCore::IExtension *Extension) {return m_Parent->raw_AddExtension(Extension);}
		STDMETHOD(raw_RemoveExtension)(long Index) {return m_Parent->raw_RemoveExtension(Index);}
		STDMETHOD(raw_QueryExtension)(GUID *riid, LPVOID *ppvObj) {return m_Parent->raw_QueryExtension(riid, ppvObj);}
	};

	//CWrapImpl *m_Wrap;
	SGCore::IExtensionContainerPtr m_Wrap;
	void LoadDefaultExtension(BSTR tagName);
	void ClearExtension();

	void LoadSaveXML(DWORD version, 
		SGGML::IXMLArchiveElement *pSource = NULL, 
		BOOL bLoad = TRUE);
public:
	void GetAdditionalPages(CArray<CLSID, CLSID> *pClsids);
private:
	//SGCore::IExtensionContainer* m_pThis;
	BOOL m_Persisting;
protected:
	std::vector<SGCore::IExtensionPtr> m_Extensions;
};

#endif //_EXTENSION_H_