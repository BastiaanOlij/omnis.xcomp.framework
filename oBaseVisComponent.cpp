/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseNVComponent.cpp
 *  Base class for our visual component
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseVisComponent.h"

/********************************************************************************************************************************************
 oBaseVisComponent
 ********************************************************************************************************************************************/

////////////////////////////////////////////////////////////////
// Properties
////////////////////////////////////////////////////////////////

ECOproperty oBaseVisProperties[] = { 
	//	ID						ResID	Type			Flags					ExFlags	EnumStart	EnumEnd
	anumForecolor,			0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $forecolor
	anumBackcolor,			0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $backcolor
	anumBackpattern,		0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $backpattern
	anumBackgroundTheme,	0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $bgtheme
	
	anumTextColor,			0,		fftInteger,		EXTD_FLAG_PROPTEXT,		0,		0,			0,		// $textcolor
	anumFont,				0,		fftCharacter,	EXTD_FLAG_PROPTEXT,		0,		0,			0,		// $font
	anumFontsize,			0,		fftInteger,		EXTD_FLAG_PROPTEXT,		0,		0,			0,		// $fontsize
	anumFontstyle,			0,		fftInteger,		EXTD_FLAG_PROPTEXT,		0,		0,			0,		// $fontstyle
	anumAlign,				0,		fftInteger,		EXTD_FLAG_PROPTEXT,		0,		0,			0,		// $align	
};

oBaseVisComponent::oBaseVisComponent(void) {
	mForecolor		= GDI_COLOR_QDEFAULT;
	mBackcolor		= GDI_COLOR_QDEFAULT;
	mOffsetX		= 0;
	mOffsetY		= 0;
	mBackpattern	= 0;
	mBKTheme		= WND_BK_NONE;
	mDrawBuffer		= true;
};

// Initialize component
qbool oBaseVisComponent::init(qapp pApp, HWND pHWnd) {
	oBaseComponent::init(pApp);
	
	mHWnd = pHWnd;
	mMouseLButtonDown = false;

	WNDsetScrollRange(mHWnd, SB_HORZ, 0, 0, 1, qfalse);
	WNDsetScrollRange(mHWnd, SB_VERT, 0, 0, 1, qfalse);
	
	return true;
};

// Add properties for visual componect
qProperties * oBaseVisComponent::properties(void) {
	qProperties *	lvProperties = oBaseComponent::properties();
	
	// Add the property definition for our visual component here...
	lvProperties->addElements(oBaseVisProperties, sizeof(oBaseVisProperties) / sizeof(ECOproperty));
	
	return lvProperties;
};

// set the value of a property
qbool oBaseVisComponent::setProperty(qlong pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI) {
	// most anum properties are managed by Omnis but some we need to do ourselves, no idea why...
	
//	addToTraceLog("Setting property %li",pPropID);
	
	switch (pPropID) {
		case anumForecolor:
			mForecolor = pNewValue.getLong();
			WNDinvalidateRect(mHWnd, NULL);
			
//			addToTraceLog("Changed color to %li",mForecolor);
			
			return 1L;
			break;
		case anumBackcolor:
			mBackcolor = pNewValue.getLong();
			WNDinvalidateRect(mHWnd, NULL);	
			
			return 1L;
			break;
		case anumBackpattern:
			mBackpattern = (qpat) pNewValue.getLong();
			WNDinvalidateRect(mHWnd, NULL);	
			
			return 1L;
			break;
		case anumBackgroundTheme:
			mBKTheme = pNewValue.getLong();
			WNDinvalidateRect(mHWnd, NULL);	
			
			return 1L;
			break;
		default:
			return oBaseComponent::setProperty(pPropID, pNewValue, pECI);
			break;
	};
};

// get the value of a property
void oBaseVisComponent::getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* eci) {
	// most anum properties are managed by Omnis but some we need to do ourselves...
	
	switch (pPropID) {
		case anumForecolor:
			pGetValue.setLong(mForecolor);
			break;
		case anumBackcolor:
			pGetValue.setLong(mBackcolor);
			break;
		case anumBackpattern:
			pGetValue.setLong(mBackpattern);
			break;
		case anumBackgroundTheme:
			pGetValue.setLong(mBKTheme);
			break;
		default:
			oBaseComponent::getProperty(pPropID, pGetValue, eci);
			
			break;
	};
};

