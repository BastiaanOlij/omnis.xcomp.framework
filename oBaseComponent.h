/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseComponent.h
 *  Base classes to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Our framework will instantiate your objects when it needs to obtain meta data or when the component is actually used.
 *  The init function is only called when the component is actualy used so put your heavy lifting stuff in here and keep your
 *  constructors simple (or just rely on the default constructor).
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - Generally complete this implementation
 *  - Add support for events
 *  - Implement visual components
 *  - Maybe split this into separate files for the three classes
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "xCompStandardIncludes.h"
#include "qArray.h"
#include "qString.h"

#ifndef obasecomponenth
#define obasecomponenth

// Made a type for non-visual objects...
const int cObjType_NVObject = 255;																	// Omnis reserved 1-9, assume they won't reach 255 any time soon...

// Some handy arrays
typedef	qArray<ECOproperty>		qProperties;
typedef qArray<ECOmethodEvent>	qMethods;
typedef qArray<ECOmethodEvent>	qEvents;

/* our base class for all component objects */
class oBaseComponent {
private:
	
protected:
	/*** Parameters ***/
	qstring	*					newStringFromParam(int pParamNo, EXTCompInfo* pECI);				// get string from parameter, call needs to delete returned object
	long						getLongFromParam(int pParamNo, EXTCompInfo* pECI);					// get long from parameter
	EXTqlist *					newQListFromParam(int pParamNo, EXTCompInfo* pECI);					// get qlist from parameter, caller needs to delete return object
	char *						newBinfromParam(int pParamNo, size_t *pLen,  EXTCompInfo* pECI);	// get binary buffer, returns NULL if the buffer is empty
	
public:
	oBaseComponent(void);																			// constructor
	virtual ~oBaseComponent(void);																	// destructor

/*** some nice support function ***/
	void addToTraceLog(const char *pData, ...);														// Add formatted string to trace log
	
/*** Properties ***/
	static  qProperties *		properties(void);													// return array of property meta data
	virtual qbool				canAssign(qlong pPropID);											// return true/false if a property can be written too
	virtual qbool				setProperty(qlong pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI);	// set the value of a property
	virtual void				getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI);	// get the value of a property
	
/*** Methods ***/
	static  qMethods *			methods(void);														// return an array of method meta data
	virtual int					invokeMethod(qlong pMethodId, EXTCompInfo* pECI);					// invoke a method

/*** Events ***/
	static	qEvents *			events(void);														// return an array of events meta data
};

/* baseclass for non visual components */
class oBaseNVComponent : public oBaseComponent {
protected:
	qobjinst					mObjInst;															// Object instance
	bool						mNeedRebuild;														// Set this to true if a rebuild of property/method definitions is required

public:
	virtual	qbool				init(qobjinst pInst);												// Initialize component

	virtual void				copyObject(oBaseNVComponent *pCopy);								// create a copy of pCopy, this MUST be implemented in a subclass

	static  qProperties *		properties(void);													// return array of property meta data
	static  qMethods *			methods(void);														// return array of method meta data
	static	qEvents *			events(void);														// return an array of events meta data

	qint						ecm_object_rebuild(EXTCompInfo* pECI);								// object is being send a rebuild message
};

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
private:	
	/*** only valid during drawing ***/
	GDItextSpecStruct			mTextSpec;															// Info on how to draw text
	
	void						setup(EXTCompInfo* pECI);											// setup our colors and fonts etc.

protected:
	qcol						mTextColor;															// Our text color
	qpat						mBackpattern;														// Our back pattern
	HBRUSH						mBackpatBrush;														// backpattern brush
	qcol						mForecolor, mBackcolor;												// Our forecolor and backcolor
	qulong						mBKTheme;															// Our background theme
	
	qbool						mDrawBuffer;														// If true (default) we'll setup our canvas buffer
	HWND						mHWnd;																// Our main window handle (not applicable for NV objects)
	qrect						mClientRect;														// Our client rect, gives the size of our visual component
	
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
	
	// called from our WndProc, don't override these directly
	void						wm_paint(EXTCompInfo* pECI);										// Paint message
	void						wm_windowposchanged(EXTCompInfo* pECI, WNDwindowPosStruct * pPos);	// Component resize/repos message
};

#endif