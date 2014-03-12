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
	mForecolor = GDI_COLOR_QDEFAULT;
	mBackcolor = GDI_COLOR_QDEFAULT;
	mBackpattern = 0;
	mBKTheme = WND_BK_NONE;
	mDrawBuffer = true;
};

// Initialize component
qbool oBaseVisComponent::init(HWND pHWnd) {
	mHWnd = pHWnd;
	
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
qbool oBaseVisComponent::setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI) {
	// most anum properties are managed by Omnis but some we need to do ourselves, no idea why...
	
	addToTraceLog("Setting property %li",pPropID);
	
	switch (pPropID) {
		case anumForecolor:
			mForecolor = pNewValue.getLong();
			WNDinvalidateRect(mHWnd, NULL);
			
			addToTraceLog("Changed color to %li",mForecolor);
			
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
void oBaseVisComponent::getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* eci) {
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
	
};

////////////////////////////////////////////////////////////////
// Drawing related
////////////////////////////////////////////////////////////////

qcol	oBaseVisComponent::mixColors(qcol pQ1, qcol pQ2) {
	// get real colors...
	pQ1	= GDIgetRealColor(pQ1);
	pQ2	= GDIgetRealColor(pQ2);
	
	int R = ((((pQ1 >> 16) & 0xFF) + ((pQ2 >> 16) & 0xFF)) >> 1) & 0xFF;
	int G = ((((pQ1 >> 8) & 0xFF) + ((pQ2 >> 8) & 0xFF)) >> 1) & 0xFF;
	int B = (((pQ1 & 0xFF) + (pQ2 & 0xFF)) >> 1) & 0xFF;
	
	return (R << 16) + (G << 8) + B;
};

// Do our drawing in here
void oBaseVisComponent::doPaint(HDC pHDC) {
	// override to implement drawing...
	if (!WNDdrawThemeBackground(mHWnd,pHDC,&mClientRect,mBKTheme)) {
		// clear our drawing field
		GDIsetTextColor(pHDC, mForecolor);
		GDIfillRect(pHDC, &mClientRect, mBackpatBrush);
		GDIsetTextColor(pHDC, mTextColor);		
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
		
		// !BAS! need to also see if we need to get our foreground color, background color and/or background pattern
	};	
};

// paint message
void oBaseVisComponent::wm_paint(EXTCompInfo* pECI) {
	WNDpaintStruct	lvPaintStruct;
	qrect			lvUpdateRect;
	HDC				lvHDC;
	void *			lvOffScreenPaint;
	
	// get current size info
	WNDgetClientRect(mHWnd, &mClientRect);
	
	if (mDrawBuffer) {
		// create our paint structure
		WNDbeginPaint( mHWnd, &lvPaintStruct );
		
		lvUpdateRect = lvPaintStruct.rcPaint;
		lvHDC = lvPaintStruct.hdc;
		
		setup(pECI);
		
		// On windows this will do a double buffer trick, on Mac OSX the OS already does the offscreen painting:)
		lvOffScreenPaint = GDIoffscreenPaintBegin(NULL, lvHDC, mClientRect, lvUpdateRect);
		if (lvOffScreenPaint) {
			// setup defaults for GDI drawing..
			HFONT		lvTextFont	= GDIcreateFont(&mTextSpec.mFnt, mTextSpec.mSty);
			HFONT		lvOldFont	= GDIselectObject(lvHDC, lvTextFont); 
			
			GDIsetBkColor(lvHDC, mBackcolor);
			GDIsetTextColor(lvHDC, mTextColor);	// if we need our forecolor for drawing we will switch..
			
			// do our real drawing
			doPaint(lvHDC);
			
			// If in design mode, then call drawDesignName, drawNumber & drawMultiKnobs to draw design
			// name, numbers and multiknobs, if required.
			if ( ECOisDesign(mHWnd) ) {
				ECOdrawDesignName(mHWnd,lvHDC);
				ECOdrawNumber(mHWnd,lvHDC);
				ECOdrawMultiKnobs(mHWnd,lvHDC);
			}
			
			GDIselectObject(lvHDC, lvOldFont);
			GDIdeleteObject(lvTextFont);
			
			GDIdeleteObject(mBackpatBrush);
			
			GDIoffscreenPaintEnd(lvOffScreenPaint);
		}
		
		
		// And finish paint...
		WNDendPaint( mHWnd, &lvPaintStruct );	
	} else {		
		// component must fully implement...
		doPaint(0);
	}
};

// Component resize/repos message
void	oBaseVisComponent::wm_windowposchanged(EXTCompInfo* pECI, WNDwindowPosStruct * pPos) {
	Resized();
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


