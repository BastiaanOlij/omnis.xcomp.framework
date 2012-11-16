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
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oXCompLib.h"

// constructor
oXCompLib::oXCompLib(void) {
	// nothing to do here right now...
};

// destructor
oXCompLib::~oXCompLib(void) {
	// Make sure we properly clear our memory
	// we aren't freeing the memory for mComponents, mECOObjects, etc. itself, that will happen when the library unloads
	// but we do need to free the memory allocated by the objects contained within.
	// I guess one day we could subclass qArray<OXFComponent> and put this logic inside of clear and call clear from the destruct.
	
	while (mComponents.numberOfElements()>0) {
		OXFcomponent lvComponent = mComponents.pop();
		if (lvComponent.mProperties!=0) {
			delete lvComponent.mProperties;
		};
	};
	
	mECOobjects.clear();
};

// get our library resource id
qint oXCompLib::getResourceID(void){
	return 1000;
};

// get major version number
qshort	oXCompLib::major() {
	return 1;
};

// get minor version number
qshort	oXCompLib::minor() {
	return 0;	
};

// return our objects as an Omnis structure
qECOobjects * oXCompLib::objects(void) {
	return &mECOobjects;
};

// Add a component
void oXCompLib::addComponent(OXFcomponent pAdd) {
	qstring	tmpString;
	tmpString.appendFormattedString("Adding component %li", pAdd.componentID);
	str255 tmpStr(tmpString.cString());
	ECOaddTraceLine(&tmpStr);

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
	
	for (uint i=0;i<mComponents.numberOfElements();i++) {
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
				oBaseNVComponent * lvNVObject = (oBaseNVComponent *) lvObject;
				lvNVObject->init((qobjinst) pParam);
				
				ECOinsertNVObject( pECI->mOmnisInstance, pParam, (void*)lvObject );				
			} else {
				oBaseVisComponent * lvVisObject = (oBaseVisComponent *) lvObject;
				lvVisObject->init(pHWND);

				ECOinsertObject( pECI, pHWND, (void*)lvObject );
			}
		}
		
		return lvObject;
	}
	
	return NULL;
};

// return property meta data for this object
qProperties * oXCompLib::properties(long pCompID) {
	OXFcomponent lvComponent = componentByID(pCompID);
	if (lvComponent.componentType!=0) {
		return lvComponent.mProperties;
	};
	
	return NULL;
};

// return method meta data for this object
qMethods * oXCompLib::methods(long pCompID) {
	OXFcomponent lvComponent = componentByID(pCompID);
	if (lvComponent.componentType!=0) {
		return lvComponent.mMethods;
	};
	
	return NULL;	
};

// return our event method resource ID
unsigned int	oXCompLib::eventMethodID(long pCompID) {
	OXFcomponent lvComponent = componentByID(pCompID);
	if (lvComponent.componentType!=0) {
		return lvComponent.mEventMethodID;
	};
	
	return 0;
};

// return event meta data for this object
qEvents *	oXCompLib::events(long pCompID) {
	OXFcomponent lvComponent = componentByID(pCompID);
	if (lvComponent.componentType!=0) {
		return lvComponent.mEvents;
	};
	
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
int	oXCompLib::invokeMethod(qlong pMethodId, EXTCompInfo* pECI){
	// no static methods by default
	qstring		tmpString;
	tmpString.appendFormattedString("Unknown static method call %li",pMethodId);
	str255		tmpStr(tmpString.cString());
	ECOaddTraceLine(&tmpStr);

	return 1L;
};		

