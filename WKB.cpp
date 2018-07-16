#include "stdafx.h"
#include "wkb.h"

WKBPoint* NextPoint(WKBPoint *geom, bool is3D = false, bool isMeasure = false)
{
	if (is3D && isMeasure)
		return (WKBPoint*)(((WKS::WKSPointZM*)&(geom->point)) + 1);
	else if (is3D || isMeasure)
		return (WKBPoint*)(((WKS::WKSPointZ*)&(geom->point)) + 1);
	return (WKBPoint*)(((WKS::WKSPoint*)&(geom->point)) + 1);
}

WKBGeometry* NextGeom(WKBGeometry *geom)
{
	if (geom == 0)
		return 0;
	long nt = geom->wkbType % wkbType;
	long st = geom->wkbType / wkbType;
	bool isZ = (st & (wkbZ / wkbType)) != 0;
	bool isM = (st & (wkbM / wkbType)) != 0;
	switch (nt)
	{
	case wkbPoint:
		return NextPoint((WKBPoint*)geom, isZ, isM);
	case wkbLineString:
		return (WKBGeometry *)NextLineString(&(((WKBLineString*)geom)->lineString), isZ, isM);
	case wkbPolygon:
		{
			WKBPolygon *Pgns = (WKBPolygon*)geom;
			WKS::WKSLinearRing *rng = Pgns->rings;
			for (long i = 0 ; i < Pgns->numRings ; i++)
				rng = NextLineString(rng, isZ, isM);
			return (WKBGeometry *)rng;
		}
	case wkbGeometryCollection:
	case wkbMultiPoint:
	case wkbMultiLineString:
	case wkbMultiPolygon:
		{
			WKBGeometryCollection *MGeom = (WKBGeometryCollection*)geom;
			WKBGeometry *tgeom = MGeom->WKBGeometries;
			for (long i = 0 ; i < MGeom->num_wkbGeometries && tgeom != 0 ; i++)
				tgeom = NextGeom(tgeom);
			return tgeom;
		}
	}
	return 0;
}