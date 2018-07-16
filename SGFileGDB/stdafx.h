// stdafx.h : 可在此標頭檔中包含標準的系統 Include 檔，
// 或是經常使用卻很少變更的
// 專案專用 Include 檔案

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 明確定義部分的 CString 建構函式

#include <afxwin.h>
#include <afxext.h>
#include <afxole.h>
#include <afxodlgs.h>
#include <afxrich.h>
#include <afxhtml.h>
#include <afxcview.h>
#include <afxwinappex.h>
#include <afxframewndex.h>
#include <afxmdiframewndex.h>

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdisp.h>        // MFC Automation 類別
#endif // _AFX_NO_OLE_SUPPORT

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

using namespace ATL;

#import "C:\Program Files (x86)\Common Files\Supergeo\SGSFCOMs.dll" named_guids
#import "C:\Program Files (x86)\Common Files\Supergeo\SGCore.dll" named_guids, rename("ResetDC", "SGResetDC")
//#import "C:\Program Files (x86)\Common Files\SuperGeo\SGCorePlus.dll" named_guids

#import "C:\Program Files (x86)\Common Files\Supergeo\SGDataAccess.dll" named_guids, rename("CreateFile", "SGCreateFile")
//#import "C:\Program Files (x86)\Common Files\Supergeo\SGRasterProcessor.dll" named_guids
#import "C:\Program Files (x86)\Common Files\Supergeo\SGMap.dll" named_guids
#import "C:\Program Files (x86)\Common Files\SuperGeo\SGGML.dll" named_guids
