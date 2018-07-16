#ifndef _LAYERIMPL_H_
#define _LAYERIMPL_H_

#include "..\InterfaceImpl.h"
#include "..\PersistXMLImpl.h"
//#include "SGMapCP.h"
#include <limits>

//#import "..\SGGML\win32\SGGML.tlb" named_guids
//HBITMAP CreateDIB(int width, int height, void **ppvBits);
void SetBitMapInfo(LPBITMAPINFO pBmpInfo, int width, int height, int band);
HBITMAP CreateDIB(LPBITMAPINFO pBmpInfo, void **ppvBits);

template <class T>
class CMapLayerImpl :
	public IDispatchImpl<T, &__uuidof(T), &SGCore::LIBID_SGCore>
{
public:
	CMapLayerImpl()
	{
		m_Visible = VARIANT_TRUE;
		m_MaximumScale = 0;
		m_MinimumScale = 0;
		m_IsModified = VARIANT_FALSE;
	}
protected:
	//IMPL_PROPERTY(CComBSTR,Title)
	IMPL_PROPERTY(CComBSTR,Name)
	IMPL_PROPERTY(VARIANT_BOOL,IsModified)

	IMPL_PROPERTY(VARIANT_BOOL,Visible)
	IMPL_PROPERTY(double,MaximumScale)
	IMPL_PROPERTY(double,MinimumScale)

	SGCore::IDisplayFilterPtr m_DisplayFilter;
	CString m_Title2;
public:
// IMapLayer
	IMPL_GETBSTR_PROPERTY(Name)
	IMPL_PUTBSTR_PROPERTY(Name)

	STDMETHOD(get_Title)(/*[out, retval]*/ BSTR* value)
	{
		*value = m_Title2.AllocSysString();//.Copy(); 
		return S_OK;
	} 
	STDMETHOD(put_Title)(/*[in]*/ BSTR value)
	{
		m_IsModified = TRUE;
		m_Title2 = value;
		return S_OK;
	}
//	IMPL_GETBSTR_PROPERTY(Title)
//	IMPL_PUTBSTR_PROPERTY(Title)
	IMPL_GET_PROPERTY(VARIANT_BOOL, IsModified)
	STDMETHOD(raw_SetModifyFlag)(VARIANT_BOOL newVal)
	{
		m_IsModified = newVal;
		return S_OK;
	}

	IMPL_GET_PROPERTY(VARIANT_BOOL, Visible)
	STDMETHOD(put_Visible)(VARIANT_BOOL newVal)
	{m_Visible = newVal;m_IsModified = VARIANT_TRUE;return S_OK;}
	IMPL_GETPUT_PROPERTY(double, MaximumScale)
	IMPL_GETPUT_PROPERTY(double, MinimumScale)

	IMPL_GETPTR_PROPERTY(SGCore::IDisplayFilter, DisplayFilter)
	IMPL_PUTPTRREF_PROPERTY(SGCore::IDisplayFilter, DisplayFilter)
};