////////////////////////////////////////////////////////////////
// $dataname property
////////////////////////////////////////////////////////////////

// Changes our primary data
qbool	oBaseVisComponent::setPrimaryData(EXTfldval &pNewValue) {
	return copyFldVal(pNewValue, mPrimaryData);
};

// Retrieves our primary data
void	oBaseVisComponent::getPrimaryData(EXTfldval &pGetValue) {
	copyFldVal(mPrimaryData, pGetValue);
};

// Compare with our primary data
qbool	oBaseVisComponent::cmpPrimaryData(EXTfldval &pWithValue) {
	if (pWithValue.compare(mPrimaryData)==0) {
		return qtrue;
	} else {
		return qfalse;
	}
};

// Get our primary data size
qlong	oBaseVisComponent::getPrimaryDataLen() {
	ffttype valuetype;
	qshort  valuesubtype;
	
	mPrimaryData.getType(valuetype, &valuesubtype);
	
	switch (valuetype) {
		case fftInteger:
			return sizeof(qlong);
			break;
		case fftNumber:
			return sizeof(qreal);
			break;
		case fftCharacter:
			return mPrimaryData.getCharLen();
			break;
		case fftBinary:
		case fftPicture:
			return mPrimaryData.getBinLen();
			break;
		default:
			return 0;
			break;
	}
};

// Omnis is just letting us know our primary data has changed, this is especially handy if we do not keep a copy ourselves and thus ignore the other functions
void oBaseVisComponent::primaryDataHasChanged() {
	// by default just trigger a redraw...
	WNDinvalidateRect(mHWnd, NULL);
};


////////////////////////////////////////////////////////////////
// Methods and events
////////////////////////////////////////////////////////////////


qMethods * oBaseVisComponent::methods(void) {
	qMethods * lvMethods = oBaseComponent::methods();
	
	// our visual class has not methods, we still need to implement this...
	
	return lvMethods;
};

// Add events for visual component
qEvents *	oBaseVisComponent::events(void) {
	qEvents * lvEvents = oBaseComponent::events();
	
	// our visual class has no events
	
	return lvEvents;
};


void	oBaseVisComponent::Resized() {
	// probably not needed but....
	WNDinvalidateRect(mHWnd, NULL);	
};

////////////////////////////////////////////////////////////////
// Drawing related
////////////////////////////////////////////////////////////////

