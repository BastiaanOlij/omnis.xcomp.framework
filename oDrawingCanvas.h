/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oDrawingCanvas.h
 *
 *  Drawing canvas class
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "xCompStandardIncludes.h"
#include "qString.h"
#include "oBaseComponent.h"

#ifndef odrawingcanvash
#define odrawingcanvash

// pixel structure as its used in a 32bit HPIXMAP
// tested on Mac, may need to change on windows..
typedef struct sPixel {
	qbyte	mA;
	qbyte	mR;
	qbyte	mG;
	qbyte	mB;
} sPixel;

struct qchar50 {
	qchar mVal[50];
	
	qchar50() {
		memset(mVal, 0, 50*sizeof(qchar));
	}
	
	qchar50(const qchar50& pCopy) {
		memcpy(mVal, pCopy.mVal, 50*sizeof(qchar));
	}
	
	qchar50(const qchar * pStr) {
		memset(mVal, 0, 50*sizeof(qchar));
		OMstrcpy(mVal, pStr);
	}
	
	// required for 'map', 'set', etc
	bool operator<(const qchar50& pCompare) const {
		return OMstrcmp(mVal, pCompare.mVal) < 0;
	}
};

typedef std::map<qchar50, qdim> qTextWidthMap;	// text width map for strings up to 50 characters

class oDrawingCanvas {
private:
	qapp						mApp;										// our application

	HDC							mHDC;										// HDC for drawing
	bool						mOwnHDC;									// We own this HDC
	qrect						mBaseRect;									// Rectangle defining our drawing area
	
	GDItextSpecStruct			mTextSpec;									// Info on how to draw text
	std::vector<qrect>			mClipStack;									// Our clip stack
	
	HFONT						mTextFont, mOldFont;						// Our font and old font
	HBRUSH						mBackpatBrush;								// backpattern brush
	qdim						mFontHeight;								// height of our current selected font
	qTextWidthMap				mTextCache;									// cache with widths of text

	void						init(qapp pApp);							// Initialize our instance
	
	// font function helpers
	void						clearTextCache();							// Free memory associated with our text cache
	qdim						getWordWidth(qstring & pWord);				// Get the width of a word (from cache)
	qdim						addWord(qstring & pTo, qstring & pWord, qstring &pPrefix, qdim pCurrWidth, qdim pMaxWidth);	// add a word to a string
	
	// passthrough drawing functions
	void						drawTextJst(GDIdrawTextStruct * pTextInfo, qrect pRect, bool pAdjJst = false);	// passthrought to GDIdrawTextJst with clipping	
	
protected:
	
public:
	oDrawingCanvas(qapp pApp, HDC pHdc, qrect pRect);						// Setup a drawing canvas based on an existing HDC
	oDrawingCanvas(qapp pApp, qdim pWidth, qdim pHeight);					// Create a bitmap based canvas
	~oDrawingCanvas();														// Destruct our canvas

	// properties
	HDC							hdc();										// access to our HDC if we really need to.
	GDItextSpecStruct			textSpec();									// get our text spec
	void						setTextSpec(GDItextSpecStruct pSpec);		// set our text spec, also sets the text color to what is specified in our text spec

	// colour functions
	qcol						mixColors(qcol pQ1, qcol pQ2);				// Mix two colors together
	void						setBackpatBrush(qpat pPat);					// setup our backpattern brush
	void						setBkColor(qcol pColor);					// set our background color
	
	// clipping functions
	bool						clipRect(qrect pRect, bool pUnion = true);	// Clip to given rectangle and put on stack, will optionally union with the current clipping. Will return false if we can't draw in the resulting rectangle and we could thus not clip.
	void						unClip();									// Pop our last clipping rectangle off the stack, do not call if clipRect returned false!
	
	// font functions
	qstring						wrapText(const qchar *pText, qdim pMaxWidth);	// wrap text, inserts newlines where needed to wrap the text
	qdim						getFontHeight();							// get the height of the current selected font
	qdim						getTextWidth(const qchar *pText, qshort pLen, bool pStyled = true);	// Get the width of text
	qdim						getTextHeight(const qchar *pText, qdim pWidth, bool pStyled = true, bool pWrap = true);	// Get the heigth of text if wrapped

	// drawing functions
	qdim						drawText(const qchar *pText, qrect pWhere, qcol pColor, qjst pJst = jstLeft, bool pStyled = true, bool pWrap = true);	// Draws the text clipped within the specified rectangle 
	void						drawIcon(qlong pIconId, qpoint pAt);																					// Draw a icon at this position
	void						drawLine(qpoint pFrom, qpoint pTo);																						// Draws a line between two points using the current selected pen
	void						drawLine(qpoint pFrom, qpoint pTo, qdim pWidth, qcol pCol, qpat pPat);													// Draws a line between two points
	void						drawRect(qrect pRect, qcol pFillColor);																					// fills a rectangle using our standard pattern brush
	void						drawRect(qrect pRect, qcol pFillColor, HBRUSH pBrush);																	// fills a rectangle with a certain pattern
	void						drawRect(qrect pRect, qcol pFillColor, qcol pBorder);																	// draw a rectangle. Note, if pBackground is set to GDI_COLOR_QDEFAULT we do not fill the rectangle
	void						drawEllipse(qrect pRect, qcol pTop, qcol pBottom, qcol pBorder = -1, qint pSpacing = 0);								// Draws a filled ellipse within the rectangle with a gradient color from top to bottom
	
};

#endif
