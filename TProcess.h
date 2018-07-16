#include <malloc.h>
#include <MATH.h>
#ifndef TPROCESS_H
#define TPROCESS_H

#ifndef PI
#define PI 3.141592653589793116
#endif

#define INCREASESIZE 256

template <class T>
void inline AddElement(T Ele,T **Eles,int *nCount)
{
	size_t pcount=0;
	if (*Eles != NULL)
		pcount = _msize(*Eles) / sizeof(T);
	if (pcount <= (unsigned int)(*nCount))
	{
		pcount+=INCREASESIZE;
		T *pts=new T[pcount];
		memcpy(pts,*Eles,(*nCount)*sizeof(T));
		delete [] *Eles;
		*Eles=pts;
	}
	(*Eles)[(*nCount)++]=Ele;
}

template <class T>
void inline AddElements(T *Ele,int nCnt,T **Eles,int *nCount)
{
	INT_PTR pcount=0;
	if (*Eles != NULL)
		pcount = _msize(*Eles) / sizeof(T);
	if (pcount<=(*nCount)+nCnt)
	{
		while (pcount < (*nCount) + nCnt)
			pcount+=INCREASESIZE;

		T *pts=new T[pcount];
		memcpy(pts,*Eles,(*nCount)*sizeof(T));
		delete [] *Eles;
		*Eles=pts;
	}
	memcpy(&((*Eles)[*nCount]),Ele,nCnt*sizeof(T));
	(*nCount)+=nCnt;
}

template <class T> void swap(T &A1, T &A2)
{
	T t = A1;
	A1 = A2;
	A2 = t;
}

template <class T> void Qsort(T *&A, long m, long n)
{
	if (m < n)
	{
		long i = m;
		long j = n + 1;
		T k = A[m];
		do
		{
			do
			{
				i++;
			} while (i <= n && A[i] < k);
			do
			{
				j--;
			} while (A[j] > k);
			if (i < j)
			{
				swap(A[i], A[j]);
			}
		} while (i < j);
		swap(A[m], A[j]);
		Qsort(A, m, j - 1);
		Qsort(A, j + 1, n);
	}
}
/////////////////////////////////////////////////////////////////////////////////
// TPoint Process
/////////////////////////////////////////////////////////////////////////////////

void ClipPolygon(POINT* pPoint, int nPtCount, POINT** pOutPoint, int* nOutPtCount, RECT rcClient);
void ClipPolyPolygon(POINT* pPoint, int* nPtCount, int nCount,POINT** pOutPoint, int** nOutPtCount, int* nOutCount, RECT rcClient);
//POINT SetPtInPolygon(POINT point, POINT* pPoints, int *pPolyCount, int nCount);

template <class T>
double inline GetVector4Dir(T pt1, T pt2)
{
	double y = (pt2.y - pt1.y) / (fabs(pt2.y - pt1.y) + fabs(pt2.x - pt1.x));
	return pt2.x > pt1.x ? y : (y > 0 ? 2 : -2) - y;
}
template <class T>
int inline GetSide(T pt, T Minp, T Maxp)
{
	int ptside = 0;
	if (pt.x < Minp.x)
		ptside = 1;
	else if (pt.x > Maxp.x)
		ptside = 2;

	if (pt.y < Minp.y)
		ptside += 4;
	else if (pt.y > Maxp.y)
		ptside += 8;
	return ptside;
}

template <class T>
void inline AddPoint(T point, T **points, int *cpos, bool duplicate = true)
{
	if (duplicate && *points != NULL && (*points)[(*cpos)-1].x == point.x && (*points)[(*cpos)-1].y == point.y)
		return;
	AddElement(point, points, cpos);
}

