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
#include "oDrawingCanvas.h"

#ifndef obaseviscomponenth
#define obaseviscomponenth

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
private:
	/*** only valid during mouse actions ***/
	bool mMouseLButtonDown; // Did we press the mouse button down while within our control?
	qpoint mMouseDownAt; // Location we pressed the mouse down at
	bool mMouseDragging; // Are we dragging?
	bool mMouseOver; // If true mouse is over our object

	// init functions
	void setup(EXTCompInfo *pECI); // setup our colors and fonts etc.

protected:
	qlong mObjType; // Object type
	HWND mHWnd; // Our main window handle (not applicable for NV objects)
	bool mShowName; // For design only, show our field name
	qpoint mMouseAt; // Last known location of the mouse as it hoovered over our control
	EXTfldval mPrimaryData; // Copy of our primary data if default implementation is used

	qcol mTextColor; // Our text color
	qpat mBackpattern; // Our back pattern
	qcol mForecolor, mBackcolor; // Our forecolor and backcolor
	qcol mBorderColor; // Our border color
	qulong mBKTheme; // Our background theme

	/*** scrollbar related, how you interpret these in your control is up to you ***/
	qdim mHorzScrollPos; // Horizontal scroll position
	qdim mVertScrollPos; // Vertical scroll position

	/*** only valid during drawing ***/
	qbool mDrawBuffer; // If true (default) we'll setup our canvas buffer
	qrect mClientRect; // Our client rect, gives the size of our visual component
	oDrawingCanvas *mCanvas; // Current canvas for drawing

	// preparation functions
	GDItextSpecStruct getStdTextSpec(EXTCompInfo *pECI); // Create text spec structure for our standard properties (used by setup or when drawing list lines)

	// handy Omnis functions
	EXTqlist *getDataList(EXTCompInfo *pECI); // Get list variable used for $dataname
	bool isEnabled(); // check if our field is enabled
	bool isActive(); // check if our field is active
