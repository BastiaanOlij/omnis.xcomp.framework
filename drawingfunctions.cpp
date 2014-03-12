/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  drawingfunctions.cpp
 *  Drawing functions that are part of oBaseVisComponent, easier to manage when in a separate file
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseVisComponent.h"

void	oBaseVisComponent::drawEllipse(HDC pHDC, qrect pRect, qcol pTop, qcol pBottom, qcol pBorder, qint pSpacing) {
	int		height = pRect.bottom - pRect.top + 1;
	float	fHeight = height;
	float	fHalfHeight = fHeight / 2;
	int		midX = (pRect.right - pRect.left + 1 - pSpacing) >> 1;
	float	fMidX = midX;
	int		lastX = 0;
	
	// get real colors...
	pTop	= GDIgetRealColor(pTop);
	pBottom = GDIgetRealColor(pBottom);
	
	// get our RGB components (might have swapped red and blue around but doesn't matter)
	int		R1 = (pTop >> 16) & 0xFF,	R2 = (pBottom >> 16) & 0xFF;
	int		G1 = (pTop >> 8) & 0xFF,	G2 = (pBottom >> 8) & 0xFF;
	int		B1 = pTop & 0xFF,			B2 = pBottom & 0xFF;
	
	// create our pens
	HPEN	fillPen = GDIcreatePen(1, pTop, patFill), borderPen;
	HPEN	oldPen = GDIselectObject(pHDC, fillPen);
	
	if (pBorder != -1) {
		borderPen = GDIcreatePen(1, pBorder, patFill);
	};
	
	for (int Y = 0; Y < height+1; Y++) {
		float   fX;
		
		fX = Y;
		fX = (fX - fHalfHeight) / fHalfHeight;
		fX = fMidX * sin(acos(fX));
		
		int X = fX;
		
		if ((pTop != pBottom) && (Y!=0)){
			// delete our pen, change our color, and select new pen
			GDIselectObject(pHDC, oldPen); // just in case our pen was seleted
			GDIdeleteObject(fillPen); // delete the pen we no longer need
			
			int R = R1 + (((R2 - R1) * Y) / height);
			int G = G1 + (((G2 - G1) * Y) / height);
			int B = B1 + (((B2 - B1) * Y) / height);
			
			pTop = (R << 16) + (G << 8) + B;
			
			GDIcreatePen(1, pTop, patFill);
			GDIselectObject(pHDC, fillPen);
		};
		
		if (pBorder == -1) {
			// no border? then draw as is
			GDImoveTo(pHDC, pRect.left + midX - X, pRect.top + Y);
			GDIlineTo(pHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);			
		} else if ((Y>0) && (Y<height)) {
			// only fill if we're not going over our border
			int left	= pRect.left + midX - X + 1;
			int right	= pRect.left + midX + pSpacing + X - 1;
			if (left <= right) {
				GDImoveTo(pHDC, left, pRect.top + Y);
				GDIlineTo(pHDC, right, pRect.top + Y);				
			};
		};
		
		if (pBorder != -1) {
			// select our border pen
			GDIselectObject(pHDC, borderPen);
			
			// move to our starting position
			if (Y==0) {
				//    ---				
				GDImoveTo(pHDC, pRect.left + midX - X, pRect.top + Y);
				GDIlineTo(pHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);								
			} else if ((Y>0) && (Y<height)) {
				//  |     |
				GDImoveTo(pHDC, pRect.left + midX - lastX, pRect.top + Y - 1);
				GDIlineTo(pHDC, pRect.left + midX - X, pRect.top + Y);
				GDImoveTo(pHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);				
				GDIlineTo(pHDC, pRect.left + midX + pSpacing + lastX, pRect.top + Y-1);
			} else {
				//   \   /
				//    ---
				GDImoveTo(pHDC, pRect.left + midX - lastX, pRect.top + Y - 1);
				GDIlineTo(pHDC, pRect.left + midX - X, pRect.top + Y);
				GDIlineTo(pHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);				
				GDIlineTo(pHDC, pRect.left + midX + pSpacing + lastX, pRect.top + Y-1);
			};
			
			GDIselectObject(pHDC, fillPen);
			
			lastX = X;
		};
	};
	
	GDIselectObject(pHDC, oldPen);
	GDIdeleteObject(fillPen);
	if (pBorder != -1) {
		GDIdeleteObject(borderPen);
	};
};
