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
 */

// our base component library already includes everything else we need...
#include "oBaseComponent.h"

#ifndef xcomplibh
#define xcomplibh

// Non-visual object definition
struct OXFNVobject {
	qlong				objectID;											// Resource ID for this object
	qlong				flags;												// flags
	qlong				groupResID;											// group resource ID
	void *				(*newObjectFunction) (void);						// Function pointer to the function that returns a new instance
	void				(*destructObjectFunc) (oBaseNVComponent * pObj);	// Function pointer to the function that destructs our instance
};

// Array of objects
typedef qArray<OXFNVobject> qObjects;
typedef qArray<ECOobject> qECOobjects;

// Visual object definition
struct OXFcomponent {
	int					componentType;										// Type of component
	unsigned int		componentID;										// Resource ID for this component, also the object ID
	unsigned int		bitmapID;											// Resource ID for this components bitmap
	void *				(*newObjectFunction) (void);						// Function pointer to the function that returns a new instance
	void				(*destructObjectFunc) (oBaseVisComponent * pObj);	// Function pointer to the function that destructs our instance
};

// Array of components
typedef qArray<OXFcomponent> qComponents;

class oXCompLib {
private:
	qECOobjects			mECOobjects;										// our object array converted to Omnis

protected:
	qObjects			mObjects;											// array of object definitions
	qComponents			mComponents;										// array of component definitions
	
public:	
	oXCompLib(void);														// constructor
	~oXCompLib(void);														// destructor
	
	// info
	virtual qint		getResourceID(void);								// get our library resource id
	
	// access to non visual object
	unsigned long		numberOfObjects(void);								// return the number of NV objects in our library
	ECOobject *			objects(void);										// return our objects as an Omnis structure
	OXFNVobject *		objectByIndex(uint pIndex);							// return object definition by index
	OXFNVobject *		objectByID(long pObjID);							// return object definition by ID
	oBaseNVComponent *	instantiateObject(long pObjID);						// instantiate an object by object ID
	
	// access to components
	unsigned long		numberOfComponents(void);							// return the number of components in our library
	OXFcomponent *		componentByIndex(uint pIndex);						// return component by index
	OXFcomponent *		componentByID(long pCompID);						// return component by component ID
	oBaseVisComponent *	instantiateComponent(long pCompID);					// instantiate a component by component ID
	
	// destruct an object we've created
	void				destructComponent(oBaseComponent *pDestruct, long pCompID);	// call the proper destruct code for this object
	
	// calls from our wndproc
	virtual qint		ecm_connect(void);									// initialize our library
	virtual qbool		ecm_disconnect(void);								// cleanup

	// need to add methods for returning static method information
	virtual int			invokeMethod(qint pMethodId, EXTCompInfo* eci);		// invoke a static method
};

#endif