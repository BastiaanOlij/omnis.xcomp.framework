/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  drawingfunctions.cpp
 *  Drawing functions that are part of oBaseVisComponent, easier to manage when in a separate file
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - Document functions 
 *  - Add more drawing functions
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseVisComponent.h"

// passthrought to GDIdrawTextJst with clipping
void	oBaseVisComponent::drawTextJst(GDIdrawTextStruct * pTextInfo, qrect pClipRect) {
	if (pClipRect.left < pTextInfo->mX) pClipRect.left = pTextInfo->mX;
	if (pClipRect.top < pTextInfo->mY) pClipRect.top = pTextInfo->mY;
	
	if (clipRect(pClipRect)) {
		GDIdrawTextJst(pTextInfo);

		unClip();
	};
};


// Get the width of text
qdim	oBaseVisComponent::getTextWidth(const qchar *pText, qshort pLen, bool pStyled) {
	if (pLen==0) {		
		return 0;
	}
	
#ifdef isunicode 
	// It seems that on the OS4 SDK GDItextWidthJst gives a wrong result on a retina display, so only using it in unicode
	GDIdrawTextStruct drawinfo(
							   mHDC,
							   0,
							   0,
							   (qchar *)pText,			// for some reason Omnis never declared this a constant but it doesn't change the buffer (i hope)..
							   pLen,
							   &mTextSpec,
							   0,						// pColumnArray
							   0,						// pColumnCount
							   pStyled ? 1 : 0,			// pFlags: 1 = styled text
							   mApp,
							   0						// pColumnJsts
	);
	
	qdim width = GDItextWidthJst(&drawinfo) + 2;
#else
	// this will not work properly if we have styled text...
	qdim width = GDItextWidth(mHDC, (qchar *) pText, pLen) + 2;
#endif
	
	return width;
};

// Get the heigth of text
qdim	oBaseVisComponent::getTextHeight(const qchar *pText, qdim pWidth, bool pStyled, bool pWrap) {
	qdim	fontheight	= GDIfontHeight(mHDC);
	qlong	len			= OMstrlen(pText);
	qshort	pos			= 0;
	qshort	start		= 0;
	qdim	top			= 0;

	// Need to find a better way to get a unicode character :)
	qchar		newline	= '\n';
	qchar		space	= ' ';
	
	// now loop through to find our lines or until we're below our drawing rectangle.
	while (pos <= len) {
		if ((pText[pos] == 0x00) || (pText[pos] == newline)) {
			if (pos > start) {
				qdim	width = pWrap ? getTextWidth(&pText[start], pos - start, pStyled) : 0;
				
				if (width > pWidth) {
					// wrap our text
					qshort wordpos = start;
					qshort lastpos = start;
					
					while (wordpos <= pos) {
						if ((pText[wordpos]==0x00) || (pText[wordpos]==space)) {
							// found a new word or the end of our text..
							
							if (wordpos == start) {
								// nothing yet to draw, keep on going..
							} else {
								// we either found the start of a new word or the end of our text
								width = getTextWidth(&pText[start], wordpos - start, pStyled);
								
								if (width>pWidth) {									
									if (lastpos==start) {
										// our entire word doesn't fit?
										lastpos		= wordpos;
										
										// some day we'll break the word up until it fits but we need to handle escapes for styled text as single characters.
									};
									
									// advance...
									lastpos++;
									start		= lastpos;
									top			+= fontheight;									
								} else {
									// so far the words still fit..
									lastpos = wordpos;
								};
							};							
						};
						
						wordpos++;
					};					
				};
			};
			
			// get start position for next line..
			start	= pos + 1;
			top		+= fontheight;
		};
		
		pos++;
	};
	
	return top;
};


