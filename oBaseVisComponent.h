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

	// passthrough drawing functions
	void						drawTextJst(GDIdrawTextStruct * pTextInfo, qrect pClipRect);		// passthrought to GDIdrawTextJst with clipping

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
	bool						clipRect(qrect pRect, bool pUnion = true);							// Clip to given rectangle and put on stack, will optionally union with the current clipping. Will return false if we can't draw in the resulting rectangle and we could thus not clip.
	void						unClip();															// Pop our last clipping rectangle off the stack, do not call if clipRect returned false!
		
	// drawing functions (in drawingfunctions.cpp)
	qdim						getTextWidth(const qchar *pText, qshort pLen, bool pStyled = true);	// Get the width of text
	qdim						getTextHeight(const qchar *pText, qdim pWidth, bool pStyled = true, bool pWrap = true);	// Get the heigth of text if wrapped
	qdim						drawText(const qchar *pText, qrect pWhere, qcol pColor, qjst pJst = jstLeft, bool pStyled = true, bool pWrap = true);			// Draws the text clipped within the specified rectangle 
	void						drawIcon(qlong pIconId, qpoint pAt);								// Draw a icon at this position
	void						drawLine(qpoint pFrom, qpoint pTo);		// Draws a line between two points using the current selected pen
	void						drawLine(qpoint pFrom, qpoint pTo, qdim pWidth, qcol pCol, qpat pPat);		// Draws a line between two points
	void						drawEllipse(qrect pRect, qcol pTop, qcol pBottom, qcol pBorder = -1, qint pSpacing = 0);					// Draws a filled ellipse within the rectangle with a gradient color from top to bottom
	
public:
	oBaseVisComponent(void);																		// constructor
	virtual	qbool				init(qapp pApp, HWND pHWnd);										// Initialize component
	
	static  qProperties *		properties(void);													// return array of property meta data
	virtual qbool				setProperty(qlong pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI);	// set the value of a property
	virtual void				getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI);	// get the value of a property

	// $dataname
	virtual qbool				setPrimaryData(EXTfldval &pNewValue);								// Changes our primary data
	virtual qbool				getPrimaryData(EXTfldval &pGetValue);								// Retrieves our primary data, return false if we do not manage a copy
	virtual qlong				cmpPrimaryData(EXTfldval &pWithValue);								// Compare with our primary data, return DATA_CMPDATA_SAME if same, DATA_CMPDATA_DIFFER if different, 0 if not implemented
	virtual qlong				getPrimaryDataLen();												// Get our primary data size, return negative if not supported
	virtual void				primaryDataHasChanged();											// Omnis is just letting us know our primary data has changed, this is especially handy if we do not keep a copy ourselves and thus ignore the other functions
	
	static  qMethods *			methods(void);														// return array of method meta data
	static	qEvents *			events(void);														// return an array of events meta data
	
	virtual void				doPaint(EXTCompInfo* pECI);											// Do our drawing in here
	virtual void				resized();															// Our component was resized
	
	// mouse related
	virtual HCURSOR				getCursor(qpoint pAt, qword2 pHitTest);								// return the mouse cursor we should show
	virtual void				evMouseLDown(qpoint pDownAt);										// mouse left button pressed down
	virtual void				evMouseLUp(qpoint pDownAt);											// mouse left button released
	virtual void				evMouseMoved(qpoint pMovedTo);										// mouse moved to this location while we are not dragging
	virtual void				evClick(qpoint pAt,EXTCompInfo* pECI);								// mouse click at this location
	
	// drag/drop related
	virtual bool				canDrag(qpoint pFrom);												// Can we drag from this location? Return false if we can't
	virtual qlong				evStartDrag(FLDdragDrop *pDragInfo);								// Started dragged, return -1 if we leave it up to Omnis to handle this
	virtual qlong				evEndDrag(FLDdragDrop *pDragInfo);									// Ended dragging, return -1 if we leave it up to Omnis to handle this

	// scrollbar functions
	virtual qdim				getHorzStepSize(void);												// get our horizontal step size
	virtual qdim				getVertStepSize(void);												// get our vertical step size
	virtual void				evWindowScrolled(qdim pNewX, qdim pNewY);							// window was scrolled
	
	// called from our WndProc, don't override these directly
	void						wm_lbutton(qpoint pAt, bool pDown,EXTCompInfo* pECI);				// left mouse button
	void						wm_mousemove(qpoint pAt, EXTCompInfo* pECI);						// mouse is being moved
	qlong						wm_dragdrop(WPARAM wParam, LPARAM lParam, EXTCompInfo* pECI);		// drag and drop handling, return -1 if we're not handling this and want default omnis logic to run
	void						wm_paint(EXTCompInfo* pECI);										// Paint message
	void						wm_windowposchanged(EXTCompInfo* pECI, WNDwindowPosStruct * pPos);	// Component resize/repos message
};

#endif

