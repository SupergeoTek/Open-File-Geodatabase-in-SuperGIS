#ifndef _GDBERROR_
#define _GDBERROR_


// MessageText:
//
//  An unknown error occurred.
//
#define GDB_E_UNKNOWN				_HRESULT_TYPEDEF_(0x80110001L)

// MessageText:
//
//  Invalid data type or name found.
//
#define GDB_E_INVALIDDATATYPE		_HRESULT_TYPEDEF_(0x80110002L)

// MessageText:
//
//  Invalid table name.
//
#define GDB_E_INVALIDTABLENAME		_HRESULT_TYPEDEF_(0x80110003L)

// MessageText:
//
//  An invalid column name was found.
//
#define GDB_E_INVALIDCOLUMNNAME		_HRESULT_TYPEDEF_(0x80110004L)

// MessageText:
//
//  Name is already used by an existing object or table.
//
#define GDB_E_NAMEISALREADYUSED		_HRESULT_TYPEDEF_(0x80110005L)

// MessageText:
//
//  Table or certain field name is too long.
//
#define GDB_E_NAMETOOLONG			_HRESULT_TYPEDEF_(0x80110006L)

// MessageText:
//
//  No column was found in this table.
//
#define GDB_E_NOCOLUMNFOUND			_HRESULT_TYPEDEF_(0x80110007L)

// MessageText:
//
//  Error occurred while updating record.
//
#define GDB_E_UPDATEFAIL			_HRESULT_TYPEDEF_(0x80110008L)

// MessageText:
//
//  Invalid characters found.
//
#define GDB_E_INVALIDCHARACTER		_HRESULT_TYPEDEF_(0x80110009L)


#endif//_GDBERROR_
