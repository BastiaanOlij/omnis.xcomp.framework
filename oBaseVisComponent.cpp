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
// Construct / destruct / init
////////////////////////////////////////////////////////////////

oBaseVisComponent::oBaseVisComponent(void) {
	mObjType			= cObjType_Basic;
    mShowName           = true;
	mForecolor			= GDI_COLOR_QDEFAULT;
	mBackcolor			= GDI_COLOR_QDEFAULT;
	mHorzScrollPos		= 0;
	mVertScrollPos		= 0;
	mBackpattern		= 0;
	mBKTheme			= WND_BK_NONE;
	mDrawBuffer			= true;
	mMouseLButtonDown	= false;
	mMouseDragging		= false;
	mCanvas				= NULL;
};

// Initialize component
qbool oBaseVisComponent::init(qapp pApp, HWND pHWnd) {
	oBaseComponent::init(pApp);
	
	mHWnd = pHWnd;
	
	WNDsetScrollRange(mHWnd, SB_HORZ, 0, 0, 1, qfalse);
	WNDsetScrollRange(mHWnd, SB_VERT, 0, 0, 1, qfalse);
	
	return true;
};

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

// Get list variable used for $dataname
EXTqlist *	oBaseVisComponent::getDataList(EXTCompInfo* pECI) {
	EXTfldval	dataNameFld;
	str255		dataName;
	ffttype		datatype;
	qshort		datasubtype;
	
	/* get the value of $dataname, i.e. "ivList" */
	ECOgetProperty(mHWnd, anumFieldname, dataNameFld);
	dataName = dataNameFld.getChar();
	
	/* and now get the field related to this.. */
	EXTfldval	dataField(dataName, qfalse, pECI->mLocLocp);
	
	/* check the type of our variable */
	dataField.getType(datatype, &datasubtype);
	if (datatype==fftItemref) {
		/* this is an item reference, lets parse the item reference, thanks to TL tech support */
		EXTfldval	calc, result;
		
		/* Add .$fullname to our reference */
		dataName.concat(str255(QTEXT(".$fullname")));
		
		/* execute this as a calculation to get the name of the variable our reference points at */
		calc.setCalculation(pECI->mInstLocp, ctyCalculation, &dataName[1], dataName[0]);
		calc.evalCalculation(result, pECI->mInstLocp);
		dataName = result.getChar(); /* this will return something like $root.$iwindows.myWindow.$objs.mySubWindow.$ivars.ivList */
		
		/* now our variable could be an instance variable for a subwindow. Our $fullname reference actually doesn't work then.. So lets check for this situation.. */
		qshort ivarspos = dataName.pos(str255(QTEXT(".$ivars.")));
		qshort objspos = dataName.pos(str255(QTEXT(".$objs.")));
		if ((ivarspos!=0) && (objspos!=0)) {
			/*
			 if we're dealing with an instance variable and we have $objs in our dataname this must be a subwindow, we need to add $subinst..
			 
			 so it becomes $root.$iwindows.myWindow.$objs.mySubWindow.$subinst().$ivars.ivList
			 */
			
			dataName.insert(str255(QTEXT(".$subinst()")), ivarspos);
		};
		
		/* and now get our real data variable */
		EXTfldval	referencedField(dataName, qfalse, pECI->mLocLocp);
		referencedField.getType(datatype, &datasubtype);
		if ((datatype==fftList) || (datatype==fftRow)) {
			/* list or row? return the list */
			return referencedField.getList(qfalse);
		} else {
			addToTraceLog("Item reference isn't a list or row");
			return NULL;
		};		
	} else if ((datatype==fftList) || (datatype==fftRow)) {
		/* list or row? return the list */
		return dataField.getList(qfalse);
	} else {
		addToTraceLog("Dataname isn't a list or row");
		return NULL;
	};
};

// check if our field is enabled
bool    oBaseVisComponent::isEnabled() {
    // !BAS! this checks just our own setting, it doesn't check if any container is inactive. Some day we need to improve this...
	EXTfldval	enabledFld;

	ECOgetProperty(mHWnd, anumEnabled, enabledFld);
	return enabledFld.getBool() == 2;
};

// check if our field is active
bool    oBaseVisComponent::isActive() {
    // !BAS! this checks just our own setting, it doesn't check if any container is inactive. Some day we need to improve this...
	EXTfldval	activeFld;

	ECOgetProperty(mHWnd, anumActive, activeFld);
	return activeFld.getBool() == 2;
};

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

