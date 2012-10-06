/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseComponent.h
 *  Base classes to subclass components from. These classes implement alot of the basics that each component shares
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
	virtual int					invokeMethod(qint pMethodId,EXTCompInfo* eci);						// invoke a metho
	
}

/* baseclass for non visual components */
class oBaseNVComponent : public oBaseComponent {
public:
	virtual void	copyObject(oBaseNVComponent *pCopy)=0;											// create a copy of our object, this MUST be implemented in a subclass

};

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
protected:
	HWND						mHWnd;																// Our main window handle

public:
	oComponent(HWND pFieldHWnd);																	// Constructor
	virtual	qbool				init();																// Initialize component
	
};

#endif