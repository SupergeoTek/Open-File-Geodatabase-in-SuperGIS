#include "stdafx.h"
#include "GDBGeometry.h"


#define SHPP_TRISTRIP   0
#define SHPP_TRIFAN     1
#define SHPP_OUTERRING  2
#define SHPP_INNERRING  3
#define SHPP_FIRSTRING  4
#define SHPP_RING       5
#define SHPP_TRIANGLES  6 

void setWKBLineString(WKBLineString* geom, int ptn, double* pXY, double* pZ, double* pM)
{
	geom->lineString.numPoints = ptn;
	double* pDest = (double*)(geom->lineString.points);
	for (int i = 0; i < ptn; i++)
	{
		memcpy(pDest, pXY, 16);
		pDest += 2;
		pXY += 2;
		if (pZ)
		{
			memcpy(pDest, pZ, 8);
			pDest++;
			pZ++;
		}
		if (pM)
		{
			memcpy(pDest, pM, 8);
			pDest++;
			pM++;
		}
	}
}

void setWKBPolygon(WKBPolygon* geom, int ptn, double* pXY, double* pZ, double* pM, bool bClose)
{
	geom->numRings = 1;
	geom->rings[0].numPoints = ptn;
	double* pDest = (double*)(geom->rings[0].points);
	for (int i = 0; i < ptn; i++)
	{
		//TRACE("%d/%d=%.2f, %.2f\n", i, ptn, pXY[0], pXY[1]);
		memcpy(pDest, pXY, 16);
		pDest += 2;
		pXY += 2;
		if (pZ)
		{
			memcpy(pDest, pZ, 8);
			pDest++;
			pZ++;
		}
		if (pM)
		{
			memcpy(pDest, pM, 8);
			pDest++;
			pM++;
		}
	}
	if (bClose)
	{
		double* pt0=(double*)(geom->rings[0].points);
		if (memcmp(pt0,(pXY-2),16) != 0)
		{
			geom->rings[0].numPoints = ptn + 1;
			memcpy(pDest, pt0, 16);
			pDest += 2;
			pt0 += 2;
			if (pZ)
			{
				memcpy(pDest, pt0, 8);
				pt0++;
				pDest++;
			}
			if (pM)
				memcpy(pDest, pt0, 8);
		}
	}
}


void WKBPolygonAddRing(WKBPolygon* geom, int ptn, double* pXY, double* pZ, double* pM)
{
	WKS::WKSLineString* ls = geom->rings;
	for (int k=0; k<geom->numRings; k++)
		ls = NextLineString(ls, (pZ!=NULL), (pM!=NULL));

	geom->numRings++;
	ls->numPoints = ptn;
	double* pDest = (double*)(ls->points);
	for (int i = 0; i < ptn; i++)
	{
		//TRACE("r%d/%d=%.2f, %.2f\n", i,ptn, pXY[0], pXY[1]);
		memcpy(pDest, pXY, 16);
		pDest += 2;
		pXY += 2;
		if (pZ)
		{
			memcpy(pDest, pZ, 8);
			pDest++;
			pZ++;
		}
		if (pM)
		{
			memcpy(pDest, pM, 8);
			pDest++;
			pM++;
		}
	}
	double* pt0 = (double*)(ls->points);
	if (memcmp(pt0, (pXY - 2), 16) != 0)
	{
		ls->numPoints = ptn + 1;
		memcpy(pDest, pt0, 16);
		pDest += 2;
		pt0 += 2;
		if (pZ)
		{
			memcpy(pDest, pt0, 8);
			pt0++;
			pDest++;
		}
		if (pM)
			memcpy(pDest, pt0, 8);
	}
}

VARIANT CreateFromMultiPatch(int nParts, int* pPartStart, int* pPtypes, int n, double* pXY, double* pZ, double* pM)
{
	VARIANT var;
	var.vt = VT_EMPTY;
	int sizept = 16;
	int wtype = 0;
	if (pZ)
	{
		wtype = wkbZ;
		sizept += 8;
	}
	if (pM)
	{
		wtype += wkbM;
		sizept += 8;
	}
	int parti, ptn;
	int n2 = 0;
	int nPart2 = 0;
	int nPartType;
	int nring = 0;
	bool bNew = false;
	for (int k = 0; k < nParts; k++)
	{
		parti = pPartStart[k];
		ptn = (k == (nParts - 1)) ? n - parti : pPartStart[k + 1] - parti;
		nPartType = pPtypes[k] & 0xf;
		if (nPartType == SHPP_OUTERRING || nPartType == SHPP_INNERRING
			|| nPartType == SHPP_FIRSTRING || nPartType == SHPP_RING)
		{
			if ((nPartType == SHPP_OUTERRING || nPartType == SHPP_FIRSTRING))
			{
				nPart2++;
				nring--;
				//if (nring < 0)
				//	TRACE("r nring<0, %d\n", k);
			}
			n2 += (ptn+1);	//closeRings
			bNew = true;
			nring++;
		}
		else
		{
			if (bNew)
			{
				nPart2++;
				bNew = false;
			}
			if (nPartType == SHPP_TRISTRIP || nPartType == SHPP_TRIFAN)
			{
				nPart2 += (ptn - 2);
				n2 += (4 * (ptn - 2));
			}
			else if (nPartType == SHPP_TRIANGLES)
			{
				for (int i = 0; i < ptn - 2; i += 3)
				{
					nPart2++;
					n2 += 4;
				}
			}
		}
	}
	var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBGeometryCollection) + nPart2*sizeof(WKBPolygon)+ nring*4 + n2*sizept);
	if (var.parray == NULL)
		return var;

	var.vt = VT_ARRAY | VT_UI1;
	WKBGeometryCollection *geom;
	SafeArrayAccessData(var.parray, (void**)&geom);
	geom->byteOrder = wkbNDR;
	geom->wkbType = wkbMultiPolygon + wtype;
	geom->num_wkbGeometries = 0; // nPart2;

	double* pXY2 = pXY;
	double* pZ2 = pZ;
	double* pM2 = pM;
	double* pXY3 = pXY;
	double* pZ3 = pZ;
	double* pM3 = pM;
	bNew = false;
	WKBPolygon* geom2=NULL;
	WKBGeometry* tgeom = geom->WKBGeometries;
	tgeom->byteOrder = wkbNDR;
	tgeom->wkbType = wkbPolygon + wtype;
	for (int k = 0; k < nParts; k++)
	{
		parti = pPartStart[k];
		ptn = (k == (nParts - 1)) ? n - parti : pPartStart[k + 1] - parti;
		pXY2 = pXY + (parti + parti);
		if (pZ)
			pZ2 = pZ + parti;
		if (pM)
			pM2 = pM + parti;

		nPartType = pPtypes[k] & 0xf;
		if (nPartType == SHPP_OUTERRING || nPartType == SHPP_INNERRING
			|| nPartType == SHPP_FIRSTRING || nPartType == SHPP_RING)
		{
			if ((nPartType == SHPP_OUTERRING || nPartType == SHPP_FIRSTRING))
			{
				if (geom2)
				{
					tgeom = NextGeom(tgeom);
					geom2 = NULL;
				}
			}
			if (geom2==NULL)
			{
				tgeom->byteOrder = wkbNDR;
				tgeom->wkbType = wkbPolygon + wtype;
				geom->num_wkbGeometries++;
				geom2 = (WKBPolygon*)tgeom;
				geom2->numRings = 0;
			}
			bNew = true;
			WKBPolygonAddRing(geom2, ptn, pXY2, pZ2, pM2);
		}
		else
		{
			if (bNew)
			{
				bNew = false;
				tgeom = NextGeom(tgeom);
			}
			if (nPartType == SHPP_TRISTRIP)
			{
				for (int i = 0; i < ptn - 2; i++)
				{
					tgeom->byteOrder = wkbNDR;
					tgeom->wkbType = wkbPolygon + wtype;
					geom->num_wkbGeometries++;
					setWKBPolygon((WKBPolygon*)tgeom, 3, pXY2, pZ2, pM2, true);
					tgeom = NextGeom(tgeom);
					pXY2 += 2;
					if (pZ2)
						pZ2++;
					if (pM2)
						pM2++;
				}
			}
			else if (nPartType == SHPP_TRIFAN)
			{
				double* pDest;
				double* p0 = pXY2;
				double* pZ0 = pZ2;
				double* pM0 = pM2;

				pXY2 += 2;
				if (pZ2)
					pZ2++;
				if (pM2)
					pM2++;
				for (int i = 0; i < ptn - 2; i++) //0, +1, +2, 0
				{
					tgeom->byteOrder = wkbNDR;
					tgeom->wkbType = wkbPolygon + wtype;
					geom->num_wkbGeometries++;
					geom2 = (WKBPolygon*)tgeom;
					geom2->numRings = 1;
					geom2->rings[0].numPoints = 4;
					pDest = (double*)(geom2->rings[0].points);
					{
						memcpy(pDest, p0, 16);
						pDest += 2;
						if (pZ2)
						{
							memcpy(pDest, pZ0, 8);
							pDest++;
						}
						if (pM2)
						{
							memcpy(pDest, pM0, 8);
							pDest++;
						}

						memcpy(pDest, pXY2, 16);
						pDest += 2;
						pXY2 += 2;
						if (pZ2)
						{
							memcpy(pDest, pZ2, 8);
							pDest++;
							pZ2++;
						}
						if (pM2)
						{
							memcpy(pDest, pM2, 8);
							pDest++;
							pM2++;
						}

						memcpy(pDest, pXY2, 16);
						pDest += 2;
						if (pZ2)
						{
							memcpy(pDest, pZ2, 8);
							pDest++;
						}
						if (pM2)
						{
							memcpy(pDest, pM2, 8);
							pDest++;
						}
						memcpy(pDest, p0, 16);
						pDest += 2;
						if (pZ2)
						{
							memcpy(pDest, pZ0, 8);
							pDest++;
						}
						if (pM2)
						{
							memcpy(pDest, pM0, 8);
							pDest++;
						}
					}
					tgeom = NextGeom(tgeom);
				}
				geom2 = NULL;
			}
			else if (nPartType == SHPP_TRIANGLES)
			{
				for (int i = 0; i < ptn - 2; i += 3)
				{
					tgeom->byteOrder = wkbNDR;
					tgeom->wkbType = wkbPolygon + wtype;
					geom->num_wkbGeometries++;
					setWKBPolygon((WKBPolygon*)tgeom, 3, pXY2, pZ2, pM2, true);
					tgeom = NextGeom(tgeom);
					pXY2 += 6;
					if (pZ2)
						pZ2+=3;
					if (pM2)
						pM2+=3;
				}
			}
		}
	}
	if (geom->num_wkbGeometries != nPart2)
		TRACE(" err numwkb=%d, nPart2=%d\n", geom->num_wkbGeometries, nPart2);
	SafeArrayUnaccessData(var.parray);
	return var;
}

