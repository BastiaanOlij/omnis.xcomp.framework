/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseVisComponent.cpp
 *  Base class for our visual component
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - See if we can put our drawing functions into a separate context object
 *  - Implement a way to create a snapshot bitmap
 *  - Complete drag and drop interface
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
	mForecolor			= GDI_COLOR_QDEFAULT;
	mBackcolor			= GDI_COLOR_QDEFAULT;
	mOffsetX			= 0;
	mOffsetY			= 0;
	mBackpattern		= 0;
	mBKTheme			= WND_BK_NONE;
	mDrawBuffer			= true;
	mMouseLButtonDown	= false;
	mMouseDragging		= false;
};

// Initialize component
qbool oBaseVisComponent::init(qapp pApp, HWND pHWnd) {
	oBaseComponent::init(pApp);
	
	mHWnd = pHWnd;

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

// get list for $dataname
EXTqlist *	oBaseVisComponent::getDataList(EXTCompInfo* pECI) {
	EXTfldval	listFld;
	str255		listName;
	
	ECOgetProperty(mHWnd, anumFieldname, listFld);
	listFld.getChar(listName);		
	EXTfldval	dataField(listName, qfalse, pECI->mLocLocp);

	ffttype		datatype;
	qshort		datasubtype;
	
	dataField.getType(datatype, &datasubtype);
	qstring		msg = QTEXT("Data type $dataname: ");
	msg += fldTypeName(datatype);
	addToTraceLog(msg.c_str());	
	
	return dataField.getList(qfalse);
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
void oBaseVisComponent::getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI) {
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
			oBaseComponent::getProperty(pPropID, pGetValue, pECI);
			
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
qbool	oBaseVisComponent::getPrimaryData(EXTfldval &pGetValue) {
	return copyFldVal(mPrimaryData, pGetValue);
};

// Compare with our primary data, return DATA_CMPDATA_SAME if same, DATA_CMPDATA_DIFFER if different
qlong	oBaseVisComponent::cmpPrimaryData(EXTfldval &pWithValue) {
	if (pWithValue.compare(mPrimaryData)==0) {
		return DATA_CMPDATA_SAME;
	} else {
		return DATA_CMPDATA_DIFFER;
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
		case fftRow:
		case fftList:
			return mPrimaryData.getBinLen(); // not sure if this makes sense...
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

void	oBaseVisComponent::resized() {
	//	WNDinvalidateRect(mHWnd, NULL);	
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

// Clip to given rectangle and put on stack, will optionally union with the current clipping. Will return false if we can't draw in the resulting rectangle and we could thus not clip.
bool	oBaseVisComponent::clipRect(qrect pRect, bool pUnion) {
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
	
		if ((pRect.left < pRect.right) && (pRect.top < pRect.bottom)) {
			// clip
			GDIsetClipRect(mHDC, &pRect);
			
			// now add our rectangle to our clipstack
			mClipStack.push(pRect);
			
			return true;
		} else {
			return false;
		};
	} else {
		return false;
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

// erase our background message
void	oBaseVisComponent::wm_erasebkgnd(EXTCompInfo* pECI) {
	/* 
		The SDK specifies we need to draw the background here and then draw whatever foreground stuff we have in wm_paint.

		This is an old optimalisation from the early Windows days where drawing stuff was CPU expensive but it also leads
		to alot of flickering of the screen.

		In our wm_paint we now use a buffer to draw too which is then blitted to the screen. In drawing that buffer we also
		draw the background.

		So we're going to cheat here, and ignore the erase background.. ;)
	*/
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
	resized();
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

// return the mouse cursor we should show
HCURSOR	oBaseVisComponent::getCursor(qpoint pAt, qword2 pHitTest) {
	return WND_CURS_DEFAULT;
};

// mouse left button pressed down
void	oBaseVisComponent::evMouseLDown(qpoint pDownAt) {
	// stub
};

// mouse left button released
void	oBaseVisComponent::evMouseLUp(qpoint pDownAt) {
	// stub
};

// mouse moved to this location while we are not dragging
void	oBaseVisComponent::evMouseMoved(qpoint pAt) {
	// stub	
};

// mouse click at this location
void	oBaseVisComponent::evClick(qpoint pAt, EXTCompInfo* pECI) {
	// stub
};	

void	oBaseVisComponent::wm_lbutton(qpoint pAt, bool pDown, EXTCompInfo* pECI) {
	mMouseAt = pAt; /* store a copy of our mouse location */
	
	if (pDown) {
		// addToTraceLog("Mouse down");
		
		mMouseLButtonDown = true;
		mMouseDragging = false;
		mMouseDownAt = pAt;
		
		this->evMouseLDown(pAt);
	} else {
		mMouseLButtonDown = false;
		if (!mMouseDragging) {
			// addToTraceLog("Click");

			this->evClick(pAt, pECI);
		};
		this->evMouseLUp(pAt);
	};	
};

void	oBaseVisComponent::wm_mousemove(qpoint pMovedTo, EXTCompInfo* pECI) {
	mMouseAt = pMovedTo; /* store a copy of our mouse location */
	
	if (mMouseDragging) {
		// for now we ignore this...
	} else {
		this->evMouseMoved(mMouseAt);		
	};
};

////////////////////////////////////////////////////////////////////////////////////////
// keyboard
////////////////////////////////////////////////////////////////////////////////////////

// let us know a key was pressed. Return true if Omnis should not do anything with this keypress
bool	oBaseVisComponent::evKeyPressed(qkey *pKey, bool pDown, EXTCompInfo* pECI) {
	// stub
	return false;
};

////////////////////////////////////////////////////////////////////////////////////////
// drag and drop
////////////////////////////////////////////////////////////////////////////////////////

// Can we drag from this location? Return false if we can't
bool	oBaseVisComponent::canDrag(qpoint pFrom) {	// stub
	return true;
};

// started dragged, return -1 if we leave it up to Omnis to handle this
qlong	oBaseVisComponent::evStartDrag(FLDdragDrop * pDragInfo) {
	// stub

	return false;
};

// mouse dragged from - to, return -1 if we leave it up to Omnis to handle this
qlong	oBaseVisComponent::evEndDrag(FLDdragDrop * pDragInfo) {
	// stub	
	
	return -1;
};

// Set drag value, update the pDragInfo structure with information about what we are dragging, return -1 if we leave it up to Omnis to handle this
qlong	oBaseVisComponent::evSetDragValue(FLDdragDrop *pDragInfo, EXTCompInfo* pECI) {
	// stub
	
	return -1;
};

// drag and drop handling, return -1 if we're not handling this and want default omnis logic to run
qlong	oBaseVisComponent::wm_dragdrop(WPARAM wParam, LPARAM lParam, EXTCompInfo* pECI) {
	switch (wParam) {
		// DD_CANDRAG_ON_DOWN - Enquiry on whether dragging can be started by a mouse button down action. 
		// Return true or false, or simply ignore the message. LParam will contain a pointer to a qpoint structure which will contain the mouse position. 
		// The point is local to the client area of the window which receives these messages.
		case DD_CANDRAG_ON_DOWN: {
			// need to move before we start dragging, we may implement this differently some day if we make a control that requires to drag right away...
			
/*			qpoint pt = *((qpoint *)lParam);
			if (this->canDrag(pt)) {
				return qtrue;
			} else {
				return qfalse;
			}; */
			
			return qfalse;
		}; break;
			
		// DD_CANDRAG_ON_MOVE -	Enquiry on whether dragging can be started by a mouse move action. 
		// Return true or false, or simply ignore the message. LParam will contain a pointer to a qpoint structure which will contain the mouse position. 
		// The point is local to the client area of the window which receives these messages.
		case DD_CANDRAG_ON_MOVE: {
			qpoint pt = *((qpoint *)lParam);
			if (this->canDrag(pt)) {
				return qtrue;
			} else {
				return qfalse;
			};
		}; break;
			
		// DD_STARTDRAG - Indicates that the drag process is starting. Normally this message is ignored. 
		// LParam will contain a pointer to the FLDdragDrop structure.
		case DD_STARTDRAG: {
			FLDdragDrop *	dragInfo = (FLDdragDrop *)lParam;
			return this->evStartDrag(dragInfo);
		}; break;
			
		// DD_ENDDRAG - Indicates that the drag process is finishing. Normally this message is ignored. 
		// LParam will contain a pointer to the FLDdragDrop structure.
		case DD_ENDDRAG: {
			FLDdragDrop *	dragInfo = (FLDdragDrop *)lParam;
			return this->evEndDrag(dragInfo);			
		};
	
		// DD_SETDRAGVALUE
		// Request for control to set the drag value and can be used, for example, 
		// to set the drag value to a selection of text. LParam will contain a pointer to the FLDdragDrop structure.	
		case DD_SETDRAGVALUE: {
			FLDdragDrop *	dragInfo = (FLDdragDrop *)lParam;
			return this->evSetDragValue(dragInfo, pECI);
		}; break;

		// we'll implement more soon, here is the info from the SDK for the messages we receive here:
			
		// DD_CHILD_STARTDRAG - Indicates that the drag process is starting. Sent to the parent of the dragging window. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_CHILD_ENDDRAG - Indicates that the drag process is finishing. Sent to the parent of the dragging window. LParam will contain a pointer to the FLDdragDrop structure.	
		// DD_CANDROP - Sent to the drop control and it can return qtrue if drop action is allowed. LParam will contain a pointer to the FLDdragDrop structure and member mDropPoint may be used to establish drop position.
		// DD_CANDROP_OVER - Sent to the drop control and it can return qtrue if dropping is allowed. LParam will contain a pointer to the FLDdragDrop structure and member mDropPoint may be used to establish mouse position.
		// DD_CANDROPPARENT - Sent to the parent of the drop control and it can return qtrue if dropping is allowed. LParam will contain a pointer to the FLDdragDrop structure and member mDropPoint may be used to establish mouse position.
		// DD_HILITE - Request to the current dropping control to hilite its acceptance to allow dropping. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_UNHILITE - Request to the current dropping control to unhilite itself. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_ALWAYS_HILITE - Request to the current dropping control to establish whether highlighting is required. Return qtrue or qfalse. LParam will contain a pointer to the FLDdragDrop structure
		// DD_SHOWDRAGSHAPE - Message to show the drag shape. Normally this is ignored. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_HIDEDRAGSHAPE - Message to hide the drag shape. Normally this is ignored. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_MOVEDRAGSHAPE - Message to move the drag shape. Normally this is ignored. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_CANSCROLL - Request to the current dropping control to establish whether scroll is required. Return qtrue or qfalse. If qtrue is returned then DD_DRAGDROPSCROLL will be sent. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_GETSCROLLRECT - Request to the current dropping control for it to adjust the scrolling rectangle, if required. Return qtrue if processed. lParam will contain a pointer to the qrect which can be adjusted.
		// DD_DRAGDROPSCROLL - Request to the current dropping control for it to scroll, if required. Return qtrue if processed. lParam will contain a pointer to the qpoint which can be used to ensure that the point is inside the control.
		// DD_GETDRAGCONTAINER - Request for control to set the drag source HWND (FLDdragDrop member mDragSourceHwnd). Normally this is ignored but can be useful for complex controls that allow dragging of multiple HWNDs. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_BUTTONDOWN - Message that the button is down during drag move. Normally this is ignored but it can be used to change drop tabs on a tabbed pane control, for example. LParam will contain a pointer to the FLDdragDrop structure.
		// DD_BUTTONUP - Message that the button is up during drag move. Normally this is ignored but it can be used to change drop tabs on a tabbed pane control, for example. LParam will contain a pointer to the FLDdragDrop structure.
		default:
			return -1;
			break;
	};
};