/*
Author:
	Cog 2003/10/29~2003/12/31
Purpose:
	LineSide 判斷指定點落在兩點形成的直線的左右側或線上。
Parameters:
	ARCPOINT fpt
		直線第一點。
	ARCPOINT tpt
		直線第二點。
	ARCPOINT pt
		指定點。
Return:
	int
		-1 左側
		0 直線上
		1 右側
Notes:
	利用三角形的相似形等比原理，以fpt為原點，
	計算tpt及pt，在fpt原點的坐標X及Y的分量比。
*/
template <class T>
int inline LineSide(T fpt,T tpt,T pt)
{
	double del=(pt.x - fpt.x) * (tpt.y-fpt.y) - (pt.y - fpt.y) * (tpt.x-fpt.x);
	if (del==0)
		return 0;
	else if (del>0)
		return 1;
	return -1;
}

template <class T>
int inline LineSide3(T fpt, T cpt, T tpt, T pt)
{
	int a = LineSide(fpt,cpt,pt);
	int b = LineSide(cpt,tpt,pt);

	double c = GetVector4Dir(cpt,tpt) - GetVector4Dir(fpt,cpt);
	while (c<0)
		c+=4;
	while (c>4)
		c-=4;
	if ((a==1 && b==1) || ((a==1 || b==1) && c<2))
		return 1;
	else if ((a==-1 && b==-1) || ((a==-1 || b==-1) && c>2))
		return -1;
	return 0;
}