VARIANT GDBGeometryToGeometry(BYTE* pBuf, int len, WKS::WKSEnvelope* pBound)
{
	VARIANT var;
	VariantInit(&var);
	int nSHPType = pBuf[0];
	if (nSHPType == FileGDBAPI::shapeNull)
		return var;
	bool bIsExtended = (nSHPType >= FileGDBAPI::shapeGeneralPolyline && nSHPType <= FileGDBAPI::shapeGeneralMultiPatch);
	bool bHasZ = (
		nSHPType == FileGDBAPI::shapePointZ || nSHPType == FileGDBAPI::shapePointZM
		|| nSHPType == FileGDBAPI::shapeMultipointZ || nSHPType == FileGDBAPI::shapeMultipointZM
		|| nSHPType == FileGDBAPI::shapePolygonZ || nSHPType == FileGDBAPI::shapePolygonZM
		|| nSHPType == FileGDBAPI::shapePolylineZ || nSHPType == FileGDBAPI::shapePolylineZM
		|| nSHPType == FileGDBAPI::shapeMultiPatch || nSHPType == FileGDBAPI::shapeMultiPatchM
		|| (bIsExtended && (pBuf[3] & 0x80) != 0));

	bool bHasM = (
		nSHPType == FileGDBAPI::shapePointM || nSHPType == FileGDBAPI::shapePointZM
		|| nSHPType == FileGDBAPI::shapeMultipointM	|| nSHPType == FileGDBAPI::shapeMultipointZM
		|| nSHPType == FileGDBAPI::shapePolygonM	|| nSHPType == FileGDBAPI::shapePolygonZM
		|| nSHPType == FileGDBAPI::shapePolylineM 	|| nSHPType == FileGDBAPI::shapePolylineZM
		|| nSHPType == FileGDBAPI::shapeMultiPatchM
		|| (bIsExtended && (pBuf[3] & 0x40) != 0));

	//if (nSHPType > 30)
	//	TRACE("shptype=%d, isext=%d,bz=%d,bm=%d\n", nSHPType, bIsExtended, bHasZ, bHasM);

	if (nSHPType == FileGDBAPI::shapeGeneralPolyline)
		nSHPType = FileGDBAPI::shapePolyline;
	else if (nSHPType == FileGDBAPI::shapeGeneralPolygon)
		nSHPType = FileGDBAPI::shapePolygon;
	else if (nSHPType == FileGDBAPI::shapeGeneralPoint)
		nSHPType = FileGDBAPI::shapePoint;
	else if (nSHPType == FileGDBAPI::shapeGeneralMultipoint)
		nSHPType = FileGDBAPI::shapeMultipoint;
	else if (nSHPType == FileGDBAPI::shapeGeneralMultiPatch)
		nSHPType = FileGDBAPI::shapeMultiPatch;

	int sizept = 16;
	int wtype = 0;
	if (bHasZ)
	{
		wtype = wkbZ;
		sizept += 8;
	}
	if (bHasM)
	{
		wtype += wkbM;
		sizept += 8;
	}

	if (nSHPType == FileGDBAPI::shapePoint || nSHPType == FileGDBAPI::shapePointZ ||
		nSHPType == FileGDBAPI::shapePointM || nSHPType == FileGDBAPI::shapePointZM)
	{
		if (pBound)
		{
			pBound->xMin = *(double*)(pBuf + 4);
			pBound->yMin = *(double*)(pBuf + 12);
			pBound->xMax = pBound->xMin;
			pBound->yMax = pBound->yMin;
			var.vt = VT_I4;
			var.lVal = 1;
		}
		else
		{
			var.vt = VT_ARRAY | VT_UI1;
			var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBGeometry)+ sizept);
			WKBPoint *geom;
			SafeArrayAccessData(var.parray, (void**)&geom);
			geom->byteOrder = wkbNDR;
			geom->wkbType = wkbPoint+wtype;
			memcpy(&geom->point, pBuf + 4, sizept);
			SafeArrayUnaccessData(var.parray);
		}
	}
	else if (nSHPType == FileGDBAPI::shapeMultipoint || nSHPType == FileGDBAPI::shapeMultipointM ||
		nSHPType == FileGDBAPI::shapeMultipointZ || nSHPType == FileGDBAPI::shapeMultipointZM)
	{
		int n = *(int*)(pBuf + 36);
		if (pBound)
		{
			double v;
			pBound->xMin = *(double*)(pBuf + 40);
			pBound->yMin = *(double*)(pBuf + 48);
			pBound->xMax = pBound->xMin;
			pBound->yMax = pBound->yMin;
			pBuf += 56;
			for (int i = 1; i < n; i++)
			{
				v = *(double*)pBuf;
				if (v < pBound->xMin)
					pBound->xMin = v;
				else if (v > pBound->xMax)
					pBound->xMax = v;

				v = *(double*)(pBuf+8);
				if (v < pBound->yMin)
					pBound->yMin = v;
				else if (v > pBound->yMax)
					pBound->yMax = v;

				pBuf += 16;
			}
			var.vt = VT_I4;
			var.lVal = 1;
		}
		else
		{
			var.vt = VT_ARRAY | VT_UI1;
			var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBGeometryCollection) +(n-1)*(sizept+sizeof(WKBGeometry))+sizept);
			WKBGeometryCollection *geom;
			SafeArrayAccessData(var.parray, (void**)&geom);
			geom->byteOrder = wkbNDR;
			geom->wkbType = wkbMultiPoint+wtype;
			geom->num_wkbGeometries = n;

			WKBPoint* ptgeom;
			WKBGeometry* tgeom=geom->WKBGeometries;
			BYTE* pSrc = pBuf + 40;
			BYTE* pZ = pSrc + 16 + 16*n;
			BYTE* pM = (bHasZ) ? (pZ + 16 + 8*n) : pZ;
			BYTE* pDest;
			for (int i = 0; i < n; i++)
			{
				ptgeom = (WKBPoint*)tgeom;
				ptgeom->byteOrder = wkbNDR;
				ptgeom->wkbType = wkbPoint+wtype;
				pDest = (BYTE*)&(ptgeom->point);
				memcpy(pDest, pSrc, 16);
				pDest += 16;
				if (bHasZ)
				{
					memcpy(pDest, pZ, 8) ;
					pZ += 8;
					pDest += 8;
				}
				if (bHasM)
				{
					memcpy(pDest, pM, 8);
					pM += 8;
					pDest += 8;
				}
				pSrc += 16;
				tgeom = NextGeom(tgeom);
			}
			SafeArrayUnaccessData(var.parray);
		}
	}
	else
	{
		int nParts = *(int*)(pBuf + 36);
		int n= *(int*)(pBuf + 40);
		if (nParts < 0 || n < 0 || len<44)
			return var;
		int nSize = 44 + 4 * nParts + 16 * n;
		if (bHasZ)
			nSize += (16 + 8 * n);
		if (bHasM)
			nSize += (16 + 8 * n);

		int nOffset = 44 + 4 * nParts;
		int* pPtypes=NULL;
		bool bIsMultiPatch = (nSHPType == FileGDBAPI::shapeMultiPatch || nSHPType == FileGDBAPI::shapeMultiPatchM);
		if (bIsMultiPatch)
		{
			nSize += (4 * nParts);
			pPtypes = (int*)(pBuf+nOffset);
			nOffset += (4 * nParts);  //PartType[]
		}
		//if (nSize != len)
		//	TRACE("err len=%d, nSize=%d, nParts=%d, points=%d\n", len, nSize, nParts, n);
		if (nSize > len)
			return var;
		
		int parti;
		int* pPartStart = (int*)(pBuf + 44);
		for (int k = 0; k < nParts; k++)
		{
			parti = pPartStart[k];
			if (parti < 0 || parti >= n)
				return var;
			if (k > 0 && parti <= pPartStart[k - 1])
				return var;
		}
		if (pBound)
		{
			double v;
			pBound->xMin = *(double*)(pBuf + nOffset);
			pBound->yMin = *(double*)(pBuf + nOffset + 8);
			pBound->xMax = pBound->xMin;
			pBound->yMax = pBound->yMin;
			pBuf += nOffset + 16;
			for (int i = 1; i < n; i++)
			{
				v = *(double*)pBuf;
				if (v < pBound->xMin)
					pBound->xMin = v;
				else if (v > pBound->xMax)
					pBound->xMax = v;

				v = *(double*)(pBuf + 8);
				if (v < pBound->yMin)
					pBound->yMin = v;
				else if (v > pBound->yMax)
					pBound->yMax = v;

				pBuf += 16;
			}
			var.vt = VT_I4;
			var.lVal = 1;
		}
		else
		{
			int ptn;
			int offi = 16*n;
			double* pXY= (double*)(pBuf + nOffset);
			double* pZ = NULL;
			double* pM = NULL;
			if (bHasZ)
			{
				pZ = (double*)(pBuf + nOffset + 16 + offi);
				offi += (8*n);
			}
			if (bHasM)
				pM = (double*)(pBuf + nOffset + 16 + offi);

			var.vt = VT_ARRAY | VT_UI1;

			if (nSHPType == FileGDBAPI::shapePolyline || nSHPType == FileGDBAPI::shapePolylineZ
				|| nSHPType == FileGDBAPI::shapePolylineM || nSHPType == FileGDBAPI::shapePolylineZM)
			{
				if (nParts <= 1)
				{
					var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBLineString) - 16 + n*sizept);
					WKBLineString *geom;
					SafeArrayAccessData(var.parray, (void**)&geom);
					geom->byteOrder = wkbNDR;
					geom->wkbType = wkbLineString + wtype;
					setWKBLineString(geom, n, pXY, pZ, pM);
				}
				else
				{
					var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBGeometryCollection)-sizeof(WKBGeometry) + nParts*(sizeof(WKBLineString)-16)+ n*sizept);
					WKBGeometryCollection *geom;
					SafeArrayAccessData(var.parray, (void**)&geom);
					geom->byteOrder = wkbNDR;
					geom->wkbType = wkbMultiLineString + wtype;
					geom->num_wkbGeometries = nParts;

					double* pXY2=pXY;
					double* pZ2 = pZ;
					double* pM2 = pM;
					WKBGeometry* tgeom = geom->WKBGeometries;
					for (int k = 0; k < nParts; k++)
					{
						parti = pPartStart[k];
						ptn = (k == (nParts - 1)) ? n - parti : pPartStart[k + 1] - parti;
						tgeom->byteOrder = wkbNDR;
						tgeom->wkbType = wkbLineString + wtype;
						pXY2 = pXY + (parti+parti);
						if (pZ)
							pZ2 = pZ + parti;
						if (pM)
							pM2 = pM + parti;
						setWKBLineString((WKBLineString*)tgeom, ptn, pXY2, pZ2, pM2);
						tgeom = NextGeom(tgeom);
					}
				}
				SafeArrayUnaccessData(var.parray);
			}
			else if (nSHPType == FileGDBAPI::shapePolygon	|| nSHPType == FileGDBAPI::shapePolygonZ
				|| nSHPType == FileGDBAPI::shapePolygonM	|| nSHPType == FileGDBAPI::shapePolygonZM)
			{
				if (nParts <= 1)
				{
					if (nParts == 1)
					{
						n -= pPartStart[0];
						pXY += (pPartStart[0]+ pPartStart[0]);
						if (pZ)
							pZ += pPartStart[0];
						if (pM)
							pM += pPartStart[0];
					}
					var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBPolygon) - 16 + n*sizept);
					WKBPolygon *geom;
					SafeArrayAccessData(var.parray, (void**)&geom);
					geom->byteOrder = wkbNDR;
					geom->wkbType = wkbPolygon + wtype;
					setWKBPolygon(geom, n, pXY, pZ, pM, false);
				}
				else
				{
					var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBPolygon)+ nParts*4 + n*sizept);
					//var.parray = SafeArrayCreateVector(VT_UI1, 0, sizeof(WKBGeometryCollection) - sizeof(WKBGeometry) + nParts*(sizeof(WKBPolygon) - 16) + n*sizept);
					//WKBGeometryCollection *geom;
					WKBPolygon *geom;
					SafeArrayAccessData(var.parray, (void**)&geom);
					geom->byteOrder = wkbNDR;
					geom->wkbType = wkbPolygon + wtype; //wkbMultiPolygon + wtype;
					//geom->num_wkbGeometries = nParts;
					geom->numRings = 0; //nParts;
					WKS::WKSLineString* lr = geom->rings;

					double* pXY2 = pXY;
					double* pZ2 = pZ;
					double* pM2 = pM;
					//WKBGeometry* tgeom = geom->WKBGeometries;
					for (int k = 0; k < nParts; k++)
					{
						parti = pPartStart[k];
						ptn = (k == (nParts - 1)) ? n - parti : pPartStart[k + 1] - parti;
						//tgeom->byteOrder = wkbNDR;
						//tgeom->wkbType = wkbPolygon + wtype;
						pXY2 = pXY + (parti + parti);
						if (pZ)
							pZ2 = pZ + parti;
						if (pM)
							pM2 = pM + parti;
						
						//setWKBPolygon((WKBPolygon*)tgeom, ptn, pXY2, pZ2, pM2, false);
						//tgeom = NextGeom(tgeom);
						WKBPolygonAddRing(geom, ptn, pXY2, pZ2, pM2);
						//lr = NextLineString(lr, (pZ ? true : false), (pM?true:false));
					}
				}
				SafeArrayUnaccessData(var.parray);
			}
			else if (bIsMultiPatch)
			{
				var = CreateFromMultiPatch(nParts, pPartStart, pPtypes, n, pXY, pZ, pM);
			}
		}
	}
	return var;
}

