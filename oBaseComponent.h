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
 */

#include "xCompStandardIncludes.h"
#include "qArray.h"
#include "qString.h"

#ifndef obasecomponenth
#define obasecomponenth

// Some handy arrays
typedef	qArray<ECOproperty>		qProperties;
typedef qArray<ECOmethodEvent>	qMethods;

/* our base class for all component objects */
class oBaseComponent {
private:
	
protected:
	HWND						mHWnd;																// Our main window handle (not applicable for NV objects)
	qProperties					*mProperties;														// Property definition for our object (lazy loading)
	qMethods					*mMethods;															// Method definition for our object (lazy loading)
	
public:
	oBaseComponent(void);																			// constructor
	virtual ~oBaseComponent(void);																	// destructor
	virtual	qbool				init(HWND pHWnd);													// Initialize component

/*** some nice support function ***/
	void addToTraceLog(const char *pData, ...);														// Add formatted string to trace log
	
/*** Properties ***/
	virtual qint				propertyCount(void);												// return the number of properties supported by this component
	virtual qProperties *		properties(void);													// return array of property meta data
	virtual qbool				canAssign(qint pPropID);											// return true/false if a property can be written too
	virtual qbool				setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI);	// set the value of a property
	virtual void				getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI);	// get the value of a property
	
/*** Methods ***/
	virtual qint				methodCount(void);													// return the number of methods supported by this component
	virtual qMethods *			methods(void);														// return an array of method meta data
	virtual int					invokeMethod(qint pMethodId, EXTCompInfo* pECI);					// invoke a method
	
};

/* baseclass for non visual components */
class oBaseNVComponent : public oBaseComponent {
public:
	virtual void				copyObject(oBaseNVComponent *pCopy);								// create a copy of pCopy, this MUST be implemented in a subclass
};

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
private:
	/*** only valid during drawing ***/
	GDItextSpecStruct			mTextSpec;															// Info on how to draw text
	qcol						mTextColor;															// Our text color
	qpat						mBackpattern;														// Our back pattern
	HBRUSH						mBackpatBrush;														// backpattern brush
	qcol						mForecolor, mBackcolor;												// Our forecolor and backcolor
	
	void						setup(EXTCompInfo* pECI);											// setup our colors and fonts etc.

protected:
	qrect						mClientRect;														// Our client rect, gives the size of our visual component
	
public:
	oBaseVisComponent(void);																		// constructor
	
	virtual qProperties *		properties(void);													// return array of property meta data
	virtual qbool				setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* eci);	// set the value of a property
	virtual void				getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* eci);	// get the value of a property

	
	virtual void				doPaint(HDC pHDC);													// Do our drawing in here

	
	// called from our WndProc, don't override these directly
	qbool						wm_paint(EXTCompInfo* pECI);														// Paint message
};

#endif