namespace LineCross
{
	enum LineSize
	{
		Left = 0x3,
		Online = 0x0,
		Right = 0x1,
	};
#define LCT_H1(v) ((((v) >> 6) & 0x03))
#define LCT_T1(v) ((((v) >> 4) & 0x03))
#define LCT_H2(v) ((((v) >> 2) & 0x03))
#define LCT_T2(v) (((v) & 0x03))
	//F 1111 Left, Left
	//C 1100 Left, Online
	//D 1101 Left, Right
	//3 0011 Online, Left
	//1 0001 Online, Right
	//7 0111 Right, Left
	//4 0100 Right, Online
	//5 0101 Right, Right
	enum LineCrossType
	{
		NotCross = 0xFFFF,//LCTVALUE(Left, Left, Left, Left),
		SameLine = 0,//LCTVALUE(Online, Online, Online, Online),
		DiagonalCross = 0x7C,//LCTVALUE(Right, Left, Left, Right),
		InverseDiagonalCross = 0xD7,//LCTVALUE(Left, Right, Right, Left),
		TCrossHH = 0x31,//LCTVALUE(Online, Left, Online, Right),
		TCrossHH1 = 0x13,//LCTVALUE(Online, Right, Online, Left),
		TCrossHL = 0x3D,//LCTVALUE(Online, Left, Left, Right),
		TCrossHL1 = 0x17,//LCTVALUE(Online, Right, Right, Left),
		TCrossHT = 0x3C,//LCTVALUE(Online, Left, Left, Online),
		TCrossHT1 = 0x14,//LCTVALUE(Online, Right, Right, Online),
		TCrossLH = 0xD3,//LCTVALUE(Left, Right, Online, Left),
		TCrossLH1 = 0x71,//LCTVALUE(Right, Left, Online, Right),
		TCrossLT = 0xD4,//LCTVALUE(Left, Right, Right, Online),
		TCrossLT1 = 0x7C,//LCTVALUE(Right, Left, Left, Online),
		TCrossTH = 0xC3,//LCTVALUE(Left, Online, Online, Left),
		TCrossTH1 = 0x41,//LCTVALUE(Right, Online, Online, Right),
		TCrossTL = 0xC7,//LCTVALUE(Left, Online, Right, Left),
		TCrossTL1 = 0x4D,//LCTVALUE(Right, Online, Left, Right),
		TCrossTT = 0xC4,//LCTVALUE(Left, Online, Right, Online),
		TCrossTT1 = 0x4C,//LCTVALUE(Right, Online, Left, Online),
		//SameLine = -1,
		//NotCross = 0,
		//DiagonalCross = 1,
		//InverseDiagonalCross = 2,
		//TCrossLH = 3,
		//TCrossLT = 4,
		//TCrossHH = 5,
		//TCrossHL = 6,
		//TCrossHT = 7,
		//TCrossTH = 8,
		//TCrossTL = 9,
		//TCrossTT = 10,
		//TCrossTail1 = 3,
		//TCrossTail2 = 4,
		//TCrossHead = 5,
	};
}
/*
Author:
	Cog 2003/10/29~2003/12/31
Purpose:
	TowLineCross 判斷由四點形成的兩線段是否相交。
Parameters:
	ARCPOINT fpt1
		第一條線段的起點。
	ARCPOINT tpt1
		第一條線段的終點。
	ARCPOINT fpt2
		第二條線段的起點。
	ARCPOINT tpt2
		第二條線段的終點。
Return:
	int
		-1 在同一直線上。
		0 不相交
		1 兩線段呈十字相交。第二線段由左至右穿過第一線段。
		2 兩線段呈十字相交。第二線段由右至左穿過第一線段。
		3 兩線段呈T形或L形相交
		4 兩線段呈T形或L形相交
		5 兩線段呈T形或L形相交
Notes:
	目前尚無法判斷是否為T形或L形相交。
	先判斷兩線段邊界是否重疊，
	再利用LineSide為基礎，取得各點在相對於另一線段的位置，
	若四點為0(既線上)則兩線段必有部份重疊，
	若兩線段的兩點皆不相等，且某一點為0，則形成T或L形相交，否則定相交於一點。
*/
template <class T,class C>
inline LineCross::LineCrossType TwoLineCross(T fpt1,T tpt1,T fpt2,T tpt2,T *result = NULL)
{
	BOOL a = (fpt1.x == fpt2.x && fpt1.y == fpt2.y);
	BOOL b = (tpt1.x == tpt2.x && tpt1.y == tpt2.y);
	BOOL c = (!a && !b);
	if (c)
	{
		a = (fpt1.x == tpt2.x && fpt1.y == tpt2.y);
		b = (tpt1.x == fpt2.x && tpt1.y == fpt2.y);
	}
	if (a && b)
		return LineCross::SameLine;
	else if (a)
	{
		if (result) *result = fpt1;
		return c ? LineCross::TCrossHT : LineCross::TCrossHH;
	}
	else if (b)
	{
		if (result) *result = tpt1;
		return c ? LineCross::TCrossTH : LineCross::TCrossTT;
	}

	if (max(fpt1.x, tpt1.x) < min(fpt2.x, tpt2.x) || 
		min(fpt1.x, tpt1.x) > max(fpt2.x, tpt2.x))
		return LineCross::NotCross;
	if (max(fpt1.y, tpt1.y) < min(fpt2.y, tpt2.y) || 
		min(fpt1.y, tpt1.y) > max(fpt2.y, tpt2.y))
		return LineCross::NotCross;

	//T Maxp1 = fpt1;
	//T Minp1 = tpt1;
	//T Maxp2 = fpt2;
	//T Minp2 = tpt2;
	//if (fpt1.x < tpt1.x)
	//{
	//	Minp1.x = fpt1.x;
	//	Maxp1.x = tpt1.x;
	//}
	//if (fpt2.x < tpt2.x)
	//{
	//	Minp2.x = fpt2.x;
	//	Maxp2.x = tpt2.x;
	//}
	//if (Maxp1.x < Minp2.x || Minp1.x > Maxp2.x)
	//	return LineCross::NotCross;

	//if (fpt1.y < tpt1.y)
	//{
	//	Minp1.y = fpt1.y;
	//	Maxp1.y = tpt1.y;
	//}
	//if (fpt2.y < tpt2.y)
	//{
	//	Minp2.y = fpt2.y;
	//	Maxp2.y = tpt2.y;
	//}
	//if (Maxp1.y<Minp2.y || Minp1.y>Maxp2.y)
	//	return LineCross::NotCross;

	int s1 = LineSide(fpt1, tpt1, fpt2);
	int s2 = LineSide(fpt1, tpt1, tpt2);
	if (s1 == 0 && s2 == 0)
		return LineCross::SameLine;
	int s3 = LineSide(fpt2, tpt2, fpt1);
	int s4 = LineSide(fpt2, tpt2, tpt1);
	if (s3 == 0 && s4 == 0)
		return LineCross::SameLine;
	if (s1 == s2 || s3 == s4)
		return LineCross::NotCross;

	if (s1 == 0)
	{
		if (result) *result = fpt2;
		return LineCross::TCrossLH;
	}
	if (s3 == 0)
	{
		if (result) *result = fpt1;
		return LineCross::TCrossHL;
	}
	if (s2 == 0)
	{
		if (result) *result = tpt2;
		return LineCross::TCrossLT;
	}
	if (s4 == 0)
	{
		if (result) *result = tpt1;
		return LineCross::TCrossTL;
	}
	if (result) *result = TwoLineCrossPoint<T,C>(fpt1, tpt1, fpt2, tpt2);
	return (s1<0) ? LineCross::DiagonalCross : LineCross::InverseDiagonalCross;
}
/*
Author:
	Cog 2003/10/29~2003/12/31
Purpose:
	計算由四點形成的兩線段交點，
Parameters:
	ARCPOINT fpt1
		第一條線段的起點。
	ARCPOINT tpt1
		第一條線段的終點。
	ARCPOINT fpt2
		第二條線段的起點。
	ARCPOINT tpt2
		第二條線段的終點。
Return:
	ARCPOINT
		兩直線交點。
Notes:
	在兩直線平行的情況下，傳回tpt2，可能因此發生問題。
	可以搭配TwoLineCross，做判斷。
*/
template <class T,class C>
T inline TwoLineCrossPoint(T fpt1,T tpt1,T fpt2,T tpt2)
{
	double x1=fpt1.x-tpt1.x;
	double y1=fpt1.y-tpt1.y;
	double x2=fpt2.x-tpt2.x;
	double y2=fpt2.y-tpt2.y;
	T pt;
	if (x1==0)
	{
		if (x2==0)
			pt=tpt2;	// FALSE
		else
		{
			pt.x = fpt1.x;
			pt.y = (C)(fpt2.y + (fpt1.x - fpt2.x)*y2/x2);
		}
	}
	else if (x2==0)
	{
		pt.x = fpt2.x;
		pt.y = (C)(fpt1.y + (fpt2.x - fpt1.x)*y1/x1);
	}
	else
	{
		double a1=y1/x1;
		double a2=y2/x2;
		double b1=fpt1.y - fpt1.x*y1/x1;
		double b2=fpt2.y - fpt2.x*y2/x2;
		if (a1==a2)
			pt=tpt2;	// FALSE
		else
		{
			pt.x = (C)((b2-b1) / (a1-a2));
			pt.y = (C)((b2-b1) / (a1-a2) * a1 + b1);
		}
	}
	return pt;
}
template <class T>
bool inline PointOnLine(T point,T* points,int Count, int &CrossTime)
{
	T oldpt = points[Count-1];
	T pt;
	int stepc=0;
	if (oldpt.y == point.y && oldpt.x == point.x) // Use ZERO
		return true;
	for (int j = 0; j < Count; j++)
	{
		pt = points[j];
		stepc = (pt.y < oldpt.y)?1:-1;
		if ((pt.y>point.y) != (oldpt.y>point.y)) //allow min <= y and max > y
		{
			if (pt.y==point.y) // Use ZERO
			{
				if ((pt.x==point.x)) // Use ZERO
					return true;
				if ((pt.x>point.x))
					CrossTime+=stepc;
			}
			else if (oldpt.y==point.y) // Use ZERO
			{
				if ((oldpt.x==point.x)) // Use ZERO
					return true;
				if ((oldpt.x>point.x))
					CrossTime+=stepc;
			}
			else
			{
				if ((pt.x==point.x) && (oldpt.x==point.x)) // Use ZERO
					return true;
				else if ((pt.x>=point.x) && (oldpt.x>=point.x))
					CrossTime+=stepc;
				else
				{
					double delx=(pt.x-point.x)-(pt.y-point.y)*(pt.x-oldpt.x)/(pt.y-oldpt.y);
					if (delx==0) // Use ZERO
						return true;
					else if (delx>0)
						CrossTime+=stepc;
				}
			}
		}
		else if (pt.y == point.y && pt.x == point.x) // Use ZERO
			return true;
		oldpt=pt;
	}
	return false;
}
//改PointOnLine函數  改成CrossTime直接判奇偶數就好
template <class T>
bool inline PointOnLine2(T point,T* points,int Count, int &CrossTime)
{
	T oldpt = points[Count-1];
	T pt;
	int stepc=0;
	if (oldpt.y == point.y && oldpt.x == point.x) // Use ZERO
		return true;
	for (int j = 0; j < Count; j++)
	{
		pt = points[j];
		if ((pt.y>point.y) != (oldpt.y>point.y)) //allow min <= y and max > y
		{
			if (pt.y==point.y) // Use ZERO
			{
				if ((pt.x==point.x)) // Use ZERO
					return true;
				if ((pt.x>point.x))
					CrossTime+=1;
			}
			else if (oldpt.y==point.y) // Use ZERO
			{
				if ((oldpt.x==point.x)) // Use ZERO
					return true;
				if ((oldpt.x>point.x))
					CrossTime+=1;
			}
			else
			{
				if ((pt.x==point.x) && (oldpt.x==point.x)) // Use ZERO
					return true;
				else if ((pt.x>=point.x) && (oldpt.x>=point.x))
					CrossTime+=1;
				else
				{
					double delx=(pt.x-point.x)-(pt.y-point.y)*(pt.x-oldpt.x)/(pt.y-oldpt.y);
					if (delx==0) // Use ZERO
						return true;
					else if (delx>0)
						CrossTime+=1;
				}
			}
		}
		else if (pt.y == point.y && pt.x == point.x) // Use ZERO
			return true;
		else if (pt.y == oldpt.y && pt.y == point.y) // 2011/4/19  水平線中的點沒有被判斷出來
		{
			if((pt.x > point.x && point.x > oldpt.x) || (pt.x < point.x && point.x < oldpt.x))
				return true;
		}
		oldpt=pt;
	}
	return false;
}
template <class T>
double inline GetArea(T* point,int Count)
{
	double Area=0;
	if (Count>2)
	{
		T *oldptY = point + Count - 2;
		T *oldpt = point + Count - 1;
		T *pt = point;
		for (int i = 0 ; i < Count ; i++, pt++)
		{
			Area += oldpt->y * (pt->x - oldptY->x);
			oldptY = oldpt;
			oldpt = pt;
		}
	}
	return Area * .5;
}
template <class T>
double inline GetAREA(T* point,int Count)
{
	double Area=0;
	if (Count>2)
	{
		T *oldptY = point + Count - 2;
		T *oldpt = point + Count - 1;
		T *pt = point;
		for (int i = 0 ; i < Count ; i++, pt++)
		{
			Area += oldpt->Y * (pt->X - oldptY->X);
			oldptY = oldpt;
			oldpt = pt;
		}
	}
	return Area * .5;
}

