/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oExample.cpp
 *  Base classes to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseComponent.h"

/********************************************************************************************************************************************
 oBaseComponent
 ********************************************************************************************************************************************/

// constructor
oBaseComponent::oBaseComponent(void) {
	// nothing to do here just yet
};

// destructor
oBaseComponent::~oBaseComponent(void) {
	// nothing to do here just yet
};

/*** some nice support function ***/

// Add formatted string to trace log
void oBaseComponent::addToTraceLog(const char *pData, ...) {
	char		tmpOutLine[2048];
	uint		tmpLen=0;
	char		lvBuffer[2048]; // hopefully 2048 is large enough to parse our buffer, note that our trace log doesn't support strings longer then 255 characters...
	uint		tmpBufLen;
	va_list		lvArgList;
	
	va_start( lvArgList, pData );
	vsprintf( lvBuffer, pData, lvArgList );
	va_end( lvArgList );
	
	tmpBufLen = strlen(lvBuffer);
	
	// remove trailing newline
	while ((tmpBufLen > 0) && (lvBuffer[tmpBufLen-1]=='\r' || lvBuffer[tmpBufLen-1]=='\n')) {
		tmpBufLen--;
	};
	
	// and output..
	for (uint tmpIndex = 0; tmpIndex<tmpBufLen; tmpIndex++) {
		if (lvBuffer[tmpIndex]=='\r') {
			// ignore...
		} else if ((lvBuffer[tmpIndex]=='\n') || (tmpLen==250)) {
			// output line..
			tmpOutLine[tmpLen]='\0';
			
			// need to convert line to qchar if on unicode!
			str255 lvAddText((qchar *)tmpOutLine);
			ECOaddTraceLine(&lvAddText);

			tmpLen=0;
		} else {
			tmpOutLine[tmpLen]=lvBuffer[tmpIndex];
			tmpLen++;
		};
	};
	tmpOutLine[tmpLen]='\0';
		
	// need to convert line to qchar if on unicode!
	str255 lvAddText((qchar *)tmpOutLine);
	ECOaddTraceLine(&lvAddText);	
};


/*** Properties ***/

// Static function, return array of property meta data
// Each subclass of our component must implement this and add their properties into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qProperties * oBaseComponent::properties(void) {
	qProperties * lvProperties = new qProperties();

	// Our base class has not properties
	
	return lvProperties;
};

// return true/false if a property can be written too
qbool oBaseComponent::canAssign(qlong pPropID) {
	return true; // assume assignable
};

// set the value of a property
qbool oBaseComponent::setProperty(qlong pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI) {
	return false;
};

// get the value of a property
void oBaseComponent::getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI) {
	// nothing to do here, base class does not have any properties...
};

/*** Methods ***/

// Static function, return array of method meta data
// Each subclass of our component must implement this and add their methods into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qMethods * oBaseComponent::methods(void) {
	qMethods * lvMethods = new qMethods();
	
	// our base class has no methods

	return lvMethods;
};

// invoke a method
int oBaseComponent::invokeMethod(qlong pMethodId,EXTCompInfo* eci) {
	// nothing to do here by default
	
	return 1L;
};

/*** Events ***/

// Static function, return array of event meta data
// Each subclass of our component must implement this and add their events into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qEvents *	oBaseComponent::events(void) {
	qEvents * lvEvents = new qEvents();
	
	// our base class has no events
	
	return lvEvents;
};


/*** Parameters ***/

// get string from parameter, call needs to delete returned object
qstring	* oBaseComponent::newStringFromParam(int pParamNo, EXTCompInfo* pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
		qstring	*			tmpNewString = new qstring(tmpFldVal);
		
		return tmpNewString;		
	} else {
		qstring	*			tmpNewString = new qstring("");
		
		return tmpNewString;
	};
};

// get long from parameter
long oBaseComponent::getLongFromParam(int pParamNo, EXTCompInfo* pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
		
		return tmpFldVal.getLong();
	} else {
		return 0;
	}
};

// get qlist from parameter, caller needs to delete return object
EXTqlist *	oBaseComponent::newQListFromParam(int pParamNo, EXTCompInfo* pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
		EXTqlist *			tmpList = tmpFldVal.getList(qfalse);
		
		return tmpList;
	} else {
		return 0;
	};
};

// get binary buffer, returns NULL if the buffer is empty
char * oBaseComponent::newBinfromParam(int pParamNo, size_t *pLen, EXTCompInfo* pECI) {
	char *			tmpBuffer = NULL;
	long			tmpLen = 0;

	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);

		tmpLen = tmpFldVal.getBinLen();
		if (tmpLen>0) {
			tmpBuffer = (char *) malloc(tmpLen);
			if (tmpBuffer!=NULL) {
				long	tmpRealLen;
				memset(tmpBuffer, 0, tmpLen); // JIC
				
				tmpFldVal.getBinary(tmpLen, (qbyte *) tmpBuffer, tmpRealLen);
			} else {
				tmpLen = 0;
			};
		};
	};

	*pLen = tmpLen;
	return tmpBuffer;
};

/********************************************************************************************************************************************
 oBaseNVComponent
 ********************************************************************************************************************************************/

// Initialize component
qbool oBaseNVComponent::init(qobjinst pInst) {
	mObjInst = pInst;
	
	return true;
};

// create a copy of pCopy, this MUST be implemented in a subclass
void oBaseNVComponent::copyObject(oBaseNVComponent *pCopy) {
	// nothing to copy...
};

// Add properties for NV componect
qProperties * oBaseNVComponent::properties(void) {
	qProperties *	lvProperties = oBaseComponent::properties();
	
	// Our non-visual class has not properties, we still need to implement this...
	
	return lvProperties;
};

// Add methods for NV component
qMethods * oBaseNVComponent::methods(void) {
	qMethods * lvMethods = oBaseComponent::methods();
	
	// our non-visual class has not methods, we still need to implement this...
	
	return lvMethods;
};

// Add events for NV component
qEvents *	oBaseNVComponent::events(void) {
	qEvents * lvEvents =oBaseComponent::events();
	
	// our non-visual class has no events
	
	return lvEvents;
};

// object is being send a rebuild message
qint	oBaseNVComponent::ecm_object_rebuild(EXTCompInfo* pECI) {
	return mNeedRebuild ? 1L : 0L;
};

/********************************************************************************************************************************************
 oBaseVisComponent
 ********************************************************************************************************************************************/

ECOproperty oBaseVisProperties[] = { 
	//	ID						ResID	Type			Flags					ExFlags	EnumStart	EnumEnd
	anumForecolor,			0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $forecolor
	anumBackcolor,			0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $backcolor
	anumBackpattern,		0,		fftInteger,		EXTD_FLAG_PROPAPP,		0,		0,			0,		// $backpattern
	
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

	switch (pPropID) {
		case anumForecolor:
			mForecolor = pNewValue.getLong();
			WNDinvalidateRect(mHWnd, NULL);	
			
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

// Do our drawing in here
void oBaseVisComponent::doPaint(HDC pHDC) {
	// override to implement drawing...
	
	// clear our drawing field
	GDIsetTextColor(pHDC, mForecolor);
	GDIfillRect(pHDC, &mClientRect, mBackpatBrush);
	GDIsetTextColor(pHDC, mTextColor);
}

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


