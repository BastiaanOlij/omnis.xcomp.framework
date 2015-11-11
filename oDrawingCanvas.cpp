/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oDrawingCanvas.cpp
 * 
 *  Drawing canvas class
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oDrawingCanvas.h"

// Initialize our instance
void	oDrawingCanvas::init(qapp pApp) {
	mApp				= pApp;
	mHDC				= 0;
	mTextFont			= 0;
	mOldFont			= 0;
	mBackpatBrush		= 0;
	mFontHeight			= 0;
};

// Setup a drawing canvas based on an existing HDC
oDrawingCanvas::oDrawingCanvas(qapp pApp, HDC pHdc, qrect pRect) {
	init(pApp);
	
	mHDC				= pHdc;
	mBaseRect			= pRect;
	mOwnHDC				= false;
};

// Create a bitmap based canvas
oDrawingCanvas::oDrawingCanvas(qapp pApp, qdim pWidth, qdim pHeight) {	
	init(pApp);

	GDIcreateAlphaDC(&mHDC, pWidth, pHeight);
	
	mBaseRect.left		= 0;
	mBaseRect.top		= 0;
	mBaseRect.right		= pWidth - 1;
	mBaseRect.bottom	= pHeight - 1;
	
	mOwnHDC				= true;
};

// Destruct our canvas
oDrawingCanvas::~oDrawingCanvas() {
	// cleanup..

	if (mBackpatBrush != 0) {
		GDIdeleteObject(mBackpatBrush);
	};
	
	if (mOldFont != 0) {
		GDIselectObject(mHDC, mOldFont);
	};
	
	if (mTextFont != 0) {
		GDIdeleteObject(mTextFont);
	};
	
	clearTextCache();
	
	if (mOwnHDC) {
		HBITMAP	bitmap;
		GDIdeleteAlphaDC(mHDC, &bitmap);
		GDIdeleteBitmap(bitmap);
		
		mOwnHDC	= false;
		mHDC	= 0;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// access to our HDC if we really need to.
HDC	oDrawingCanvas::hdc() {
	return mHDC;
};

// get our text spec
GDItextSpecStruct	oDrawingCanvas::textSpec() {
	return mTextSpec;
};

// set our text spec
void	oDrawingCanvas::setTextSpec(GDItextSpecStruct pSpec) {
	mTextSpec = pSpec;
	clearTextCache();
	
	// and select our font
	HFONT		lvNewFont	= GDIcreateFont(&mTextSpec.mFnt, mTextSpec.mSty);
	HFONT		lvOldFont	= GDIselectObject(mHDC, lvNewFont);
	
	if (mTextFont != 0) {
		// save to delete the old selected font now that we've created a new one
		GDIdeleteObject(mTextFont);
	}
	mTextFont = lvNewFont;
	mFontHeight = GDIfontHeight(mHDC);
	
	if (mOldFont == 0) {
		// remember this so we reselect it when our canvas gets destructed..
		mOldFont = lvOldFont;
	};

	GDIsetTextColor(mHDC, mTextSpec.mTextColor);
};

////////////////////////////////////////////////////////////////
// colour functions
////////////////////////////////////////////////////////////////

qcol	oDrawingCanvas::mixColors(qcol pQ1, qcol pQ2) {
	// get real colors...
	pQ1	= GDIgetRealColor(pQ1);
	pQ2	= GDIgetRealColor(pQ2);
	
	short			cnt;
	qcol			res;
	unsigned char	mix;
	unsigned char	*cA = (unsigned char *)&pQ1; 
	unsigned char	*cB = (unsigned char *)&pQ2;
	unsigned char	*cR = (unsigned char *)&res;
	
	// This should mix R, G, B and alpha independently of eachother...
	for (cnt = 0; cnt < sizeof(qcol); cnt++) {
		if (*cA==*cB) {
			*cR = *cA;
		} else if (*cB > *cA) {
			mix = *cB - *cA;
			mix = mix >> 1;
			*cR = *cA + mix;
		} else {
			mix = *cA - *cB;
			mix = mix >> 1;
			*cR = *cB + mix;
		};
		
		cA++;
		cB++;
		cR++;
	};
	
	return res;
};

// setup our backpattern brush
void	oDrawingCanvas::setBackpatBrush(qpat pPat) {
	if (mBackpatBrush != 0) {
		GDIdeleteObject(mBackpatBrush);
	};

	mBackpatBrush = GDIcreateBrush(pPat);		
};

// set our background color
void	oDrawingCanvas::setBkColor(qcol pColor) {
	GDIsetBkColor(mHDC, pColor);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clipping functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Clip to given rectangle and put on stack, will optionally union with the current clipping. Will return false if we can't draw in the resulting rectangle and we could thus not clip.
bool	oDrawingCanvas::clipRect(qrect pRect, bool pUnion) {
	if (mHDC != 0) {
		if (pUnion) {
			qrect lvCliprect;
			int lvStackSize = mClipStack.size();
			
			if (mClipStack.empty()) {
				lvCliprect = mBaseRect;
			} else {
				lvCliprect = mClipStack.back();
			};
			
			// union the two rectangles so we clip only where the two rectangles overlap
			// note that if the rectangles do not overlap left will be bigger then right or top below the bottom.
			
			pRect.left		= pRect.left > lvCliprect.left ? pRect.left : lvCliprect.left;
			pRect.right		= pRect.right < lvCliprect.right ? pRect.right : lvCliprect.right;
			pRect.top		= pRect.top > lvCliprect.top ? pRect.top : lvCliprect.top;
			pRect.bottom	= pRect.bottom < lvCliprect.bottom ? pRect.bottom : lvCliprect.bottom;
		};
		
		if ((pRect.left < pRect.right) && (pRect.top < pRect.bottom)) {
			// clip
			GDIsetClipRect(mHDC, &pRect);
			
			// now add our rectangle to our clipstack
			mClipStack.push_back(pRect);
			
			return true;
		} else {
			return false;
		};
	} else {
		return false;
	};
};

// Pop our last clipping rectangle off the stack, do not call if clipRect returned false!
void	oDrawingCanvas::unClip() {
	if (mHDC != 0) {
		// remove the top one, that is our current clipping
		mClipStack.pop_back();
		
		if (mClipStack.empty()) {
			GDIclearClip(mHDC);
		} else {
			qrect lvCliprect = mClipStack.back();
			
			GDIsetClipRect(mHDC, &lvCliprect);			
		};
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Font functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Free memory associated with our text cache
void	oDrawingCanvas::clearTextCache() {
	while (!mTextCache.empty()) {
		mTextCache.erase(mTextCache.begin());		// remove it from our cache
	};
};

// passthrought to GDIdrawTextJst with clipping
void	oDrawingCanvas::drawTextJst(GDIdrawTextStruct * pTextInfo, qrect pRect, bool pAdjJst) {
	qrect lvClipRect = pRect;
	
	if (lvClipRect.left < pTextInfo->mX) lvClipRect.left = pTextInfo->mX;
	if (lvClipRect.top < pTextInfo->mY) lvClipRect.top = pTextInfo->mY;
	
	if (clipRect(lvClipRect, true)) {
		if (pAdjJst && (pTextInfo->mColumnJsts != 0)) {
			qdim	wasX = pTextInfo->mX;
			qint1*	wasJst = pTextInfo->mColumnJsts;
			qint1	jst = pTextInfo->mColumnJsts[0];
			
			pTextInfo->mColumnJsts = 0; // unset this... we're doing our own.
			
			if (jst!=jstLeft) {
				qdim	availWidth = pRect.right - pTextInfo->mX + 1;
				qdim	textWidth = getTextWidth(pTextInfo->mText, pTextInfo->mTextLen, pTextInfo->mFlags==1);
				
				if (jst == jstRight) {
					pTextInfo->mX = pRect.right - textWidth;
				} else if ((jst == jstCenter) && (availWidth > textWidth)) {
					pTextInfo->mX = pRect.left + ((availWidth - textWidth) / 2);
				};
			};
			
			// now draw it
			GDIdrawTextJst(pTextInfo);
			
			// undo any changes
			pTextInfo->mX = wasX;
			pTextInfo->mColumnJsts = wasJst;
		} else {
			GDIdrawTextJst(pTextInfo);
		};
		
		unClip();
	};
};

// get the height of the current selected font
qdim	oDrawingCanvas::getFontHeight() {
	return mFontHeight;
};

// Get the width of a word (from cache)
qdim	oDrawingCanvas::getWordWidth(qstring & pWord) {
	int		len = pWord.length();
	
	if (len<=50) {
		qTextWidthMap::iterator it;
		
		it = mTextCache.find(pWord.cString());
		if (it!=mTextCache.end()) {
//			oBaseComponent::addToTraceLog("Found cached word %qs, size = %li", &pWord, it->second);
			return it->second;
		};
	};
	
	// not found?


	GDItextSpecStruct	lvTextSpec	= mTextSpec;				// Copy of our text spec we're using
    lvTextSpec.mJst = jstLeft;                                  // Set to left justification or our text width fails

	GDIdrawTextStruct drawinfo(
							   mHDC,
							   0,
							   0,
							   (qchar *)pWord.cString(),	// for some reason Omnis never declared this a constant but it doesn't change the buffer (i hope)..
							   pWord.length(),
							   &lvTextSpec,
							   0,							// pColumnArray
							   0,							// pColumnCount
							   1,							// pFlags: 1 = styled text
							   mApp,
							   0							// pColumnJsts
					   );
		
	qdim width = GDItextWidthJst(&drawinfo) + 2;
	
	if (len<=50) {
		// can we cache it?
		mTextCache.insert(std::pair<qchar50, qdim>(pWord.cString(),width));
		
//		oBaseComponent::addToTraceLog("Cached word %qs, size = %li", &pWord, width);
	} else {
//		oBaseComponent::addToTraceLog("Width word %qs, size = %li", &pWord, width);
	};

	return width;
};

// add a word to a string
qdim	oDrawingCanvas::addWord(qstring & pTo, qstring & pWord, qstring &pPrefix, qdim pCurrWidth, qdim pMaxWidth) {
	if (pWord.length()==0) {
		return pCurrWidth; // No word to add..
	} else {
		qdim	wordWidth = getWordWidth(pWord);
		if (pCurrWidth==0) {
			// just add it even if it doesn't fit, if it doesn't fit it doesn't fit.. maybe some day we'll break up the word..
			pTo += pWord;
			return wordWidth;
		} else {
			qdim prefixWidth = getWordWidth(pPrefix);
			if (pCurrWidth+prefixWidth+wordWidth <= pMaxWidth) {
				// it fits! hurray!
				pTo += pPrefix;
				pTo += pWord;
				return pCurrWidth+prefixWidth+wordWidth;
			} else {
				// it doesn't fit :( add to a new line
				pTo += (qchar) '\n'; // prevent expensive UTF-8 => UTF-32 conversion
				pTo += pWord;
				return wordWidth;
			};
		};
	};
};

// wrap text, inserts newlines where needed to wrap the text
qstring	oDrawingCanvas::wrapText(const qchar *pText, qdim pMaxWidth) {
	qstring		retval;
	qstring		word;
	qstring		prefix;
	qstring		characters(":;?/\\'\".,<>(){}[]!@#$%^&*-_=+");
	int			pos = 0;
	qdim		width = 0;

	if (pMaxWidth < 10) {
		retval += pText;
	} else {
		while (pText[pos] != 0) {
			if (pText[pos]==(qchar)'\r') {
				width = addWord(retval, word, prefix, width, pMaxWidth);			
				word	= "";
				prefix	= "";
				
				retval += (qchar) '\n'; // prevent expensive UTF-8 => UTF-32 conversion
			} else if (pText[pos]==(qchar)'\n') {
				if (pText[pos+1]==(qchar)'\r') {	// windows newline?
					pos++;
				};
				width = addWord(retval, word, prefix, width, pMaxWidth);
				word	= "";
				prefix	= "";
				
				retval += (qchar) '\n'; // prevent expensive UTF-8 => UTF-32 conversion
			} else if (pText[pos]==(qchar)' ') {
				if (word.length()>0) {
					width	= addWord(retval, word, prefix, width, pMaxWidth);
					word	= "";
					prefix	= "";
				};
				prefix += pText[pos];
			} else if (pText[pos]==txtEsc && pText[pos+10] == txtAsciiEnd) {
				if (word.length()>0) {
					width = addWord(retval, word, prefix, width, pMaxWidth);
					word	= "";
					prefix	= "";
				};
				
				retval.appendString(&pText[pos], 11); // just add our style, we ignore tabs and only images take space, we'll do something for those in due time
				pos+=10; // we'll add one more in a minute			
			} else if (characters.pos(pText[pos])>=0) {
				if (word.length()>0) {
					width	= addWord(retval, word, prefix, width, pMaxWidth);
					word	= "";
					prefix	= "";
				};
	
				// these we just add in...
				word	+= pText[pos];
				width	= addWord(retval, word, prefix, width, pMaxWidth);
				word	= "";
				prefix	= "";
			} else {
				word += pText[pos];			
			};
			
			pos++;
		};
		
		// any last words to add?
		if (word.length()>0) {
			width	= addWord(retval, word, prefix, width, pMaxWidth);
		};
	};
			
	return retval;
};


// Get the width of text
qdim	oDrawingCanvas::getTextWidth(const qchar *pText, qshort pLen, bool pStyled) {
	if (pLen==0) {		
		return 0;
	}

	GDItextSpecStruct	lvTextSpec	= mTextSpec;				// Copy of our text spec we're using
    lvTextSpec.mJst = jstLeft;                                  // Set to left justification or our text width fails
	
	// It seems that on the OS4 SDK GDItextWidthJst gives a wrong result on a retina display, so only using it in unicode
	GDIdrawTextStruct drawinfo(
							   mHDC,
							   0,
							   0,
							   (qchar *)pText,			// for some reason Omnis never declared this a constant but it doesn't change the buffer (i hope)..
							   pLen,
							   &lvTextSpec,
							   0,						// pColumnArray
							   0,						// pColumnCount
							   pStyled ? 1 : 0,			// pFlags: 1 = styled text
							   mApp,
							   0						// pColumnJsts
							   );
	
	qdim width = GDItextWidthJst(&drawinfo) + 2;
	
	return width;
};

// Get the heigth of text
qdim	oDrawingCanvas::getTextHeight(const qchar *pText, qdim pWidth, bool pStyled, bool pWrap) {
	if (pWrap) {
		qstring		wrapped = wrapText(pText, pWidth);
		return getTextHeight(wrapped.cString(), pWidth, pStyled, false);
	} else {
		qdim	top = 0;
		int		pos = 0;
		bool	hasWord = false;
		
		while (pText[pos] != 0) {
			if (pText[pos] == '\n') {
				top+=mFontHeight;
				hasWord = false;
			} else {
				hasWord = true;
			};
			
			pos++;
		};
		
		if (hasWord) top+=mFontHeight;
		
		return top;
	};
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Drawing functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Draws text with a specific text struct (left aligned, no wrapping or clipping)
void    oDrawingCanvas::drawText(const qchar *pText, qpoint pWhere, const GDItextSpecStruct &pTextSpec) {
	GDIdrawTextStruct drawinfo(
        mHDC,
        pWhere.h,
        pWhere.v,
        (qchar *)pText,		// for some reason Omnis never declared this a constant but it doesn't change the buffer (i hope)..
        OMstrlen(pText),
        (GDItextSpecStruct *)&pTextSpec,
        0,                  // pColumnArray
		0,					// pColumnCount
		1,                  // pFlags: 1 = styled text
        mApp,
        0                   // pColumnJsts
    );

    // draw the whole text
    GDIdrawTextJst(&drawinfo);
};

// Draws the text clipped within the specified rectangle 
qdim	oDrawingCanvas::drawText(const qchar *pText, qrect pWhere, qcol pColor, qjst pJst, bool pStyled, bool pWrap) {
	if ((pWhere.right <= pWhere.left) || (pWhere.bottom <= pWhere.top)) {
		// fluffy don't fit!
		return 0;
	};

	if (pWrap) {
		qstring		wrapped = wrapText(pText, pWhere.right - pWhere.left + 1);
		return drawText(wrapped.cString(), pWhere, pColor, pJst, pStyled, false);
	} else {
		GDItextSpecStruct	lvTextSpec	= mTextSpec;				// Copy of our text spec we're using
		qcol				wascol		= GDIgetTextColor(mHDC);
		qlong				len			= OMstrlen(pText);
		qdim				fontheight	= mFontHeight;
		qshort				pos			= 0;
		qshort				start		= 0;
		qdim				top			= pWhere.top;
		
		// Need to find a better way to get a unicode character :)
		qchar		newline	= '\n';
		qchar		space	= ' ';

		// Set our text color
		if (pColor != GDI_COLOR_QDEFAULT) lvTextSpec.mTextColor = pColor;

		// setup our justification
        if (pJst == jstNone) pJst = lvTextSpec.mJst;
        lvTextSpec.mJst = jstLeft;
        
		// now loop through to find our lines or until we're below our drawing rectangle.
//		while ((pos <= len) && ((top + fontheight) <= pWhere.bottom)) { // note that we add fontheight to our check because our clipping doesn't work very well with drawTextJst
		while ((pos <= len) && (top <= pWhere.bottom)) { // Clipping should be fixed in 6.1
				if ((pText[pos] == 0x00) || (pText[pos] == newline)) {
				if (pos > start) {
					// draw our text...
                    qdim left = pWhere.left;
                    
                    // justify
                    if (pJst != jstLeft) {
                        qdim width = getTextWidth((qchar *)&pText[start], pos-start, pStyled);
                        if (pJst == jstRight) {
                            left += pWhere.width() - width;
                        } else if (pJst == jstCenter) {
                            left += (pWhere.width() - width) / 2;
                        };
                    };
					
					GDIdrawTextStruct drawinfo(
											   mHDC,
											   left,
											   top,
											   (qchar *)&pText[start],		// for some reason Omnis never declared this a constant but it doesn't change the buffer (i hope)..
											   pos-start,
											   &lvTextSpec,
											   0,                       // pColumnArray
											   0,						// pColumnCount
											   pStyled ? 1 : 0,			// pFlags: 1 = styled text
											   mApp,
											   0						// pColumnJsts
											   );
					
					// draw the whole text
					drawTextJst(&drawinfo, pWhere, true);
					
					if (pStyled) {
						// As suggested by Stefan Csomor, check the text we just drew and update our text spec
						qshort checkUntil = pos <= len - 11 ? pos : len - 11; // make sure we stay within bounds
						qshort checkstyle = start;
						while (checkstyle < checkUntil) {
							if (pText[checkstyle] == txtEsc && pText[checkstyle+10] == txtAsciiEnd) {
								switch (pText[checkstyle + 1]) {
									case txtEscSty: {
										lvTextSpec.mSty  = (qsty) oBaseComponent::HexToLong((qchar *) &pText[checkstyle + 2]);
									}; break;
									case txtEscCol: {
										lvTextSpec.mTextColor	= (qcol) oBaseComponent::HexToLong((qchar *) &pText[checkstyle + 2]);
									}; break;
									default:
										break;
								};
								checkstyle+=11;
							} else {
								checkstyle++;
							};
						};
					};				
				};
				
				// get start position for next line..
				start	= pos + 1;
				top		+= fontheight;
			};
			
			pos++;
		};

		// restore our text color
		GDIsetTextColor(mHDC, wascol);

		return top;
	};
};

// Draw a icon at this position, height/width of rectangle is only used for centering, no clipping!
qdim	oDrawingCanvas::drawIcon(qlong pIconId, qrect pAt, qjst pHorzJst, qjst pVertJst, bool pEnabled) {
	EXTBMPref	tmpIcon(pIconId);
    
    if (tmpIcon.getIconId() == 0) {
        // pIconId could just hold our size info if someone reset the iconid back to 0
        return 0;
    } else {
        qdim		pxsize = 16;
        ePicSize	picsize = tmpIcon.getBmpSize(pIconId);

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
                // don't know the size, assume 48, need to find out how to get our default size...
                pxsize = 48;
                break;
        };
	
        if (clipRect(pAt, true)) {
            tmpIcon.draw (mHDC, &pAt, picsize, picNormal, !pEnabled, colNone, qfalse, pHorzJst, pVertJst);
		
            unClip();
        };
    
        return pxsize;
    };
};


// Draws a line between two points using the current selected pen
void	oDrawingCanvas::drawLine(qpoint pFrom, qpoint pTo) {
	GDImoveTo(mHDC, &pFrom);
	GDIlineTo(mHDC, &pTo);	
};

// Draws a line between two points
void	oDrawingCanvas::drawLine(qpoint pFrom, qpoint pTo, qdim pWidth, qcol pCol, qpat pPat) {
	HPEN	newPen = GDIcreatePen(pWidth, pCol, pPat);
	HPEN	oldPen = GDIselectObject(mHDC, newPen);
	qcol	oldCol = GDIgetTextColor(mHDC);
	
	GDIsetTextColor(mHDC, pCol);	// shouldn't be needed but for some reason in the SDK documentation they also set the text color...
	drawLine(pFrom, pTo);
	
	GDIsetTextColor(mHDC, oldCol);
	GDIselectObject(mHDC, oldPen);
};

// fills a rectangle using our standard pattern brush
void	oDrawingCanvas::drawRect(qrect pRect, qcol pFillColor) {
	drawRect(pRect, pFillColor, mBackpatBrush);
};

// fills a rectangle with a certain pattern
void	oDrawingCanvas::drawRect(qrect pRect, qcol pFillColor, HBRUSH pBrush) {
	qcol	wasColor = GDIgetTextColor(mHDC);

	GDIsetTextColor(mHDC, pFillColor);
	GDIfillRect(mHDC, &pRect, pBrush);

	// leave it as it was...
	GDIsetTextColor(mHDC, wasColor);
};

// draw a rectangle. Note, if pFillColor is set to GDI_COLOR_QDEFAULT we do not fill the rectangle
void	oDrawingCanvas::drawRect(qrect pRect, qcol pFillColor, qcol pBorder) {
	qcol	wasColor = GDIgetTextColor(mHDC);
	
	if (pFillColor != GDI_COLOR_QDEFAULT) {
		drawRect(pRect, pFillColor, GDIgetStockBrush(BLACK_BRUSH));
	};
	
	if ((pFillColor != pBorder) && (pBorder != GDI_COLOR_QDEFAULT)) {
		HPEN	borderPen	= GDIcreatePen(1, pBorder, patFill);
		HPEN	oldPen		= GDIselectObject(mHDC, borderPen);
		
		// draw our border
		GDIsetTextColor(mHDC, pBorder);
		GDIframeRect(mHDC, &pRect);
		
		GDIselectObject(mHDC, oldPen);
		GDIdeleteObject(borderPen);
	};
	
	// leave it as it was...
	GDIsetTextColor(mHDC, wasColor);
};

// draw a rounded rectangle. Note, if pFillColor is set to GDI_COLOR_QDEFAULT we do not fill the rectangle
void    oDrawingCanvas::drawRoundedRect(qrect pRect, qdim pDiameter, qcol pFillColor, qcol pBorder) {
	qcol	wasColor = GDIgetTextColor(mHDC);
	
	if (pFillColor != GDI_COLOR_QDEFAULT) {
        GDIsetTextColor(mHDC, pFillColor);
		GDIfillRoundRect(mHDC, &pRect, pDiameter, pDiameter, GDIgetStockBrush(BLACK_BRUSH));
	};
	
	if ((pFillColor != pBorder) && (pBorder != GDI_COLOR_QDEFAULT)) {
		HPEN	borderPen	= GDIcreatePen(1, pBorder, patFill);
		HPEN	oldPen		= GDIselectObject(mHDC, borderPen);
		
		// draw our border
		GDIsetTextColor(mHDC, pBorder);
		GDIframeRoundRect(mHDC, &pRect, pDiameter, pDiameter);
		
		GDIselectObject(mHDC, oldPen);
		GDIdeleteObject(borderPen);
	};
	
	// leave it as it was...
	GDIsetTextColor(mHDC, wasColor);
};

// Draws a filled ellipse within the rectangle with a gradient color from top to bottom
void	oDrawingCanvas::drawEllipse(qrect pRect, qcol pTop, qcol pBottom, qcol pBorder, qint pSpacing) {
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
