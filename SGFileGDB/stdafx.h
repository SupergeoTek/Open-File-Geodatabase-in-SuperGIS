// stdafx.h : �i�b�����Y�ɤ��]�t�зǪ��t�� Include �ɡA
// �άO�g�`�ϥΫo�ܤ��ܧ�
// �M�ױM�� Include �ɮ�

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ���T�w�q������ CString �غc�禡

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
#include <afxdisp.h>        // MFC Automation ���O
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