int GeometryToGDB(BYTE* pDataSrc, int nLen, FileGDBAPI::ShapeBuffer* pShape, bool b2Z, bool b2M)
{
	int nShpSize = 4; // All types start with integer type number
	int nShpZSize = 0; // Z gets tacked onto the end
	int nPoints = 0;
	int nParts = 0;

	if (pDataSrc==NULL || nLen<=4)
	{
		if (!pShape->Allocate(4))
			return -1;
		pShape->inUseLength = 4;
		int zero = FileGDBAPI::shapeNull;
		memcpy(pShape->shapeBuffer, &zero, nShpSize);
		return 4;
	}
	WKBGeometry* pGeom = (WKBGeometry*)pDataSrc;
	int wType = pGeom->wkbType % wkbGeometryType::wkbType;
	int nt= pGeom->wkbType / wkbGeometryType::wkbType;

	bool bZ = ((nt&1)==1);
	bool bM = ((nt&2)==2);
	int sizePt = 16;
	if (bZ)
		sizePt+=8;
	if (bM)
		sizePt += 8;

	int n2CoordDims = 2;
	if (b2Z)
		n2CoordDims++;
	if (b2M)
		n2CoordDims++;

	int size2Pt = n2CoordDims * 8;
	bool bSize = false;
	if (wType == wkbGeometryType::wkbPoint)
	{
		nShpSize += size2Pt;
		bSize = true;
	}
	else if (wType == wkbGeometryType::wkbLineString)
	{
		WKBLineString *poLine = (WKBLineString*)pGeom;
		nPoints = poLine->lineString.numPoints;
		nParts = 1;
	}
	else if (wType == wkbGeometryType::wkbPolygon)
	{
		WKBPolygon *poPoly = (WKBPolygon*)pGeom;
		nParts = poPoly->numRings;
		WKS::WKSLineString* ls = poPoly->rings;
		for (int i = 0; i < poPoly->numRings; i++)
		{
			nPoints += ls->numPoints;
			ls=NextLineString(ls, bZ, bM);
		}
	}
	else
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		nParts = pGC->num_wkbGeometries;
		if (wType == wkbGeometryType::wkbGeometryCollection)
			wType = tgeom->wkbType % wkbGeometryType::wkbType;

		if (wType == wkbGeometryType::wkbMultiPoint)
		{
			nPoints = nParts;
			nShpSize += 16 * n2CoordDims; // xy(z)(m) box 
			nShpSize += 4; // npoints 
			nShpSize += size2Pt * nPoints; // points 
			nShpZSize = 16 + 8 * nPoints;
			if (bZ) nShpSize += 16;
			if (bM) nShpSize += 16;
			bSize = true;
		}
		else if (wType == wkbGeometryType::wkbMultiLineString)
		{
			WKBLineString *poLine;
			for (int i = 0; i < pGC->num_wkbGeometries; i++)
			{
				poLine = (WKBLineString*)tgeom;
				nPoints += poLine->lineString.numPoints;
				tgeom = NextGeom(tgeom);
			}
		}
		else if (wType == wkbGeometryType::wkbMultiPolygon)
		{
			nParts = 0;
			WKBPolygon *poPoly;
			for (int i = 0; i < pGC->num_wkbGeometries; i++)
			{
				poPoly = (WKBPolygon*)tgeom;
				nParts += poPoly->numRings;
				WKS::WKSLineString* ls = poPoly->rings;
				for (int k = 0; k < poPoly->numRings; k++)
				{
					nPoints += ls->numPoints;
					ls = NextLineString(ls, bZ, bM);
				}
				tgeom = NextGeom(tgeom);
			}
		}
	}
	if (!bSize)
	{
		nShpSize += 16 * n2CoordDims; // xy(z)(m) box 
		nShpSize += 4; // nparts 
		nShpSize += 4; // npoints 
		nShpSize += 4 * nParts; // parts[nparts] 
		nShpSize += size2Pt * nPoints; // points 
		nShpZSize = 16 + 8 * nPoints;
		if (bZ) nShpSize += 16;
		if (bM) nShpSize += 16;
	}
	if (!pShape->Allocate(nShpSize))
		return -1;

	pShape->inUseLength = nShpSize;
	unsigned char *pabyPtr = pShape->shapeBuffer;
	unsigned char *pabyPtrZ = NULL;
	unsigned char *pabyPtrM = NULL;
	memset(pabyPtr, 0, nShpSize);
	if (b2M)
		pabyPtrM = pabyPtr + nShpSize - nShpZSize;
	if (b2Z)
	{
		if (b2M)
			pabyPtrZ = pabyPtrM - nShpZSize;
		else
			pabyPtrZ = pabyPtr + nShpSize - nShpZSize;
	}

	int nGType = FileGDBAPI::shapeNull;
	if (wType==wkbPoint)
	{
		nGType = (b2Z && b2M) ? FileGDBAPI::shapePointZM :
			(b2Z) ? FileGDBAPI::shapePointZ :
			(b2M) ? FileGDBAPI::shapePointM :
			FileGDBAPI::shapePoint;
	}
	else if (wType == wkbMultiPoint)
	{
		nGType = (b2Z && b2M) ? FileGDBAPI::shapeMultipointZM :
			(b2Z) ? FileGDBAPI::shapeMultipointZ :
			(b2M) ? FileGDBAPI::shapeMultipointM :
			FileGDBAPI::shapeMultipoint;
	}
	else if (wType == wkbLineString || wType== wkbMultiLineString)
	{
		nGType = (b2Z && b2M) ? FileGDBAPI::shapePolylineZM :
			(b2Z) ? FileGDBAPI::shapePolylineZ :
			(b2M) ? FileGDBAPI::shapePolylineM : //FileGDBAPI::shapeGeneralPolyline;
			FileGDBAPI::shapePolyline;  
	}
	else if (wType == wkbPolygon || wType==wkbMultiPolygon)
	{
		nGType = (b2Z && b2M) ? FileGDBAPI::shapePolygonZM :
			(b2Z) ? FileGDBAPI::shapePolygonZ :
			(b2M) ? FileGDBAPI::shapePolygonM : //FileGDBAPI::shapeGeneralPolygon;
			FileGDBAPI::shapePolygon;	
	}

	memcpy(pabyPtr, &nGType, 4);
	pabyPtr += 4;
	if (wType == wkbPoint)
	{
		WKBPoint *poPoint = (WKBPoint*)pGeom;
		memcpy(pabyPtr, &poPoint->point, min(sizePt, size2Pt));
		return nShpSize;
	}

	WKS::WKSEnvelope envelope;
	envelope.xMin = FLT_MAX;
	envelope.xMax = -FLT_MAX;
	envelope.yMin = FLT_MAX;
	envelope.yMax = -FLT_MAX;
	BYTE* pabyPtrBounds = pabyPtr;
	//memcpy(pabyPtr, &(envelope.xMin), 8);
	//memcpy(pabyPtr + 8, &(envelope.yMin), 8);
	//memcpy(pabyPtr + 16, &(envelope.xMax), 8);
	//memcpy(pabyPtr + 24, &(envelope.yMax), 8);
	pabyPtr += 32;

	BYTE* pabyPtrZBounds = NULL;
	double minZ = FLT_MAX;
	double maxZ = -minZ;
	if (b2Z)
	{
		pabyPtrZBounds = pabyPtrZ;
		pabyPtrZ += 16;
	}

	double v;
	// Reserve space for the M bounds at the end of the XY buffer
	BYTE* pabyPtrMBounds = NULL;
	double dfMinM = FLT_MAX;
	double dfMaxM = -dfMinM;
	if (b2M)
	{
		pabyPtrMBounds = pabyPtrM;
		pabyPtrM += 16;
	}

	if (wType == wkbGeometryType::wkbLineString)
	{
		WKBLineString *poLine = (WKBLineString*)pGeom;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		int nPartIndex = 0;
		memcpy(pabyPtr, &nPartIndex, 4);
		pabyPtr += 4;

		// Write in the point data
		double* pt=(double*)(poLine->lineString.points);
		for (int i = 0; i < poLine->lineString.numPoints; i++)
		{
			memcpy(pabyPtr, pt, 16);
			pabyPtr += 16;
			v = *pt;
			if (v < envelope.xMin) envelope.xMin = v;
			if (v > envelope.xMax) envelope.xMax = v;
			v = pt[1];
			if (v < envelope.yMin) envelope.yMin = v;
			if (v > envelope.yMax) envelope.yMax = v;
			pt += 2;
			if (bZ)
			{
				if (b2Z)
				{
					memcpy(pabyPtrZ, pt, 8);
					pabyPtrZ += 8;
					v = *pt;
					if (v < minZ) minZ = v;
					if (v > maxZ) maxZ = v;
				}
				pt++;
			}
			if (bM)
			{
				if (b2M)
				{
					memcpy(pabyPtrM, pt, 8);
					pabyPtrM += 8;
					v = *pt;
					if (v < dfMinM) dfMinM = v;
					if (v > dfMaxM) dfMaxM = v;
				}
				pt++;
			}
		}
	}
	else if (wType == wkbGeometryType::wkbPolygon)
	{
		WKBPolygon *poPoly = (WKBPolygon*)pGeom;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		// Just past the partindex[nparts] array
		unsigned char* pabyPoints = pabyPtr + 4 * nParts;

		int nPointIndexCount = 0;
		WKS::WKSLineString* ls = poPoly->rings;
		double* pt;
		// Outer ring must be clockwise
		for (int i = 0; i < poPoly->numRings; i++)
		{
			int nRingNumPoints = ls->numPoints;

			if (nRingNumPoints <= 2) // || !poRing->get_IsClosed())
				TRACE("polygon ring point error=%d\n", i);

			// Write in the part index
			memcpy(pabyPtr, &nPointIndexCount, 4);
			pabyPtr += 4;
			pt = (double*)(ls->points);
			for (int k = 0; k < nRingNumPoints; k++)
			{
				memcpy(pabyPoints, pt, 16);
				pabyPoints += 16;
				v = *pt;
				if (v < envelope.xMin) envelope.xMin = v;
				if (v > envelope.xMax) envelope.xMax = v;
				v = pt[1];
				if (v < envelope.yMin) envelope.yMin = v;
				if (v > envelope.yMax) envelope.yMax = v;
				pt += 2;
				if (bZ)
				{
					if (b2Z)
					{
						memcpy(pabyPtrZ, pt, 8);
						pabyPtrZ += 8;
						v = *pt;
						if (v < minZ) minZ = v;
						if (v > maxZ) maxZ = v;
					}
					pt++;
				}
				if (bM)
				{
					if (b2M)
					{
						memcpy(pabyPtrM, pt, 8);
						pabyPtrM += 8;
						v = *pt;
						if (v < dfMinM) dfMinM = v;
						if (v > dfMaxM) dfMaxM = v;
					}
					pt++;
				}
			}
			nPointIndexCount += nRingNumPoints;
			ls = NextLineString(ls, bZ, bM);
		}
	}
	else if (wType == wkbMultiPoint)
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;
		double* pt;
		for (int i = 0; i < pGC->num_wkbGeometries; i++)
		{
			pt = (double*)&(((WKBPoint*)tgeom)->point);
			memcpy(pabyPtr, pt, 16);
			pabyPtr += 16;
			v = *pt;
			if (v < envelope.xMin) envelope.xMin = v;
			if (v > envelope.xMax) envelope.xMax = v;
			v = pt[1];
			if (v < envelope.yMin) envelope.yMin = v;
			if (v > envelope.yMax) envelope.yMax = v;
			pt += 2;
			if (bZ)
			{
				if (b2Z)
				{
					memcpy(pabyPtrZ, pt, 8);
					pabyPtrZ += 8;
					v = *(pt);
					if (v < minZ) minZ = v;
					if (v > maxZ) maxZ = v;
				}
				pt++;
			}
			if (bM)
			{
				if (b2M)
				{
					memcpy(pabyPtrM, pt, 8);
					pabyPtrM += 8;
					v = *(pt);
					if (v < dfMinM) dfMinM = v;
					if (v > dfMaxM) dfMaxM = v;
				}
				pt++;
			}
			tgeom = NextGeom(tgeom);
		}
	}
	else if (wType == wkbMultiLineString)
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		unsigned char* pabyPoints = pabyPtr + 4 * nParts;
		int nPointIndexCount = 0;

		WKBLineString *poLine;
		for (int i = 0; i < pGC->num_wkbGeometries; i++)
		{
			poLine = (WKBLineString*)tgeom;
			int nLineNumPoints = poLine->lineString.numPoints;
			memcpy(pabyPtr, &nPointIndexCount, 4);
			pabyPtr += 4;
			double* pt = (double*)(poLine->lineString.points);
			for (int k = 0; k < nLineNumPoints; k++)
			{
				memcpy(pabyPoints, pt, 16);
				pabyPoints += 16;
				v = *pt;
				if (v < envelope.xMin) envelope.xMin = v;
				if (v > envelope.xMax) envelope.xMax = v;
				v = pt[1];
				if (v < envelope.yMin) envelope.yMin = v;
				if (v > envelope.yMax) envelope.yMax = v;
				pt += 2;
				if (bZ)
				{
					if (b2Z)
					{
						memcpy(pabyPtrZ, pt, 8);
						pabyPtrZ += 8;
						v = *(pt);
						if (v < minZ) minZ = v;
						if (v > maxZ) maxZ = v;
					}
					pt++;
				}
				if (bM)
				{
					if (b2M)
					{
						memcpy(pabyPtrM, pt, 8);
						pabyPtrM += 8;
						v = *(pt);
						if (v < dfMinM) dfMinM = v;
						if (v > dfMaxM) dfMaxM = v;
					}
					pt++;
				}
			}
			nPointIndexCount += nLineNumPoints;
			tgeom = NextGeom(tgeom);
		}
	}
	else // if ( nOGRType == wkbMultiPolygon ) 
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		unsigned char* pabyPoints = pabyPtr + 4 * nParts;

		int nPointIndexCount = 0;
		WKBPolygon *poPoly;
		for (int i = 0; i < pGC->num_wkbGeometries; i++)
		{
			poPoly = (WKBPolygon*)tgeom;
			WKS::WKSLineString* ls = poPoly->rings;
			double* pt;
			for (int j = 0; j < poPoly->numRings; j++)
			{
				int nRingNumPoints = ls->numPoints;
				memcpy(pabyPtr, &nPointIndexCount, 4);
				pabyPtr += 4;
				pt = (double*)(ls->points);
				for (int k = 0; k < nRingNumPoints; k++)
				{
					memcpy(pabyPoints, pt, 16);
					pabyPoints += 16;
					v = *pt;
					if (v < envelope.xMin) envelope.xMin = v;
					if (v > envelope.xMax) envelope.xMax = v;
					v = pt[1];
					if (v < envelope.yMin) envelope.yMin = v;
					if (v > envelope.yMax) envelope.yMax = v;
					pt += 2;
					if (bZ)
					{
						if (b2Z)
						{
							memcpy(pabyPtrZ, pt, 8);
							pabyPtrZ += 8;
							v = *(pt);
							if (v < minZ) minZ = v;
							if (v > maxZ) maxZ = v;
						}
						pt++;
					}
					if (bM)
					{
						if (b2M)
						{
							memcpy(pabyPtrM, pt, 8);
							pabyPtrM += 8;
							v = *(pt);
							if (v < dfMinM) dfMinM = v;
							if (v > dfMaxM) dfMaxM = v;
						}
						pt++;
					}
				}
				nPointIndexCount += nRingNumPoints;
				ls = NextLineString(ls, bZ, bM);
			}
			tgeom = NextGeom(tgeom);
		}
	}
	if (envelope.xMin > envelope.xMax)
	{
		memset(&envelope, 0, sizeof(WKS::WKSEnvelope));
	}
	memcpy(pabyPtrBounds, &envelope, 32);

	if (b2Z && bZ)
	{
		if (minZ > maxZ)
		{
			minZ = 0.0;
			maxZ = 0.0;
		}
		memcpy(pabyPtrZBounds, &(minZ), 8);
		memcpy(pabyPtrZBounds + 8, &(maxZ), 8);
	}
	if (b2M && bM)
	{
		if (dfMinM > dfMaxM)
		{
			dfMinM = 0.0;
			dfMaxM = 0.0;
		}
		memcpy(pabyPtrMBounds, &(dfMinM), 8);
		memcpy(pabyPtrMBounds + 8, &(dfMaxM), 8);
	}
	return nShpSize;
}


