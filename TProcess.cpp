#include "stdafx.h"
#include "TProcess.h"

BOOL RoundBoundary(int &oldoffset, int &oldside)
{
	if (oldoffset > 0)
	{
		switch (oldside)
		{
		case 9: oldoffset--;
		case 1: oldoffset--;
			oldside = 5; break;
		case 6: oldoffset--;
		case 2: oldoffset--;
			oldside = 10; break;
		case 5: oldoffset--;
		case 4: oldoffset--;
			oldside = 6; break;
		case 10: oldoffset--;
		case 8: oldoffset--;
			oldside = 9; break;
		}
		if (oldoffset > 0) return TRUE;
	}
	else if (oldoffset < 0)
	{
		switch (oldside)
		{
		case 5: oldoffset++;
		case 1: oldoffset++;
			oldside = 9; break;
		case 10: oldoffset++;
		case 2: oldoffset++;
			oldside = 6; break;
		case 6: oldoffset++;
		case 4: oldoffset++;
			oldside = 5; break;
		case 9: oldoffset++;
		case 8: oldoffset++;
			oldside = 10; break;
		}
		if (oldoffset < 0) return TRUE;
	}
	return FALSE;
}

int RoundOffset(int ptside)
{
	int offset = 0;
	switch (ptside)
	{
	case 15: offset++;
	case 7: case 11: case 13: case 14: offset++;
	case 3: case 5: case 6: case 9: case 10: case 12: offset++;
	case 1: case 2: case 4: case 8: offset++;
	case 0:
		break;
	}
	return offset;
}

void ClipPolygon(POINT* pPoint, int nPtCount, POINT** pOutPoint, int* nOutPtCount, RECT rcClient)
{
	if (nPtCount < 3)
		return;

	POINT *ptClip = (POINT*)*pOutPoint;
	int ptClipCurrent = *nOutPtCount;

	POINT lefttop = { rcClient.left, rcClient.top };
	POINT righttop = { rcClient.right, rcClient.top };
	POINT leftbottom = { rcClient.left, rcClient.bottom };
	POINT rightbottom = { rcClient.right, rcClient.bottom };
	POINT centerpoint = { (rcClient.left + rcClient.right) / 2, (rcClient.top + rcClient.bottom) / 2 };

	POINT prept = pPoint[nPtCount - 1];
	int ptside, preside, oldside;
	int oldoffset = 0, headoffset = 0;
	ptside = GetSide(prept, lefttop, rightbottom);

	if (ptside == 0)
		AddPoint(prept, &ptClip, &ptClipCurrent);
	oldside = preside = ptside;
	int i = 0;
	bool notfirst = false;
	while (i < nPtCount)
	{
		POINT ptTemp = pPoint[i];
		ptside = GetSide(ptTemp, lefttop, rightbottom);
		bool iscross = false;
		if (ptside != preside)
		{
			if ((ptside & preside) == 0)
			{
				if (ptside == 0 || preside == 0)
					iscross = true;
				else
				{
					if ((preside & 1) && TwoLineCross<POINT,LONG>(prept, ptTemp, leftbottom, lefttop, (POINT*)NULL))
						iscross = true;
					else if ((preside & 2) && TwoLineCross<POINT,LONG>(prept, ptTemp, righttop, rightbottom, (POINT*)NULL))
						iscross = true;
					if ((preside & 4) && TwoLineCross<POINT,LONG>(prept, ptTemp, lefttop, righttop, (POINT*)NULL))
						iscross = true;
					else if ((preside & 8) && TwoLineCross<POINT,LONG>(prept, ptTemp, rightbottom, leftbottom, (POINT*)NULL))
						iscross = true;
				}
				if (iscross)
				{
					if (preside != 0)
					{
						if (!notfirst)
							headoffset = oldoffset;
						else
						{
							while (RoundBoundary(oldoffset, oldside))
							{
								if (oldside == 5) AddPoint(lefttop, &ptClip, &ptClipCurrent);
								else if (oldside == 6) AddPoint(righttop, &ptClip, &ptClipCurrent);
								else if (oldside == 9) AddPoint(leftbottom, &ptClip, &ptClipCurrent);
								else if (oldside == 10) AddPoint(rightbottom, &ptClip, &ptClipCurrent);
							}
						}
						oldoffset = 0;
						AddPoint(prept, &ptClip, &ptClipCurrent, notfirst);
						notfirst = true;
					}
					AddPoint(ptTemp, &ptClip, &ptClipCurrent, notfirst);
					notfirst = true;
					if (ptside != 0)
						oldside = ptside;
				}
				else
				{
					if (LineSide(prept, ptTemp, centerpoint) < 0)
						oldoffset += RoundOffset(ptside ^ preside);
					else
						oldoffset -= RoundOffset(ptside ^ preside);
				}
			}
			else
			{
				if ((preside == 1 && ptside == 5) ||
					(preside == 2 && ptside == 10) ||
					(preside == 4 && ptside == 6) ||
					(preside == 8 && ptside == 9) ||
					(preside == 5 && (ptside == 4 || ptside == 6)) ||
					(preside == 6 && (ptside == 2 || ptside == 10)) ||
					(preside == 9 && (ptside == 1 || ptside == 5)) ||
					(preside == 10 && (ptside == 8 || ptside == 9)))
					oldoffset += RoundOffset(ptside ^ preside);
				else
					oldoffset -= RoundOffset(ptside ^ preside);
			}
		}
		else if (ptside == 0)
		{
			AddPoint(ptTemp, &ptClip, &ptClipCurrent, notfirst);
			notfirst = true;
		}

		prept = ptTemp;
		preside = ptside;
		i++;
	}
	oldoffset += headoffset;
	if (oldoffset != 0 && !notfirst)
	{
		if (oldside == 5) AddPoint(lefttop, &ptClip, &ptClipCurrent, false);
		else if (oldside == 6) AddPoint(righttop, &ptClip, &ptClipCurrent, false);
		else if (oldside == 9) AddPoint(leftbottom, &ptClip, &ptClipCurrent, false);
		else if (oldside == 10) AddPoint(rightbottom, &ptClip, &ptClipCurrent, false);
	}
	while (RoundBoundary(oldoffset, oldside))
	{
		if (oldside == 5) AddPoint(lefttop, &ptClip, &ptClipCurrent, false);
		else if (oldside == 6) AddPoint(righttop, &ptClip, &ptClipCurrent, false);
		else if (oldside == 9) AddPoint(leftbottom, &ptClip, &ptClipCurrent, false);
		else if (oldside == 10) AddPoint(rightbottom, &ptClip, &ptClipCurrent, false);
	}

	if (ptClipCurrent > *nOutPtCount)
		AddPoint(ptClip[*nOutPtCount], &ptClip, &ptClipCurrent);
	*pOutPoint = ptClip;
	*nOutPtCount = ptClipCurrent;
}

