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
 */

#ifndef obasecomponenth
#define obasecomponenth

#include "omnis.xcomp.framework.h"

/* our base class for all component objects */
class oBaseComponent {
protected:
	qProperties					*mProperties;														// Property definition for our object (lazy loading)
	qMethods					*mMethods;															// Method definition for our object (lazy loading)
	
public:
	oBaseComponent(void);																			// constructor
	~oBaseComponent(void);																			// destructor
	
/*** Properties ***/
	virtual qint				propertyCount(void);												// return the number of properties supported by this component
	virtual qProperties *		properties(void);													// return array of property meta data
	virtual qbool				canAssign(qint pPropID);											// return true/false if a property can be written too
	virtual qbool				setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* eci);	// set the value of a property
	virtual void				getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* eci);	// get the value of a property
	
/*** Methods ***/
	virtual qint				methodCount(void);													// return the number of methods supported by this component
	virtual qMethods *			methods(void);														// return an array of method meta data
	virtual int					invokeMethod(qint pMethodId,EXTCompInfo* eci);						// invoke a method
	
};

/* baseclass for non visual components */
class oBaseNVComponent : public oBaseComponent {
public:
	virtual	qbool				init(void);															// Initialize component
	virtual void				copyObject(oBaseNVComponent *pCopy);								// create a copy of pCopy, this MUST be implemented in a subclass
};

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
protected:
	HWND						mHWnd;																// Our main window handle

public:
	virtual	qbool				init(HWND pHWnd);													// Initialize component
	
};

#endif