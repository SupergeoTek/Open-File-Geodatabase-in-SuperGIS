#include "stdafx.h"
#include "XML.h"
#include <Limits>

extern double Q_NAN = std::numeric_limits<double>::quiet_NaN();

IStreamPtr StreamFromXMLDoc(SGGML::IXMLArchive *pArchive)
{
	if (pArchive==NULL)
		return NULL;
	IStreamPtr spStream;
	if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &spStream)))
		return NULL;
	SaveDocument(pArchive, CComVariant((IStream *)spStream), FALSE);
	return spStream;
}

CString GetModuleFilePath(HMODULE hModule)
{
    // Get the executable file path
    TCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(hModule, szFilePath, _MAX_PATH);

	TCHAR Drive[_MAX_DRIVE];
	TCHAR Path[_MAX_PATH];
	TCHAR Filename[_MAX_PATH];
	TCHAR Extname[_MAX_PATH];
	_tsplitpath_s(szFilePath, Drive, _MAX_DRIVE, Path, _MAX_PATH, 
			Filename, _MAX_PATH, Extname, _MAX_PATH);

	CString str = Drive;
	str += Path;
	return str;
}
CString GetModuleFileName(HMODULE hModule)
{
    // Get the executable file path
    TCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(hModule, szFilePath, _MAX_PATH);

	TCHAR Drive[_MAX_DRIVE];
	TCHAR Path[_MAX_PATH];
	TCHAR Filename[_MAX_PATH];
	TCHAR Extname[_MAX_PATH];
	_tsplitpath_s(szFilePath, Drive, _MAX_DRIVE, Path, _MAX_PATH, 
		Filename, _MAX_PATH, Extname, _MAX_PATH);

	return Filename;
}

SGGML::IXMLArchivePtr LoadModuleConfig(HINSTANCE hInst)
{
	CString mdlPath = GetModuleFilePath(hInst);
	CString mdlName = GetModuleFileName(hInst);
	_variant_t varString = CT2W(mdlPath + mdlName + _T(".xml"));
	SGGML::IXMLArchivePtr pAppDoc = NewDocument();
	LoadDocument(pAppDoc, varString);
	return pAppDoc;
}

SGGML::IXMLArchivePtr NewDocument()
{
	SGGML::IXMLArchivePtr pArchive;
	pArchive.CreateInstance(MSXML2::CLSID_DOMDocument);
	if (pArchive)
		pArchive->put_async(FALSE);
	return pArchive;
}

SGGML::IXMLArchivePtr CreateDocument(CComBSTR RootName, LPCWSTR strNamespace)
{
	MSXML2::IXMLDOMDocumentPtr pArchive = NewDocument();
	if (pArchive==NULL)
		return NULL;
	MSXML2::IXMLDOMProcessingInstructionPtr pProcInst;
	pArchive->raw_createProcessingInstruction(L"xml", L"version='1.0' encoding='UTF-8'", &pProcInst);
	if(pProcInst)
		pArchive->raw_appendChild( pProcInst, NULL);

	MSXML2::IXMLDOMElementPtr pElem;
	if (strNamespace!=NULL && wcslen(strNamespace)>0)
	{
		CString strTemp = RootName;
		int idx = strTemp.Find(':');
		CComBSTR xmlns = L"xmlns";
		if (idx >= 0)
			xmlns = CString("xmlns:") + strTemp.Left(idx);

		pArchive->raw_createElement(RootName, &pElem);		
		AddAttribute(pElem, xmlns, (LPWSTR)strNamespace);

//		MSXML2::IXMLDOMNodePtr pNode;
//		pArchive->raw_createNode(_variant_t((long)MSXML2::NODE_ELEMENT), RootName, strNamespace, &pNode);
//		pElem = pNode;
	}
	else
		pArchive->raw_createElement(RootName, &pElem);
	pArchive->putref_documentElement(pElem);
	return pArchive;
}

SGGML::IXMLArchiveElementPtr GetRootElement(SGGML::IXMLArchive* pDoc)
{
	if (pDoc==NULL)
		return NULL;
	SGGML::IXMLArchiveElementPtr pElem;
	pDoc->get_documentElement(&pElem);
	return pElem;
}

BOOL LoadDocument(SGGML::IXMLArchive* pDoc, VARIANT var)
{
	if (pDoc==NULL)
		return FALSE;
	VARIANT_BOOL result;
	pDoc->raw_load(var, &result);
	return result;
}