// Draws the text clipped within the specified rectangle 
qdim	oBaseVisComponent::drawText(const qchar *pText, qrect pWhere, qcol pColor, qjst pJst, bool pStyled, bool pWrap) {
	if ((pWhere.right <= pWhere.left) || (pWhere.bottom <= pWhere.top)) {
		// fluffy don't fit!
		return 0;
	};
	
	qcol	wascol		= GDIgetTextColor(mHDC);
	qlong	len			= OMstrlen(pText);
	qdim	fontheight	= GDIfontHeight(mHDC);
	qshort	pos			= 0;
	qshort	start		= 0;
	qdim	left		= pWhere.left;
	qdim	top			= pWhere.top;
	qshort	columns[2];
	qint1	jsts[2];
	
	
	// Need to find a better way to get a unicode character :)
	qchar		newline	= '\n';
	qchar		space	= ' ';
	
	// Set our text color
	GDIsetTextColor(mHDC, pColor);
	
	// setup our columns
	columns[0]			= 0;
	columns[1]			= pWhere.right - pWhere.left + 1;
	jsts[0]				= pJst;
	jsts[1]				= pJst;
	
	// now loop through to find our lines or until we're below our drawing rectangle.
	while ((pos <= len) && (top < pWhere.bottom)) {
		if ((pText[pos] == 0x00) || (pText[pos] == newline)) {
			if (pos > start) {
				// draw our text...
				
				GDIdrawTextStruct drawinfo(
										   mHDC,
										   left,
										   top,
										   (qchar *)&pText[start],		// for some reason Omnis never declared this a constant but it doesn't change the buffer (i hope)..
										   pos-start,
										   &mTextSpec,
										   columns,					// pColumnArray
										   1,							// pColumnCount
										   pStyled ? 1 : 0,			// pFlags: 1 = styled text
										   mApp,
										   jsts						// pColumnJsts
										   );
				
				qdim	width = pWrap ? getTextWidth(drawinfo.mText, drawinfo.mTextLen, pStyled) : 0;
				
				if (width > columns[1]) {
					// wrap our text
					qshort wordpos = start;
					qshort lastpos = start;
					
					while (wordpos <= pos) {
						if ((pText[wordpos]==0x00) || (pText[wordpos]==space)) {
							// found a new word or the end of our text..
							
							if (wordpos == start) {
								// nothing yet to draw, keep on going..
							} else {
								// we either found the start of a new word or the end of our text
								drawinfo.mTextLen = wordpos - start;
								width = getTextWidth(drawinfo.mText, drawinfo.mTextLen, pStyled);
								
								if (width>columns[1]) {									
									if (lastpos==start) {
										// our entire word doesn't fit?
										lastpos = wordpos;
										
										// some day we'll break the word up until it fits but we need to handle escapes for styled text as single characters.
									};
									
									drawinfo.mTextLen = lastpos - start;
									if (drawinfo.mTextLen>0) {
										drawTextJst(&drawinfo, pWhere);
									};
									
									// advance...
									lastpos++;
									start				= lastpos;
									left				= pWhere.left;
									top					+= fontheight;
									
									// and update our drawinfo
									drawinfo.mText		= (qchar *)&pText[start];
									drawinfo.mTextLen	= pos - start; // our remaining text
									drawinfo.mX			= left;
									drawinfo.mY			= top;
								} else {
									// so far the words still fit..
									lastpos = wordpos;
								};
							};							
						};
						
						wordpos++;
					};
					
					// draw any remainder?
					if (drawinfo.mTextLen>0) {
						drawTextJst(&drawinfo, pWhere);
					};
				} else {
					// draw the whole text
					drawTextJst(&drawinfo, pWhere);
				};
			};
			
			// get start position for next line..
			start	= pos + 1;
			left	= pWhere.left;
			top		+= fontheight;
		};
		
		pos++;
	};
	
	// restore our text color
	GDIsetTextColor(mHDC, wascol);
	
	return top;
};

// Draw a icon at this position
void	oBaseVisComponent::drawIcon(qlong pIconId, qpoint pAt) {
	EXTBMPref	tmpIcon(pIconId);
	qrect		iconRect;
	qdim		pxsize = 16;
	ePicSize	picsize = tmpIcon.getBmpSize(pIconId);
	qjst		imgjst = jstCenter;
	
	switch (picsize) {
		case ePic16:
			pxsize = 16;
			break;
		case ePic32:
			pxsize = 32;
			break;
		case ePic48:
			pxsize = 48;
			break;
		default:
			// don't know the size...
			pxsize = 48;
			imgjst = jstLeft;
			break;
	};
	
	iconRect.left	= pAt.h;
	iconRect.top	= pAt.v;
	iconRect.right	= iconRect.left + pxsize;
	iconRect.bottom	= iconRect.top + pxsize;
	
	if (clipRect(iconRect, true)) {
		tmpIcon.draw (mHDC, &iconRect, picsize, picNormal, qfalse, colNone, qfalse, imgjst, imgjst);		
		
		unClip();
	};
};


// Draws a line between two points using the current selected pen
void	oBaseVisComponent::drawLine(qpoint pFrom, qpoint pTo) {
	GDImoveTo(mHDC, &pFrom);
	GDIlineTo(mHDC, &pTo);	
};