template <class T>
class CLegendItemImpl : 
	public IDispatchImpl<SGCore::ILegendItem, &SGCore::IID_ILegendItem>
{
	typedef CLegendItemImpl<T> LegendItemImplType;
public:
	CLegendItemImpl(LPCTSTR lpbmp)
	{
		if (m_sImgList==NULL)
			m_sImgList = ImageList_LoadBitmap(_AtlBaseModule.GetResourceInstance(), lpbmp, 16, 2, RGB(255,0,255));
		m_sRefCount++;
	}

	virtual ~CLegendItemImpl()
	{
		m_sRefCount--;
		if (m_sRefCount==0)
		{
			ImageList_Destroy(m_sImgList);
			m_sImgList = NULL;
		}
	}

// ILegendItem
	STDMETHOD(get_Label)(/*[out, retval]*/ BSTR *pVal)
	{
		((T *)this)->get_Title(pVal);
		return S_OK;
	}
	STDMETHOD(get_Symbol)(/*[out, retval]*/ SGCore::ISymbol **pVal) {return S_OK;}
	STDMETHOD(raw_DrawLegend)(HANDLE hDC, SGCore::tagSGRect *Rect, VARIANT_BOOL *result)
	{
		*result = VARIANT_FALSE;
		long x = (Rect->left + Rect->right) /2 - 8;
		long y = (Rect->top + Rect->bottom) /2 - 8;

		::SetBkColor((HDC)hDC, GetSysColor(COLOR_WINDOW));
		RECT rc = {x+1, y+1, x + 15, y + 15};
		::ExtTextOut((HDC)hDC, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

		VARIANT_BOOL Visible = VARIANT_FALSE;
		((T *)this)->get_Visible(&Visible);
		ImageList_Draw(m_sImgList, (Visible)?2:1,(HDC)hDC,x,y,ILD_NORMAL);
		*result = VARIANT_TRUE;
		return S_OK;
	}

	STDMETHOD(raw_ItemClick)(SGCore::tagClickStruct *lpClick, VARIANT_BOOL *result)
	{
		*result = VARIANT_FALSE;
		long x = (lpClick->Rect.left + lpClick->Rect.right) /2 - 8;
		long y = (lpClick->Rect.top + lpClick->Rect.bottom) /2 - 8;
		RECT rc = {x,y,x+16,y+16};
		if (PtInRect(&rc, *(POINT*)&lpClick->Point))
		{
			VARIANT_BOOL Visible = VARIANT_FALSE;
			((T *)this)->get_Visible(&Visible);
			((T *)this)->put_Visible(!Visible);
			::InvalidateRect((HWND)lpClick->hWnd,NULL,FALSE);
			*result = VARIANT_TRUE;
		}
		return S_OK;
	}

	STDMETHOD(raw_ItemDblClick)(SGCore::tagClickStruct *lpClick, VARIANT_BOOL *result)
	{
		*result = VARIANT_FALSE;
		return S_OK;
	}

protected:
	static HIMAGELIST m_sImgList;
	static int m_sRefCount;
};
template <class T> HIMAGELIST CLegendItemImpl<T>::m_sImgList = NULL;
template <class T> int CLegendItemImpl<T>::m_sRefCount = 0;

class CLayerLegendItem : public CComObjectRoot,
//	public IDispatchImpl<IMapLayer, &SGCore::IID_IMapLayer, &SGCore::LIBID_SGCore>,
	public CLegendItemImpl<CLayerLegendItem>
{
public:
	CLayerLegendItem();

BEGIN_COM_MAP(CLayerLegendItem)
//	COM_INTERFACE_ENTRY(IMapLayer)
	COM_INTERFACE_ENTRY(ILegendItem)
	COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_pMapLayer.p)
END_COM_MAP()

	void SetMapLayer(SGCore::IMapLayer *pMapLayer);
protected:
	CComPtr<SGCore::IMapLayer> m_pMapLayer;
public:
// IMapLayer
//	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR* pVal);
//	STDMETHOD(put_Name)(/*[out, retval]*/ BSTR newVal);

	STDMETHOD(get_Title)(/*[out, retval]*/ BSTR* pVal);
//	STDMETHOD(put_Title)(/*[out, retval]*/ BSTR newVal);
	
//	STDMETHOD(raw_Draw)(IMap *pMap, IDisplay *Display, ICancelTracker *Tracker);
//	STDMETHOD(get_IsModified)(/*[out, retval]*/ VARIANT_BOOL* pVal);
//	STDMETHOD(raw_SetModifyFlag)(VARIANT_BOOL newVal);

	STDMETHOD(get_Visible)(VARIANT_BOOL *pVal);
	STDMETHOD(put_Visible)(VARIANT_BOOL newVal);
//	STDMETHOD(get_MaximumScale)(double *pVal);
//	STDMETHOD(put_MaximumScale)(double newVal);
//	STDMETHOD(get_MinimumScale)(double *pVal);
//	STDMETHOD(put_MinimumScale)(double newVal);
//	STDMETHOD(get_DisplayFilter)(IDisplayFilter **pVal);
//	STDMETHOD(putref_DisplayFilter)(IDisplayFilter *newVal);
};