void SaveDocument(SGGML::IXMLArchive* pDoc, VARIANT var, BOOL indent, BSTR encode)
{
	if (pDoc==NULL)
		return;
	if (indent)
	{
		MSXML2::IMXWriterPtr wrt;
		wrt.CreateInstance(MSXML2::CLSID_MXXMLWriter);
		MSXML2::ISAXXMLReaderPtr rdr;
		rdr.CreateInstance(MSXML2::CLSID_SAXXMLReader);
		CComPtr<IStream> pStream;
		CreateStreamOnHGlobal(NULL, TRUE, &pStream);

		wrt->put_output(CComVariant(pStream));
		wrt->put_indent(-1);
		wrt->put_encoding(encode);
		rdr->putContentHandler(MSXML2::ISAXContentHandlerPtr(wrt));
		rdr->raw_parse(_variant_t((IUnknown *)pDoc));
		wrt->flush();

		LARGE_INTEGER pos = {0,0};
		pStream->Seek(pos, STREAM_SEEK_SET, NULL);
		VARIANT_BOOL result;
		pDoc->raw_load(CComVariant(pStream), &result);
	}
	pDoc->raw_save(var);
}

CComBSTR ConfirmNamespace(SGGML::IXMLArchive* pDoc, LPCWSTR strNamespace, BOOL bCreate, LPCWSTR strURI)
{
	if (pDoc == NULL || strNamespace == NULL || wcslen(strNamespace) == 0)
		return NULL;
	SGGML::IXMLArchiveElementPtr pElem = GetRootElement(pDoc);
	CComBSTR xmlns = GetNodeAttributeB(pElem, L"xmlns");
	if (xmlns != NULL && wcscmp(xmlns, strNamespace)==0)
		return L"";

	CComBSTR nsuri;
	nsuri = L"@xmlns:*[. = \"";
	nsuri += strNamespace;
	nsuri += "\"]";
	MSXML2::IXMLDOMNodePtr pNode2;
	pElem->raw_selectSingleNode(nsuri, &pNode2);
	if (pNode2!=NULL)
	{
		CComBSTR bstr;
		pNode2->get_baseName(&bstr);
		if (bstr.Length() != 0)
			return bstr;
	}
	else if (bCreate)
	{
		const WCHAR *pChr = strURI;
		if (pChr == NULL)
		{
			pChr = strNamespace + wcslen(strNamespace);
			if (pChr[1] == L'\0')
				pChr = wcsrchr(strNamespace, L'/');
			if (pChr!=NULL)
				pChr++;
		}
		if (pChr!=NULL)
		{
			_bstr_t bstr = pChr;
			_bstr_t ns = L"xmlns:";
			AddAttribute(pElem, BSTR(ns + bstr), CComBSTR(strNamespace));
			return (BSTR)bstr;
		}
	}
	return NULL;
}

CComBSTR QueryNamespace(SGGML::IXMLArchive* pDoc, LPCWSTR strURI)
{
	if (strURI == NULL)
		return NULL;
	SGGML::IXMLArchiveElementPtr pElem = GetRootElement(pDoc);
	CComBSTR nsuri;
	nsuri = L"@xmlns:";
	nsuri += strURI;
	MSXML2::IXMLDOMNodePtr pNode2;
	pElem->raw_selectSingleNode(nsuri, &pNode2);
	if (pNode2 != NULL)
	{
		CComBSTR bstr;
		pNode2->get_text(&bstr);
		return bstr;
	}
	return NULL;
}

CComBSTR AppendNodePrefix(SGGML::IXMLArchive* pDoc, LPCWSTR strNamespace, BSTR strTag, BOOL bCreate)
{
	CComBSTR bstr = ConfirmNamespace(pDoc, strNamespace, bCreate);
	if (bstr.Length()!=0)
		return BSTR(bstr + _bstr_t(L":") + strTag);
	return strTag;
}

CComBSTR TruncateNodePrefix(BSTR strTag, SGGML::IXMLArchive* pDoc, LPCWSTR strNamespace)
{
	if (pDoc == NULL || strNamespace == NULL)
	{
		WCHAR *pBuf = wcschr((WCHAR *)strTag, L':');
		if (pBuf)
			return pBuf + 1;
	}
	else
	{
		CComBSTR bstr = ConfirmNamespace(pDoc, strNamespace, FALSE);
		if (bstr.Length()!=0)
			if (wcsncmp(bstr, strTag, bstr.Length()) == 0)
				if (((WCHAR *)strTag)[bstr.Length()] == L':')
					return ((WCHAR *)strTag) + bstr.Length() + 1;
	}
	return strTag;
}