qcol	oBaseVisComponent::mixColors(qcol pQ1, qcol pQ2) {
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

// Do our drawing in here
void oBaseVisComponent::doPaint(EXTCompInfo* pECI) {
	// override to implement drawing...
	if (!WNDdrawThemeBackground(mHWnd,mHDC,&mClientRect,mBKTheme)) {
		// clear our drawing field
		GDIsetTextColor(mHDC, mForecolor);
		GDIfillRect(mHDC, &mClientRect, mBackpatBrush);
		GDIsetTextColor(mHDC, mTextColor);		
	};
};

// setup our fonts and brushes
void oBaseVisComponent::setup(EXTCompInfo* pECI) {
	qshort			fontSize;
	qsty			fontStyle;
	qjst			fontAlign;
	EXTfldval		fval, font; 
	str255			fieldStyle;	
	
	// Create our pattern brush, it will be deleted at the end of drawing..
	mBackpatBrush = GDIcreateBrush(mBackpattern);
	
	// get all our text properties...
	ECOgetProperty(mHWnd,anumFldStyle,fval); 
	fval.getChar(fieldStyle);
	ECOgetProperty(mHWnd,anumFont,font); 
	ECOgetProperty(mHWnd,anumFontsize,fval); 
	fontSize = (qshort) fval.getLong();
	ECOgetProperty(mHWnd,anumFontstyle,fval); 
	fontStyle = (qsty) fval.getLong();
	ECOgetProperty(mHWnd,anumTextColor,fval); 
	mTextColor = fval.getLong();
	ECOgetProperty(mHWnd,anumAlign,fval); 
	fontAlign = (qjst) fval.getLong();
	
	// Start with loading from our normal settings (defaults for now)
	ECOgetFont(mHWnd, &(mTextSpec.mFnt), ECOgetFontIndex(mHWnd, font), fontSize);
	mTextSpec.mSty = fontStyle;
	mTextSpec.mJst = fontAlign;
	mTextSpec.mTextColor = mTextColor;
	
	// Now see if we need to override any of these defaults with our fieldstyle...
	if (fieldStyle[0]>0) {
		ECOgetStyle( ECOgetApp(pECI->mInstLocp), &fieldStyle[1], fieldStyle[0], &mTextSpec );
		
		// !BAS! need to also see if we need to get our foreground color, background color and/or background pattern from our style
		// not sure if Omnis feeds this info back to us already
	};	
};

// clipping
void	oBaseVisComponent::clipRect(qrect pRect, bool pUnion) {
	if (mHDC != 0) {
		if (pUnion) {
			qrect lvCliprect;
			int lvStackSize = mClipStack.numberOfElements();
			
			if (lvStackSize > 0) {
				lvCliprect = mClipStack[lvStackSize-1];
			} else {
				lvCliprect = mClientRect;
			};
			
			// union the two rectangles so we clip only where the two rectangles overlap
			// note that if the rectangles do not overlap left will be bigger then right or top below the bottom.
			
			pRect.left		= pRect.left > lvCliprect.left ? pRect.left : lvCliprect.left;
			pRect.right		= pRect.right < lvCliprect.right ? pRect.right : lvCliprect.right;
			pRect.top		= pRect.top > lvCliprect.top ? pRect.top : lvCliprect.top;
			pRect.bottom	= pRect.bottom < lvCliprect.bottom ? pRect.bottom : lvCliprect.bottom;
		};
	
		// clip
		GDIsetClipRect(mHDC, &pRect);
	
		// now add our rectangle to our clipstack
		mClipStack.push(pRect);
	};
};

void	oBaseVisComponent::unClip() {
	if (mHDC != 0) {
		// remove the top one, that is our current clipping
		mClipStack.pop();
		
		int lvStackSize = mClipStack.numberOfElements();
		if (lvStackSize > 0) {
			qrect lvCliprect = mClipStack[lvStackSize-1];
			
			GDIsetClipRect(mHDC, &lvCliprect);			
		} else {
			GDIclearClip(mHDC);
		};
	};
};

// paint message
void oBaseVisComponent::wm_paint(EXTCompInfo* pECI) {
	WNDpaintStruct	lvPaintStruct;
	qrect			lvUpdateRect;
	void *			lvOffScreenPaint;
	
	// clear our clip stack
	mClipStack.clear();
	
	// get current size info
	WNDgetClientRect(mHWnd, &mClientRect);
	
	if (mDrawBuffer) {
		// create our paint structure
		WNDbeginPaint( mHWnd, &lvPaintStruct );
		
		lvUpdateRect = lvPaintStruct.rcPaint;
		mHDC = lvPaintStruct.hdc;
		
		setup(pECI);
		
		// On windows this will do a double buffer trick, on Mac OSX the OS already does the offscreen painting:)
		// note that mHDC and mClienRect may be altered as a result of this call which is good!
		lvOffScreenPaint = GDIoffscreenPaintBegin(NULL, mHDC, mClientRect, lvUpdateRect);
		if (lvOffScreenPaint) {		
			// setup defaults for GDI drawing..
			HFONT		lvTextFont	= GDIcreateFont(&mTextSpec.mFnt, mTextSpec.mSty);
			HFONT		lvOldFont	= GDIselectObject(mHDC, lvTextFont); 
			
			// default our colors
			GDIsetBkColor(mHDC, mBackcolor);
			GDIsetTextColor(mHDC, mTextColor);	// if we need our forecolor for drawing we will switch..
			
			// do our real drawing
			doPaint(pECI);
			
			// If in design mode, then call drawDesignName, drawNumber & drawMultiKnobs to draw design
			// name, numbers and multiknobs, if required.
			if ( ECOisDesign(mHWnd) ) {
				ECOdrawDesignName(mHWnd,mHDC);
				ECOdrawNumber(mHWnd,mHDC);
				ECOdrawMultiKnobs(mHWnd,mHDC);
			}
			
			GDIselectObject(mHDC, lvOldFont);
			GDIdeleteObject(lvTextFont);
			
			GDIdeleteObject(mBackpatBrush);
			
			GDIoffscreenPaintEnd(lvOffScreenPaint);
		}
		
		
		// And finish paint...
		WNDendPaint( mHWnd, &lvPaintStruct );	
	} else {		
		// component must fully implement and is responsible for setting HDC...
		mHDC = 0;
		doPaint(pECI);
	}
	
	// Just free up memory..
	mClipStack.clear();
};

// Component resize/repos message
void	oBaseVisComponent::wm_windowposchanged(EXTCompInfo* pECI, WNDwindowPosStruct * pPos) {
	Resized();
};

////////////////////////////////////////////////////////////////////////////////////////
// Scrollbar related functions
////////////////////////////////////////////////////////////////////////////////////////

// get our horizontal step size
qdim	oBaseVisComponent::getHorzStepSize(void) {
	return 8;
};

// get our vertical step size
qdim	oBaseVisComponent::getVertStepSize(void) {
	return 8;	
};

// window was scrolled
void	oBaseVisComponent::evWindowScrolled(qdim pNewX, qdim pNewY) {
	if ((mOffsetX!=pNewX) || (mOffsetY!=pNewY)) {
		WNDsetScrollPos(mHWnd, SB_HORZ, pNewX, qfalse); 
		WNDsetScrollPos(mHWnd, SB_VERT, pNewY, qfalse);
		
		// we may not need to do this..
		WNDscrollWindow(mHWnd, mOffsetX - pNewX, mOffsetY-pNewY);

		// redraw the whole thing...
		WNDinvalidateRect(mHWnd, NULL);

		if (mMouseLButtonDown) {
			mMouseDownAt.h += pNewX-mOffsetX;
			mMouseDownAt.v += pNewY-mOffsetY;
		};	

		mOffsetX = pNewX;
		mOffsetY = pNewY;
	};
};


////////////////////////////////////////////////////////////////////////////////////////
// mouse related functions
////////////////////////////////////////////////////////////////////////////////////////


void	oBaseVisComponent::evMouseMoved(qpoint pAt) {
	// stub	
};

void	oBaseVisComponent::evClick(qpoint pAt) {
	// stub
};	

void	oBaseVisComponent::evStartDrag(qpoint pFrom) {
	// stub
};

void	oBaseVisComponent::evDragging(qpoint pFrom, qpoint pAt) {
	// stub
};

void	oBaseVisComponent::evEndDrag(qpoint pFrom, qpoint pTop) {
	// stub	
};

void	oBaseVisComponent::evCancelledDrag() {
	// stub
};

void	oBaseVisComponent::wm_lbutton(qpoint pAt, bool pDown) {
	mMouseAt = pAt; /* store a copy of our mouse location */
	
	if (pDown) {
		mMouseLButtonDown = true;
		mMouseDragging = false;
		mMouseDownAt = pAt;
	} else {
		mMouseLButtonDown = false;
		if (mMouseDragging) {
			this->evEndDrag(mMouseDownAt, pAt);
		} else {
			this->evClick(pAt);
		};
	};	
};

void	oBaseVisComponent::wm_mousemove(qpoint pMovedTo) {
	mMouseAt = pMovedTo; /* store a copy of our mouse location */
	
	if (!WNDmouseLeftButtonDown() && mMouseLButtonDown) {
		// I guess we missed a button up somewhere...
		
		if (mMouseDragging) {
			mMouseDragging = false;
			this->evCancelledDrag();
		};
		
		this->wm_lbutton(mMouseAt, false);
	};
	
	if (mMouseLButtonDown) {
		if (!mMouseDragging) {
			qlong distance = mMouseAt.h > mMouseDownAt.h ? mMouseAt.h - mMouseDownAt.h : mMouseDownAt.h - mMouseAt.h;
			distance += mMouseAt.v > mMouseDownAt.v ? mMouseAt.v - mMouseDownAt.v : mMouseDownAt.v - mMouseAt.v;
			
			if (distance > 5) {
				// need to move the mouse more then 5 pixels to start dragging..
				mMouseDragging = qtrue;
				this->evStartDrag(mMouseDownAt);
				this->evDragging(mMouseDownAt, mMouseAt);				
			};
		} else {
			this->evDragging(mMouseDownAt, mMouseAt);
		};
	} else {
		if (mMouseDragging) {
			// somehow we have an unfinished drag..
			mMouseDragging = qfalse;
			this->evCancelledDrag();
		};
		
		this->evMouseMoved(mMouseAt);
	};
};