template <class T, class D>
inline T GetCentroid(T* pPoint, int nCount)
{
	T Ctpt;
	if (nCount==0)
	{
		Ctpt.x = (D)-1;
		Ctpt.y = (D)-1;
	}
	else 
	{
		double dArea = 0;
		double dSumX = 0;
		double dSumY = 0;
		//T oldptY = pPoint[nCount-2];
		T orgpt = pPoint[nCount-1];
		T oldpt = pPoint[nCount-1];
		oldpt.x -= orgpt.x;
		oldpt.y -= orgpt.y;
		T pt;
		for (int i = 0; i < nCount; i++)
		{
			pt = pPoint[i];
			pt.x -= orgpt.x;
			pt.y -= orgpt.y;
			double da = oldpt.x * pt.y - pt.x * oldpt.y;
			dSumX += (oldpt.x + pt.x) * da;
			dSumY += (oldpt.y + pt.y) * da;
			dArea += da;
			//dSumX += (pt.y - oldpt.y) * 
			//		(oldpt.x * oldpt.x + oldpt.x * pt.x + pt.x * pt.x);
			//dSumY += (oldpt.x - pt.x) * 
			//		(oldpt.y * oldpt.y + oldpt.y * pt.y + pt.y * pt.y);
			//dArea += oldpt.x * (pt.y - oldptY.y);
			//oldptY.y = oldpt.y;
			oldpt = pt;
		}
		if (dArea != 0)
		{
			dArea = dArea * 3;
			//if (dArea<0) dArea = -dArea;
			Ctpt.x = (D)(dSumX / dArea + orgpt.x);
			Ctpt.y = (D)(dSumY / dArea + orgpt.y);
		}
		else
		{
			Ctpt.x = (D)(orgpt.x);
			Ctpt.y = (D)(orgpt.y);
		}
	}
	return Ctpt;
}