CComBSTR InterceptNodePrefix(BSTR strTag, SGGML::IXMLArchive* pDoc)
{
	WCHAR *pChr = wcschr(strTag, L':');
	if (pChr == NULL)
		return NULL;

	int len = (int)(pChr - (WCHAR *)strTag);
	WCHAR *strNamespace = new WCHAR[len + 1];
	wcsncpy_s(strNamespace, len + 1, (WCHAR *)strTag, len);
	strNamespace[len] = L'\0';
	CComBSTR strURI = strNamespace;
	delete strNamespace;
	if (pDoc == NULL)
		return strURI;
	else
	{
		CComBSTR nsuri;
		nsuri = L"@xmlns:";
		nsuri += strURI;
		SGGML::IXMLArchiveElementPtr pElem = GetRootElement(pDoc);
		MSXML2::IXMLDOMNodePtr pNode2;
		pElem->raw_selectSingleNode(nsuri, &pNode2);
		if (pNode2!=NULL)
			return strURI;
	}
	return NULL;
}

SGGML::IXMLArchiveElementPtr CreateNode(SGGML::IXMLArchive *pDoc, BSTR strTag)
{
	MSXML2::IXMLDOMElementPtr pElem;
	if (pDoc)
	{
		pDoc->raw_createElement((BSTR)strTag, &pElem);
	}
	return pElem;
}

SGGML::IXMLArchiveElementPtr CreateElement(SGGML::IXMLArchiveElement* pSource, CComBSTR strTag, LPCWSTR strNamespace)
{
	if (pSource!=NULL)
	{
		MSXML2::IXMLDOMDocumentPtr pDoc;
		pSource->get_ownerDocument(&pDoc);
		CComBSTR bstr = AppendNodePrefix(pDoc, strNamespace, strTag, TRUE);
		MSXML2::IXMLDOMElementPtr pElem = CreateNode(pDoc, bstr);
		pSource->raw_appendChild(pElem, NULL);
		return pElem;
		//return pSource->CreateChild((BSTR)strTag, strCharacter);
	}
	return NULL;
}

void SetNodeText(SGGML::IXMLArchiveElement* pNode, BSTR bstr)
{
	if (pNode==NULL || bstr==NULL)
		return;
	pNode->put_text(bstr);
}

void SetNodeCDataText(SGGML::IXMLArchiveElement* pNode, BSTR bstr)
{
	if (pNode==NULL || bstr==NULL)
		return;
	MSXML2::IXMLDOMDocumentPtr pDoc = pNode->ownerDocument;
	MSXML2::IXMLDOMCDATASectionPtr pCData = pDoc->createCDATASection(bstr);
	pNode->raw_appendChild(pCData, NULL);
}

void SetNodeVARIANT(SGGML::IXMLArchiveElement* pNode, VARIANT *var)
{
	if (pNode==NULL || var==NULL)
		return;

	AddAttribute(pNode, L"type", FromInteger((long)V_VT(var)));
	CComBSTR bstr;
	if ((V_VT(var)&VT_ARRAY) == VT_ARRAY)
	{
		SAFEARRAY *psa = V_ARRAY(var);
		VARTYPE VT;
		SafeArrayGetVartype(psa, &VT);
		AddAttribute(pNode, L"safearraytype", FromInteger((long)VT));

		IStreamPtr pStream;
		CreateStreamOnHGlobal(NULL, TRUE, &pStream);
		if (pStream == NULL)
			return;

		long sz = 1;
		SGGML::IXMLArchiveElementPtr pNodeDims = CreateElement(pNode, L"Dims");
		for (long i = 0 ; i < psa->cDims ; i++)
		{
			sz *= psa->rgsabound[i].cElements;
			SGGML::IXMLArchiveElementPtr pNodeDim = CreateElement(pNodeDims, L"Dim");
			AddAttribute(pNodeDim, L"Elements", FromInteger(psa->rgsabound[i].cElements));
			AddAttribute(pNodeDim, L"Lbound", FromInteger(psa->rgsabound[i].lLbound));
		}
		void *pVoid;
		SafeArrayAccessData(psa, &pVoid);
		pStream->Write(pVoid, sz * psa->cbElements, NULL);
		SafeArrayUnaccessData(psa);
		SetNodeText(CreateElement(pNode, L"Data"), EncodeStream(pStream));
		//void *pVoid;
		//SafeArrayAccessData(psa, &pVoid);
		//pStream->Write(pVoid, psa->rgsabound->cElements * psa->cbElements, NULL);
		//SafeArrayUnaccessData(psa);
		//SetNodeText(pNode, EncodeStream(pStream));
	}
	else if (V_VT(var) != VT_NULL)
		SetNodeText(pNode, FromVariant(var));
}