// Do our drawing in here
void oBaseVisComponent::doPaint(EXTCompInfo* pECI) {
	// override to implement drawing...
	if (!WNDdrawThemeBackground(mHWnd,mCanvas->hdc(),&mClientRect,mBKTheme==WND_BK_CONTROL ? WND_BK_PARENT : mBKTheme)) { // if control theme we'll actually draw our parent, we're expecting to draw something on top
		// clear our drawing field
		mCanvas->drawRect(mClientRect, mForecolor);
	};
};

// Do our list content drawing here (what we see when the list is collapsed, for cObjType_DropList only)
bool	oBaseVisComponent::drawListContents(EXTListLineInfo *pInfo, EXTCompInfo* pECI) {
	return false;
};

// Do our list line drawing here (for cObjType_List or cObjType_DropList)
bool	oBaseVisComponent::drawListLine(EXTListLineInfo *pInfo, EXTCompInfo* pECI) {
	return false;
};


// Create text spec structure for our standard properties (used by setup or when drawing list lines)
GDItextSpecStruct oBaseVisComponent::getStdTextSpec(EXTCompInfo* pECI) {
	qshort			fontSize;
	qsty			fontStyle;
	qjst			fontAlign;
	EXTfldval		fval, font; 
	str255			fieldStyle;	

	GDItextSpecStruct	textSpec;
	
	// get all our text properties...
	ECOgetProperty(mHWnd,anumFldStyle,fval); 
	fval.getChar(fieldStyle);
	
	ECOgetProperty(mHWnd,anumFont,font); 
	// use this as is...
	
	ECOgetProperty(mHWnd,anumFontsize,fval); 
	fontSize = (qshort) fval.getLong();
	
	ECOgetProperty(mHWnd,anumFontstyle,fval); 
	fontStyle = (qsty) fval.getLong();
	
	ECOgetProperty(mHWnd,anumAlign,fval); 
	fontAlign = (qjst) fval.getLong();
	
	// Start with loading from our normal settings (defaults for now)
	ECOgetFont(mHWnd, &(textSpec.mFnt), ECOgetFontIndex(mHWnd, font), fontSize);
	textSpec.mSty = fontStyle;
	textSpec.mJst = fontAlign;
	textSpec.mTextColor = mTextColor;
	
	// Now see if we need to override any of these defaults with our fieldstyle...
	if (fieldStyle[0]>0) {
		ECOgetStyle( ECOgetApp(pECI->mInstLocp), &fieldStyle[1], fieldStyle[0], &textSpec );
		
		// !BAS! need to also see if we need to get our foreground color, background color and/or background pattern from our style
		// not sure if Omnis feeds this info back to us already
	};	
	
	return textSpec;
};

// setup our fonts and brushes
void oBaseVisComponent::setup(EXTCompInfo* pECI) {
	EXTfldval		fval; 
	
	// Omnis is maintaining these so grab our copies
	if (mObjType != cObjType_Basic) {
		ECOgetProperty(mHWnd,anumForecolor,fval); 
		mForecolor = fval.getLong();
		ECOgetProperty(mHWnd,anumBackcolor,fval); 
		mBackcolor = fval.getLong();
		ECOgetProperty(mHWnd,anumBackpattern,fval); 
		mBackpattern = fval.getLong();
		ECOgetProperty(mHWnd,anumBackgroundTheme,fval); 
		mBKTheme = fval.getLong();
	};

	// always get these omnis ones...
	ECOgetProperty(mHWnd,anumTextColor,fval); 
	mTextColor = fval.getLong();	
	
	// set background color
	mCanvas->setBkColor(mBackcolor);
	
	// Create our pattern brush, it will be deleted at the end of drawing..
	mCanvas->setBackpatBrush(mBackpattern);
	
	// and our standard text spec
	mCanvas->setTextSpec(getStdTextSpec(pECI));
};

// erase our background message
bool	oBaseVisComponent::wm_erasebkgnd(EXTCompInfo* pECI) {
	if ((mObjType==cObjType_List) || (mObjType==cObjType_DropList)) {
		// We're going to be handling this through ECM_PAINTCONTENTS
		return false;
	} else {
		/* 
		 The SDK specifies we need to draw the background here and then draw whatever foreground stuff we have in wm_paint.

		 This is an old optimalisation from the early Windows days where drawing stuff was CPU expensive but it also leads
		 to alot of flickering of the screen.

		 In our wm_paint we now use a buffer to draw too which is then blitted to the screen. In drawing that buffer we also
		 draw the background.

		 So we're going to cheat here, and ignore the erase background.. ;)
		*/
		
		return true;		
	};
	
};

