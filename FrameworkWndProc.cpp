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
extern "C" qlong OMNISWNDPROC FrameworkWndProc(HWND hwnd, LPARAM Msg,WPARAM wParam,LPARAM lParam,EXTCompInfo* eci) {
	// Initialize callback tables - THIS MUST BE DONE 
	ECOsetupCallbacks(hwnd, eci);		
	
	switch (Msg) {
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

		// ECM_GETCOMPLIBINFO - this is sent by OMNIS to find out the name of the library, and
		// the number of components this library supports
		case ECM_GETCOMPLIBINFO: {
			return ECOreturnCompInfo( gInstLib, eci, gXCompLib->getResourceID(), gXCompLib->numberOfComponents() );
		}

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
				return ECOreturnCompID( gInstLib, eci, lvComponent->componentID, lvComponent->componentType );				
			}
		} break;
			
		// ECM_GETCOMPICON - this is sent by OMNIS to get an icon for the OMNIS component store and
		// external component browser. You need to always supply an icon in your resource file.
		case ECM_GETCOMPICON: {
			// OMNIS will call you once per component for an icon.
			// GENERIC_ICON is defined in the header and included in the resource file
			OXFcomponent *lvComponent = gXCompLib->componentByID(eci->mCompId);
			if (lvComponent!=NULL) {
				return ECOreturnIcon( gInstLib, eci, lvComponent->bitmapID );				
			}
			
			return qfalse;
		}
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// construct/destruct and initialisation of objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// ECM_OBJCONSTRUCT - this is a message to create a new object.
		case ECM_OBJCONSTRUCT: {
			oBaseComponent * lvObject = gXCompLib->instantiateComponent(eci->mCompId);
			if (lvObject!=NULL) {
				// !BAS! Need to check if we're initializing a visual or non-visual object and react accordingly!
				lvObject->init(hwnd);
				
				// and insert into a chain of objects. The OMNIS library will maintain this chain
				ECOinsertObject( eci, hwnd, (void*)lvObject );
				return qtrue;				
			};

			return qfalse;
		} break;
			
		// ECM_OBJDESTRUCT - this is a message to inform you to delete the object
		case ECM_OBJDESTRUCT: {
			// !BAS! need to alter this to destruct the object properly
			
			// First find the object in the libraries chain of objects, 
			// this call if ok also removes the object from the chain.
			oBaseComponent* lvObject = (oBaseComponent*)ECOremoveObject( eci, hwnd );
			if ( NULL!=lvObject ) {
				// Now you can delete the object you previous allocated
				// Note: The hwnd passed on ECM_OBJCONSTRUCT should not be deleted, as
				// it was created and will be destroyed by OMNIS
				delete lvObject;
			}
			return qtrue;
		} break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// ECM_GETPROPNAME - this message is sent by OMNIS to get information about the properties of an object
		case ECM_GETPROPNAME: { 
			oBaseComponent * lvObject = gXCompLib->instantiateComponent(eci->mCompId);
			if (lvObject!=NULL) {
				qlong retVal = ECOreturnProperties( gInstLib, eci, (ECOproperty *) lvObject->properties()->getArray(), lvObject->propertyCount() );
				
				delete lvObject;
				
				return retVal; 				
			};
		}; break;
			
		// ECM_PROPERTYCANASSIGN: Is the property assignable
		case ECM_PROPERTYCANASSIGN: {	
			oBaseComponent* lvObject = (oBaseComponent*)ECOfindObject( eci, hwnd );
			if (lvObject != NULL) {
				return lvObject->canAssign(ECOgetId(eci));
			};
		}; break;
			
		// ECM_SETPROPERTY: Assignment to a property
		case ECM_SETPROPERTY:	{	
			EXTParamInfo* lvNewParam = ECOfindParamNum( eci, 1 );
			
			oBaseComponent* lvObject = (oBaseComponent*)ECOfindObject( eci, hwnd );
			if ((lvObject != NULL) && (lvNewParam !=NULL)) {
				EXTfldval lvValue( (qfldval)lvNewParam->mData );
				
				return lvObject->setProperty(ECOgetId(eci), lvValue, eci);
			};			
		}; break;
		
		// ECM_GETPROPERTY: Retrieve value from property
		case ECM_GETPROPERTY:	{	
			oBaseComponent* lvObject = (oBaseComponent*)ECOfindObject( eci, hwnd );
			if (lvObject != NULL) {
				EXTfldval lvValue;
				
				lvObject->getProperty(ECOgetId(eci), lvValue, eci);
				ECOaddParam(eci, &lvValue);
				
				return 1L;
			};			
			
		}; break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		case ECM_GETMETHODNAME : {
			oBaseComponent * lvObject = gXCompLib->instantiateComponent(eci->mCompId);
			if (lvObject != NULL) {				
				qlong retVal = ECOreturnMethods( gInstLib, eci, (ECOmethodEvent *) lvObject->methods()->getArray(), lvObject->methodCount() );
				
				delete lvObject;				
				return retVal; 				
			};
		}; break ;
			
		case ECM_METHODCALL : {
			qlong lvResult;
			qlong methodID = ECOgetId(eci);
			
			oBaseComponent* lvObject = (oBaseComponent*)ECOfindObject( eci, hwnd );
			if (lvObject != NULL) { 
				lvResult = lvObject->invokeMethod(methodID, eci);
			} else {
				// must be static method call
				lvResult = gXCompLib->invokeMethod(methodID, eci);
			};
			return lvResult;
		}; break;
			
			
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// window messaging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// WM_PAINT - standard paint message
		case WM_PAINT: {
			 // This should only be called on visual object
			 oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( eci, hwnd );
			 // and if its good, call the paint function
			 if ( NULL!=lvObject && lvObject->wm_paint(eci) )
			 return qtrue;
			 
		} break;
			
	}
	
	// As a final result this must ALWAYS be called. It handles all other messages that this component
	// decides to ignore.
	return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
}
