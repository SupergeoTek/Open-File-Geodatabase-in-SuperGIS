

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IGDBWorkspace,0x92FEBB53,0x40C9,0x4E59,0xA0,0xB0,0x6E,0x2C,0xC0,0x76,0xDB,0x5C);


MIDL_DEFINE_GUID(IID, IID_IGDBFeatureClass,0x91177865,0x8A0F,0x4EB1,0x88,0x19,0x37,0x6F,0x2C,0xC0,0x6B,0x7A);


MIDL_DEFINE_GUID(IID, LIBID_SGFileGDB,0x1E3C210F,0xF5C8,0x43DA,0x88,0x52,0x58,0x62,0x43,0x49,0xBF,0x8A);


MIDL_DEFINE_GUID(CLSID, CLSID_GDBWorkspace,0x991921D7,0x4F7E,0x4285,0x9B,0x7B,0xA2,0x5F,0x8B,0x6B,0x80,0x61);


MIDL_DEFINE_GUID(CLSID, CLSID_GDBFeatureClass,0xB2BF00F4,0x8FBF,0x46BE,0x9E,0x59,0x6D,0xFE,0x14,0x48,0x5D,0x75);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



