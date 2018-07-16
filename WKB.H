#ifndef _WKB_H_
#define _WKB_H_

#include "wks.h"

#pragma pack(push, enter_WKB, 1)

typedef enum wkbGeometryType
{
	wkbPoint = 1,
	wkbLineString = 2,
	wkbPolygon = 3,
	wkbMultiPoint = 4,
	wkbMultiLineString = 5,
	wkbMultiPolygon = 6,
	wkbGeometryCollection = 7,
	wkbType = 1000,
	wkbZ = 1000,
	wkbM = 2000
	//wkbPointZ = wkbZ + wkbPoint,
	//wkbLineStringZ = wkbZ + wkbLineString,
	//wkbPolygonZ = wkbZ + wkbPolygon,
	//wkbMultiPointZ = wkbZ + wkbMultiPoint,
	//wkbMultiLineStringZ = wkbZ + wkbMultiLineString,
	//wkbMultiPolygonZ = wkbZ + wkbMultiPolygon,
	//wkbGeometryCollectionZ = wkbZ + wkbGeometryCollection,

	//wkbPointM = wkbM + wkbPoint,
	//wkbLineStringM = wkbM + wkbLineString,
	//wkbPolygonM = wkbM + wkbPolygon,
	//wkbMultiPointM = wkbM + wkbMultiPoint,
	//wkbMultiLineStringM = wkbM + wkbMultiLineString,
	//wkbMultiPolygonM = wkbM + wkbMultiPolygon,
	//wkbGeometryCollectionM = wkbM + wkbGeometryCollection,

	//wkbPointZM = wkbZ + wkbM + wkbPoint,
	//wkbLineStringZM = wkbZ + wkbM + wkbLineString,
	//wkbPolygonZM = wkbZ + wkbM + wkbPolygon,
	//wkbMultiPointZM = wkbZ + wkbM + wkbMultiPoint,
	//wkbMultiLineStringZM = wkbZ + wkbM + wkbMultiLineString,
	//wkbMultiPolygonZM = wkbZ + wkbM + wkbMultiPolygon,
	//wkbGeometryCollectionZM = wkbZ + wkbM + wkbGeometryCollection,
} wkbGeometryType;

typedef enum wkbByteOrder
{
	wkbXDR = 0,	//Big Endian
	wkbNDR = 1	//Little Endian
} wkbByteOrder;

struct WKBGeometry
{
	unsigned char byteOrder;
	long wkbType;
/*		union
	{
		WKBPoint				point;
		WKBLineString			linestring;
		WKBPolygon				polygon;
		WKBGeometryCollection	collection;
		WKBMultiPoint			mpoint;
		WKBMultiLineString		mlinestring;
		WKBMultiPolygon			mpolygon;
	};
	*/
};

struct WKBPoint : public WKBGeometry	//1
{
	WKS::WKSPoint point;
};

struct WKBLineString : public WKBGeometry	//2
{
	WKS::WKSLinearRing lineString;
//	long numPoints;
//	WKSPoint points[1];
};

struct WKBPolygon : public WKBGeometry	//3
{
	long numRings;
	WKS::WKSLinearRing rings[1];
};

struct WKBGeometryCollection : public WKBGeometry	//7
{
	long num_wkbGeometries;
	WKBGeometry WKBGeometries[1];
};

struct WKBMultiPoint : public WKBGeometryCollection	//4
{
//	long num_wkbPoints;
//	WKBPoint WKBPoints[1];
};

struct WKBMultiLineString : public WKBGeometryCollection	//5
{
//	long num_wkbLineStrings;
//	WKBLineString WKBLineStrings[1];
};

struct WKBMultiPolygon : public WKBGeometryCollection	//6
{
//	long num_wkbPolygons;
//	WKBPolygon WKBPolygons[1];
};

WKBGeometry* NextGeom(WKBGeometry *geom);

#pragma pack(pop, enter_WKB)
#endif //_STRUCT_H_