/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oExample.cpp
 *  Base classes to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Bastiaan Olij
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

// Initialize component
qbool oBaseComponent::init(HWND pHWnd) {
	mHWnd = pHWnd;			// We'll move this into oBaseVisComponent shortly..
	
	return true;
};

/*** some nice support function ***/

// Add formatted string to trace log
void oBaseComponent::addToTraceLog(const char *pData, ...) {
	char		lvBuffer[2048]; // hopefully 2048 is large enough to parse our buffer, note that our trace log doesn't support strings longer then 255 characters...
	va_list		lvArgList;
	
	va_start( lvArgList, pData );
	vsprintf( lvBuffer, pData, lvArgList );
	va_end( lvArgList );
	
	// need to find out if a conversion between char and qchar is needed here in unicode..
	str255 lvAddText((qchar *)lvBuffer);
	
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
qbool oBaseComponent::canAssign(qint pPropID) {
	return true; // assume assignable
};

// set the value of a property
qbool oBaseComponent::setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI) {
	return false;
};

// get the value of a property
void oBaseComponent::getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI) {
	// nothing to do here, base class does not have any properties...
};

/*** Methods ***/

// Static function, return array of property meta data
// Each subclass of our component must implement this and add their properties into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qMethods * oBaseComponent::methods(void) {
	qMethods * lvMethods = new qMethods();
	
	// our base class has not methods

	return lvMethods;
};

// invoke a method
int oBaseComponent::invokeMethod(qint pMethodId,EXTCompInfo* eci) {
	// nothing to do here by default
	
	return 1L;
};

/*** Parameters ***/

// get string from parameter, call needs to delete returned object
qstring	* oBaseComponent::newStringFromParam(int pParamNo, EXTCompInfo* pECI) {
	EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
	EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
	qstring	*			tmpNewString = new qstring(tmpFldVal);

	return tmpNewString;
};

// get long from parameter
long oBaseComponent::getLongFromParam(int pParamNo, EXTCompInfo* pECI) {
	EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
	EXTfldval			tmpFldVal((qfldval) tmpParam->mData);

	return tmpFldVal.getLong();
};

// get qlist from parameter, caller needs to delete return object
EXTqlist *	oBaseComponent::newQListFromParam(int pParamNo, EXTCompInfo* pECI) {
	EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
	EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
	EXTqlist *			tmpList = tmpFldVal.getList(qfalse);
	
	return tmpList;
};


/********************************************************************************************************************************************
 oBaseNVComponent
 ********************************************************************************************************************************************/

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
			mBackpattern = pNewValue.getLong();
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
qbool oBaseVisComponent::wm_paint(EXTCompInfo* pECI) {
	WNDpaintStruct	lvPaintStruct;
	qrect			lvUpdateRect;
	HDC				lvHDC;
	void *			lvOffScreenPaint;
	
	// create our paint structure
	WNDbeginPaint( mHWnd, &lvPaintStruct );
	WNDgetClientRect(mHWnd, &mClientRect);
	
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
	return qtrue;
	
}