GUID GetGUID(CString strGUID, GUID defaultvalue)
{
	if (strGUID.IsEmpty())
		return defaultvalue;
	GUID guid;
	BSTR bstr = strGUID.AllocSysString();
	HRESULT  hr = CLSIDFromString(bstr, &guid);
	SysFreeString(bstr);
	if (hr == NOERROR)
		return guid;
	return defaultvalue;
}

COLORREF GetColor(CString strColor, COLORREF defaultvalue)
{
	if (strColor.IsEmpty())
		return defaultvalue;
	if (strColor.GetAt(0) == '#')
	{
		int nColor = _tcstoul(strColor.Mid(1),NULL,16);
		return (nColor&0xFF000000) | RGB(GetBValue(nColor),GetGValue(nColor),GetRValue(nColor));
	}
	else if (strColor.GetLength()>1 && strColor.GetAt(0) == '0')
	{
		if (strColor.GetAt(1) == 'x')
			return _tcstoul(strColor, NULL, 16);
		else
			return _tcstoul(strColor, NULL, 8);
	}
	return _tcstoul(strColor, NULL, 10);
}

long GetInteger(CString strInteger, long defaultvalue)
{
	if (strInteger.IsEmpty())
		return defaultvalue;
	if (_tcsncmp(strInteger, _T("0x"), 2)==0 || _tcsncmp(strInteger, _T("#"), 1)==0)
		return _tcstoul(strInteger.Mid(2), NULL, 16);
	else if (_tcsncmp(strInteger, _T("0"), 1)==0 && strInteger.GetLength()>1)
		return _tcstoul(strInteger.Mid(1), NULL, 8);
	return _tcstol(strInteger, NULL ,10);
}

double GetDouble(CString strDouble, double defaultvalue)
{
	if (strDouble.IsEmpty())
		return defaultvalue;
	_variant_t var((BSTR)_bstr_t(strDouble));
	try
	{
		if (VariantChangeTypeEx(&var, &var, 
			MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT ), 
			VARIANT_NOVALUEPROP, VT_R8) == S_OK)
			return V_R8(&var);
		//var.ChangeType(VT_R8);
		//return _tcstod(strDouble, NULL);
	}
	catch(_com_error& /*e*/)
	{
	}
	return defaultvalue;
	//if (VariantChangeType(&var, &var, VARIANT_NOVALUEPROP, VT_R8) == S_OK)
	//	return V_R8(&var);
	//return _tcstod(strInteger, NULL);
}

BOOL GetBoolean(CString strBool, BOOL defaultvalue)
{
	if (strBool.IsEmpty())
		return defaultvalue;
	if (strBool.CompareNoCase(_T("TRUE"))==0)
		return TRUE;
	else if (strBool.CompareNoCase(_T("FALSE"))==0)
		return FALSE;
	return GetInteger(strBool, defaultvalue);
}

VARIANT_BOOL GetVBoolean(CString strBool, VARIANT_BOOL defaultvalue)
{
	if (GetBoolean(strBool, defaultvalue ? TRUE : FALSE))
		return VARIANT_TRUE;
	return VARIANT_FALSE;
}

_bstr_t FromGUID(GUID value)
{
	LPOLESTR str;
	StringFromCLSID(value, &str);
	_bstr_t sGUID = str;
	CoTaskMemFree(str);
	return sGUID;
}

_bstr_t FromColor(COLORREF value)
{
	CString strTemp;
	if (value & 0xFF000000)
		strTemp.Format(_T("#%X%02X%02X%02X"),value >> 24,GetRValue(value),GetGValue(value),GetBValue(value));
	else
		strTemp.Format(_T("#%02X%02X%02X"),GetRValue(value),GetGValue(value),GetBValue(value));
	return (BSTR)CT2W(strTemp);
}

