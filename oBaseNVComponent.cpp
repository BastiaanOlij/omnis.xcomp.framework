/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseNVComponent.cpp
 *  Base class for our non visual component
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseNVComponent.h"

/********************************************************************************************************************************************
 oBaseNVComponent
 ********************************************************************************************************************************************/

// Initialize component
qbool oBaseNVComponent::init(qapp pApp, qobjinst pInst) {
	oBaseComponent::init(pApp);
	
	mObjInst = pInst;
	
	return true;
};

// create a copy of pCopy, this MUST be implemented in a subclass
void oBaseNVComponent::copyObject(oBaseNVComponent *pCopy) {
	// nothing to copy...
};

// Add properties for NV componect
qProperties * oBaseNVComponent::properties(void) {
	qProperties *	lvProperties = oBaseComponent::properties();
	
	// Our non-visual class has not properties, we still need to implement this...
	
	return lvProperties;
};

// Add methods for NV component
qMethods * oBaseNVComponent::methods(void) {
	qMethods * lvMethods = oBaseComponent::methods();
	
	// our non-visual class has not methods, we still need to implement this...
	
	return lvMethods;
};

// Add events for NV component
qEvents *	oBaseNVComponent::events(void) {
	qEvents * lvEvents =oBaseComponent::events();
	
	// our non-visual class has no events
	
	return lvEvents;
};

// object is being send a rebuild message
qint	oBaseNVComponent::ecm_object_rebuild(EXTCompInfo* pECI) {
	return mNeedRebuild ? 1L : 0L;
};