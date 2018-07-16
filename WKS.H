#ifndef _WKS_H_
#define _WKS_H_

namespace WKS
{

#pragma pack(push, enter_WKS, 1)
typedef struct tagWKSPoint
{
	double x;
	double y;
} WKSPoint;
typedef struct tagWKSPointZ
{
	double x;
	double y;
	double z;
} WKSPointZ;
typedef struct tagWKSPointZM
{
	double x;
	double y;
	double z;
	double m;
} WKSPointZM;
typedef struct tagWKSEnvelope
{
	double xMin;
	double yMin;
	double xMax;
	double yMax;
} WKSEnvelope;

typedef struct tagWKSLineString
{
	long numPoints;
	WKSPoint points[1];
} WKSLineString, WKSLinearRing;

};
#pragma pack(pop, enter_WKS)

inline double GetZ(WKS::WKSPoint *pt, bool isZ)
{
	return (isZ ? ((WKS::WKSPointZ *)pt)->z : 0);
}
inline double GetM(WKS::WKSPoint *pt, bool isZ, bool isM)
{
	return (isM ? isZ ? ((WKS::WKSPointZM *)pt)->m : ((WKS::WKSPointZ *)pt)->z : 0);
}
//inline WKS::WKSLineString* NextLineString(WKS::WKSLineString *ls, bool is3D = false, bool isMeasure = false);
inline WKS::WKSPoint* NextPoint(WKS::WKSPoint *pt, long cnt, bool is3D = false, bool isMeasure = false)
{
	if (pt == 0)
		return 0;
	if (is3D && isMeasure)
		return (WKS::WKSPoint*)(((WKS::WKSPointZM*)pt) + cnt);
	else if (is3D || isMeasure)
		return (WKS::WKSPoint*)(((WKS::WKSPointZ*)pt) + cnt);
	return (WKS::WKSPoint*)(((WKS::WKSPoint*)pt) + cnt);
}
inline WKS::WKSLineString* NextLineString(WKS::WKSLineString *ls, bool is3D = false, bool isMeasure = false)
{
	if (ls == 0)
		return 0;
	return (WKS::WKSLineString*)NextPoint(ls->points, ls->numPoints, is3D, isMeasure);
}

#endif //_WKS_H_