_bstr_t FromInteger(long value, long base)
{
	CString strTemp;
	if (base == 16)
		strTemp.Format(_T("0x%X"), value);
	else
		strTemp.Format(_T("%d"), value);
	return (BSTR)CT2W(strTemp);
}

_bstr_t FromDouble(double value)
{
	_variant_t var(value);
	return FromVariant(&var);
}

_bstr_t FromBoolean(BOOL value)
{
	return value?L"True":L"False";
}

_bstr_t FromVariant(VARIANT *value)
{
	_variant_t var;
	VariantChangeTypeEx(&var, value, 
		MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT ), 
		VARIANT_NOVALUEPROP, VT_BSTR);
	return V_BSTR(&var);
}

void AddAttribute(SGGML::IXMLArchiveElement* pSource, CComBSTR Name, BSTR value, LPCWSTR strNamespace)
{
	if (pSource!=NULL)
	{
		MSXML2::IXMLDOMDocumentPtr pDoc;
		pSource->get_ownerDocument(&pDoc);
		CComBSTR bstr = AppendNodePrefix(pDoc, strNamespace, Name, TRUE);
		pSource->raw_setAttribute(bstr, _variant_t(value));
	}
}

/*
long GetNodeCount(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName)
{
	if (pNode)
	{
		MSXML2::IXMLDOMNodeListPtr pNodeList;
		pNode->raw_selectNodes(lpName, &pNodeList);
		if (pNodeList)
			return pNodeList->Getlength();
	}
	return 0;
}

SGGML::IXMLArchiveElementPtr FindNode(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName, long nIndex)
{
	if (pNode)
	{
		MSXML2::IXMLDOMNodeListPtr pNodeList;
		pNode->raw_selectNodes(lpName, &pNodeList);
		if (pNodeList)
		{
			if (pNodeList->Getlength() > nIndex)
			{
				MSXML2::IXMLDOMNodePtr pNode2;
				pNodeList->get_item(nIndex, &pNode2);
				return pNode2;
			}
		}
	}
	return NULL;
}
*/

SGGML::IXMLArchiveElementPtr GetNode(SGGML::IXMLArchiveElementList* pNodes, long nIndex)
{
	if (pNodes)
	{
		if (pNodes->Getlength() > nIndex)
		{
			MSXML2::IXMLDOMNodePtr pNode;
			pNodes->get_item(nIndex, &pNode);
			return pNode;
			//SGGML::IXMLArchiveElementPtr pNode2;
			//pNodes->get_Element(nIndex, &pNode2);
			//return pNode2;
		}
	}
	return NULL;
}

long GetNodeCount(SGGML::IXMLArchiveElementList* pNodes)
{
	if (pNodes)
	{
		long cnt;
		pNodes->get_length(&cnt);
		return cnt;
	}
	return 0;
}

SGGML::IXMLArchiveElementPtr FindSingleNode(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName, LPCWSTR strNamespace)
{
	if (pNode)
	{
		MSXML2::IXMLDOMDocumentPtr pDoc;
		pNode->get_ownerDocument(&pDoc);
		MSXML2::IXMLDOMNodePtr pNode2;
		pNode->raw_selectSingleNode(AppendNodePrefix(pDoc, strNamespace, lpName, FALSE), &pNode2);
		return pNode2;
	}
	return NULL;
}

SGGML::IXMLArchiveElementListPtr FindNodeList(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName, LPCWSTR strNamespace)
{
	if (pNode)
	{
		MSXML2::IXMLDOMDocumentPtr pDoc;
		pNode->get_ownerDocument(&pDoc);
		SGGML::IXMLArchiveElementListPtr pNodeList;
		pNode->raw_selectNodes(AppendNodePrefix(pDoc, strNamespace, lpName, FALSE), &pNodeList);
		//pNode->raw_FindChilds(lpName, &pNodeList);
		return pNodeList;
	}
	return NULL;
}

SGGML::IXMLArchiveElementListPtr GetChilds(SGGML::IXMLArchiveElement* pNode)
{
	if (pNode)
	{
		SGGML::IXMLArchiveElementListPtr pNodeList;
		pNode->get_childNodes(&pNodeList);
		//pNode->get_Childs(&pNodeList);
		return pNodeList;
	}
	return NULL;
}

