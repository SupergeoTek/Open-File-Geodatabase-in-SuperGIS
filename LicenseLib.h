#ifndef _LICENSELIB_H_
#define _LICENSELIB_H_

#import "C:\Program Files\Common Files\SuperGeo\SGLicense.dll" named_guids

BOOL DemonstrateLicense(SGLicense::ILicense *pLicense, UINT FeatureKey);
SGLicense::ILicensePtr EnsureLicense(UINT FeatureKey, BOOL bOwn = FALSE);

#define DECLARE_CLASSFACTORYLIC(licType) DECLARE_CLASSFACTORY_EX(CComClassFactory2< licType >)

template <UINT dwKey>
class CComClassLicense
{
public:
	CComClassLicense()
	{
//		m_bKeepLic = FALSE;
//		if (s_License == NULL)
//		{
//			SGLicense::ILicensePtr pLic;
//			if (EnsureLicense(dwKey, &pLic, bOwn) && pLic!=NULL)
//			{
//				s_License = pLic.Detach();
//				m_bKeepLic = TRUE;
//			}
//		}
//		else
//		{
//			s_License->AddRef();
//			m_bKeepLic = TRUE;
//		}
	};
	virtual ~CComClassLicense()
	{
//		TRACE("CComClassLicense::~CComClassLicense()\n");
//		if (m_bKeepLic && s_License)
//		{
//			if (s_License->Release()==0)
//				s_License = NULL;
//			m_bKeepLic = FALSE;
//		}
	};

	BOOL EnsureLicense(UINT FeatureKey, SGLicense::ILicense **Lic)
	{
		SGLicense::ILicensePtr pLic = ::EnsureLicense(FeatureKey, FALSE);
		if (pLic)
		{
			pLic->QueryInterface(SGLicense::IID_ILicense, (void **)Lic);
			return TRUE;
		}
		return FALSE;
	}

protected:
	BOOL VerifyLicenseKey(BSTR bstr)
	{
		//USES_CONVERSION;		//ken
		
		if (wcstoul(bstr, NULL, 16) == dwKey)
		{
			SGLicense::ILicensePtr pLic;
			if (EnsureLicense(dwKey, &pLic) && pLic!=NULL)
				return TRUE;
		}
		return FALSE;

//		if (s_License == NULL)
//			return FALSE;
//		return wcstoul(bstr, NULL, 16) == dwKey;
	};
	BOOL GetLicenseKey(DWORD dwReserved, BSTR* pBstr)
	{
		//USES_CONVERSION;	//ken
		CString strTemp;
		strTemp.Format(_T("%08X"), dwKey);
		*pBstr = strTemp.AllocSysString();
		return TRUE;
	};
	BOOL IsLicenseValid()
	{
		SGLicense::ILicensePtr pLic;
		if (EnsureLicense(dwKey, &pLic) && pLic!=NULL)
			return TRUE;
		return FALSE;
//		if (s_License == NULL)
//			return FALSE;
//		if (m_bKeepLic==FALSE)
//		{
//			s_License->AddRef();
//			m_bKeepLic = TRUE;
//		}
//		return TRUE;
	};
//	BOOL m_bKeepLic;
   
	//*static */SGLicense::ILicensePtr s_License;
};

//template <UINT dwKey, BOOL bOwn>
//SGLicense::ILicense *CComClassLicense<dwKey, bOwn>::s_License = NULL;


#endif//_LICENSELIB_H_