// paint message
bool oBaseVisComponent::wm_paint(EXTCompInfo* pECI) {
	if ((mObjType==cObjType_List) || (mObjType==cObjType_DropList)) {
		// We're going to be handling this through ECM_PAINTCONTENTS
		return false;
	} else {
		oDrawingCanvas *	lvWasCanvas = mCanvas; // this should be NULL but not taking any chances...
		HDC					lvHDC;
		WNDpaintStruct		lvPaintStruct;
		qrect				lvUpdateRect;
		void *				lvOffScreenPaint;
			
		// get current size info
		WNDgetClientRect(mHWnd, &mClientRect);
		
		if (mDrawBuffer) {
			// create our paint structure
			WNDbeginPaint( mHWnd, &lvPaintStruct );
			
			lvUpdateRect = lvPaintStruct.rcPaint;
			lvHDC = lvPaintStruct.hdc;
						
			// On windows this will do a double buffer trick, on Mac OSX the OS already does the offscreen painting:)
			// note that mHDC and mClienRect may be altered as a result of this call which is good!
			lvOffScreenPaint = GDIoffscreenPaintBegin(NULL, lvHDC, mClientRect, lvUpdateRect);
			if (lvOffScreenPaint) {
				mCanvas = new oDrawingCanvas(mApp, lvHDC, mClientRect);
				if (mCanvas!=NULL) {
					setup(pECI);
					
					// do our real drawing
					doPaint(pECI);
					
					// If in design mode, then call drawDesignName, drawNumber & drawMultiKnobs to draw design
					// name, numbers and multiknobs, if required.
					if ( ECOisDesign(mHWnd) ) {
						if (mShowName) ECOdrawDesignName(mHWnd,lvHDC);
						ECOdrawNumber(mHWnd,lvHDC);
						ECOdrawMultiKnobs(mHWnd,lvHDC);
					}
					
					// delete our canvas, we no longer need it.
					delete mCanvas;
					
					GDIoffscreenPaintEnd(lvOffScreenPaint);
				};	
			};
			
			// And finish paint...
			WNDendPaint( mHWnd, &lvPaintStruct );	
		} else {		
			// component must fully implement and is responsible for setting up our context...
			mCanvas = NULL;
			doPaint(pECI);
		}
		
		return true;
	};
};

// Draw cObjType_DropList content
bool	oBaseVisComponent::ecm_paintcontents(EXTListLineInfo *pInfo, EXTCompInfo* pECI) {
	oDrawingCanvas *	lvWasCanvas = mCanvas;	// this should be NULL but just in case
	bool				retval = false;
	
	// setup our drawing info
	mCanvas = new oDrawingCanvas(mApp, pInfo->mHdc, pInfo->mLineRect);
	if (mCanvas != NULL) {
		setup(pECI);
		
		retval = this->drawListContents(pInfo, pECI);
		
		delete mCanvas;
	};
	
	mCanvas = lvWasCanvas;
	
	return retval;
};

// Draw line for cObjType_List or cObjTypeDropList
bool	oBaseVisComponent::ecm_listdrawline(EXTListLineInfo *pInfo, EXTCompInfo* pECI) {
	oDrawingCanvas *	lvWasCanvas = mCanvas;	// this should be NULL but just in case
	bool				retval = false;
	
	// setup our drawing info
	mCanvas = new oDrawingCanvas(mApp, pInfo->mHdc, pInfo->mLineRect);
	if (mCanvas != NULL) {
		setup(pECI);

		// draw our background..
		HBRUSH brush = GDIcreateBrush( patFill );
		GDIsetTextColor(pInfo->mHdc, mForecolor);
		GDIfillRect(pInfo->mHdc,&pInfo->mLineRect,brush);
		GDIdeleteObject(brush);

		// start hiliting
		if (pInfo->mSelected) {
			GDIhiliteTextStart(pInfo->mHdc, &pInfo->mLineRect, mTextColor);

	        GDItextSpecStruct   textSpec = mCanvas->textSpec();
			textSpec.mTextColor = GDIgetTextColor(pInfo->mHdc);
			mCanvas->setTextSpec(textSpec);
		};

		retval = this->drawListLine(pInfo, pECI);	
	
		if (pInfo->mSelected) {
			GDIhiliteTextEnd( pInfo->mHdc, &pInfo->mLineRect, mTextColor);
		};

		// Draw focus rectangle
		if (pInfo->mDrawFocusRect) {
			pInfo->mLineRect.right++;
			GDIdrawFocusRect(pInfo->mHdc, &pInfo->mLineRect);
		};

		delete mCanvas;
	};
	
	mCanvas = lvWasCanvas;
	
	return retval;
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
	if ((mHorzScrollPos!=pNewX) || (mVertScrollPos!=pNewY)) {
		WNDsetScrollPos(mHWnd, SB_HORZ, pNewX, qfalse); 
		WNDsetScrollPos(mHWnd, SB_VERT, pNewY, qfalse);
		
		// we may not need to do this..
		WNDscrollWindow(mHWnd, mHorzScrollPos - pNewX, mVertScrollPos-pNewY);

		// redraw the whole thing...
		WNDinvalidateRect(mHWnd, NULL);

		if (mMouseLButtonDown) {
			mMouseDownAt.h += pNewX-mHorzScrollPos;
			mMouseDownAt.v += pNewY-mVertScrollPos;
		};	

		mHorzScrollPos = pNewX;
		mVertScrollPos = pNewY;
	};
};