CComBSTR GetNodeAttributeB(SGGML::IXMLArchiveElement* pNode, CComBSTR lpAttr, LPCWSTR strNamespace)
{
	CComBSTR strResult;
	if (pNode)
	{
		MSXML2::IXMLDOMDocumentPtr pDoc;
		pNode->get_ownerDocument(&pDoc);
		CComBSTR bstr = AppendNodePrefix(pDoc, strNamespace, lpAttr, FALSE);
		_variant_t val;
		pNode->raw_getAttribute(bstr, &val);
		if (V_VT(&val)==VT_BSTR)
		{
			strResult = V_BSTR(&val);
			val.Clear();
		}
	}
	return strResult;
}

CString GetNodeAttribute(SGGML::IXMLArchiveElement* pNode, CComBSTR lpAttr, LPCWSTR strNamespace)
{
	return CString(GetNodeAttributeB(pNode,lpAttr, strNamespace));
}

CComBSTR GetNodeTagB(SGGML::IXMLArchiveElement* pNode)
{
	CComBSTR bstr;
	pNode->get_baseName(&bstr);
	return bstr;
}

CString GetNodeTag(SGGML::IXMLArchiveElement* pNode)
{
	CString strResult = GetNodeTagB(pNode);
	return strResult;
}

CComBSTR GetNodeTextB(SGGML::IXMLArchiveElement* pNode)
{
	CComBSTR bstr;
	if (pNode)
		pNode->get_text(&bstr);
	return bstr;
}

CString GetNodeText(SGGML::IXMLArchiveElement* pNode)
{
	CString strResult = GetNodeTextB(pNode);
	return strResult;
}

void GetNodeTextVARIANT(SGGML::IXMLArchiveElement* pNode, VARIANT *var)
{
	long vt = GetInteger(GetNodeAttribute(pNode, L"type"), VT_BSTR);
	V_VT(var) = (VARTYPE)vt; //.ChangeType(vt);

	if ((vt & VT_ARRAY) == VT_ARRAY)
	{
		VARTYPE VT = (VARTYPE)GetInteger(GetNodeAttribute(pNode, L"safearraytype"), vt & ~VT_ARRAY);

		SGGML::IXMLArchiveElementPtr pNodeData = FindSingleNode(pNode, L"Data");
		IStreamPtr pStream = DecodeStream(GetNodeTextB(pNodeData == NULL ? pNode : pNodeData));
		HGLOBAL hGlob;
		GetHGlobalFromStream(pStream, &hGlob);

		SAFEARRAY *psa = NULL;
		unsigned long sz = 0;
		if (pNodeData)
		{
			SGGML::IXMLArchiveElementListPtr pNodeDims = FindNodeList(pNode, L"Dims/Dim");
			sz = 1;
			long cDims = GetNodeCount(pNodeDims);
			SAFEARRAYBOUND *rgsabound = new SAFEARRAYBOUND[cDims];
			for (long i = 0 ; i < cDims ; i++)
			{
				SGGML::IXMLArchiveElementPtr pNodeDim = GetNode(pNodeDims, i);
				rgsabound[i].cElements = GetInteger(GetNodeAttribute(pNodeDim, L"Elements"), 0);
				rgsabound[i].lLbound = GetInteger(GetNodeAttribute(pNodeDim, L"Lbound"), 0);
				sz *= rgsabound[i].cElements;
			}
			psa = SafeArrayCreate(VT, cDims, rgsabound);
			delete [] rgsabound;
		}
		else
		{
			psa = SafeArrayCreateVector(VT, 0, (ULONG)1);
			sz = (unsigned long)(GlobalSize(hGlob) / psa->cbElements);
			SafeArrayDestroy(psa);
			psa = SafeArrayCreateVector(VT, 0, (ULONG)sz);
		}

		void *buf  = GlobalLock(hGlob);
		void *buf2;
		SafeArrayAccessData(psa, &buf2);
		memcpy(buf2, buf, sz * psa->cbElements);
		SafeArrayUnaccessData(psa);
		GlobalUnlock(hGlob);
		V_ARRAY(var) = psa;
	}
	else if (vt != VT_NULL)
	{
		switch (vt)
		{
		case VT_BSTR:
			V_BSTR(var) = GetNodeTextB(pNode).Copy();
			break;
		case VT_UI1: case VT_UI2: case VT_UI4:
		case VT_I1: case VT_I2: case VT_I4:
			V_I4(var) = GetInteger(GetNodeText(pNode), 0);
			break;
		case VT_R4:
			V_R4(var) = (float)GetDouble(GetNodeText(pNode), Q_NAN);
			break;
		case VT_R8:
			V_R8(var) = GetDouble(GetNodeText(pNode), Q_NAN);
			break;
		default:
			{
				_variant_t tvar = GetNodeTextB(pNode);
				tvar.ChangeType((VARTYPE)vt);
				VariantCopy(var, &tvar) ;
			}
			break;
		};
	}
}

