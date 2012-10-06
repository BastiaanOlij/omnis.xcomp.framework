/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseComponent.cpp
 *  Base classes to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Bastiaan Olij
 */

#include "oBaseComponent.h"

/********************************************************************************************************************************************
 oBaseComponent
 ********************************************************************************************************************************************/

// constructor
oBaseComponent::oBaseComponent(void) {
	mProperties = NULL;
	mMethods = NULL;
};

// destructor
oBaseComponent::~oBaseComponent(void) {
	if (mProperties!=NULL) {
		delete mProperties;
		mProperties = NULL;
	};
	
	if (mMethods!=NULL) {
		delete mMethods;
		mMethods = NULL;
	}
};

/*** Properties ***/
// return the number of properties supported by this component
qint oBaseComponent::propertyCount(void) {
	return this->properties()->numberOfElements();
};

// return array of property meta data
// Lazy loading approach, check if mProperties is not set:
// - if not set call superclass and add your own properties
// - if set, return as is
qProperties * oBaseComponent::properties(void) {
	if (mProperties==NULL) {
		mProperties = new qProperties();
	};
	return mProperties;
};

// return true/false if a property can be written too
qbool oBaseComponent::canAssign(qint pPropID) {
	return false;
};

// set the value of a property
qbool oBaseComponent::setProperty(qint pPropID,EXTfldval &pNewValue,EXTCompInfo* eci) {
	return false;
};

// get the value of a property
void oBaseComponent::getProperty(qint pPropID,EXTfldval &pGetValue,EXTCompInfo* eci) {
	// nothing to do here, base class does not have any properties...
};

/*** Methods ***/
// return the number of methods supported by this component
qint oBaseComponent::methodCount(void) {
	return this->methods()->numberOfElements();
};

// return an array of method meta data
// Lazy loading approach, check if mMethods is not set:
// - if not set call superclass and add your own methods
// - if set, return as is
qMethods * oBaseComponent::methods(void) {
	if (mMethods==NULL) {
		mMethods = new qMethods();
	};
	return mMethods;
};

// invoke a method
int oBaseComponent::invokeMethod(qint pMethodId,EXTCompInfo* eci) {
	// nothing to do here by default
	
	return 1L;
};

/********************************************************************************************************************************************
 oBaseNVComponent
 ********************************************************************************************************************************************/

// Initialize component
qbool oBaseNVComponent::init(void) {
	// nothing to do here by default
	
	return true;
};

// create a copy of pCopy, this MUST be implemented in a subclass
void oBaseNVComponent::copyObject(oBaseNVComponent *pCopy) {
	// nothing to copy...
};


/********************************************************************************************************************************************
 oBaseVisComponent
 ********************************************************************************************************************************************/

// Initialize component
qbool oBaseVisComponent::init(HWND pHWnd) {
	mHWnd = pHWnd;
	
	return true;
};


