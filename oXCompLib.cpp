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

// return our objects as an Omnis structure
qECOobjects * oXCompLib::objects(void) {
	return &mECOobjects;
};

// Add a component
void oXCompLib::addComponent(OXFcomponent pAdd) {
	if (pAdd.componentType == cObjType_NVObject) {
		// also add it to our mECOobjects list, this is how Omnis requires it...
		ECOobject	lvNVObject;
		
		lvNVObject.mObjectID	= pAdd.componentID;			// Components' ID for the object 
		lvNVObject.mNameResID	= pAdd.componentID;			// Objects' name resource id (equals our component ID
		lvNVObject.mFlags		= pAdd.flags;					// Flags
		lvNVObject.mGroupResID	= pAdd.groupResID;				// Objects' group resource id (may be 0 for default)
		
		mECOobjects.push(lvNVObject);
	} else {
		// We just need to map it as Omnis does not expect our visual components to be mixed with our non-visual ones.
		mVisIndex.push(mComponents.numberOfElements()); // as we're about to add our component but our index is 0 based, this works fine:)
	};
	
	mComponents.push(pAdd);
};

// return number of visual components
unsigned long oXCompLib::numberOfvisComps(void) {
	return mVisIndex.numberOfElements();
};

// return visual component by index using our vis index
OXFcomponent oXCompLib::visCompByIndex(uint pIndex) {
	uint lvIndex = (uint) mVisIndex[pIndex];
	return componentByIndex(lvIndex);
};

// return the number of components in our library
unsigned long oXCompLib::numberOfComponents(void){
	return mComponents.numberOfElements();
};

// return component by index
OXFcomponent oXCompLib::componentByIndex(uint pIndex) {
	return mComponents[pIndex];
};

// return component by component ID
OXFcomponent oXCompLib::componentByID(long pCompID) {
	OXFcomponent lvEmpty;
	
	for (int i=0;i<mComponents.numberOfElements();i++) {
		OXFcomponent lvComponent = mComponents[i];
		if (lvComponent.componentID == pCompID) {
			return lvComponent;
		}
	}
	
	lvEmpty.componentType = 0;
	return lvEmpty;
};

// instantiate a component by component ID
oBaseComponent * oXCompLib::instantiateComponent(long pCompID
													, EXTCompInfo* pECI
													, HWND pHWND
													, LPARAM pParam) {
	
	// Once we redo how we handle parameters and methods we can remove pCompID and use pECI->mCompId instead
	OXFcomponent lvComponent = componentByID(pCompID);
	
	if (lvComponent.componentType!=0) {
		oBaseComponent* lvObject = (oBaseComponent *) lvComponent.newObjectFunction();
		
		if ((lvObject!=NULL) && (pECI!=NULL)) {
			// and insert into a chain of objects. The OMNIS library will maintain this chain

			if (lvComponent.componentType == cObjType_NVObject) {
				ECOinsertNVObject( pECI->mOmnisInstance, pParam, (void*)lvObject );				
			} else {
				ECOinsertObject( pECI, pHWND, (void*)lvObject );
			}
		}
		
		return lvObject;
	}
	
	return NULL;
};

// initialize our library
qint oXCompLib::ecm_connect(void){
	qint	lvFlags = EXT_FLAG_LOADED|EXT_FLAG_ALWAYS_USABLE|EXT_FLAG_REMAINLOADED;
	
	// !BAS! We probably need to add a check for static functions here too.

	if (mECOobjects.numberOfElements()!=0) {
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
int	oXCompLib::invokeMethod(qint pMethodId, EXTCompInfo* pECI){
	// no static methods by default
	return qfalse;
};		