int GeometryToGDBMultiPatch(BYTE* pDataSrc, int nLen, FileGDBAPI::ShapeBuffer* pShape, bool b2Z, bool b2M)
{
	WKBGeometry* pGeom = (WKBGeometry*)pDataSrc;
	int wType = pGeom->wkbType % wkbGeometryType::wkbType;
	int nt = pGeom->wkbType / wkbGeometryType::wkbType;

	bool bZ = ((nt & 1) == 1);
	bool bM = ((nt & 2) == 2);
	int nCoordDims = 2;
	if (bZ)
		nCoordDims++;
	if (bM)
		nCoordDims++;

	int sizePt = nCoordDims * 8;

	//目前一律為Z的
	b2Z = true;
	b2M = false;
	int n2CoordDims = 2;
	if (b2Z)
		n2CoordDims++;
	if (b2M)
		n2CoordDims++;

	int size2Pt = n2CoordDims * 8;
	WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
	WKBGeometry* tgeom;
	
	if (wType == wkbGeometryType::wkbGeometryCollection)
	{
		tgeom = pGC->WKBGeometries;
		wType = tgeom->wkbType % wkbGeometryType::wkbType;
	}
	if (wType != wkbGeometryType::wkbMultiPolygon && wType!= wkbGeometryType::wkbPolygon)
		return -1;

	int nParts = 0;
	int* panPartStart = NULL;
	int* panPartType = NULL;
	int nPoints = 0;
	WKS::tagWKSPoint* poPoints = NULL;
	double* padfZ = NULL;
	int nBeginLastPart = 0;
	double* pt;

	int lastPartStart = 0 ;
	int lastPartType = -1;
	WKS::tagWKSPointZ beginLastpt;
	WKS::tagWKSPointZ lastpt;
	WKS::tagWKSPointZ lastpt2;
	memset(&beginLastpt,0,sizeof(WKS::tagWKSPointZ));
	memset(&lastpt, 0, sizeof(WKS::tagWKSPointZ));
	memset(&lastpt2, 0, sizeof(WKS::tagWKSPointZ));

	WKS::tagWKSPointZ ptz[4];
	memset(ptz, 0, sizeof(WKS::tagWKSPointZ) * 4);
	WKBPolygon *poPoly;
	tgeom = pGC->WKBGeometries;
	for (int j = 0; j < pGC->num_wkbGeometries; j++)
	{
		poPoly = (WKBPolygon*)tgeom;
		int nRings = poPoly->numRings;

		WKS::WKSLineString* ls = poPoly->rings;
		if (nRings == 1 && ls->numPoints == 4)
		{
			pt = (double*)ls->points;
			for (int i = 0; i < 4; i++)
			{
				memcpy(ptz+i, pt, 16);
				if (bZ)
					ptz[i].z = pt[2];
				pt += nCoordDims;
			}
			if (nParts > 0 && 
				((lastPartType == SHPP_TRIANGLES && nPoints - lastPartStart == 3) ||
					lastPartType == SHPP_TRIFAN) &&
				memcmp(ptz, &beginLastpt, 24)==0 &&
				memcmp(ptz+1, &lastpt, 24) == 0)
			{
				lastPartType = SHPP_TRIFAN;
				lastpt2 = lastpt;
				lastpt = ptz[2];
				nPoints++;
			}
			else if (nParts > 0 && 
				((lastPartType == SHPP_TRIANGLES && nPoints - lastPartStart == 3) ||
					lastPartType == SHPP_TRISTRIP) &&
				(memcmp(ptz, &lastpt2, 24) == 0) &&
				(memcmp(ptz + 1, &lastpt, 24) == 0))
			{
				lastPartType = SHPP_TRISTRIP;
				lastpt2 = lastpt;
				lastpt = ptz[2];
				nPoints++;
			}
			else
			{
				if (nParts == 0 || lastPartType != SHPP_TRIANGLES)
				{
					nBeginLastPart = nPoints;
					lastPartStart = nPoints;
					lastPartType = SHPP_TRIANGLES;
					nParts++;
				}
				lastpt2 = lastpt;
				//pt=NextPoint(pt, 2, bZ, bM);
				lastpt = ptz[2];
				nPoints += 3;
			}
		}
		else
		{
			panPartStart = new int[(nParts + nRings)];
			panPartType = new int[(nParts + nRings)];

			for (int i = 0; i < nRings; i++)
			{
				lastPartStart = nPoints;
				if (i == 0)
					lastPartType = SHPP_OUTERRING;
				else
					lastPartType = SHPP_INNERRING;

				pt = (double*)ls->points;
				pt += (sizePt*(ls->numPoints-1));
				lastpt2 = lastpt;
				lastpt.x = pt[0];
				lastpt.y = pt[1];
				lastpt.z = (bZ)? pt[2] : 0;

				nPoints += ls->numPoints;
				ls = NextLineString(ls, bZ, bM);
			}
			nParts += nRings;
		}
		tgeom = NextGeom(tgeom);
	}

	int nShpSize = 4; // All types start with integer type number 
	nShpSize += 16 * 2; // xy bbox 
	nShpSize += 4; // nparts 
	nShpSize += 4; // npoints 
	nShpSize += 4 * nParts; // panPartStart[nparts] 
	nShpSize += 4 * nParts; // panPartType[nparts] 
	nShpSize += 8 * 2 * nPoints; // xy points 
	nShpSize += 16; // z bbox 
	nShpSize += 8 * nPoints; // z points 

	if (!pShape->Allocate(nShpSize))
		return -1;

	pShape->inUseLength = nShpSize;
	BYTE* pabyPtr = pShape->shapeBuffer;
	memset(pabyPtr, 0, nShpSize);

	int nGType = (b2M) ? FileGDBAPI::shapeMultiPatchM : FileGDBAPI::shapeMultiPatch;
	memcpy(pabyPtr, &nGType, 4);
	pabyPtr += 4;

	WKS::WKSEnvelope envelope;
	envelope.xMin = FLT_MAX;
	envelope.xMax = -FLT_MAX;
	envelope.yMin = FLT_MAX;
	envelope.yMax = -FLT_MAX;
	double minZ = FLT_MAX;
	double maxZ = -minZ;

	BYTE* pabyPtrBounds = pabyPtr;
	pabyPtr += 32;
	memcpy(pabyPtr, &nParts, 4);
	pabyPtr += 4;

	memcpy(pabyPtr, &nPoints, 4);
	pabyPtr += 4;

	panPartStart = (int*)pabyPtr;
	pabyPtr += (4*nParts);
	panPartType = (int*)pabyPtr;
	pabyPtr += (4 * nParts);

	poPoints = (WKS::tagWKSPoint*)pabyPtr;
	pabyPtr += (2 * 8 * nPoints);

	BYTE* pZBounds = pabyPtr;
	pabyPtr += 16;
	padfZ = (double*)pabyPtr;

	nParts = 0;
	nPoints = 0;
	nBeginLastPart = 0;
	tgeom = pGC->WKBGeometries;
	for (int j = 0; j < pGC->num_wkbGeometries; j++)
	{
		poPoly = (WKBPolygon*)tgeom;
		int nRings = poPoly->numRings;

		WKS::WKSLineString* ls = poPoly->rings;
		if (nRings == 1 && ls->numPoints == 4)
		{
			pt = (double*)ls->points;
			for (int i = 0; i < 3; i++)
			{
				memcpy(ptz + i, pt, 16);
				if (bZ)
					ptz[i].z = pt[2];
				pt += nCoordDims;
			}
			if (nParts > 0 && poPoints != NULL &&
				((panPartType[nParts - 1] == SHPP_TRIANGLES && nPoints - panPartStart[nParts - 1] == 3) ||
					panPartType[nParts - 1] == SHPP_TRIFAN) &&
				ptz[0].x == poPoints[nBeginLastPart].x &&
				ptz[0].y == poPoints[nBeginLastPart].y &&
				ptz[0].z == padfZ[nBeginLastPart] &&
				ptz[1].x == poPoints[nPoints - 1].x &&
				ptz[1].y == poPoints[nPoints - 1].y &&
				ptz[1].z == padfZ[nPoints - 1])
			{
				panPartType[nParts - 1] = SHPP_TRIFAN;
				poPoints[nPoints].x = ptz[2].x;
				poPoints[nPoints].y = ptz[2].y;
				padfZ[nPoints] = ptz[2].z;
				nPoints++;
			}
			else if (nParts > 0 && poPoints != NULL &&
				((panPartType[nParts - 1] == SHPP_TRIANGLES && nPoints - panPartStart[nParts - 1] == 3) ||
					panPartType[nParts - 1] == SHPP_TRISTRIP) &&
				ptz[0].x == poPoints[nPoints - 2].x &&
				ptz[0].y == poPoints[nPoints - 2].y &&
				ptz[0].z == padfZ[nPoints - 2] &&
				ptz[1].x == poPoints[nPoints - 1].x &&
				ptz[1].y == poPoints[nPoints - 1].y &&
				ptz[1].z == padfZ[nPoints - 1])
			{
				panPartType[nParts - 1] = SHPP_TRISTRIP;
				poPoints[nPoints].x = ptz[2].x;
				poPoints[nPoints].y = ptz[2].y;
				padfZ[nPoints] = ptz[2].z;
				nPoints++;
			}
			else
			{
				if (nParts == 0 || panPartType[nParts - 1] != SHPP_TRIANGLES)
				{
					nBeginLastPart = nPoints;
					panPartStart[nParts] = nPoints;
					panPartType[nParts] = SHPP_TRIANGLES;
					nParts++;
				}

				for (int i = 0; i<3; i++)
				{
					poPoints[nPoints + i].x = ptz[i].x;
					poPoints[nPoints + i].y = ptz[i].y;
					padfZ[nPoints + i] = ptz[i].z;
				}
				nPoints += 3;
			}
		}
		else
		{
			for (int i = 0; i < nRings; i++)
			{
				panPartStart[nParts + i] = nPoints;
				if (i == 0)
					panPartType[nParts + i] = SHPP_OUTERRING;
				else
					panPartType[nParts + i] = SHPP_INNERRING;

				pt = (double*)ls->points;
				for (int k = 0; k < ls->numPoints; k++)
				{
					poPoints[nPoints + k].x = pt[0];
					poPoints[nPoints + k].y = pt[1];
					if (bZ)
						padfZ[nPoints + k] = pt[2];
					pt += sizePt;
				}
				nPoints += ls->numPoints;
				ls = NextLineString(ls, bZ, bM);
			}
			nParts += nRings;
		}
		tgeom = NextGeom(tgeom);
	}
	if (envelope.xMin <= envelope.xMax)
		memcpy(pabyPtrBounds, &envelope, 32);
	if (bZ && (minZ <= maxZ))
	{
		memcpy(pZBounds, &(minZ), 8);
		memcpy(pZBounds + 8, &(maxZ), 8);
	}
	return nShpSize;
}


