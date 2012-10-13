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
 */

#include "oXCompLib.h"

// constructor
oXCompLib::oXCompLib(void) {
	// nothing to do here right now...
};

// destructor
oXCompLib::~oXCompLib(void) {
	// nothing to do here right now...
};

// get our library resource id
qint oXCompLib::getResourceID(void){
	return 1000;
};

// return the number of NV objects in our library
unsigned long oXCompLib::numberOfObjects(void) {
	return mObjects.numberOfElements();
};

// return our objects as an Omnis structure
ECOobject * oXCompLib::objects(void) {
	if (mECOobjects.numberOfElements()!=mObjects.numberOfElements()) {
		mECOobjects.clear();
		
		// convert our array to something Omnis likes..
		for (qlong i=0;i<mObjects.numberOfElements();i++) {
			ECOobject	lvECOobject;
			OXFNVobject	*lvNVObject = mObjects[i];
			
			lvECOobject.mObjectID = lvNVObject->objectID; // we use our resource ID for our object ID, this makes sure that our ID is unique over objects and components
			lvECOobject.mNameResID = lvNVObject->objectID;
			lvECOobject.mFlags = lvNVObject->flags;
			lvECOobject.mGroupResID = lvNVObject->groupResID;
			
			mECOobjects.push(&lvECOobject);
		};
	};
	
	return (ECOobject *) mECOobjects.getArray();
};


// return object definition by index
OXFNVobject * oXCompLib::objectByIndex(uint pIndex){
	return mObjects[pIndex];
};

// return object definition by ID
OXFNVobject * oXCompLib::objectByID(long pObjID) {
	for (int i=0;i<mObjects.numberOfElements();i++) {
		OXFNVobject *lvNVObject = mObjects[i];
		if (lvNVObject->objectID == pObjID) {
			return lvNVObject;
		};
	};
	
	return NULL;
};

// instantiate an object by object ID
oBaseNVComponent * oXCompLib::instantiateObject(long pObjID) {
	OXFNVobject *	lvNVObject;
	lvNVObject = objectByID(pObjID);
	if (lvNVObject != NULL) {
		oBaseNVComponent * lvObject = (oBaseNVComponent *) lvNVObject->newObjectFunction();
		
		return lvObject;
	};
	
	return NULL;
};


// return the number of components in our library
unsigned long oXCompLib::numberOfComponents(void){
	return mComponents.numberOfElements();
};

// return component by index
OXFcomponent * oXCompLib::componentByIndex(uint pIndex) {
	return mComponents[pIndex];
};

// return component by component ID
OXFcomponent * oXCompLib::componentByID(long pCompID) {
	for (int i=0;i<mComponents.numberOfElements();i++) {
		OXFcomponent *lvComponent = mComponents[i];
		if (lvComponent->componentID == pCompID) {
			return lvComponent;
		}
	}
	return NULL;
};

// instantiate a component by component ID
oBaseVisComponent * oXCompLib::instantiateComponent(long pCompID) {
	OXFcomponent *	lvComponent;
	lvComponent = componentByID(pCompID);
	
	if (lvComponent!=NULL) {
		oBaseVisComponent* lvObject = (oBaseVisComponent *) lvComponent->newObjectFunction(); 
		
		return lvObject;
	}
	
	return NULL;
};

// initialize our library
qint oXCompLib::ecm_connect(void){
	qint	lvFlags = EXT_FLAG_LOADED|EXT_FLAG_ALWAYS_USABLE|EXT_FLAG_REMAINLOADED;
	
	// !BAS! We probably need to add a check for static functions here too.

	if (numberOfObjects()!=0) {
		lvFlags = lvFlags | EXT_FLAG_NVOBJECTS;		// Let Omnis know we also include non-visual object, this means ECM_GETOBJECT will be called
	}
	
	return lvFlags;	
};

// cleanup
qbool oXCompLib::ecm_disconnect(void){
	// should clear out gComponents...
	
	return qtrue;
};

// invoke a static method
int	oXCompLib::invokeMethod(qint pMethodId, EXTCompInfo* eci){
	// no static methods by default
	return qfalse;
};		