void ClipPolyPolygon(POINT* pPoint, int* nPtCount, int nCount, POINT** pOutPoint, int** nOutPtCount, int* nOutCount,
					 RECT rcClient)
{
	POINT *pts = pPoint;
	POINT *tOutPoint = NULL;
	int *tOutPtCount = new int[nCount];
	int pCount = 0, oldCount = 0, tCount = 0;
	int i;
	for (i = 0; i < nCount; i++)
	{
		ClipPolygon(pts, nPtCount[i], &tOutPoint, &pCount, rcClient);
		tOutPtCount[tCount] = pCount - oldCount;
		if (tOutPtCount[tCount] > 3)
		{
			tCount++;
			oldCount = pCount;
		}
		else
		{
			pCount = oldCount;
		}
		pts += nPtCount[i];
	}
	*pOutPoint = tOutPoint;
	*nOutPtCount = tOutPtCount;
	*nOutCount = tCount;
}

//POINT SetPtInPolygon(POINT point, POINT* pPoints, int *pPolyCount, int nCount)
//{
//	POINT pt;
//	int CDelX = 0;
//	int CDelY = 0;
//	double *XDel = NULL;
//	double *YDel = NULL;
//	int i, j;
//	for (i = 0; i < nCount; i++)
//	{
//		POINT oldpt = pPoints[pPolyCount[i] - 1];
//		for (j = 0; j < pPolyCount[i]; j++)
//		{
//			pt = pPoints[j];
//			if ((pt.y > point.y) != (oldpt.y > point.y)) //allow min <= y and max > y
//			{
//				double delx = (pt.x - point.x) - (pt.y - point.y) * (pt.x - oldpt.x) / (pt.y - oldpt.y);
//				AddElement(delx, &XDel, &CDelX);
//			}
//			if ((pt.x > point.x) != (oldpt.x > point.x)) //allow min <= y and max > y
//			{
//				double dely = (pt.y - point.y) - (pt.x - point.x) * (pt.y - oldpt.y) / (pt.x - oldpt.x);
//				AddElement(dely, &YDel, &CDelY);
//			}
//			oldpt = pt;
//		}
//		pPoints += pPolyCount[i];
//	}
//	Qsort(XDel, 0, CDelX - 1);
//	Qsort(YDel, 0, CDelY - 1);
//	int dist = 0;
//	i = 0;
//	BOOL isX = TRUE;
//	for (j = 1; j < CDelX; j += 2)
//	{
//		if (XDel[j] - XDel[j - 1] > dist)
//		{
//			dist = int(XDel[j] - XDel[j - 1]);
//			i = int(XDel[j] + XDel[j - 1]);
//		}
//	}
//	for (j = 1; j < CDelY; j += 2)
//	{
//		if (YDel[j] - YDel[j - 1] > dist)
//		{
//			dist = int(YDel[j] - YDel[j - 1]);
//			i = int(YDel[j] + YDel[j - 1]);
//			isX = FALSE;
//		}
//	}
//	delete [] XDel;
//	delete [] YDel;
//	if (isX)
//		point.x += i / 2;
//	else
//		point.y += i / 2;
//	return point;
//}