/*

int GeometryToGDB(BYTE* pDataSrc, int nLen, FileGDBAPI::ShapeBuffer& shape, bool b2Z, bool b2M)
{
	int nShpSize = 4; // All types start with integer type number
	int nShpZSize = 0; // Z gets tacked onto the end
	int nPoints = 0;
	int nParts = 0;

	if (pDataSrc == NULL || nLen <= 4)
	{
		if (!shape.Allocate(4))
			return -1;
		shape.inUseLength = 4;
		int zero = SHPT_NULL;
		memcpy(shape.shapeBuffer, &zero, nShpSize);
		return 4;
	}
	WKBGeometry* pGeom = (WKBGeometry*)pDataSrc;
	int wType = pGeom->wkbType % wkbGeometryType::wkbType;
	int nt = pGeom->wkbType / wkbGeometryType::wkbType;

	bool bZ = ((nt & 1) == 1);
	bool bM = ((nt & 2) == 2);
	int nCoordDims = 2;
	if (bZ)
		nCoordDims++;
	if (bM)
		nCoordDims++;

	int sizePt = nCoordDims * 8;
	if (wType == wkbGeometryType::wkbPoint)
	{
		nShpSize += sizePt;
	}
	else if (wType == wkbGeometryType::wkbLineString)
	{
		WKBLineString *poLine = (WKBLineString*)pGeom;
		nPoints = poLine->lineString.numPoints;
		nParts = 1;
		nShpSize += 16 * nCoordDims; // xy(z)(m) box
		nShpSize += 4; // nparts 
		nShpSize += 4; // npoints 
		nShpSize += 4; // parts[1] 
		nShpSize += sizePt * nPoints; // points 
		nShpZSize = 16 + 8 * nPoints;
	}
	else if (wType == wkbGeometryType::wkbPolygon)
	{
		WKBPolygon *poPoly = (WKBPolygon*)pGeom;
		nParts = poPoly->numRings;
		WKS::WKSLineString* ls = poPoly->rings;
		for (int i = 0; i < nParts; i++)
		{
			nPoints += ls->numPoints;
			ls = NextLineString(ls, bZ, bM);
		}
		nShpSize += 16 * nCoordDims; // xy(z)(m) box 
		nShpSize += 4; // nparts 
		nShpSize += 4; // npoints 
		nShpSize += 4 * nParts; // parts[nparts] 
		nShpSize += sizePt * nPoints; // points 
		nShpZSize = 16 + 8 * nPoints;
	}
	else
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		nParts = pGC->num_wkbGeometries;
		if (wType == wkbGeometryType::wkbGeometryCollection)
			wType = tgeom->wkbType % wkbGeometryType::wkbType;

		if (wType == wkbGeometryType::wkbMultiPoint)
		{
			nPoints = nParts;
			nShpSize += 16 * nCoordDims; // xy(z)(m) box 
			nShpSize += 4; // npoints 
			nShpSize += sizePt * nPoints; // points 
			nShpZSize = 16 + 8 * nPoints;
		}
		else if (wType == wkbGeometryType::wkbMultiLineString)
		{
			WKBLineString *poLine;
			for (int i = 0; i < nParts; i++)
			{
				poLine = (WKBLineString*)tgeom;
				nPoints += poLine->lineString.numPoints;
				tgeom = NextGeom(tgeom);
			}
			nShpSize += 16 * nCoordDims; // xy(z)(m) box 
			nShpSize += 4; // nparts 
			nShpSize += 4; // npoints 
			nShpSize += 4 * nParts; // parts[nparts] 
			nShpSize += sizePt * nPoints; // points 
			nShpZSize = 16 + 8 * nPoints;
		}
		else if (wType == wkbGeometryType::wkbMultiPolygon)
		{
			WKBPolygon *poPoly;
			for (int i = 0; i < pGC->num_wkbGeometries; i++)
			{
				poPoly = (WKBPolygon*)tgeom;
				nParts += poPoly->numRings;
				WKS::WKSLineString* ls = poPoly->rings;
				for (int k = 0; k < nParts; k++)
				{
					nPoints += ls->numPoints;
					ls = NextLineString(ls, bZ, bM);
				}
				tgeom = NextGeom(tgeom);
			}
			nShpSize += 16 * nCoordDims; // xy(z)(m) box 
			nShpSize += 4; // nparts 
			nShpSize += 4; // npoints 
			nShpSize += 4 * nParts; // parts[nparts] 
			nShpSize += sizePt * nPoints; // points 
			nShpZSize = 16 + 8 * nPoints;
		}
	}

	if (!shape.Allocate(nShpSize))
		return -1;

	shape.inUseLength = nShpSize;
	unsigned char *pabyPtr = shape.shapeBuffer;
	unsigned char *pabyPtrZ = NULL;
	unsigned char *pabyPtrM = NULL;
	if (bM)
		pabyPtrM = pabyPtr + nShpSize - nShpZSize;
	if (bZ)
	{
		if (bM)
			pabyPtrZ = pabyPtrM - nShpZSize;
		else
			pabyPtrZ = pabyPtr + nShpSize - nShpZSize;
	}

	int nGType = SHPT_NULL;
	if (wType == wkbPoint)
	{
		nGType = (bZ && bM) ? SHPT_POINTZM :
			(bZ) ? SHPT_POINTZ :
			(bM) ? SHPT_POINTM :
			SHPT_POINT;
	}
	else if (wType == wkbMultiPoint)
	{
		nGType = (bZ && bM) ? SHPT_MULTIPOINTZM :
			(bZ) ? SHPT_MULTIPOINTZ :
			(bM) ? SHPT_MULTIPOINTM :
			SHPT_MULTIPOINT;
	}
	else if (wType == wkbLineString || wType == wkbMultiLineString)
	{
		nGType = (bZ && bM) ? SHPT_ARCZM :
			(bZ) ? SHPT_ARCZ :
			(bM) ? SHPT_ARCM :
			SHPT_ARC;
	}
	else if (wType == wkbPolygon || wType == wkbMultiPolygon)
	{
		nGType = (bZ && bM) ? SHPT_POLYGONZM :
			(bZ) ? SHPT_POLYGONZ :
			(bM) ? SHPT_POLYGONM :
			SHPT_POLYGON;
	}

	memcpy(pabyPtr, &nGType, 4);
	pabyPtr += 4;
	if (wType == wkbPoint)
	{
		WKBPoint *poPoint = (WKBPoint*)pGeom;
		memcpy(pabyPtr, &poPoint->point, sizePt);
		return nShpSize;
	}

	WKS::WKSEnvelope envelope;
	envelope.xMin = FLT_MAX;
	envelope.xMax = -FLT_MAX;
	envelope.yMin = FLT_MAX;
	envelope.yMax = -FLT_MAX;
	BYTE* pabyPtrBounds = pabyPtr;
	//memcpy(pabyPtr, &(envelope.xMin), 8);
	//memcpy(pabyPtr + 8, &(envelope.yMin), 8);
	//memcpy(pabyPtr + 16, &(envelope.xMax), 8);
	//memcpy(pabyPtr + 24, &(envelope.yMax), 8);
	pabyPtr += 32;

	BYTE* pabyPtrZBounds = NULL;
	double minZ = FLT_MAX;
	double maxZ = -minZ;
	if (bZ)
	{
		pabyPtrZBounds = pabyPtrZ;
		pabyPtrZ += 16;
	}

	double v;
	// Reserve space for the M bounds at the end of the XY buffer
	BYTE* pabyPtrMBounds = NULL;
	double dfMinM = FLT_MAX;
	double dfMaxM = -dfMinM;
	if (bM)
	{
		pabyPtrMBounds = pabyPtrM;
		pabyPtrM += 16;
	}

	if (wType == wkbGeometryType::wkbLineString)
	{
		WKBLineString *poLine = (WKBLineString*)pGeom;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		int nPartIndex = 0;
		memcpy(pabyPtr, &nPartIndex, 4);
		pabyPtr += 4;

		// Write in the point data
		double* pt = (double*)(poLine->lineString.points);
		for (int i = 0; i < poLine->lineString.numPoints; i++)
		{
			memcpy(pabyPtr, pt, 16);
			pabyPtr += 16;
			v = *pt;
			if (v < envelope.xMin) envelope.xMin = v;
			if (v > envelope.xMax) envelope.xMax = v;
			v = pt[1];
			if (v < envelope.yMin) envelope.yMin = v;
			if (v > envelope.yMax) envelope.yMax = v;
			pt += 2;
			if (bZ)
			{
				memcpy(pabyPtrZ, pt, 8);
				pabyPtrZ += 8;
				v = *pt;
				if (v < minZ) minZ = v;
				if (v > maxZ) maxZ = v;
				pt++;
			}
			if (bM)
			{
				memcpy(pabyPtrM, pt, 8);
				pabyPtrM += 8;
				v = *pt;
				if (v < dfMinM) dfMinM = v;
				if (v > dfMaxM) dfMaxM = v;
				pt++;
			}
		}
	}
	else if (wType == wkbGeometryType::wkbPolygon)
	{
		WKBPolygon *poPoly = (WKBPolygon*)pGeom;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		// Just past the partindex[nparts] array
		unsigned char* pabyPoints = pabyPtr + 4 * nParts;

		int nPointIndexCount = 0;
		WKS::WKSLineString* ls = poPoly->rings;
		double* pt;
		// Outer ring must be clockwise
		for (int i = 0; i < nParts; i++)
		{
			int nRingNumPoints = ls->numPoints;

			if (nRingNumPoints <= 2) // || !poRing->get_IsClosed())
				TRACE("polygon ring point error=%d\n", i);

			// Write in the part index
			memcpy(pabyPtr, &nPointIndexCount, 4);
			pabyPtr += 4;
			for (int k = 0; k < nRingNumPoints; k++)
			{
				pt = (double*)(ls->points);
				memcpy(pabyPoints, pt, 16);
				pabyPoints += 16;
				v = *pt;
				if (v < envelope.xMin) envelope.xMin = v;
				if (v > envelope.xMax) envelope.xMax = v;
				v = pt[1];
				if (v < envelope.yMin) envelope.yMin = v;
				if (v > envelope.yMax) envelope.yMax = v;
				pt += 2;
				if (bZ)
				{
					memcpy(pabyPtrZ, pt, 8);
					pabyPtrZ += 8;
					v = *pt;
					if (v < minZ) minZ = v;
					if (v > maxZ) maxZ = v;
					pt++;
				}
				if (bM)
				{
					memcpy(pabyPtrM, pt, 8);
					pabyPtrM += 8;
					v = *pt;
					if (v < dfMinM) dfMinM = v;
					if (v > dfMaxM) dfMaxM = v;
					pt++;
				}
			}
			nPointIndexCount += nRingNumPoints;
			ls = NextLineString(ls, bZ, bM);
		}
	}
	else if (wType == wkbMultiPoint)
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;
		double* pt;
		for (int i = 0; i < nPoints; i++)
		{
			pt = (double*)&(((WKBPoint*)tgeom)->point);
			memcpy(pabyPtr, pt, 16);
			pabyPtr += 16;
			v = *pt;
			if (v < envelope.xMin) envelope.xMin = v;
			if (v > envelope.xMax) envelope.xMax = v;
			v = pt[1];
			if (v < envelope.yMin) envelope.yMin = v;
			if (v > envelope.yMax) envelope.yMax = v;
			pt += 2;
			if (bZ)
			{
				memcpy(pabyPtrZ, pt, 8);
				pabyPtrZ += 8;
				v = *(pt);
				if (v < minZ) minZ = v;
				if (v > maxZ) maxZ = v;
				pt++;
			}
			if (bM)
			{
				memcpy(pabyPtrM, pt, 8);
				pabyPtrM += 8;
				v = *(pt);
				if (v < dfMinM) dfMinM = v;
				if (v > dfMaxM) dfMaxM = v;
				pt++;
			}
			tgeom = NextGeom(tgeom);
		}
	}
	else if (wType == wkbMultiLineString)
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		unsigned char* pabyPoints = pabyPtr + 4 * nParts;
		int nPointIndexCount = 0;

		WKBLineString *poLine;
		for (int i = 0; i < nParts; i++)
		{
			poLine = (WKBLineString*)tgeom;
			int nLineNumPoints = poLine->lineString.numPoints;
			memcpy(pabyPtr, &nPointIndexCount, 4);
			pabyPtr += 4;
			double* pt = (double*)(poLine->lineString.points);
			for (int k = 0; k < nLineNumPoints; k++)
			{
				memcpy(pabyPoints, pt, 16);
				pabyPoints += 16;
				v = *pt;
				if (v < envelope.xMin) envelope.xMin = v;
				if (v > envelope.xMax) envelope.xMax = v;
				v = pt[1];
				if (v < envelope.yMin) envelope.yMin = v;
				if (v > envelope.yMax) envelope.yMax = v;
				pt += 2;
				if (bZ)
				{
					memcpy(pabyPtrZ, pt, 8);
					pabyPtrZ += 8;
					v = *(pt);
					if (v < minZ) minZ = v;
					if (v > maxZ) maxZ = v;
					pt++;
				}
				if (bM)
				{
					memcpy(pabyPtrM, pt, 8);
					pabyPtrM += 8;
					v = *(pt);
					if (v < dfMinM) dfMinM = v;
					if (v > dfMaxM) dfMaxM = v;
					pt++;
				}
			}
			nPointIndexCount += nLineNumPoints;
		}
	}
	else // if ( nOGRType == wkbMultiPolygon ) 
	{
		WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
		WKBGeometry* tgeom = pGC->WKBGeometries;
		memcpy(pabyPtr, &nParts, 4);
		pabyPtr += 4;
		memcpy(pabyPtr, &nPoints, 4);
		pabyPtr += 4;

		unsigned char* pabyPoints = pabyPtr + 4 * nParts;

		int nPointIndexCount = 0;
		WKBPolygon *poPoly;
		for (int i = 0; i < pGC->num_wkbGeometries; i++)
		{
			poPoly = (WKBPolygon*)pGeom;
			WKS::WKSLineString* ls = poPoly->rings;
			double* pt;
			for (int j = 0; j < poPoly->numRings; j++)
			{
				int nRingNumPoints = ls->numPoints;
				memcpy(pabyPtr, &nPointIndexCount, 4);
				pabyPtr += 4;
				for (int k = 0; k < nRingNumPoints; k++)
				{
					pt = (double*)(ls->points);
					memcpy(pabyPoints, pt, 16);
					pabyPoints += 16;
					v = *pt;
					if (v < envelope.xMin) envelope.xMin = v;
					if (v > envelope.xMax) envelope.xMax = v;
					v = pt[1];
					if (v < envelope.yMin) envelope.yMin = v;
					if (v > envelope.yMax) envelope.yMax = v;
					pt += 2;
					if (bZ)
					{
						memcpy(pabyPtrZ, pt, 8);
						pabyPtrZ += 8;
						v = *(pt);
						if (v < minZ) minZ = v;
						if (v > maxZ) maxZ = v;
						pt++;
					}
					if (bM)
					{
						memcpy(pabyPtrM, pt, 8);
						pabyPtrM += 8;
						v = *(pt);
						if (v < dfMinM) dfMinM = v;
						if (v > dfMaxM) dfMaxM = v;
						pt++;
					}
				}
				nPointIndexCount += nRingNumPoints;
			}
		}
	}
	if (envelope.xMin > envelope.xMax)
	{
		memset(&envelope, 0, sizeof(WKS::WKSEnvelope));
	}
	memcpy(pabyPtrBounds, &envelope, 32);

	if (bZ)
	{
		if (minZ > maxZ)
		{
			minZ = 0.0;
			maxZ = 0.0;
		}
		memcpy(pabyPtrZBounds, &(minZ), 8);
		memcpy(pabyPtrZBounds + 8, &(maxZ), 8);
	}
	if (bM)
	{
		if (dfMinM > dfMaxM)
		{
			dfMinM = 0.0;
			dfMaxM = 0.0;
		}
		memcpy(pabyPtrMBounds, &(dfMinM), 8);
		memcpy(pabyPtrMBounds + 8, &(dfMaxM), 8);
	}

	return nShpSize;
}


int GeometryToGDBMultiPatch(BYTE* pDataSrc, int nLen, FileGDBAPI::ShapeBuffer& shape, bool b2Z, bool b2M)
{
	WKBGeometry* pGeom = (WKBGeometry*)pDataSrc;
	int wType = pGeom->wkbType % wkbGeometryType::wkbType;
	int nt = pGeom->wkbType / wkbGeometryType::wkbType;

	bool bZ = ((nt & 1) == 1);
	bool bM = ((nt & 2) == 2);
	int nCoordDims = 2;
	if (bZ)
		nCoordDims++;
	if (bM)
		nCoordDims++;

	WKBGeometryCollection *pGC = (WKBGeometryCollection*)pGeom;
	WKBGeometry* tgeom;
	int sizePt = nCoordDims * 8;
	if (wType == wkbGeometryType::wkbGeometryCollection)
	{
		tgeom = pGC->WKBGeometries;
		wType = tgeom->wkbType % wkbGeometryType::wkbType;
	}
	if (wType != wkbGeometryType::wkbMultiPolygon && wType != wkbGeometryType::wkbPolygon)
		return -1;

	int nParts = 0;
	int* panPartStart = NULL;
	int* panPartType = NULL;
	int nPoints = 0;
	WKS::tagWKSPoint* poPoints = NULL;
	double* padfZ = NULL;
	int nBeginLastPart = 0;
	double* pt;

	int lastPartStart = 0;
	int lastPartType = -1;
	WKS::tagWKSPointZ beginLastpt;
	WKS::tagWKSPointZ lastpt;
	WKS::tagWKSPointZ lastpt2;
	memset(&beginLastpt, 0, sizeof(WKS::tagWKSPointZ));
	memset(&lastpt, 0, sizeof(WKS::tagWKSPointZ));
	memset(&lastpt2, 0, sizeof(WKS::tagWKSPointZ));

	WKBPolygon *poPoly;
	tgeom = pGC->WKBGeometries;
	for (int j = 0; j < pGC->num_wkbGeometries; j++)
	{
		poPoly = (WKBPolygon*)tgeom;
		int nRings = poPoly->numRings;

		WKS::WKSLineString* ls = poPoly->rings;
		pt = (double*)ls->points;
		if (nRings == 1 && ls->numPoints == 4)
		{
			if (nParts > 0 &&
				((lastPartType == SHPP_TRIANGLES && nPoints - lastPartStart == 3) ||
					lastPartType == SHPP_TRIFAN) &&
				pt[0] == beginLastpt.x && pt[1] == beginLastpt.y && pt[2] == beginLastpt.z &&
				pt[3] == lastpt.x && pt[4] == lastpt.y && pt[5] == lastpt.z)
			{
				lastPartType = SHPP_TRIFAN;
				lastpt2 = lastpt;
				lastpt.x = pt[6];
				lastpt.y = pt[7];
				lastpt.z = pt[8];
				nPoints++;
			}
			else if (nParts > 0 &&
				((lastPartType == SHPP_TRIANGLES && nPoints - lastPartStart == 3) ||
					lastPartType == SHPP_TRISTRIP) &&
				pt[0] == lastpt2.x && pt[1] == lastpt2.y && pt[2] == lastpt2.z &&
				pt[3] == lastpt.x && pt[4] == lastpt.y && pt[5] == lastpt.z)
			{
				lastPartType = SHPP_TRISTRIP;
				lastpt2 = lastpt;
				lastpt.x = pt[6];
				lastpt.y = pt[7];
				lastpt.z = pt[8];
				nPoints++;
			}
			else
			{
				if (nParts == 0 || lastPartType != SHPP_TRIANGLES)
				{
					nBeginLastPart = nPoints;
					lastPartStart = nPoints;
					lastPartType = SHPP_TRIANGLES;
					nParts++;
				}
				lastpt2 = lastpt;
				//pt=NextPoint(pt, 2, bZ, bM);
				lastpt.x = pt[6];
				lastpt.y = pt[7];
				lastpt.z = pt[8];
				nPoints += 3;
			}
		}
		else
		{
			panPartStart = new int[(nParts + nRings)];
			panPartType = new int[(nParts + nRings)];

			for (int i = 0; i < nRings; i++)
			{
				lastPartStart = nPoints;
				if (i == 0)
					lastPartType = SHPP_OUTERRING;
				else
					lastPartType = SHPP_INNERRING;

				pt = (double*)ls->points;
				pt += (3 * (ls->numPoints - 1));
				lastpt2 = lastpt;
				lastpt.x = pt[0];
				lastpt.y = pt[1];
				lastpt.z = pt[2];

				nPoints += ls->numPoints;
				ls = NextLineString(ls, bZ, bM);
			}
			nParts += nRings;
		}
		tgeom = NextGeom(tgeom);
	}

	int nShpSize = 4; // All types start with integer type number 
	nShpSize += 16 * 2; // xy bbox 
	nShpSize += 4; // nparts 
	nShpSize += 4; // npoints 
	nShpSize += 4 * nParts; // panPartStart[nparts] 
	nShpSize += 4 * nParts; // panPartType[nparts] 
	nShpSize += 8 * 2 * nPoints; // xy points 
	nShpSize += 16; // z bbox 
	nShpSize += 8 * nPoints; // z points 

	if (!shape.Allocate(nShpSize))
		return -1;

	BYTE* pabyPtr = shape.shapeBuffer;
	memset(pabyPtr, 0, nShpSize);

	int nGType = SHPT_MULTIPATCH;
	memcpy(pabyPtr, &nGType, 4);
	pabyPtr += 4;

	WKS::WKSEnvelope envelope;
	envelope.xMin = FLT_MAX;
	envelope.xMax = -FLT_MAX;
	envelope.yMin = FLT_MAX;
	envelope.yMax = -FLT_MAX;
	double minZ = FLT_MAX;
	double maxZ = -minZ;

	BYTE* pabyPtrBounds = pabyPtr;
	pabyPtr += 32;
	memcpy(pabyPtr, &nParts, 4);
	pabyPtr += 4;

	memcpy(pabyPtr, &nPoints, 4);
	pabyPtr += 4;

	panPartStart = (int*)pabyPtr;
	pabyPtr += (4 * nParts);
	panPartType = (int*)pabyPtr;
	pabyPtr += (4 * nParts);

	poPoints = (WKS::tagWKSPoint*)pabyPtr;
	pabyPtr += (2 * 8 * nPoints);

	BYTE* pZBounds = pabyPtr;
	pabyPtr += 16;
	padfZ = (double*)pabyPtr;

	nParts = 0;
	nPoints = 0;
	nBeginLastPart = 0;
	tgeom = pGC->WKBGeometries;
	for (int j = 0; j < pGC->num_wkbGeometries; j++)
	{
		poPoly = (WKBPolygon*)tgeom;
		int nRings = poPoly->numRings;

		WKS::WKSLineString* ls = poPoly->rings;
		pt = (double*)ls->points;
		if (nRings == 1 && ls->numPoints == 4)
		{
			if (nParts > 0 && poPoints != NULL &&
				((panPartType[nParts - 1] == SHPP_TRIANGLES && nPoints - panPartStart[nParts - 1] == 3) ||
					panPartType[nParts - 1] == SHPP_TRIFAN) &&
				pt[0] == poPoints[nBeginLastPart].x &&
				pt[1] == poPoints[nBeginLastPart].y &&
				pt[2] == padfZ[nBeginLastPart] &&
				pt[3] == poPoints[nPoints - 1].x &&
				pt[4] == poPoints[nPoints - 1].y &&
				pt[5] == padfZ[nPoints - 1])
			{
				panPartType[nParts - 1] = SHPP_TRIFAN;
				poPoints[nPoints].x = pt[6];
				poPoints[nPoints].y = pt[7];
				padfZ[nPoints] = pt[8];
				nPoints++;
			}
			else if (nParts > 0 && poPoints != NULL &&
				((panPartType[nParts - 1] == SHPP_TRIANGLES && nPoints - panPartStart[nParts - 1] == 3) ||
					panPartType[nParts - 1] == SHPP_TRISTRIP) &&
				pt[0] == poPoints[nPoints - 2].x &&
				pt[1] == poPoints[nPoints - 2].y &&
				pt[2] == padfZ[nPoints - 2] &&
				pt[3] == poPoints[nPoints - 1].x &&
				pt[4] == poPoints[nPoints - 1].y &&
				pt[5] == padfZ[nPoints - 1])
			{
				panPartType[nParts - 1] = SHPP_TRISTRIP;
				poPoints[nPoints].x = pt[6];
				poPoints[nPoints].y = pt[7];
				padfZ[nPoints] = pt[8];
				nPoints++;
			}
			else
			{
				if (nParts == 0 || panPartType[nParts - 1] != SHPP_TRIANGLES)
				{
					nBeginLastPart = nPoints;
					panPartStart[nParts] = nPoints;
					panPartType[nParts] = SHPP_TRIANGLES;
					nParts++;
				}

				for (int i = 0; i<3; i++)
				{
					poPoints[nPoints + i].x = pt[0];
					poPoints[nPoints + i].y = pt[1];
					padfZ[nPoints + i] = pt[2];
					pt += 3;
				}
				nPoints += 3;
			}
		}
		else
		{
			for (int i = 0; i < nRings; i++)
			{
				panPartStart[nParts + i] = nPoints;
				if (i == 0)
					panPartType[nParts + i] = SHPP_OUTERRING;
				else
					panPartType[nParts + i] = SHPP_INNERRING;

				pt = (double*)ls->points;
				for (int k = 0; k < ls->numPoints; k++)
				{
					poPoints[nPoints + k].x = pt[0];
					poPoints[nPoints + k].y = pt[1];
					padfZ[nPoints + k] = pt[2];
					pt += 3;
				}
				nPoints += ls->numPoints;
				ls = NextLineString(ls, bZ, bM);
			}
			nParts += nRings;
		}
		tgeom = NextGeom(tgeom);
	}
	if (envelope.xMin <= envelope.xMax)
		memcpy(pabyPtrBounds, &envelope, 32);
	if (minZ <= maxZ)
	{
		memcpy(pZBounds, &(minZ), 8);
		memcpy(pZBounds + 8, &(maxZ), 8);
	}
	return nShpSize;
}
*/