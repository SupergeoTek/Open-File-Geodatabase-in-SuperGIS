

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Tue Jul 10 15:30:32 2018
 */
/* Compiler settings for SGFileGDB.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __SGFileGDB_i_h__
#define __SGFileGDB_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGDBWorkspace_FWD_DEFINED__
#define __IGDBWorkspace_FWD_DEFINED__
typedef interface IGDBWorkspace IGDBWorkspace;

#endif 	/* __IGDBWorkspace_FWD_DEFINED__ */


#ifndef __IGDBFeatureClass_FWD_DEFINED__
#define __IGDBFeatureClass_FWD_DEFINED__
typedef interface IGDBFeatureClass IGDBFeatureClass;

#endif 	/* __IGDBFeatureClass_FWD_DEFINED__ */


#ifndef __GDBWorkspace_FWD_DEFINED__
#define __GDBWorkspace_FWD_DEFINED__

#ifdef __cplusplus
typedef class GDBWorkspace GDBWorkspace;
#else
typedef struct GDBWorkspace GDBWorkspace;
#endif /* __cplusplus */

#endif 	/* __GDBWorkspace_FWD_DEFINED__ */


#ifndef __GDBFeatureClass_FWD_DEFINED__
#define __GDBFeatureClass_FWD_DEFINED__

#ifdef __cplusplus
typedef class GDBFeatureClass GDBFeatureClass;
#else
typedef struct GDBFeatureClass GDBFeatureClass;
#endif /* __cplusplus */

#endif 	/* __GDBFeatureClass_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IGDBWorkspace_INTERFACE_DEFINED__
#define __IGDBWorkspace_INTERFACE_DEFINED__

/* interface IGDBWorkspace */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IGDBWorkspace;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("92FEBB53-40C9-4E59-A0B0-6E2CC076DB5C")
    IGDBWorkspace : public IDispatch
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ BSTR FilePath,
            /* [retval][out] */ VARIANT_BOOL *Result) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Create( 
            /* [in] */ BSTR FilePath,
            /* [retval][out] */ VARIANT_BOOL *Result) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IGDBWorkspaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGDBWorkspace * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGDBWorkspace * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGDBWorkspace * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGDBWorkspace * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGDBWorkspace * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGDBWorkspace * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGDBWorkspace * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Open )( 
            IGDBWorkspace * This,
            /* [in] */ BSTR FilePath,
            /* [retval][out] */ VARIANT_BOOL *Result);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *Create )( 
            IGDBWorkspace * This,
            /* [in] */ BSTR FilePath,
            /* [retval][out] */ VARIANT_BOOL *Result);
        
        END_INTERFACE
    } IGDBWorkspaceVtbl;

    interface IGDBWorkspace
    {
        CONST_VTBL struct IGDBWorkspaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGDBWorkspace_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGDBWorkspace_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGDBWorkspace_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGDBWorkspace_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGDBWorkspace_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGDBWorkspace_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGDBWorkspace_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IGDBWorkspace_Open(This,FilePath,Result)	\
    ( (This)->lpVtbl -> Open(This,FilePath,Result) ) 

#define IGDBWorkspace_Create(This,FilePath,Result)	\
    ( (This)->lpVtbl -> Create(This,FilePath,Result) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGDBWorkspace_INTERFACE_DEFINED__ */


#ifndef __IGDBFeatureClass_INTERFACE_DEFINED__
#define __IGDBFeatureClass_INTERFACE_DEFINED__

/* interface IGDBFeatureClass */
/* [unique][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IGDBFeatureClass;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("91177865-8A0F-4EB1-8819-376F2CC06B7A")
    IGDBFeatureClass : public IDispatch
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct IGDBFeatureClassVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGDBFeatureClass * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGDBFeatureClass * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGDBFeatureClass * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGDBFeatureClass * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGDBFeatureClass * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGDBFeatureClass * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGDBFeatureClass * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } IGDBFeatureClassVtbl;

    interface IGDBFeatureClass
    {
        CONST_VTBL struct IGDBFeatureClassVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGDBFeatureClass_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IGDBFeatureClass_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IGDBFeatureClass_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IGDBFeatureClass_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IGDBFeatureClass_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IGDBFeatureClass_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IGDBFeatureClass_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IGDBFeatureClass_INTERFACE_DEFINED__ */



#ifndef __SGFileGDB_LIBRARY_DEFINED__
#define __SGFileGDB_LIBRARY_DEFINED__

/* library SGFileGDB */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_SGFileGDB;

EXTERN_C const CLSID CLSID_GDBWorkspace;

#ifdef __cplusplus

class DECLSPEC_UUID("991921D7-4F7E-4285-9B7B-A25F8B6B8061")
GDBWorkspace;
#endif

EXTERN_C const CLSID CLSID_GDBFeatureClass;

#ifdef __cplusplus

class DECLSPEC_UUID("B2BF00F4-8FBF-46BE-9E59-6DFE14485D75")
GDBFeatureClass;
#endif
#endif /* __SGFileGDB_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