template <class T>
int qsortFunc(const void *arg1, const void *arg2)
{
	if ((*(T*)arg1) > (*(T*)arg2))
		return 1;
	else if ((*(T*)arg1) < (*(T*)arg2))
		return -1;
	return 0;
}

template <class T, class D>
T SetPtInPolygon(T point, T* pPoints, int *pPolyCount, int nCount)
{
	T pt;
	int CDelX = 0;
	int CDelY = 0;
	double *XDel = NULL;
	double *YDel = NULL;
	for (long i = 0; i < nCount; i++)
	{
		T oldpt = pPoints[pPolyCount[i] - 1];
		for (long j = 0; j < pPolyCount[i]; j++)
		{
			pt = pPoints[j];
			if ((pt.y > point.y) != (oldpt.y > point.y)) //allow min <= y and max > y
			{
				double delx = (pt.x - point.x) - (pt.y - point.y) * (pt.x - oldpt.x) / (pt.y - oldpt.y);
				AddElement(delx, &XDel, &CDelX);
			}
			if ((pt.x > point.x) != (oldpt.x > point.x)) //allow min <= y and max > y
			{
				double dely = (pt.y - point.y) - (pt.x - point.x) * (pt.y - oldpt.y) / (pt.x - oldpt.x);
				AddElement(dely, &YDel, &CDelY);
			}
			oldpt = pt;
		}
		pPoints += pPolyCount[i];
	}
	qsort(XDel, CDelX, sizeof(double), qsortFunc<double>);
	qsort(YDel, CDelY, sizeof(double), qsortFunc<double>);
	double dist = 0;
	double i = 0;
	BOOL isX = TRUE;
	for (long j = 1; j < CDelX; j += 2)
	{
		if (XDel[j] - XDel[j - 1] > dist)
		{
			dist = XDel[j] - XDel[j - 1];
			i = XDel[j] + XDel[j - 1];
		}
	}
	for (long j = 1; j < CDelY; j += 2)
	{
		if (YDel[j] - YDel[j - 1] > dist)
		{
			dist = YDel[j] - YDel[j - 1];
			i = YDel[j] + YDel[j - 1];
			isX = FALSE;
		}
	}
	delete [] XDel;
	delete [] YDel;
	if (isX)
		point.x += (D)(i / 2);
	else
		point.y += (D)(i / 2);
	return point;
}

