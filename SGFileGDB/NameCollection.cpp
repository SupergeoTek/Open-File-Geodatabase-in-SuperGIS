#include "StdAfx.h"
#include "NameCollection.h"

CNameCollection::CNameCollection(void)
{
}

INT_PTR CNameCollection::Add(CString Name)
{
	//AFX_MANAGE_STATE(AfxGetStaticModuleState())
	if (Name.CompareNoCase(_T("SGLayerInformation")) == 0 ||
		Name.CompareNoCase(_T("SGLayerAttachment")) == 0 ||
		Name.CompareNoCase(_T("SGParcelInformation")) == 0 ||
		Name.CompareNoCase(_T("SGRasterInformation")) == 0 ||
		Name.CompareNoCase(_T("SGTopologyInformation")) == 0)
		return -1;
	if (Name.Left(8).CompareNoCase(L"block_")==0 || Name.Left(6).CompareNoCase(L"aux_")==0)
		return -1;
	
	return m_Names.Add(Name);
}

INT_PTR CNameCollection::Find(CString Name)
{
	INT_PTR n=m_Names.GetCount();
	for (INT_PTR i = 0; i < n; i++)
	{
		if (m_Names.GetAt(i).CompareNoCase(Name) == 0)
			return i;
	}
	return -1;
}

STDMETHODIMP CNameCollection::get_NameCount(long *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pVal = (long)m_Names.GetSize();
	return S_OK;
}
STDMETHODIMP CNameCollection::get_Name(long Index, BSTR *pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pVal = m_Names[Index].AllocSysString();
	return S_OK;
}