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

qComponents gComponents;

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
			return LoadLibrary(&gComponents); // Return external flags
		} break;
			
		// ECM_DISCONNECT - this message is sent only once when the OMNIS session is ending and should not be confused
		// with ECM_OBJDESTRUCT which is sent once per object. ECM_DISCONNECT can be used to free other libraries
		// loaded on ECM_CONNECT or other general global actions that need to be done only once.
		// 
		// For most components this can be removed - see other BLYTH component examples
		case ECM_DISCONNECT: { 
			// should clear out gComponents somehow

			return UnloadLibrary();
		} break;

		// ECM_GETCOMPLIBINFO - this is sent by OMNIS to find out the name of the library, and
		// the number of components this library supports
		case ECM_GETCOMPLIBINFO: {
			// !BAS! Need to initilize our object array here
			return ECOreturnCompInfo( gInstLib, eci, RES_LIBNAME, 1 );
		}

		// ECM_GETCOMPID - this message is sent by OMNIS to get information about each component in this library
		// wParam is a number from 1 to the number of components return on the ECM_GETCOMPLIBINFO message.
		//
		// For each call you should return the internal object ID and the type of object it is.
		// The object id will be used for other calls such as ECM_GETCOMPICON 
		// 
		// The object type is for example : cObjType_Basic 		- a basic window object or 
		//																	cRepObjType_Basic	- a basic report object.
		// 																	There are others 	- see BLYTH examples and headers
		case ECM_GETCOMPID: {
			return ECOreturnCompID( gInstLib, eci, gComponents[wParam-1]->componentID, gComponents[wParam-1]->componentType );
		} break;
			
		// ECM_GETCOMPICON - this is sent by OMNIS to get an icon for the OMNIS component store and
		// external component browser. You need to always supply an icon in your resource file.
		case ECM_GETCOMPICON: {
			// OMNIS will call you once per component for an icon.
			// GENERIC_ICON is defined in the header and included in the resource file
			for (int i=0;i<gComponents.numberOfElements();i++) {
				if (gComponents[i]->componentID == eci->mCompId) {
					return ECOreturnIcon( gInstLib, eci, gComponents[i]->bitmapID );
				}
			}
			
			return qfalse;
		}
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// construct/destruct and initialisation of objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// ECM_OBJCONSTRUCT - this is a message to create a new object.
		case ECM_OBJCONSTRUCT: {
			for (int i=0;i<gComponents.numberOfElements();i++) {
				if (gComponents[i]->componentID == eci->mCompId) {
					// Allocate a new object
					oBaseComponent* object = (oBaseComponent *) gComponents[i]->newObjectFunction(); // ( hwnd )
					// and insert into a chain of objects. The OMNIS library will maintain this chain
					ECOinsertObject( eci, hwnd, (void*)object );
					return qtrue;					
				}
			}

			return qfalse;
		} break;
			
		// ECM_OBJDESTRUCT - this is a message to inform you to delete the object
		case ECM_OBJDESTRUCT: {
			// !BAS! need to alter this to destruct the object properly
			
			// First find the object in the libraries chain of objects, 
			// this call if ok also removes the object from the chain.
			oBaseComponent* object = (oBaseComponent*)ECOremoveObject( eci, hwnd );
			if ( NULL!=object )
			{
				// Now you can delete the object you previous allocated
				// Note: The hwnd passed on ECM_OBJCONSTRUCT should not be deleted, as
				// it was created and will be destroyed by OMNIS
				delete object;
			}
			return qtrue;
		} break;
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// window messaging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// WM_PAINT - standard paint message
/*		case WM_PAINT: {
			 // First find the object in the libraries chain of objects
			 tqfGenericObject* object = (tqfGenericObject*)ECOfindObject( eci, hwnd );
			 // and if its good, call the paint function
			 if ( NULL!=object && object->paint() )
			 return qtrue;
			 
		} break; */
			
	}
	
	// As a final result this must ALWAYS be called. It handles all other messages that this component
	// decides to ignore.
	return WNDdefWindowProc(hwnd,Msg,wParam,lParam,eci);
}