template <class T>
inline T VariantToValue(const VARIANT *var)
{
	return VariantToValue(*var, (T)0);
}

template <class T>
inline T VariantToValue(const VARIANT &var, T defaultvalue)
{
	T value = defaultvalue;
	switch(V_VT(&var))
	{
	case VT_UI1:
		value = (T)V_UI1(&var);
		break;
	case VT_UI2:
		value = (T)V_UI2(&var);
		break;
	case VT_UI4:
		value = (T)V_UI4(&var);
		break;
	case VT_UINT:
		value = (T)V_UINT(&var);
		break;
	case VT_I1:
		value = (T)V_I1(&var);
		break;
	case VT_I2:
		value = (T)V_I2(&var);
		break;
	case VT_I4:
		value = (T)V_I4(&var);
		break;
#ifdef _WIN64
	case VT_UI8:
		value = (T)V_UI8(&var);
		break;
	case VT_I8:
		value = (T)V_I8(&var);
		break;
#endif
	case VT_INT:
		value = (T)V_INT(&var);
		break;
	case VT_R4:
		value = (T)V_R4(&var);
		break;
	case VT_R8:
		value = (T)V_R8(&var);
		break;
	case VT_DATE:
		value = (T)V_DATE(&var);
		break;
	case VT_CY:
		{
			double val;
			VarR8FromCy(V_CY(&var), &val);
			value = (T)val;
		}
		break;
	case VT_DECIMAL:
		{
			double val;
			DECIMAL dc = V_DECIMAL(&var);
			VarR8FromDec(&dc, &val);
			value = (T)val;
		}
		break;
	case VT_BSTR:
		{
			WCHAR *endptr = V_BSTR(&var);
			if (endptr != NULL && wcslen(endptr) != 0)
			{
				T tVal = (T)wcstod(endptr, &endptr);
				if (endptr != NULL && *endptr == L'\0')
					value = tVal;
			}
		}
		break;
	default:
		break;
	}
	return value;
}