class CLayerGroupImpl : public IDispatchImpl<SGCore::ILayerGroup, &SGCore::IID_ILayerGroup>,
	public IDispatchImpl<SGCore::ILegendGroup, &SGCore::IID_ILegendGroup>,
	public IDispatchImpl<SGCore::IGeoDataset, &SGCore::IID_IGeoDataset>
{
public:
	CLayerGroupImpl();
	virtual ~CLayerGroupImpl();

protected:
	CTypedPtrArray< CPtrArray, SGCore::IMapLayer* > m_pLayers;
	virtual void Modified() {};
	virtual void OnAddLayer(SGCore::IMapLayer * Layer) = 0;
	virtual void OnRemoveLayer(SGCore::IMapLayer * Layer) = 0;
public:
// ILayerGroup
	STDMETHOD(get_Layer)(long Index, SGCore::IMapLayer **pVal);
	STDMETHOD(get_LayerCount)(long *pVal);
	STDMETHOD(raw_AddLayer)(SGCore::IMapLayer *Layer);
	STDMETHOD(raw_RemoveLayer)(SGCore::IMapLayer *Layer);
	STDMETHOD(raw_ClearLayer)();
	
// ILegendGroup
	STDMETHOD(get_ItemCount)(long *pVal);
	STDMETHOD(get_Item)(long Index, SGCore::ILegendItem **pVal);
	STDMETHOD(raw_AllowInsert)(SGCore::ILegendItem *pItem, SGCore::ILegendItem *pInsertAfter, VARIANT_BOOL *result);
	STDMETHOD(raw_InsertItem)(SGCore::ILegendItem *pItem, SGCore::ILegendItem *pInsertAfter);
	STDMETHOD(raw_AllowRemove)(SGCore::ILegendItem *pItem, VARIANT_BOOL *result);
	STDMETHOD(raw_RemoveItem)(SGCore::ILegendItem *pItem);
	
// IGeoDataset
	STDMETHOD(get_SpatialReference)(SGSFCOMS::ISpatialReference **pVal);
	STDMETHOD(get_GeographicTransform)(SGSFCOMS::IGeographicTransform **pVal);
	STDMETHOD(putref_GeographicTransform)(SGSFCOMS::IGeographicTransform *newVal);
	STDMETHOD(get_Extent)(SGSFCOMS::ISpatialReference *SpatialRef, SGSFCOMS::IEnvelope **pVal);
};

class CDCBase
{
public:
	CDCBase();
	virtual ~CDCBase();
	void StartDrawing(HDC hDC, RECT *extent);
	void FinishDrawing(HDC hDC, RECT *extent);
	void DrawCache(HDC hDC, RECT *extent);
	void Clear();

public:
	HDC m_hDC;
	HBITMAP m_hOldBitmap;
	long m_cx;
	long m_cy;
	int m_nSavedDC;
	BYTE *m_ppvBits;
};

class xDispFilter : 
	public IDispEventImpl<0, xDispFilter, &__uuidof(SGMap::_ICancelTrackerEvents), &SGMap::LIBID_SGMap, 1, 0>
{
public:
	xDispFilter(SGCore::IDisplayFilter *filter, HANDLE hWnd, HANDLE hDC, SGCore::tagSGRect rc, IUnknown *pUnk, BOOL bCache);
	xDispFilter(SGCore::IDisplayFilter *filter, SGCore::IDisplay *pDisp, SGCore::tagSGRect rc, IUnknown *pUnk, BOOL bCache);
	virtual ~xDispFilter();

	SGCore::IDisplayPtr StartDrawDisplay(SGCore::IDisplay *pDisp);
	BOOL StartDrawBits();

	void FinishDraw();
	COLORREF GetPixel(long nx, long ny);
	void SetPixel(long nx, long ny, COLORREF color);
public:
	STDMETHOD(OnStep)(void);
BEGIN_SINK_MAP(xDispFilter) 
		SINK_ENTRY_EX(0, (__uuidof(SGMap::_ICancelTrackerEvents)), 0x00000001, OnStep)   
END_SINK_MAP()

protected:
	//CDCBase *pFBuf;
	//CDCBase *pBBuf;
	SGCore::IDisplayPtr pFBuf;
	SGCore::IDisplayPtr pBBuf;
	SGCore::IDisplayPtr pDist;
	long Width;
	long Height;
	long Stride;
	long BytesPerPixel;
	BYTE *Buffer;
	HANDLE DistWnd;
	//HANDLE DistDC;
	SGCore::tagSGRect clip;
	SGCore::tagSGPoint offset;
	//long ew;
	SGCore::IDisplayFilterPtr DispFilter;
	//SGCore::IDisplayPtr Display;
	IUnknownPtr m_pUnk;
};

