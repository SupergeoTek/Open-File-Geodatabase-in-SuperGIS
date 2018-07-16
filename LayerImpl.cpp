#include "stdafx.h"
//#include "resource.h"       // main symbols
//#include "SGMap.h"
#include "LayerImpl.h"
#include "../GeoProcess.h"
#include <limits>


CLayerLegendItem::CLayerLegendItem() : LegendItemImplType(NULL) // (LPCTSTR)IDB_LEGEND)
{
}

void CLayerLegendItem::SetMapLayer(SGCore::IMapLayer *pMapLayer)
{
	m_pMapLayer = pMapLayer;
}

//STDMETHODIMP CLayerLegendItem::get_Name(/*[out, retval]*/ BSTR* pVal)
//{return m_pMapLayer->get_Name(pVal);} 
//STDMETHODIMP CLayerLegendItem::put_Name(/*[out, retval]*/ BSTR newVal)
//{return m_pMapLayer->put_Name(newVal);} 
STDMETHODIMP CLayerLegendItem::get_Title(/*[out, retval]*/ BSTR* pVal)
{return m_pMapLayer->get_Title(pVal);} 
//STDMETHODIMP CLayerLegendItem::put_Title(/*[out, retval]*/ BSTR newVal)
//{return m_pMapLayer->put_Title(newVal);} 

//STDMETHODIMP CLayerLegendItem::raw_Draw(IMap *pMap, IDisplay *Display, ICancelTracker *Tracker)
//{return m_pMapLayer->raw_Draw(pMap, Display, Tracker);} 
//STDMETHODIMP CLayerLegendItem::get_IsModified(/*[out, retval]*/ VARIANT_BOOL* pVal)
//{return m_pMapLayer->get_IsModified(pVal);} 
//STDMETHODIMP CLayerLegendItem::raw_SetModifyFlag(VARIANT_BOOL newVal)
//{return m_pMapLayer->raw_SetModifyFlag(newVal);} 

STDMETHODIMP CLayerLegendItem::get_Visible(VARIANT_BOOL *pVal)
{return m_pMapLayer->get_Visible(pVal);}
STDMETHODIMP CLayerLegendItem::put_Visible(VARIANT_BOOL newVal)
{return m_pMapLayer->put_Visible(newVal);}

//STDMETHODIMP CLayerLegendItem::get_MaximumScale(double *pVal)
//{return m_pMapLayer->get_MaximumScale(pVal);} 
//STDMETHODIMP CLayerLegendItem::put_MaximumScale(double newVal)
//{return m_pMapLayer->put_MaximumScale(newVal);} 

//STDMETHODIMP CLayerLegendItem::get_MinimumScale(double *pVal)
//{return m_pMapLayer->get_MinimumScale(pVal);} 
//STDMETHODIMP CLayerLegendItem::put_MinimumScale(double newVal)
//{return m_pMapLayer->put_MinimumScale(newVal);} 
//STDMETHODIMP CLayerLegendItem::get_DisplayFilter(IDisplayFilter **pVal)
//{return m_pMapLayer->get_DisplayFilter(pVal);} 
//STDMETHODIMP CLayerLegendItem::putref_DisplayFilter(IDisplayFilter *newVal)
//{return m_pMapLayer->putref_DisplayFilter(newVal);} 

CLayerGroupImpl::CLayerGroupImpl()
{
}
CLayerGroupImpl::~CLayerGroupImpl()
{
	raw_ClearLayer();
}

