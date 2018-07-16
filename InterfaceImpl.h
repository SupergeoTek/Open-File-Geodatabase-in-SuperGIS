#ifndef _INTERFACEIMPL_H_
#define _INTERFACEIMPL_H_

#define IMPL_PROPERTY(datatype, name) \
	datatype m_##name;

//#define IMPL_GETEMPTY_PROPERTY(basetype, name) \
//	STDMETHOD(get_##name)(/*[out, retval]*/ basetype* value) \
//	{return S_OK;} 

//#define IMPL_PUTEMPTY_PROPERTY(basetype, name) \
//	STDMETHOD(put_##name)(/*[in]*/ basetype value) \
//	{return S_OK;} 

#define IMPL_GET_PROPERTY_MBR(basetype, name, mbr) \
	STDMETHOD(get_##name)(/*[out, retval]*/ basetype* value) \
	{*value = (basetype)m_##mbr; return S_OK;} 

#define IMPL_PUT_PROPERTY_MBR(basetype, name, mbr) \
	STDMETHOD(put_##name)(/*[in]*/ basetype value) \
	{m_##mbr = value; return S_OK;}

#define IMPL_PUT_PROPERTYEX_MBR(basetype, name, mbr, dirty) \
	STDMETHOD(put_##name)(/*[in]*/ basetype value) \
	{m_##mbr = value; m_##dirty = VARIANT_TRUE; return S_OK;}

#define IMPL_GETPUT_PROPERTY_MBR(basetype, name, mbr) \
	IMPL_GET_PROPERTY_MBR(basetype, name, mbr) \
	IMPL_PUT_PROPERTY_MBR(basetype, name, mbr)

#define IMPL_GETPUT_PROPERTYEX_MBR(basetype, name, mbr, dirty) \
	IMPL_GET_PROPERTY_MBR(basetype, name, mbr) \
	IMPL_PUT_PROPERTYEX_MBR(basetype, name, mbr, dirty)

#define IMPL_GET_PROPERTY(basetype, name) \
	IMPL_GET_PROPERTY_MBR(basetype, name, name)

#define IMPL_PUT_PROPERTY(basetype, name) \
	IMPL_PUT_PROPERTY_MBR(basetype, name, name)

#define IMPL_GETPUT_PROPERTY(basetype, name) \
	IMPL_GET_PROPERTY(basetype, name) \
	IMPL_PUT_PROPERTY(basetype, name)

#define IMPL_PUT_PROPERTYEX(basetype, name, dirty) \
	IMPL_PUT_PROPERTYEX_MBR(basetype, name, name, dirty)

#define IMPL_GETPUT_PROPERTYEX(basetype, name, dirty) \
	IMPL_GET_PROPERTY(basetype, name) \
	IMPL_PUT_PROPERTYEX(basetype, name, dirty)

#define IMPL_GETBSTR_PROPERTY(name) \
	STDMETHOD(get_##name)(/*[out, retval]*/ BSTR* value) \
	{*value = m_##name.Copy(); return S_OK;} 

#define IMPL_GETBSTR_PROPERTY2(name) \
	STDMETHOD(get_##name)(/*[out, retval]*/ BSTR* value) \
	{*value = m_##name.copy(); return S_OK;} 

#define IMPL_GETBSTRC_PROPERTY(name) \
	STDMETHOD(get_##name)(/*[out, retval]*/ BSTR* value) \
	{*value = m_##name.AllocSysString();return S_OK;} 

#define IMPL_PUTBSTR_PROPERTY(name) \
	STDMETHOD(put_##name)(/*[in]*/ BSTR value) \
	{m_##name = value; return S_OK;}

#define IMPL_GETPTR_PROPERTY(basetype, name) \
	STDMETHOD(get_##name)(/*[out, retval]*/ basetype** value) \
	{*value = NULL; return (m_##name != NULL)?m_##name->QueryInterface(__uuidof(basetype), (void **)value):S_OK;} 

#define IMPL_PUTPTR_PROPERTYEX(basetype, name, dirty) \
	STDMETHOD(put_##name)(/*[in]*/ basetype* value) \
	{m_##dirty = VARIANT_TRUE; \
		if (m_##name != NULL) m_##name.Release(); \
		return (value != NULL)?value->QueryInterface(__uuidof(basetype), (void **) &m_##name):S_OK;}

#define IMPL_PUTPTRREF_PROPERTYEX(basetype, name, dirty) \
	STDMETHOD(putref_##name)(/*[in]*/ basetype* value) \
	{m_##name = value; m_##dirty = VARIANT_TRUE; return S_OK;}

#define IMPL_PUTPTRREF_PROPERTY2EX(basetype, name, dirty) \
	STDMETHOD(putref_##name)(/*[in]*/ basetype* value) \
	{m_##dirty = VARIANT_TRUE; \
		if (m_##name != NULL) m_##name.Release(); \
		return (value != NULL)?value->QueryInterface(__uuidof(basetype), (void **) &m_##name):S_OK;}

#define IMPL_PUTPTR_PROPERTY(basetype, name) \
	STDMETHOD(put_##name)(/*[in]*/ basetype* value) \
	{if (m_##name != NULL) m_##name.Release(); \
		return (value != NULL)?value->QueryInterface(__uuidof(basetype), (void **) &m_##name):S_OK;}

#define IMPL_PUTPTRREF_PROPERTY(basetype, name) \
	STDMETHOD(putref_##name)(/*[in]*/ basetype* value) \
	{m_##name = value; return S_OK;}

#define IMPL_PUTPTRREF_PROPERTY2(basetype, name) \
	STDMETHOD(putref_##name)(/*[in]*/ basetype* value) \
	{if (m_##name != NULL) m_##name.Release(); \
		return (value != NULL)?value->QueryInterface(__uuidof(basetype), (void **) &m_##name):S_OK;}

#endif //_INTERFACEIMPL_H_