/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseNVComponent.h
 *  Base class for our non visual component
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseComponent.h"

#ifndef obasenvcomponenth
#define obasenvcomponenth

/* baseclass for non visual components */
class oBaseNVComponent : public oBaseComponent {
protected:
	qobjinst mObjInst; // Object instance
	bool mNeedRebuild; // Set this to true if a rebuild of property/method definitions is required

public:
	virtual qbool init(qapp pApp, qobjinst pInst); // Initialize component

	virtual void copyObject(oBaseNVComponent *pCopy); // create a copy of pCopy, this MUST be implemented in a subclass

	static qProperties *properties(void); // return array of property meta data
	static qMethods *methods(void); // return array of method meta data
	static qEvents *events(void); // return an array of events meta data

	qint ecm_object_rebuild(EXTCompInfo *pECI); // object is being send a rebuild message
};

#endif