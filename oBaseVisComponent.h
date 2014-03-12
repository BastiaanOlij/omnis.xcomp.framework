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
	
	void						setup(EXTCompInfo* pECI);											// setup our colors and fonts etc.
	
protected:
	qpoint						mMouseAt;															// Last known location of the mouse as it hoovered over our control
	
	qcol						mTextColor;															// Our text color
	qpat						mBackpattern;														// Our back pattern
	HBRUSH						mBackpatBrush;														// backpattern brush
	qcol						mForecolor, mBackcolor;												// Our forecolor and backcolor
	qulong						mBKTheme;															// Our background theme
	
	qbool						mDrawBuffer;														// If true (default) we'll setup our canvas buffer
	HWND						mHWnd;																// Our main window handle (not applicable for NV objects)
	qrect						mClientRect;														// Our client rect, gives the size of our visual component
	
	// colour function
	qcol						mixColors(qcol pQ1, qcol pQ2);										// Mix two colors together
	
	// drawing functions
	void						drawEllipse(HDC pHDC, qrect pRect, qcol pTop, qcol pBottom, qcol pBorder = -1, qint pSpacing = 0);		// Draws a filled ellipse within the rectangle with a gradient color from top to bottom
	
public:
	oBaseVisComponent(void);																		// constructor
	virtual	qbool				init(HWND pHWnd);													// Initialize component
	
	static  qProperties *		properties(void);													// return array of property meta data
	virtual qbool				setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* eci);	// set the value of a property
	virtual void				getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* eci);	// get the value of a property
	
	static  qMethods *			methods(void);														// return array of method meta data
	static	qEvents *			events(void);														// return an array of events meta data
	
	virtual void				doPaint(HDC pHDC);													// Do our drawing in here
	virtual void				Resized();															// Our component was resized
	
	virtual void				evMouseMoved(qpoint pMovedTo);										// mouse moved to this location while mouse button is not down
	virtual void				evClick(qpoint pAt);												// mouse click at this location
	virtual void				evStartDrag(qpoint pFrom);											// mouse started dragged
	virtual void				evDragging(qpoint pFrom, qpoint pAt);								// mouse being dragged
	virtual void				evEndDrag(qpoint pFrom, qpoint pTop);								// mouse dragged from - to	
	virtual void				evCancelledDrag();													// cancelled dragging
	
	// called from our WndProc, don't override these directly
	void						wm_lbutton(qpoint pAt, bool pDown);									// left mouse button
	void						wm_mousemove(qpoint pAt);											// mouse is being moved
	void						wm_paint(EXTCompInfo* pECI);										// Paint message
	void						wm_windowposchanged(EXTCompInfo* pECI, WNDwindowPosStruct * pPos);	// Component resize/repos message
};

#endif