typedef struct tagRotateDef
{
	double dx,dy,m_angle;
} RotateDef;
inline RotateDef rotatedef(double dAngle)
{
	RotateDef tdef;
	switch ((int)(dAngle*100))
	{
	case 0:
		tdef.dx=1;
		tdef.dy=0;
		break;
	case 9000:
		tdef.dx=0;
		tdef.dy=1;
		break;
	case 18000:
		tdef.dx=-1;
		tdef.dy=0;
		break;
	case 27000:
		tdef.dx=0;
		tdef.dy=-1;
		break;
	default:
		tdef.dx=cos(dAngle/180*PI);
		tdef.dy=sin(dAngle/180*PI);
		break;
	}
	tdef.m_angle=dAngle;
	return tdef;
}
template <class T, class C>
inline T rotate(T spoint,RotateDef rd,BOOL clockwise = TRUE)
{
	T tpoint;
	tpoint.x = (C)(rd.dx*spoint.x+((clockwise) ? -1:1)*rd.dy*spoint.y);
	tpoint.y = (C)(rd.dx*spoint.y+((clockwise) ? 1:-1)*rd.dy*spoint.x);
	return tpoint;
}
template <class T, class C>
inline T rotate(T spoint,double dAngle,BOOL clockwise = TRUE)
{return rotate<T, C>(spoint,rotatedef(dAngle),clockwise);}

template <class T>
class SmartMAlloc
{
public:
	SmartMAlloc()
	{
		m_pAlloc = NULL;
		m_Size = 0;
	}
	virtual ~SmartMAlloc()
	{
		if (m_pAlloc != NULL) free(m_pAlloc);
	}
	inline T *SRealloc(unsigned long size)
	{
		if (m_Size < size)
		{
			if (m_pAlloc != NULL) free(m_pAlloc);
			m_pAlloc = (T *)malloc(size * sizeof(T));
			m_Size = size;
		}
		return m_pAlloc;
	}
private:
	T *m_pAlloc;
	unsigned long m_Size;
};

template <class T>
T POINTPLUS(T pt1, T pt2)
{
	T pt = {pt1.x+pt2.x,pt1.y+pt2.y};
	return pt;
}

#endif //TPROCESS_H