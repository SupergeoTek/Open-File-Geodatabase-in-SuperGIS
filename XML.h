#ifndef _XML_H_
#define _XML_H_

#import "msxml3.dll" named_guids

extern double Q_NAN;// = std::numeric_limits<double>::quiet_NaN();

namespace SGGML
{
	typedef MSXML2::IXMLDOMElement IXMLArchiveElement;
	typedef MSXML2::IXMLDOMElementPtr IXMLArchiveElementPtr;
	typedef MSXML2::IXMLDOMNodeList IXMLArchiveElementList;
	typedef MSXML2::IXMLDOMNodeListPtr IXMLArchiveElementListPtr;
	typedef MSXML2::IXMLDOMDocument IXMLArchive;
	typedef MSXML2::IXMLDOMDocumentPtr IXMLArchivePtr;
}
IStreamPtr StreamFromXMLDoc(SGGML::IXMLArchive *pArchive);

CString GetModuleFilePath(HMODULE hModule);
CString GetModuleFileName(HMODULE hModule);
SGGML::IXMLArchivePtr LoadModuleConfig(HINSTANCE hInst);

SGGML::IXMLArchivePtr NewDocument();
SGGML::IXMLArchivePtr CreateDocument(CComBSTR RootName, LPCWSTR strNamespace = NULL);
SGGML::IXMLArchiveElementPtr GetRootElement(SGGML::IXMLArchive* pDoc);
BOOL LoadDocument(SGGML::IXMLArchive* pDoc, VARIANT var);
void SaveDocument(SGGML::IXMLArchive* pDoc, VARIANT var, BOOL indent = FALSE, BSTR encode = L"UTF-8");

CComBSTR ConfirmNamespace(SGGML::IXMLArchive* pDoc, LPCWSTR strNamespace, BOOL bCreate, LPCWSTR strURI = NULL);
CComBSTR QueryNamespace(SGGML::IXMLArchive* pDoc, LPCWSTR strURI);
CComBSTR AppendNodePrefix(SGGML::IXMLArchive* pDoc, LPCWSTR strNamespace, BSTR strTag, BOOL bCreate);
CComBSTR TruncateNodePrefix(BSTR strTag, SGGML::IXMLArchive* pDoc = NULL, LPCWSTR strNamespace = NULL);
CComBSTR InterceptNodePrefix(BSTR strTag, SGGML::IXMLArchive* pDoc = NULL);
SGGML::IXMLArchiveElementPtr CreateNode(SGGML::IXMLArchive *pDoc, BSTR strTag);
SGGML::IXMLArchiveElementPtr CreateElement(SGGML::IXMLArchiveElement* pSource, CComBSTR strTag, LPCWSTR strNamespace = NULL);

void SetNodeText(SGGML::IXMLArchiveElement* pNode, BSTR bstr);
void SetNodeCDataText(SGGML::IXMLArchiveElement* pNode, BSTR bstr);
void SetNodeVARIANT(SGGML::IXMLArchiveElement* pNode, VARIANT *var);

GUID GetGUID(CString strGUID, GUID defaultvalue = GUID_NULL);
COLORREF GetColor(CString strColor, COLORREF defaultvalue);
long GetInteger(CString strInteger, long defaultvalue);
double GetDouble(CString strInteger, double defaultvalue);
BOOL GetBoolean(CString strBool, BOOL defaultvalue);
VARIANT_BOOL GetVBoolean(CString strBool, VARIANT_BOOL defaultvalue);

_bstr_t FromGUID(GUID value);
_bstr_t FromColor(COLORREF value);
_bstr_t FromInteger(long value, long base = 10);
_bstr_t FromDouble(double value);
_bstr_t FromBoolean(BOOL value);
_bstr_t FromVariant(VARIANT *value);
_bstr_t HexFromInteger(long value);

void AddAttribute(SGGML::IXMLArchiveElement* pSource, CComBSTR Name, BSTR value, LPCWSTR strNamespace = NULL);

void ExportEnvelope(SGGML::IXMLArchiveElement* pSource, double XMin, double YMin, double XMax, double YMax);
void ImportEnvelope(SGGML::IXMLArchiveElement* pSource, double *XMin, double *YMin, double *XMax, double *YMax);

//long GetNodeCount(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName);
//SGGML::IXMLArchiveElementPtr FindNode(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName, long nIndex);
SGGML::IXMLArchiveElementPtr FindSingleNode(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName, LPCWSTR strNamespace = NULL);
SGGML::IXMLArchiveElementListPtr FindNodeList(SGGML::IXMLArchiveElement* pNode, CComBSTR lpName, LPCWSTR strNamespace = NULL);
SGGML::IXMLArchiveElementListPtr GetChilds(SGGML::IXMLArchiveElement* pNode);
SGGML::IXMLArchiveElementPtr GetNode(SGGML::IXMLArchiveElementList* pNodes, long nIndex);
long GetNodeCount(SGGML::IXMLArchiveElementList* pNodes);

CComBSTR GetNodeTagB(SGGML::IXMLArchiveElement* pNode);
CString GetNodeTag(SGGML::IXMLArchiveElement* pNode);
CComBSTR GetNodeTextB(SGGML::IXMLArchiveElement* pNode);
CString GetNodeText(SGGML::IXMLArchiveElement* pNode);
void GetNodeTextVARIANT(SGGML::IXMLArchiveElement* pNode, VARIANT *var);

CComBSTR GetNodeAttributeB(SGGML::IXMLArchiveElement* pNode, CComBSTR lpAttr, LPCWSTR strNamespace = NULL);
CString GetNodeAttribute(SGGML::IXMLArchiveElement* pNode, CComBSTR lpAttr, LPCWSTR strNamespace = NULL);

CComBSTR EncodeStream(IStream* pStream);
IStreamPtr DecodeStream(CComBSTR bstr);

SIZE_T Base64Encoder(PBYTE DataIn, SIZE_T DataLength, TCHAR *DataOut, SIZE_T DataOutLength);
SIZE_T Base64Decoder(TCHAR *DataIn, SIZE_T DataLength, PBYTE DataOut, SIZE_T DataOutLength);

#endif //_XML_H_