CComBSTR EncodeStream(IStream* pStream)
{
	//Base64½s½X
	HGLOBAL hGlob;
	GetHGlobalFromStream(pStream, &hGlob);
	SIZE_T dwSize = GlobalSize(hGlob);
	BYTE *buf  = (BYTE *)GlobalLock(hGlob);
	
	SIZE_T dwSize2 = Base64Encoder(buf, dwSize, NULL, 0);

	//HGLOBAL hBuffer = ::GlobalAlloc(GHND, (dwSize2 + 1) * sizeof(TCHAR));
	TCHAR *buf2 = new TCHAR[dwSize2 + 1]; //(TCHAR *)GlobalLock(hBuffer);
	memset(buf2, 0, (dwSize2 + 1) * sizeof(TCHAR));

	Base64Encoder(buf, dwSize, buf2, dwSize2);

	//¿é¥XXML
	CComBSTR buf3(buf2);
	delete [] buf2;
//	GlobalUnlock(hBuffer);
//	GlobalFree(hBuffer);

	GlobalUnlock(hGlob);

	return buf3;
}

IStreamPtr DecodeStream(CComBSTR bstr)
{
	CString str = bstr;

	DWORD dwSize = str.GetLength();
	TCHAR *buf = str.GetBuffer(dwSize);
	SIZE_T dwSize2 = Base64Decoder(buf, dwSize, NULL, 0);

	HGLOBAL hBuffer = ::GlobalAlloc(GHND, dwSize2);
	PBYTE pBuf = (PBYTE)GlobalLock(hBuffer);
	Base64Decoder(buf, dwSize, (PBYTE)pBuf, dwSize2);
	GlobalUnlock(hBuffer);
	str.ReleaseBuffer();

	IStreamPtr pStream;
	CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);
	return pStream;
}


//0x41-0x5A,0x61-0x7A,0x30-0x39,0x2B,0x2F,0x3D
static unsigned char cBase64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
static unsigned char Lookup64Char[] = {
0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0, 0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0, 0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0, 0xC0,0xC0,0xC0,0x3E,0xC0,0xC0,0xC0,0x3F,
0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B, 0x3C,0x3D,0xC0,0xC0,0x40,0xC0,0xC0,0xC0,
0xC0,0x00,0x01,0x02,0x03,0x04,0x05,0x06, 0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16, 0x17,0x18,0x19,0xC0,0xC0,0xC0,0xC0,0xC0,
0xC0,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20, 0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30, 0x31,0x32,0x33,0xC0,0xC0,0xC0,0xC0,0xC0,
};

#define BASE64_t2	0x30 //00110000
#define BASE64_m2	0x0c //00001100
#define BASE64_b2	0x03 //00000011
#define BASE64_t4	0x3c //00111100
#define BASE64_b4	0x0f //00001111
#define BASE64_h2	0xc0 //11000000
#define BASE64_b6	0x3f //00111111

SIZE_T Base64Decoder(TCHAR *DataIn, SIZE_T DataLength, PBYTE DataOut, SIZE_T DataOutLength)
{
	TCHAR *DataInEnd = DataIn + DataLength;
	while (DataInEnd > DataIn && *DataInEnd == _T('='))
		DataInEnd--;
	DataLength = DataInEnd - DataIn;

	SIZE_T lOutputBufferLength = (DataLength * 3) / 4;
	if (DataOut == NULL || DataOutLength < lOutputBufferLength)
		return lOutputBufferLength;

	PBYTE DataOutEnd = DataOut;
	while (DataIn < DataInEnd)
	{
		unsigned char a = Lookup64Char[*DataIn++] & 0x3F;				// 00aaaaaa

		// first 64
		if (DataIn >= DataInEnd)
			break;
		unsigned char b = Lookup64Char[*DataIn++] & 0x3F;				// 00bbbbbb
		*DataOutEnd++ = (a << 2) + (b >> 4);		// aaaaaa00 + 000000bb = aaaaaabb

		// second 64
		if (DataIn >= DataInEnd)
			break;
		unsigned char c = Lookup64Char[*DataIn++] & 0x3F;				// 00cccccc
		*DataOutEnd++ = (b << 4) + (c >> 2);		// bbbb0000 + 0000cccc = bbbbcccc

		// third 64
		if (DataIn >= DataInEnd)
			break;
		unsigned char d = Lookup64Char[*DataIn++] & 0x3F;				// 00dddddd
		*DataOutEnd++ = (c << 6) + d;			// cc000000 + 00dddddd = ccdddddd
	}
	return DataOutEnd - DataOut;
}