////////////////////////////////////////////////////////////////////////////////////////
// mouse related functions
////////////////////////////////////////////////////////////////////////////////////////

// return the mouse cursor we should show
HCURSOR	oBaseVisComponent::getCursor(qpoint pAt, qword2 pHitTest) {
	// We return WND_CURS_DEFAULT which will cause our framework to let Omnis override this with any user setting or else use the default cursor.
	// If you override this and return anything but WND_CURS_DEFAULT what you return will be used.
	return WND_CURS_DEFAULT;
};

// returns true if the mouse is over our object
bool oBaseVisComponent::mouseIsOver() {
    return mMouseOver;
};

// mouse moved over our object
void oBaseVisComponent::evMouseEnter() {
    // stub
};

// mouse moved away from our object
void oBaseVisComponent::evMouseLeave() {
    // stub
};

// mouse left button pressed down (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::evMouseLDown(qpoint pDownAt) {
	// stub
	
	return true;
};

// mouse left button released (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::evMouseLUp(qpoint pDownAt) {
	// stub

	return true;
};

// mouse left button double clicked (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::evDoubleClick(qpoint pAt, EXTCompInfo* pECI) {
	// stub
	return false;
};

// mouse right button pressed down (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::evMouseRDown(qpoint pDownAt, EXTCompInfo* pECI) {
	// stub
	return false;	
};

// mouse right button released (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::evMouseRUp(qpoint pUpAt, EXTCompInfo* pECI) {
	// stub
	return false;
};

// mouse moved to this location while we are not dragging
void	oBaseVisComponent::evMouseMoved(qpoint pAt) {
	// stub	
};

// mouse click at this location
void	oBaseVisComponent::evClick(qpoint pAt, EXTCompInfo* pECI) {
	// stub
};	

// left mouse up/down (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::wm_lbutton(qpoint pAt, bool pDown, EXTCompInfo* pECI) {
	mMouseAt = pAt; /* store a copy of our mouse location */
	
	if (pDown) {
		// addToTraceLog("Mouse down");
		
		mMouseLButtonDown = true;
		mMouseDragging = false;
		mMouseDownAt = pAt;
		
		return this->evMouseLDown(pAt);
	} else if (mMouseLButtonDown) {
		mMouseLButtonDown = false;
		if (!mMouseDragging) {
			// addToTraceLog("Click");

			this->evClick(pAt, pECI);
		};
		return this->evMouseLUp(pAt);
	} else {
        // mouse didn't go down on this so...
        return false;
    };
};

// left mouse button double click (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::wm_lbDblClick(qpoint pAt, EXTCompInfo* pECI) {
	return this->evDoubleClick(pAt, pECI);
};


// right mouse button (return true if we finished handling this, false if we want Omnis internal logic)
bool	oBaseVisComponent::wm_rbutton(qpoint pAt, bool pDown, EXTCompInfo* pECI) {
	if (pDown) {
		return this->evMouseRDown(pAt, pECI);
	} else {
		return this->evMouseRUp(pAt, pECI);
	};
};

void	oBaseVisComponent::wm_mousemove(qpoint pMovedTo, EXTCompInfo* pECI, bool IsOver) {
	mMouseAt = pMovedTo; /* store a copy of our mouse location */
	
    if (IsOver) {
        if (!mMouseOver) {
            mMouseOver = true;
            
            this->evMouseEnter();
        };
    
        if (mMouseDragging) {
            // for now we ignore this...
        } else {
            this->evMouseMoved(mMouseAt);
        };
    } else if (mMouseOver) {
        mMouseOver = false;
        this->evMouseLeave();
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

