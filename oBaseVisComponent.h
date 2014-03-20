/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseVisComponent.h
 *  Base class for our visual component
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseComponent.h"

#ifndef obaseviscomponenth
#define obaseviscomponenth

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
private:
	/*** only valid during mouse actions ***/	
	bool						mMouseLButtonDown;													// Did we press the mouse button down while within our control? 
	qpoint						mMouseDownAt;														// Location we pressed the mouse down at
	bool						mMouseDragging;														// Are we dragging?
	
	/*** only valid during drawing ***/
	GDItextSpecStruct			mTextSpec;															// Info on how to draw text
	qRectArray					mClipStack;															// Our clip stack
	
	void						setup(EXTCompInfo* pECI);											// setup our colors and fonts etc.
	
protected:
	HWND						mHWnd;																// Our main window handle (not applicable for NV objects)
	qpoint						mMouseAt;															// Last known location of the mouse as it hoovered over our control
	EXTfldval					mPrimaryData;														// Copy of our primary data if default implementation is used
	
	qcol						mTextColor;															// Our text color
	qpat						mBackpattern;														// Our back pattern
	HBRUSH						mBackpatBrush;														// backpattern brush
	qcol						mForecolor, mBackcolor;												// Our forecolor and backcolor
	qulong						mBKTheme;															// Our background theme

	/*** scrollbar related ***/
	qdim						mOffsetX;															// Horizontal offset (scrollbar), this is not automatically taken into account when drawing (yet)!
	qdim						mOffsetY;															// Vertical offset (scrollbar)

	/*** only valid during drawing ***/
	qbool						mDrawBuffer;														// If true (default) we'll setup our canvas buffer
	qrect						mClientRect;														// Our client rect, gives the size of our visual component
	HDC							mHDC;																// Current HDC for drawing
		
	// colour functions
	qcol						mixColors(qcol pQ1, qcol pQ2);										// Mix two colors together
	
	// clipping functions
	void						clipRect(qrect pRect, bool pUnion = true);							// Clip drawing to a rectangle on screen, optionaly union with current clipping. Adds it to our clip stack
	void						unClip();															// Pop our last clipping rectangle off the stack
	
	// drawing functions (in drawingfunctions.cpp)
	qdim						getTextWidth(const qchar *pText, qshort pLen, bool pStyled = true);	// Get the width of text
	qdim						getTextHeight(const qchar *pText, qdim pWidth, bool pStyled = true, bool pWrap = true);	// Get the heigth of text if wrapped
	qdim						drawText(const qchar *pText, qrect pWhere, qcol pColor, qjst pJst = jstLeft, bool pStyled = true, bool pWrap = true);			// Draws the text clipped within the specified rectangle 
	void						drawLine(qpoint pFrom, qpoint pTo);		// Draws a line between two points using the current selected pen
	void						drawLine(qpoint pFrom, qpoint pTo, qdim pWidth, qcol pCol, qpat pPat);		// Draws a line between two points
	void						drawEllipse(qrect pRect, qcol pTop, qcol pBottom, qcol pBorder = -1, qint pSpacing = 0);					// Draws a filled ellipse within the rectangle with a gradient color from top to bottom
	
public:
	oBaseVisComponent(void);																		// constructor
	virtual	qbool				init(qapp pApp, HWND pHWnd);										// Initialize component
	
	static  qProperties *		properties(void);													// return array of property meta data
	virtual qbool				setProperty(qlong pPropID,EXTfldval &pNewValue,EXTCompInfo* eci);	// set the value of a property
	virtual void				getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* eci);	// get the value of a property

	// $dataname
	virtual qbool				setPrimaryData(EXTfldval &pNewValue);								// Changes our primary data
	virtual void				getPrimaryData(EXTfldval &pGetValue);								// Retrieves our primary data
	virtual qbool				cmpPrimaryData(EXTfldval &pWithValue);								// Compare with our primary data
	virtual qlong				getPrimaryDataLen();												// Get our primary data size
	virtual void				primaryDataHasChanged();											// Omnis is just letting us know our primary data has changed, this is especially handy if we do not keep a copy ourselves and thus ignore the other functions
	
	static  qMethods *			methods(void);														// return array of method meta data
	static	qEvents *			events(void);														// return an array of events meta data
	
	virtual void				doPaint(EXTCompInfo* pECI);											// Do our drawing in here
	virtual void				Resized();															// Our component was resized
	
	virtual void				evMouseMoved(qpoint pMovedTo);										// mouse moved to this location while mouse button is not down
	virtual void				evClick(qpoint pAt);												// mouse click at this location
	virtual void				evStartDrag(qpoint pFrom);											// mouse started dragged
	virtual void				evDragging(qpoint pFrom, qpoint pAt);								// mouse being dragged
	virtual void				evEndDrag(qpoint pFrom, qpoint pTop);								// mouse dragged from - to	
	virtual void				evCancelledDrag();													// cancelled dragging

	// scrollbar functions
	virtual qdim				getHorzStepSize(void);												// get our horizontal step size
	virtual qdim				getVertStepSize(void);												// get our vertical step size
	virtual void				evWindowScrolled(qdim pNewX, qdim pNewY);							// window was scrolled
	
	// called from our WndProc, don't override these directly
	void						wm_lbutton(qpoint pAt, bool pDown);									// left mouse button
	void						wm_mousemove(qpoint pAt);											// mouse is being moved
	void						wm_paint(EXTCompInfo* pECI);										// Paint message
	void						wm_windowposchanged(EXTCompInfo* pECI, WNDwindowPosStruct * pPos);	// Component resize/repos message
};

#endif

