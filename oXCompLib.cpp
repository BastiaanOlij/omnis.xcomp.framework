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
oBaseComponent * oXCompLib::instantiateComponent(long pCompID) {
	OXFcomponent *	lvComponent;
	lvComponent = componentByID(pCompID);
	
	if (lvComponent!=NULL) {
		oBaseComponent* lvObject = (oBaseComponent *) lvComponent->newObjectFunction(); 
		
		return lvObject;
	}
	
	return NULL;
};	


// initialize our library
qint oXCompLib::ecm_connect(void){
	return EXT_FLAG_LOADED|EXT_FLAG_ALWAYS_USABLE|EXT_FLAG_REMAINLOADED; // also return EXT_FLAG_NVOBJECTS if we have non-visual objects	
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

