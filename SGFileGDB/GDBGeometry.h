#pragma once

#include "../wkb.h"
#include "filegdbapi.h"


VARIANT GDBGeometryToGeometry(BYTE* pBuf, int len, WKS::WKSEnvelope* pBound);

int GeometryToGDB(BYTE* pDataSrc, int nLen, FileGDBAPI::ShapeBuffer* pShape, bool b2Z, bool b2M);
int GeometryToGDBMultiPatch(BYTE* pDataSrc, int nLen, FileGDBAPI::ShapeBuffer* pShape, bool b2Z, bool b2M);