SIZE_T Base64Encoder(PBYTE DataIn, SIZE_T DataLength, TCHAR *DataOut, SIZE_T DataOutLength)
{
	SIZE_T lOutputBufferLength;
	SIZE_T n;
	SIZE_T iGroupsOf3;
	SIZE_T iLeftover;

	iGroupsOf3 = DataLength / 3;
	iLeftover = 3 - (DataLength % 3);

	lOutputBufferLength = iGroupsOf3;
	if (iLeftover!=3)
		lOutputBufferLength++;
	lOutputBufferLength *= 4;

	if (DataOut && DataOutLength >= lOutputBufferLength)
	{
		unsigned char a;
		unsigned char b;
		unsigned char c;
		unsigned char r;

		TCHAR *pBuffer;
		pBuffer = DataOut;

		// encode
		for (n = 0; n < (iGroupsOf3 * 3); n += 3)
		{
			a = DataIn[n + 0];				// aaaaaabb
			b = DataIn[n + 1];				// bbbbcccc
			c = DataIn[n + 2];				// ccdddddd

			// first character
			r = a;							// aaaaaabb
			r = r >> 2;						// 00aaaaaa
			*pBuffer++ = cBase64Alphabet[r];

			// second character
			r = a;							// aaaaaabb
			r = r & BASE64_b2;				// 000000bb
			r = r << 4;						// 00bb0000
			r = r + (b >> 4);				// 00bb0000 + 0000bbbb = 00bbbbbb
			*pBuffer++ = cBase64Alphabet[r];

			// third character
			r = b;							// bbbbcccc
			r = b << 2;						// bbcccc00
			r = r & BASE64_t4;				// 00cccc00
			r = r + (c >> 6);				// 00cccc00 + 000000cc = 00cccccc
			*pBuffer++ = cBase64Alphabet[r];

			// fourth character
			r = c;							// ccdddddd
			r = r & BASE64_b6;				// 00dddddd
			*pBuffer++ = cBase64Alphabet[r];
		}

		// handle non multiple of 3 data and insert padding
		if (iLeftover)
		{
			n = (iGroupsOf3 * 3);
			switch (iLeftover)
			{
			case 2:
				a = DataIn[n + 0];				// aaaaaabb
				// first character
				r = a;							// aaaaaabb
				r = r >> 2;						// 00aaaaaa
				*pBuffer++ = cBase64Alphabet[r];
				// second character
				r = a;							// aaaaaabb
				r = r & BASE64_b2;				// 000000bb
				r = r << 4;						// 00bb0000
				*pBuffer++ = cBase64Alphabet[r];
				// insert padding x 2
				*pBuffer++ = cBase64Alphabet[64];
				*pBuffer++ = cBase64Alphabet[64];
				break;
			case 1:
				a = DataIn[n + 0];				// aaaaaabb
				b = DataIn[n + 1];				// bbbbcccc
				// first character
				r = a;							// aaaaaabb
				r = r >> 2;						// 00aaaaaa
				*pBuffer++ = cBase64Alphabet[r];
				// second character
				r = a;							// aaaaaabb
				r = r & BASE64_b2;				// 000000bb
				r = r << 4;						// 00bb0000
				r = r + (b >> 4);				// 00bbbbbb
				*pBuffer++ = cBase64Alphabet[r];
				// third character
				r = b;							// bbbbcccc
				r = b << 2;						// bbcccc00
				r = r & BASE64_t4;				// 00cccc00
				*pBuffer++ = cBase64Alphabet[r];
				// insert padding
				*pBuffer++ = cBase64Alphabet[64];
				break;
			}
		}
	}

	return lOutputBufferLength;
}