// ILayerGroup
STDMETHODIMP CLayerGroupImpl::get_Layer(long Index, SGCore::IMapLayer **pVal)
{
	if (Index < m_pLayers.GetSize())
		return m_pLayers[Index]->QueryInterface(SGCore::IID_IMapLayer, (void **)pVal);
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::get_LayerCount(long *pVal)
{
	*pVal = (long)m_pLayers.GetSize();
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::raw_AddLayer(SGCore::IMapLayer *Layer)
{
	SGCore::IMapLayerPtr pLayer = Layer;
	if (pLayer)
	{
		Modified();
		m_pLayers.InsertAt(0,pLayer.Detach());
		OnAddLayer(Layer);
	}
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::raw_RemoveLayer(SGCore::IMapLayer *Layer)
{
	for (int i=0;i<m_pLayers.GetSize();i++)
	{
		if (m_pLayers[i] == Layer)
		{
			Modified();
			m_pLayers.RemoveAt(i);
			OnRemoveLayer(Layer);
			Layer->Release();
		}
	}
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::raw_ClearLayer()
{
	Modified();
	for (INT_PTR i=m_pLayers.GetSize()-1;i>=0;i--)
	{
		SGCore::IMapLayer *Layer = m_pLayers[i];
		m_pLayers.RemoveAt(i);
		OnRemoveLayer(Layer);
		Layer->Release();
	}
	m_pLayers.RemoveAll();
	return S_OK;
}

// ILegendGroup
STDMETHODIMP CLayerGroupImpl::get_ItemCount(long *pVal)
{
	*pVal = (long)m_pLayers.GetSize();
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::get_Item(long Index, SGCore::ILegendItem **pVal)
{
	SGCore::IMapLayerPtr pLayer;
	get_Layer(Index,&pLayer);

//	CLayerLegendItem *pItme = new CComObject<CLayerLegendItem>;
//	pItme->AddRef();
//	pItme->SetMapLayer(pLayer);
	HRESULT r = pLayer->QueryInterface(SGCore::IID_ILegendItem, (void **)pVal);
//	pItme->Release();
	return r;
}
STDMETHODIMP CLayerGroupImpl::raw_AllowInsert(SGCore::ILegendItem *pItem, SGCore::ILegendItem *pInsertAfter, VARIANT_BOOL *result)
{
	//if (!(*result = (pItem!=pInsertAfter)))
	//	return S_OK;
	SGCore::IMapLayerPtr pLayer = pItem;
	*result = (pLayer!=NULL);
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::raw_InsertItem(SGCore::ILegendItem *pItem, SGCore::ILegendItem *pInsertAfter)
{
	SGCore::IMapLayerPtr pLayer = pItem;
	if (pLayer!=NULL)
	{
		SGCore::IMapLayerPtr pAfterLayer = pInsertAfter;
		INT_PTR i = -1;
		if (pAfterLayer!=NULL)
		{
			for (i = m_pLayers.GetSize()-1;i>=0;i--)
			{
				if (m_pLayers[i] == pAfterLayer)
					break;
			}
		}
		Modified();
		SGCore::IMapLayer *pLay = pLayer.Detach();
		m_pLayers.InsertAt(i+1, pLay);


		OnAddLayer(pLay);
	}
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::raw_AllowRemove(SGCore::ILegendItem *pItem, VARIANT_BOOL *result)
{
	*result=TRUE;return S_OK;
}
STDMETHODIMP CLayerGroupImpl::raw_RemoveItem(SGCore::ILegendItem *pItem)
{
	SGCore::IMapLayerPtr pLayer = pItem;
	return raw_RemoveLayer(pLayer);
}

// IGeoDataset
STDMETHODIMP CLayerGroupImpl::get_SpatialReference(SGSFCOMS::ISpatialReference **pVal) 
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	*pVal = NULL;
	INT_PTR count = m_pLayers.GetSize();
	for (INT_PTR i = 0 ; i < count ; i++)
	{
		SGCore::IGeoDatasetPtr pGSet = m_pLayers[i];
		if (pGSet == NULL)
			continue;
		SGSFCOMS::ISpatialReferencePtr pSpatialRef;
		pGSet->get_SpatialReference(&pSpatialRef);
		if (pSpatialRef != NULL)
		{
			pSpatialRef->QueryInterface(SGSFCOMS::IID_ISpatialReference, (void **)pVal);
			return S_OK;
		}
	}
	return S_OK;
}
STDMETHODIMP CLayerGroupImpl::get_GeographicTransform(SGSFCOMS::IGeographicTransform **pVal)
{
	*pVal = NULL;
	return E_NOTIMPL;
}
STDMETHODIMP CLayerGroupImpl::putref_GeographicTransform(SGSFCOMS::IGeographicTransform *newVal)
{
	return E_NOTIMPL;
}
STDMETHODIMP CLayerGroupImpl::get_Extent(SGSFCOMS::ISpatialReference *SpatialRef, SGSFCOMS::IEnvelope **pVal)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState())
	double dMinX, dMaxX, dMinY, dMaxY;
	dMinX = dMaxX = dMinY = dMaxY = 
		std::numeric_limits<double>::quiet_NaN();

	SGSFCOMS::ISpatialReferencePtr pSpatialRef = SpatialRef;
	INT_PTR count = m_pLayers.GetSize();
	for (INT_PTR i = 0 ; i < count ; i++)
	{
		SGCore::IGeoDatasetPtr pGSet = m_pLayers[i];
		if (pGSet == NULL)
			continue;

		SGSFCOMS::IEnvelopePtr pExt;
		pGSet->get_Extent(pSpatialRef, &pExt);
		if (pExt == NULL)
			continue;

		double minx, miny, maxx, maxy;
		pExt->raw_Extent2D(&minx, &miny, &maxx, &maxy);
		if (_isnan(dMinX) || dMinX > minx) dMinX = minx;
		if (_isnan(dMaxX) || dMaxX < maxx) dMaxX = maxx;
		if (_isnan(dMinY) || dMinY > miny) dMinY = miny;
		if (_isnan(dMaxY) || dMaxY < maxy) dMaxY = maxy;
		//if (pSpatialRef == NULL)
		//	pExt->get_SpatialReference(&pSpatialRef);
	}
	SGSFCOMS::IEnvelopePtr pEnv;
	pEnv.CreateInstance(SGSFCOMS::CLSID_Envelope);
	pEnv->put_minX(dMinX);
	pEnv->put_maxX(dMaxX);
	pEnv->put_minY(dMinY);
	pEnv->put_maxY(dMaxY);
	pEnv->putref_SpatialReference(pSpatialRef);
	return pEnv->QueryInterface(SGSFCOMS::IID_IEnvelope, (void **)pVal);
}

void SetBitMapInfo(LPBITMAPINFO pBmpInfo, int width, int height, int band)
{
	memset(&(pBmpInfo->bmiHeader), 0, sizeof(BITMAPINFOHEADER));
	if (band==1)
	{
		for (short i = 0; i < 256; i++)
		{
			pBmpInfo->bmiColors[i].rgbBlue = (BYTE)i;
			pBmpInfo->bmiColors[i].rgbRed = (BYTE)i;
			pBmpInfo->bmiColors[i].rgbGreen = (BYTE)i;
			pBmpInfo->bmiColors[i].rgbReserved = (BYTE)i;
		}
		pBmpInfo->bmiHeader.biBitCount = 8;
	}
	else
	{
		pBmpInfo->bmiHeader.biBitCount = 24;
	}
	pBmpInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pBmpInfo->bmiHeader.biWidth = width;
	pBmpInfo->bmiHeader.biHeight = -height;	// negative indicates a top-down image
	pBmpInfo->bmiHeader.biPlanes = 1;
	pBmpInfo->bmiHeader.biCompression = BI_RGB;
	pBmpInfo->bmiHeader.biSizeImage = 0;
	pBmpInfo->bmiHeader.biXPelsPerMeter = 0;
	pBmpInfo->bmiHeader.biYPelsPerMeter = 0;
	pBmpInfo->bmiHeader.biClrUsed = 0;
	pBmpInfo->bmiHeader.biClrImportant = 0;
}

HBITMAP CreateDIB(LPBITMAPINFO pBmpInfo, void **ppvBits)
{
	return CreateDIBSection(NULL, pBmpInfo, DIB_RGB_COLORS, ppvBits, 0, 0L);
}

CDCBase::CDCBase()
{
	m_hDC = CreateCompatibleDC(NULL);
	SetBkMode(m_hDC,TRANSPARENT);
	m_hOldBitmap = NULL;
	m_cx = 0;
	m_cy = 0;
	m_nSavedDC = 0;
	m_ppvBits = NULL;
};
CDCBase::~CDCBase()
{
	if (m_hDC != NULL)
	{
		if (m_hOldBitmap != NULL)
			DeleteObject(SelectObject(m_hDC, m_hOldBitmap));
		DeleteDC(m_hDC);
	}
};
void CDCBase::StartDrawing(HDC hDC, RECT *extent)
{
	if (m_nSavedDC!=0)
		RestoreDC(m_hDC, m_nSavedDC);
	m_nSavedDC=0;

	long cmaxx = max(m_cx, extent->right);
	long cmaxy = max(m_cy, extent->bottom );
	if (m_cx!=cmaxx || m_cy!=cmaxy)
	{
		m_cx=cmaxx;
		m_cy=cmaxy;

		if (m_hOldBitmap != NULL)
			DeleteObject(SelectObject(m_hDC, m_hOldBitmap));
		ASSERT(m_hDC != NULL);

		BITMAPINFO BmpInfo;
		SetBitMapInfo(&BmpInfo, cmaxx, cmaxy, 3);
		m_hOldBitmap = (HBITMAP)SelectObject(m_hDC, CreateDIB(&BmpInfo, (void **)&m_ppvBits));
		Clear();
	}
	if (hDC != NULL)
		BitBlt(m_hDC, extent->left, extent->top, 
				extent->right - extent->left, extent->bottom - extent->top, 
				hDC, extent->left, extent->top, SRCCOPY);
	m_nSavedDC = SaveDC(m_hDC);
};
void CDCBase::FinishDrawing(HDC hDC, RECT *extent)
{
	DrawCache(hDC, extent);
	RestoreDC(m_hDC, m_nSavedDC);
	m_nSavedDC = 0;
};
void CDCBase::DrawCache(HDC hDC, RECT *extent)
{
	if (hDC != NULL)
	{
		BitBlt(hDC, extent->left, extent->top, 
			extent->right - extent->left, extent->bottom - extent->top, 
			m_hDC, extent->left, extent->top, SRCCOPY);
	}
};
void CDCBase::Clear()
{
	RECT extent;
	SetRect(&extent, 0, 0, m_cx, m_cy);
	ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &extent, NULL, 0, NULL);
};

xDispFilter::xDispFilter(SGCore::IDisplayFilter *filter, SGCore::IDisplay *pDisp, SGCore::tagSGRect rc, IUnknown *pUnk, BOOL bCache)
{
	pFBuf = NULL;
	pBBuf = NULL;
	Buffer = NULL;
	DistWnd = NULL;
	//DistDC = NULL;
	clip = rc;
	//ew = (((3 * clip.right + 3) / 4) * 4);
	offset.x = rc.left;
	offset.y = rc.top;
	DispFilter = filter;
	pDist = pDisp;
	Width = 0;
	Height = 0;
	Stride = 0;
	BytesPerPixel = 0;
	Buffer = NULL;

	//if (pDisp) DistDC = (HDC)pDisp->hDC;
	SGCore::IScreenDisplayPtr pScrDisp(pDisp);
	if (pScrDisp) DistWnd = pScrDisp->hWnd;
	SGCore::IDisplayBufferPtr pBuf = pDisp;
	if (pBuf != NULL)
	{
		if (DispFilter != NULL || bCache)
			pFBuf = pBuf->CreateBuffer();
		if (DispFilter != NULL)
			pBBuf = pBuf->CreateBuffer();
	}
	//if (filter)
	//{
	//	pFBuf = new CDCBase;
	//	pBBuf = new CDCBase;
	//	pFBuf->StartDrawing((HDC)DistDC, (RECT *)&rc);
	//	pBBuf->StartDrawing((HDC)DistDC, (RECT *)&rc);
	//}
	//else if (bCache)
	//{
	//	pFBuf = new CDCBase;
	//	pFBuf->StartDrawing((HDC)DistDC, (RECT *)&rc);
	//}
	m_pUnk = pUnk;
	if (m_pUnk) DispEventAdvise(m_pUnk);
}

xDispFilter::~xDispFilter()
{
	//if (pFBuf) delete pFBuf;
	//if (pBBuf) delete pBBuf;
	if (m_pUnk) DispEventUnadvise(m_pUnk);
}

SGCore::IDisplayPtr xDispFilter::StartDrawDisplay(SGCore::IDisplay *pDisp)
{
	if (pFBuf == NULL)
		return pDisp;
	return pFBuf;
	////SGCore::IScreenDisplayPtr pScrDisp = pDisp;
	////if (pScrDisp == NULL)
	////	Display = CloneImplement(pDisp);
	////else
	//	Display.CreateInstance(SGRender::CLSID_Display);
	//Display->putref_DisplayTransformation(pDisp->DisplayTransformation);
	//Display->raw_StartDrawing(pFBuf->m_hDC, &clip);
	//Display->putref_LabelPlacer(pDisp->LabelPlacer);
	//return Display;
}

BOOL xDispFilter::StartDrawBits()
{
	SGCore::IDisplayBufferPtr pFB = pFBuf;
	if (pFB != NULL)
		pFB->LockPixels(&Width, &Height, &Stride, &BytesPerPixel, &Buffer);
	return (Buffer != NULL);
	//return (pFBuf != NULL && pFBuf->m_ppvBits != NULL);
}

void xDispFilter::FinishDraw()
{
	SGCore::IDisplayBufferPtr pFB = pFBuf;
	if (pFB != NULL)
		pFB->UnlockPixels();
	Width = 0;
	Height = 0;
	Stride = 0;
	BytesPerPixel = 0;
	Buffer = NULL;

	OnStep();
	if (pFB != NULL)
		pFB->DestroyBuffer();
	SGCore::IDisplayBufferPtr pBB = pBBuf;
	if (pBB != NULL)
		pBB->DestroyBuffer();
}

COLORREF xDispFilter::GetPixel(long nx, long ny)
{
	if (Buffer == NULL)
		return 0;
	BYTE *pcurdst = Buffer + ny * Stride + nx * BytesPerPixel;
	return RGB(pcurdst[2], pcurdst[1], pcurdst[0]);

	//SGCore::IDisplayBufferPtr pFB = pFBuf;
	//if (pFB != NULL)
	//	return pFB->GetPixel(nx, ny);

	//if (pFBuf == NULL || pFBuf->m_ppvBits == NULL)
	//	return 0;
	//BYTE *pcurdst = pFBuf->m_ppvBits + ny * ew + nx * 3;
	//return RGB(pcurdst[2], pcurdst[1], pcurdst[0]);
	return 0;
}

void xDispFilter::SetPixel(long nx, long ny, COLORREF color)
{
	if (Buffer == NULL)
		return;
	BYTE *pcurdst = Buffer + ny * Stride + nx * BytesPerPixel;
	pcurdst[0] = GetBValue(color);
	pcurdst[1] = GetGValue(color);
	pcurdst[2] = GetRValue(color);
	if (BytesPerPixel == 4)
		pcurdst[3] = 255;

	//SGCore::IDisplayBufferPtr pFB = pFBuf;
	//if (pFB != NULL)
	//	pFB->SetPixel(nx, ny, color);

	//if (pFBuf == NULL || pFBuf->m_ppvBits == NULL)
	//	return 0;
	//BYTE *pcurdst = pFBuf->m_ppvBits + ny * ew + nx * 3;
	//pcurdst[0] = GetBValue(color);
	//pcurdst[1] = GetGValue(color);
	//pcurdst[2] = GetRValue(color);
}

STDMETHODIMP xDispFilter::OnStep(void)
{
	BOOL bLocked = (Buffer != NULL);
	SGCore::IDisplayBufferPtr pFB = pFBuf;
	if (bLocked && pFB != NULL)
		pFB->UnlockPixels();
	if (DispFilter != NULL)
	{
		SGCorePlus::IDisplayFilterPlusPtr pDFPlus = DispFilter;
		if (pDFPlus != NULL)
			pDFPlus->raw_ApplyPlus(pBBuf, pFBuf, pDist, &clip, &offset);
		else
			DispFilter->raw_Apply(pBBuf->hDC, pFBuf->hDC, pDist->hDC, &clip, &offset);
	}
	else
	{
		if (pFB != NULL)
			pFB->Copy(pDist, &clip, &offset, 1.0);
		//BitBlt((HDC)DistDC, offset.x, offset.y, clip.right - clip.left, clip.bottom - clip.top, 
		//	pFBuf->m_hDC, clip.left, clip.top, SRCCOPY);
	}
	if (bLocked && pFB != NULL)
		pFB->LockPixels(&Width, &Height, &Stride, &BytesPerPixel, &Buffer);

	if (DistWnd!=NULL)
		InvalidateRect((HWND)DistWnd, NULL,FALSE);
	return S_OK;
}