public:
	oBaseVisComponent(void); // constructor
	virtual qbool init(qapp pApp, HWND pHWnd); // Initialize component

	static qProperties *properties(void); // return array of property meta data
	virtual qbool inBuildOverride(qlong pPropID); // return true if our component is handling this build in property?
	virtual qbool setProperty(qlong pPropID, EXTfldval &pNewValue, EXTCompInfo *pECI); // set the value of a property (return true if property was handled, false if Omnis needs to do its thing..)
	virtual qbool getProperty(qlong pPropID, EXTfldval &pGetValue, EXTCompInfo *pECI); // get the value of a property (return true if property was handled, false if Omnis needs to do its thing..)

	// $dataname
	virtual qbool enablePrimaryData(); // Do we use our primary data logic? If we return false we do not story a copy of the primary data
	virtual qbool setPrimaryData(EXTfldval &pNewValue); // Changes our primary data
	virtual qbool getPrimaryData(EXTfldval &pGetValue); // Retrieves our primary data, return false if we do not manage a copy
	virtual qlong cmpPrimaryData(EXTfldval &pWithValue); // Compare with our primary data, return DATA_CMPDATA_SAME if same, DATA_CMPDATA_DIFFER if different, 0 if not implemented
	virtual qlong getPrimaryDataLen(); // Get our primary data size, return negative if not supported
	virtual void primaryDataHasChanged(); // Omnis is just letting us know our primary data has changed, this is especially handy if we do not keep a copy ourselves and thus ignore the other functions

	static qMethods *methods(void); // return array of method meta data
	static qEvents *events(void); // return an array of events meta data

	virtual void doPaint(EXTCompInfo *pECI); // Do our drawing in here
	virtual bool drawListContents(EXTListLineInfo *pInfo, EXTCompInfo *pECI); // Do our list content drawing here (what we see when the list is collapsed, for cObjType_DropList only)
	virtual bool drawListLine(EXTListLineInfo *pInfo, EXTCompInfo *pECI); // Do our list line drawing here (for cObjType_List or cObjType_DropList)
	virtual void resized(); // Our component was resized

	// mouse related (note, our mouseover/enter/leave functionality gave us issues so has been deprecated)
	virtual HCURSOR getCursor(qpoint pAt, qword2 pHitTest); // return the mouse cursor we should show
	DEPRECATED bool mouseIsOver(); // returns true if the mouse is over our object
	virtual DEPRECATED void evMouseEnter(); // mouse moved over our object
	virtual DEPRECATED void evMouseLeave(); // mouse moved away from our object
	virtual bool evMouseLDown(qpoint pDownAt); // mouse left button pressed down (return true if we finished handling this, false if we want Omnis internal logic)
	virtual bool evMouseLUp(qpoint pUpAt); // mouse left button released (return true if we finished handling this, false if we want Omnis internal logic)
	virtual bool evDoubleClick(qpoint pAt, EXTCompInfo *pECI); // mouse left button double clicked (return true if we finished handling this, false if we want Omnis internal logic)
	virtual bool evMouseRDown(qpoint pDownAt, EXTCompInfo *pECI); // mouse right button pressed down (return true if we finished handling this, false if we want Omnis internal logic)
	virtual bool evMouseRUp(qpoint pUpAt, EXTCompInfo *pECI); // mouse right button released (return true if we finished handling this, false if we want Omnis internal logic)
	virtual void evMouseMoved(qpoint pMovedTo); // mouse moved to this location while we are not dragging
	virtual void evClick(qpoint pAt, EXTCompInfo *pECI); // mouse click at this location

	// drag/drop related
	virtual bool canDrag(qpoint pFrom); // Can we drag from this location? Return false if we can't
	virtual qlong evStartDrag(FLDdragDrop *pDragInfo); // Started dragged, return -1 if we leave it up to Omnis to handle this
	virtual qlong evSetDragValue(FLDdragDrop *pDragInfo, EXTCompInfo *pECI); // Set drag value, update the pDragInfo structure with information about what we are dragging, return -1 if we leave it up to Omnis to handle this
	virtual qlong evEndDrag(FLDdragDrop *pDragInfo); // Ended dragging, return -1 if we leave it up to Omnis to handle this

	// scrollbar functions
	virtual qdim getHorzStepSize(void); // get our horizontal step size
	virtual qdim getVertStepSize(void); // get our vertical step size
	virtual void evWindowScrolled(qdim pNewX, qdim pNewY); // window was scrolled

	// keyboard
	virtual bool evKeyPressed(qkey *pKey, bool pDown, EXTCompInfo *pECI); // let us know a key was pressed. Return true if Omnis should not do anything with this keypress

	// called from our WndProc, don't override these directly
	bool wm_lbutton(qpoint pAt, bool pDown, EXTCompInfo *pECI); // left mouse up/down (return true if we finished handling this, false if we want Omnis internal logic)
	bool wm_lbDblClick(qpoint pAt, EXTCompInfo *pECI); // left mouse button double click (return true if we finished handling this, false if we want Omnis internal logic)
	bool wm_rbutton(qpoint pAt, bool pDown, EXTCompInfo *pECI); // right mouse button (return true if we finished handling this, false if we want Omnis internal logic)
	void wm_mousemove(qpoint pAt, EXTCompInfo *pECI, bool IsOver); // mouse is being moved
	qlong wm_dragdrop(WPARAM wParam, LPARAM lParam, EXTCompInfo *pECI); // drag and drop handling, return -1 if we're not handling this and want default omnis logic to run
	bool wm_geteraseinfo(WNDeraseInfoStruct *pInfo, EXTCompInfo *pECI); // get info about background drawing
	bool wm_erasebkgnd(EXTCompInfo *pECI); // erase our background message
	bool wm_paint(EXTCompInfo *pECI); // Paint message
	bool ecm_paintcontents(EXTListLineInfo *pInfo, EXTCompInfo *pECI); // Draw cObjType_DropList content
	bool ecm_listdrawline(EXTListLineInfo *pInfo, EXTCompInfo *pECI); // Draw line fro cObjType_List or cObjTypeDropList
#if OMNISSDK >= 70
	void wm_windowposchanged(EXTCompInfo *pECI, WNDsetWindowPosStruct *pPos); // Component resize/repos message
#else
	void wm_windowposchanged(EXTCompInfo *pECI, WNDwindowPosStruct *pPos); // Component resize/repos message
#endif
};

#endif
