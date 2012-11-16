/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oXCompLib.h
 *  Base class for our library definition. 
 *
 *  You need to subclass this and implement the needed methods then instantiate your class as a global called gXCompLib
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - finish implementation of static methods
 *
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

// our base component library already includes everything else we need...
#include "oBaseComponent.h"

#ifndef xcomplibh
#define xcomplibh

// Array of objects
typedef qArray<ECOobject>		qECOobjects;

// Visual object definition
struct OXFcomponent {
	int					componentType;										// Type of component
	unsigned int		componentID;										// Resource ID for this component, also the object ID
	unsigned int		bitmapID;											// Resource ID for this components bitmap (only for visible objects)
	qlong				flags;												// flags (only for NV-objects)
	qlong				groupResID;											// group resource ID (only for NV-objects)
	void *				(*newObjectFunction) (void);						// Function pointer to the function that returns a new instance
	qProperties *		mProperties;										// Array containing property definitions
	qMethods *			mMethods;											// Array containing method definitions
	unsigned int		mEventMethodID;										// Resource ID for the first line of our $event method text for our object
	qEvents *			mEvents;											// Array containing our event definitions
};

// Array of components
typedef qArray<OXFcomponent> qComponents;

class oXCompLib {
private:
	qECOobjects			mECOobjects;										// our object array converted to Omnis
	qArray<uint>		mVisIndex;											// index of visual components
	qComponents			mComponents;										// array of component definitions

protected:
	
public:	
	oXCompLib(void);														// constructor
	~oXCompLib(void);														// destructor
	
	// info
	qint				getResourceID(void);								// get our library resource id
	virtual qshort		major();											// get major version number
	virtual qshort		minor();											// get minor version number
		
	// access to components
	qECOobjects *		objects(void);										// return our objects as an Omnis structure
	void				addComponent(OXFcomponent pAdd);					// Add a component
	unsigned long		numberOfvisComps(void);								// return number of visual components
	OXFcomponent		visCompByIndex(uint pIndex);						// return visual component by index using our vis index
	unsigned long		numberOfComponents(void);							// return the number of components in our library
	OXFcomponent		componentByIndex(uint pIndex);						// return component by index
	OXFcomponent		componentByID(long pCompID);						// return component by component ID
	oBaseComponent *	instantiateComponent(long pCompID
											 , EXTCompInfo* pECI
											 , HWND pHWND
											 , LPARAM pParam);				// instantiate a component by component ID
	
	// access to meta data
	qProperties *		properties(long pCompID);							// return property meta data for this object
	qMethods *			methods(long pCompID);								// return method meta data for this object
	unsigned int		eventMethodID(long pCompID);						// return our event method resource ID
	qEvents *			events(long pCompID);								// return event meta data for this object
	
	// calls from our wndproc
	virtual qint		ecm_connect(void);									// initialize our library
	virtual qbool		ecm_disconnect(void);								// cleanup

	// need to add methods for returning static method information
	virtual int			invokeMethod(qlong pMethodId, EXTCompInfo* pECI);		// invoke a static method
};

// This class must be implemented in your library (on windows its not enough to just have a class definition...) 
// An instance of this class is created to manage your library.
class mainlib : public oXCompLib {
public:	
	virtual qshort		major();											// get major version number
	virtual qshort		minor();											// get minor version number

	virtual qint		ecm_connect(void);
	virtual qbool		ecm_disconnect(void);
	virtual int			invokeMethod(qlong pMethodId, EXTCompInfo* pECI);
};

#endif