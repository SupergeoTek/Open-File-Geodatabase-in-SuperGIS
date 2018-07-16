#pragma once

class CNameCollection :
	public CComObjectRootEx<CComMultiThreadModel>,
	public SGCore::INameCollection
{
public:
	CNameCollection(void);

BEGIN_COM_MAP(CNameCollection)
	COM_INTERFACE_ENTRY(INameCollection)
END_COM_MAP()

protected:
	CStringArray m_Names;
public:
	INT_PTR Add(CString Name);
	INT_PTR Find(CString Name);

public:
	STDMETHOD(get_NameCount)(long *pVal);
	STDMETHOD(get_Name)(long Index, BSTR *pVal);
};
