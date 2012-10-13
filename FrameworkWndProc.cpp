/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  FrameworkWndProc.cpp
 *  This contains our main entry point for our xcomp
 *
 *  Note that the basis of this code comes straight from the generic examples in the Omnis SDK
 *
 *  Bastiaan Olij
 */

// Include our framework files
#include "omnis.xcomp.framework.h"

// this class should be defined in our implementation, I need to find a better way do this.
class mainlib : public oXCompLib {
	
};

mainlib *gXCompLib = NULL;

// Component library entry point (name as declared in resource 31000 )
extern "C" qlong OMNISWNDPROC FrameworkWndProc(HWND pHWND, LPARAM pMsg,WPARAM wParam,LPARAM lParam,EXTCompInfo* pECI) {
	// Initialize callback tables - THIS MUST BE DONE 
	ECOsetupCallbacks(pHWND, pECI);		
	
	switch (pMsg) {
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// library initialisation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// ECM_CONNECT - this message is sent once per OMNIS session and should not be confused 
		// with ECM_OBJCONSTRUCT which is sent once per object. ECM_CONNECT can be used if load other libraries
		// once or other general global actions that need to be done only once.
		//
		// For most components this can be removed - see other BLYTH component examples
		case ECM_CONNECT: {
			// instantiate our library
			gXCompLib = new mainlib();
			
			return gXCompLib->ecm_connect(); // Return external flags
		} break;
			
		// ECM_DISCONNECT - this message is sent only once when the OMNIS session is ending and should not be confused
		// with ECM_OBJDESTRUCT which is sent once per object. ECM_DISCONNECT can be used to free other libraries
		// loaded on ECM_CONNECT or other general global actions that need to be done only once.
		// 
		// For most components this can be removed - see other BLYTH component examples
		case ECM_DISCONNECT: { 
			qbool lvResult = gXCompLib->ecm_disconnect();
			
			// and we no longer need our library...
			delete gXCompLib;
			return lvResult;
		} break;
			
		// ECM_GETOBJECT - this is sent by OMNIS to find out which non-visual objects are part of our library
		case ECM_GETOBJECT: {
			str255	lvMsg("Testing ECM_GETOBJECT");
			ECOaddTraceLine(&lvMsg);
			
			return ECOreturnObjects( gInstLib, pECI, gXCompLib->objects(), gXCompLib->numberOfObjects());
		} break;

		// ECM_GETCOMPLIBINFO - this is sent by OMNIS to find out the name of the library, and
		// the number of components this library supports
		case ECM_GETCOMPLIBINFO: {
			return ECOreturnCompInfo( gInstLib, pECI, gXCompLib->getResourceID(), gXCompLib->numberOfComponents() );
		} break;

		// ECM_GETCOMPID - this message is sent by OMNIS to get information about each component in this library
		// wParam is a number from 1 to the number of components return on the ECM_GETCOMPLIBINFO message.
		//
		// For each call you should return the internal object ID and the type of object it is.
		// The object id will be used for other calls such as ECM_GETCOMPICON 
		// 
		// The object type is for example :		cObjType_Basic 		- a basic window object or 
		//										cRepObjType_Basic	- a basic report object.
		//										There are others 	- see BLYTH examples and headers
		case ECM_GETCOMPID: {
			OXFcomponent *lvComponent = gXCompLib->componentByIndex(wParam-1);
			if (lvComponent!=NULL) {
				return ECOreturnCompID( gInstLib, pECI, lvComponent->componentID, lvComponent->componentType );				
			}
		} break;
			
		// ECM_GETCOMPICON - this is sent by OMNIS to get an icon for the OMNIS component store and
		// external component browser. You need to always supply an icon in your resource file.
		case ECM_GETCOMPICON: {
			// OMNIS will call you once per component for an icon.
			// GENERIC_ICON is defined in the header and included in the resource file
			OXFcomponent *lvComponent = gXCompLib->componentByID(pECI->mCompId);
			if (lvComponent!=NULL) {
				return ECOreturnIcon( gInstLib, pECI, lvComponent->bitmapID );				
			}
			
			return qfalse;
		}
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// construct/destruct and initialisation of objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// ECM_OBJCONSTRUCT - this is a message to create a new object.
		case ECM_OBJCONSTRUCT: {
			oBaseComponent * lvObject;
			
			// first try and see if we have an existing NV object for this
			lvObject = (oBaseComponent *) ECOfindNVObject(pECI->mOmnisInstance, lParam);
			if (lvObject!=NULL) {
				// nothing more to do...
				return qtrue;
			}
			
			// see if we can instantiate this as a non-visual component
			lvObject = gXCompLib->instantiateObject(pECI->mCompId);
			if (lvObject!=NULL) {
				lvObject->init(pHWND); // pHWND is not really needed here but this made it easier to get our class structure in place...
				
				// and insert into a chain of non-visual object. The OMNIS library will maintain this chain
				ECOinsertNVObject( pECI->mOmnisInstance, lParam, (void*)lvObject );
				
				return qtrue;
			};
			
			// finally see if we can instantiate it as a visual component
			lvObject = gXCompLib->instantiateComponent(pECI->mCompId);
			if (lvObject!=NULL) {
				lvObject->init(pHWND);
				
				// and insert into a chain of objects. The OMNIS library will maintain this chain
				ECOinsertObject( pECI, pHWND, (void*)lvObject );
				return qtrue;				
			};

			return qfalse;
		} break;
			
		// ECM_OBJDESTRUCT - this is a message to inform you to delete the object
		case ECM_OBJDESTRUCT: {			
			oBaseComponent* lvObject;

			// See if we can remove this as a visual component
			lvObject = (oBaseComponent*)ECOremoveObject( pECI, pHWND );			
			if ( NULL!=lvObject ) {
				gXCompLib->destructComponent(lvObject, pECI->mCompId);
			}

			// checking for non-visual component we only do if wParam == ECM_WPARAM_OBJINFO
			if (wParam == ECM_WPARAM_OBJINFO) {
				lvObject = (oBaseComponent*)ECOremoveNVObject(pECI->mOmnisInstance, lParam);
				if (lvObject != NULL) {
					gXCompLib->destructComponent(lvObject, pECI->mCompId);
				}				
			}
			
			return qtrue;
		} break;

		// ECM_OBJECT_COPY - this is a message to inform you that a copy of our object is required
		case ECM_OBJECT_COPY: {
			objCopyInfo * lvCopyInfo = (objCopyInfo*) lParam;
			oBaseNVComponent * lvSourceObj = (oBaseNVComponent *) ECOfindNVObject(pECI->mOmnisInstance, lvCopyInfo->mSourceObject);
			if (lvSourceObj != NULL) {
				oBaseNVComponent * lvDestObj = (oBaseNVComponent *) ECOfindNVObject(pECI->mOmnisInstance, lvCopyInfo->mDestinationObject);
				if (lvDestObj == NULL) {
					lvDestObj = gXCompLib->instantiateObject(pECI->mCompId); // hopefully we can trust mCompID here..
					if (lvDestObj != NULL) {
						lvDestObj->init(pHWND);
						ECOinsertNVObject(pECI->mOmnisInstance, lvCopyInfo->mDestinationObject, (void*)lvDestObj);
					};
				};
				if (lvDestObj != NULL) {
					lvDestObj->copyObject(lvSourceObj);
				};
			};
		} break;
		
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// ECM_GETPROPNAME - this message is sent by OMNIS to get information about the properties of an object
		case ECM_GETPROPNAME: { 
			oBaseComponent * lvObject;
			lvObject = gXCompLib->instantiateObject(pECI->mCompId); // try and instantiate a non visual object
			if (lvObject == NULL) {
				lvObject = gXCompLib->instantiateComponent(pECI->mCompId); // no? must be a component
			}
			if (lvObject!=NULL) {
				qlong retVal = ECOreturnProperties( gInstLib, pECI, (ECOproperty *) lvObject->properties()->getArray(), lvObject->propertyCount() );
				
				gXCompLib->destructComponent(lvObject, pECI->mCompId);
				
				return retVal; 				
			};
		}; break;
			
		// ECM_PROPERTYCANASSIGN: Is the property assignable
		case ECM_PROPERTYCANASSIGN: {	
			oBaseComponent* lvObject;
			lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			if (lvObject == NULL) {
				lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
			}
			if (lvObject != NULL) {
				return lvObject->canAssign(ECOgetId(pECI));
			};
		}; break;
			
		// ECM_SETPROPERTY: Assignment to a property
		case ECM_SETPROPERTY:	{	
			EXTParamInfo* lvNewParam = ECOfindParamNum( pECI, 1 );
			
			oBaseComponent* lvObject;
			lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			if (lvObject == NULL) {
				lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
			}
			if ((lvObject != NULL) && (lvNewParam !=NULL)) {
				EXTfldval lvValue( (qfldval)lvNewParam->mData );
				
				return lvObject->setProperty(ECOgetId(pECI), lvValue, pECI);
			};			
		}; break;
		
		// ECM_GETPROPERTY: Retrieve value from property
		case ECM_GETPROPERTY:	{	
			oBaseComponent* lvObject;
			lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			if (lvObject == NULL) {
				lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
			}
			if (lvObject != NULL) {
				EXTfldval lvValue;
				
				lvObject->getProperty(ECOgetId(pECI), lvValue, pECI);
				ECOaddParam(pECI, &lvValue);
				
				return 1L;
			};			
			
		}; break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		case ECM_GETMETHODNAME : {
			oBaseComponent * lvObject;
			lvObject = gXCompLib->instantiateObject(pECI->mCompId); // try and instantiate a non visual object
			if (lvObject == NULL) {
				lvObject = gXCompLib->instantiateComponent(pECI->mCompId); // no? must be a component
			}
			if (lvObject != NULL) {				
				qlong retVal = ECOreturnMethods( gInstLib, pECI, (ECOmethodEvent *) lvObject->methods()->getArray(), lvObject->methodCount() );
				
				gXCompLib->destructComponent(lvObject, pECI->mCompId);
				return retVal; 				
			};
		}; break ;
			
		case ECM_METHODCALL : {
			qlong lvResult;
			qlong methodID = ECOgetId(pECI);
			
			oBaseComponent* lvObject;
			lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			if (lvObject == NULL) {
				lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
			}
			if (lvObject != NULL) { 
				lvResult = lvObject->invokeMethod(methodID, pECI);
			} else {
				// must be static method call
				lvResult = gXCompLib->invokeMethod(methodID, pECI);
			};
			return lvResult;
		}; break;
			
			
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// window messaging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// WM_PAINT - standard paint message
		case WM_PAINT: {
			 // This should only be called on visual object
			 oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			 // and if its good, call the paint function
			 if ( NULL!=lvObject && lvObject->wm_paint(pECI) )
			 return qtrue;
			 
		} break;
			
	}
	
	// As a final result this must ALWAYS be called. It handles all other messages that this component
	// dpECIdes to ignore.
	return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);
}