SGGML::IXMLArchivePtr GetAppDoc();

template <class T>
class CSpecifyPropertyPagesImpl : public ISpecifyPropertyPages
{
protected:
	typedef CSpecifyPropertyPagesImpl<T> CSpecifyPropertyPagesImplType;
public:
	CSpecifyPropertyPagesImpl(BSTR tagName)
	{
		SGGML::IXMLArchiveElementPtr xmlEle = GetRootElement(GetAppDoc());
		SGGML::IXMLArchiveElementPtr pElem = FindSingleNode(xmlEle, tagName);
		if (pElem == NULL)
			return;

		SGGML::IXMLArchiveElementListPtr pNodes = FindNodeList(pElem, L"PropertyPage");
		long cnt = GetNodeCount(pNodes);
		for (int i=0;i<cnt;i++)
		{
			_bstr_t bstr = GetNodeAttributeB(GetNode(pNodes, i), CComBSTR("clsid"));
			if (bstr.length() > 0)
			{
				CLSID clsid;
				if (CLSIDFromString(bstr, &clsid) == S_OK)
					m_Clsid.Add(clsid);
			}
		}
	}
	
	STDMETHOD(GetPages)(CAUUID* pPages)
	{
		if (pPages == NULL)
			return E_POINTER;
		const ATL_PROPMAP_ENTRY* pMap = T::GetPropertyMap();
		CArray<CLSID, CLSID> addsClsid;
		((T*)this)->GetAdditionalPages(&addsClsid);

		CArray<CLSID, CLSID> tmpClsid;
		for (INT_PTR i = 0 ; i < m_Clsid.GetSize() != NULL ; i++)
			if (!Exists(tmpClsid.GetData(), tmpClsid.GetSize(), m_Clsid[i]))
				tmpClsid.Add(m_Clsid[i]);
		if (pMap != NULL)
			for (int i = 0; pMap[i].pclsidPropPage != NULL; i++)
				if (pMap[i].vt == 0 && !InlineIsEqualGUID(*pMap[i].pclsidPropPage, CLSID_NULL))
					if (!Exists(tmpClsid.GetData(), tmpClsid.GetSize(), *pMap[i].pclsidPropPage))
						tmpClsid.Add(*pMap[i].pclsidPropPage);
		for (INT_PTR i = 0 ; i < addsClsid.GetSize() != NULL ; i++)
			if (!Exists(tmpClsid.GetData(), tmpClsid.GetSize(), addsClsid[i]))
				tmpClsid.Add(addsClsid[i]);

		long nCnt = static_cast<int>(tmpClsid.GetSize());
		if (nCnt <= 0)
			return E_POINTER;
		pPages->pElems = (GUID*) ::ATL::AtlCoTaskMemCAlloc(nCnt, static_cast<ULONG>(sizeof(CLSID)));
		if (pPages->pElems == NULL)
			return E_OUTOFMEMORY;
		memcpy(pPages->pElems, tmpClsid.GetData(), nCnt * sizeof(CLSID));
		pPages->cElems = nCnt;
		return S_OK;
	}
//protected:
	//virtual void GetAdditionalPages(CArray<CLSID, CLSID> *pClsids) {};
private:
	BOOL Exists(GUID *pGUIDs, INT_PTR Cnt, GUID guid)
	{
		for (INT_PTR nFound = 0 ; nFound < Cnt ; nFound++)
			if (InlineIsEqualGUID(guid, pGUIDs[nFound]))
				return TRUE;
		return FALSE;
	}
protected:
	CArray<CLSID, CLSID> m_Clsid;
};

#endif //_LAYERIMPL_H_