// Draws a line between two points
void	oBaseVisComponent::drawLine(qpoint pFrom, qpoint pTo, qdim pWidth, qcol pCol, qpat pPat) {
	HPEN	newPen = GDIcreatePen(pWidth, pCol, pPat);
	HPEN	oldPen = GDIselectObject(mHDC, newPen);
	qcol	oldCol = GDIgetTextColor(mHDC);

	GDIsetTextColor(mHDC, pCol);	// shouldn't be needed but for some reason in the SDK documentation they also set the text color...
	drawLine(pFrom, pTo);
	
	GDIsetTextColor(mHDC, oldCol);
	GDIselectObject(mHDC, oldPen);
};


// Draws a filled ellipse within the rectangle with a gradient color from top to bottom
void	oBaseVisComponent::drawEllipse(qrect pRect, qcol pTop, qcol pBottom, qcol pBorder, qint pSpacing) {
	int		height		= pRect.bottom - pRect.top + 1;
	float	fHeight		= (float) height;
	float	fHalfHeight	= fHeight / 2;
	int		midX		= (pRect.right - pRect.left + 1 - pSpacing) >> 1;
	float	fMidX		= (float) midX;
	int		lastX		= 0;
	
	// get real colors...
	pTop	= GDIgetRealColor(pTop);
	pBottom = GDIgetRealColor(pBottom);
	
	// create our pens
	HPEN	fillPen = GDIcreatePen(1, pTop, patFill), borderPen;
	HPEN	oldPen = GDIselectObject(mHDC, fillPen);
	
	if (pBorder != -1) {
		borderPen = GDIcreatePen(1, pBorder, patFill);
	};
	
	for (int Y = 0; Y < height+1; Y++) {
		float   fX;
		
		fX = (float) Y;
		fX = (fX - fHalfHeight) / fHalfHeight;
		fX = fMidX * sin(acos(fX));
		
		int X = (int) fX;
		
		if ((pTop != pBottom) && (Y!=0)){
			// delete our pen, change our color, and select new pen
			GDIselectObject(mHDC, oldPen); // just in case our pen was seleted
			GDIdeleteObject(fillPen); // delete the pen we no longer need

			// mix our colours
			qcol			gradient;
			unsigned char	*Col1 = (unsigned char *)&pTop;
			unsigned char	*Col2 = (unsigned char *)&pBottom;
			unsigned char	*Grad = (unsigned char *)&gradient;

			for (int cnt=0; cnt < sizeof(qcol); cnt++) {
				if (*Col1==*Col2) {
					*Grad = *Col1;
				} else {
					int		diff = *Col2 - *Col1;
					diff = diff * Y;
					diff = diff / height;
					diff = diff + *Col1;
					*Grad = diff & 0xFF;
				};

				Col1++;
				Col2++;
				Grad++;
			};
			
			// create a new pen with our new colour
			fillPen = GDIcreatePen(1, gradient, patFill);
			GDIselectObject(mHDC, fillPen);
		};
		
		if (pBorder == -1) {
			// no border? then draw as is
			GDImoveTo(mHDC, pRect.left + midX - X, pRect.top + Y);
			GDIlineTo(mHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);			
		} else if ((Y>0) && (Y<height)) {
			// only fill if we're not going over our border
			int left	= pRect.left + midX - X + 1;
			int right	= pRect.left + midX + pSpacing + X - 1;
			if (left <= right) {
				GDImoveTo(mHDC, left, pRect.top + Y);
				GDIlineTo(mHDC, right, pRect.top + Y);				
			};
		};
		
		if (pBorder != -1) {
			// select our border pen
			GDIselectObject(mHDC, borderPen);

			// move to our starting position
			if (Y==0) {
				//    ---				
				GDImoveTo(mHDC, pRect.left + midX - X, pRect.top + Y);
				GDIlineTo(mHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);								
			} else if ((Y>0) && (Y<height)) {
				//  |     |
				GDImoveTo(mHDC, pRect.left + midX - lastX, pRect.top + Y - 1);
				GDIlineTo(mHDC, pRect.left + midX - X, pRect.top + Y);
				GDImoveTo(mHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);				
				GDIlineTo(mHDC, pRect.left + midX + pSpacing + lastX, pRect.top + Y-1);
			} else {
				//   \   /
				//    ---
				GDImoveTo(mHDC, pRect.left + midX - lastX, pRect.top + Y - 1);
				GDIlineTo(mHDC, pRect.left + midX - X, pRect.top + Y);
				GDIlineTo(mHDC, pRect.left + midX + pSpacing + X, pRect.top + Y);				
				GDIlineTo(mHDC, pRect.left + midX + pSpacing + lastX, pRect.top + Y-1);
			};
			
			GDIselectObject(mHDC, fillPen);
			
			lastX = X;
		};
	};
	
	GDIselectObject(mHDC, oldPen);
	GDIdeleteObject(fillPen);
	if (pBorder != -1) {
		GDIdeleteObject(borderPen);
	};
};
