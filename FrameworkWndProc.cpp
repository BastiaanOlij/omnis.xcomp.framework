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
 *
 *  Todos:
 *  - find a way to prevent having to pass ECI along to everything
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

// Include our framework files
#include "omnis.xcomp.framework.h"

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
			qECOobjects * lvObject = gXCompLib->objects();
			
			return ECOreturnObjects( gInstLib, pECI, (ECOobject *) lvObject->getArray(), (qshort) lvObject->numberOfElements());
		} break;

		// ECM_GETCOMPLIBINFO - this is sent by OMNIS to find out the name of the library, and
		// the number of components this library supports
		case ECM_GETCOMPLIBINFO: {
			return ECOreturnCompInfo( gInstLib, pECI, gXCompLib->getResourceID(), (qshort) gXCompLib->numberOfvisComps() );
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
			OXFcomponent lvComponent = gXCompLib->visCompByIndex(wParam-1);
			if (lvComponent.componentType!=0) {
				return ECOreturnCompID( gInstLib, pECI, lvComponent.componentID, lvComponent.componentType );				
			}
		} break;
			
		// ECM_GETCOMPICON - this is sent by OMNIS to get an icon for the OMNIS component store and
		// external component browser. You need to always supply an icon in your resource file.
		case ECM_GETCOMPICON: {
			// OMNIS will call you once per component for an icon.
			// GENERIC_ICON is defined in the header and included in the resource file
			OXFcomponent lvComponent = gXCompLib->componentByID(pECI->mCompId);
			if (lvComponent.componentType!=0) {
				return ECOreturnIcon( gInstLib, pECI, lvComponent.bitmapID );				
			}
			
			return qfalse;
		} break;
			
		case ECM_GETVERSION: {
			qshort	major = gXCompLib->major();
			qshort	minor = gXCompLib->minor();
			
			return ECOreturnVersion(major, minor);
		} break;
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// construct/destruct and initialisation of objects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// ECM_OBJCONSTRUCT - this is a message to create a new object.
		case ECM_OBJCONSTRUCT: {
			oBaseComponent *	lvObject;
			
			if (pHWND==0) {
				// Try and see if we have an existing NV object for this
				lvObject = (oBaseComponent *) ECOfindNVObject(pECI->mOmnisInstance, lParam);
				if (lvObject!=NULL) {
					// nothing more to do...
					return qtrue;
				};
			};

			// instantiate our component
			lvObject = gXCompLib->instantiateComponent(pECI->mCompId, pECI, pHWND, lParam);
			if (lvObject!=NULL) {
				return qtrue;				
			};

			return qfalse;
		} break;
			
		// ECM_OBJDESTRUCT - this is a message to inform you to delete the object
		case ECM_OBJDESTRUCT: {			
			oBaseComponent* lvObject;

			// See if we can remove this as a visual component
			if (pHWND!=0) {
				lvObject = (oBaseComponent*)ECOremoveObject( pECI, pHWND );			
				if (lvObject != NULL ) {
					delete lvObject;
				};
			} else if (wParam == ECM_WPARAM_OBJINFO) {
				// checking for non-visual component we only do if wParam == ECM_WPARAM_OBJINFO
			
				lvObject = (oBaseComponent*)ECOremoveNVObject(pECI->mOmnisInstance, lParam);
				if (lvObject != NULL) {
					delete lvObject;
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
					lvDestObj = (oBaseNVComponent *)gXCompLib->instantiateComponent(pECI->mCompId, pECI, pHWND, lvCopyInfo->mDestinationObject); // hopefully we can trust mCompID here..
				};
				if (lvDestObj != NULL) {
					lvDestObj->copyObject(lvSourceObj);
				};
			};
		} break;

		// ECM_OBJECT_REBUILD is send to query if the non-visual object needs a rebuild
		case ECM_OBJECT_REBUILD: {
			oBaseNVComponent * lvObject = (oBaseNVComponent *) ECOfindNVObject(pECI->mOmnisInstance, lParam);
			if (lvObject != NULL) {
				return lvObject->ecm_object_rebuild(pECI); 
			};
		}; break;
		
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// properties
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		// ECM_GETPROPNAME - this message is sent by OMNIS to get information about the properties of an object
		case ECM_GETPROPNAME: { 
			qProperties * lvProperties = gXCompLib->properties(pECI->mCompId);
			if (lvProperties!=NULL) {
				qlong retVal = ECOreturnProperties( gInstLib, pECI, (ECOproperty *) lvProperties->getArray(), (qshort) lvProperties->numberOfElements() );
				
				return retVal; 				
			}; 
		}; break;
			
		// ECM_PROPERTYCANASSIGN: Is the property assignable
		case ECM_PROPERTYCANASSIGN: {	
			oBaseComponent* lvObject;
			if (pHWND==0) {
				lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			} else {
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
			if (pHWND==0) {
				lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			} else {
				lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
			}
			if ((lvObject != NULL) && (lvNewParam !=NULL)) {
				EXTfldval lvValue( (qfldval)lvNewParam->mData );
				
				if (lvObject->setProperty(ECOgetId(pECI), lvValue, pECI)) {
                    return 1L;
                };

                // let Omnis do its default logic...
			};
		}; break;
		
		// ECM_GETPROPERTY: Retrieve value from property
		case ECM_GETPROPERTY:	{	
			oBaseComponent* lvObject;
			if (pHWND==0) {
				lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
			} else {
				lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
			}
			if (lvObject != NULL) {
				EXTfldval lvValue;
				
				if (lvObject->getProperty(ECOgetId(pECI), lvValue, pECI)) {
                    ECOaddParam(pECI, &lvValue);
				
                    return 1L;
                };
                
                // let Omnis do its default logic...
			};
			
		}; break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Our special $dataname property
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		// ECM_SETPRIMARYDATA: The contents of our $dataname property has changed
		case ECM_SETPRIMARYDATA: {
			EXTParamInfo* lvNewParam = ECOfindParamNum( pECI, 1 );

			oBaseVisComponent* lvObject = (oBaseVisComponent *)ECOfindObject( pECI, pHWND );
			if ((lvObject != NULL) && (lvNewParam !=NULL)) {
				EXTfldval lvValue( (qfldval)lvNewParam->mData );
				
				if (lvObject->setPrimaryData(lvValue) == qtrue) {
					return 1L;
				} else {
					return 0L;
				};
			};			
		}; break;
			
		// ECM_GETPRIMARYDATA: Omnis would like to get a copy of our data. 
		case ECM_GETPRIMARYDATA: {
			oBaseVisComponent* lvObject = (oBaseVisComponent *)ECOfindObject( pECI, pHWND );
			if (lvObject != NULL) {
				EXTfldval lvValue;
				
				if (lvObject->getPrimaryData(lvValue) == qtrue) {
					ECOaddParam(pECI, &lvValue);								
					return 1L;					
				} else {
					return 0L;
				};
			};			
		}; break;
			
		// ECM_CMPPRIMARYDATA: Omnis would like to know if our copy of the data has changed
		case ECM_CMPPRIMARYDATA: {
			EXTParamInfo* lvNewParam = ECOfindParamNum( pECI, 1 );

			oBaseVisComponent* lvObject = (oBaseVisComponent *)ECOfindObject( pECI, pHWND );
			if ((lvObject != NULL) && (lvNewParam !=NULL)) {
				EXTfldval lvValue( (qfldval)lvNewParam->mData );
				return lvObject->cmpPrimaryData(lvValue);
			};			
		}; break;
			
		// ECM_GETPRIMARYDATALEN: Omnis would like to know how big our copy of the data is
		case ECM_GETPRIMARYDATALEN: {
			oBaseVisComponent* lvObject = (oBaseVisComponent *)ECOfindObject( pECI, pHWND );
			if (lvObject != NULL) {
				qlong datalen = lvObject->getPrimaryDataLen();
				if (datalen>=0) {					
					EXTfldval lvValue;
					
					lvValue.setLong(datalen);
					ECOaddParam(pECI, &lvValue);
					
					return 1L;
				} else {
					return 0L;
				};
			};			
		}; break;
			
		// ECM_PRIMARYDATACHANGE: lets our component know the primary data has changed
		case ECM_PRIMARYDATACHANGE: {
			oBaseVisComponent* lvObject = (oBaseVisComponent *)ECOfindObject( pECI, pHWND );
			if (lvObject != NULL) {
				lvObject->primaryDataHasChanged();
				return 1L;
			};			
			
		}; break;
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		case ECM_GETMETHODNAME : {
			qMethods * lvMethods = gXCompLib->methods(pECI->mCompId);
			if (lvMethods != NULL) {				
				qlong retVal = ECOreturnMethods( gInstLib, pECI, (ECOmethodEvent *) lvMethods->getArray(), (qshort) lvMethods->numberOfElements() );
				
				return retVal; 				
			};
		}; break ;
			
		case ECM_METHODCALL : {
			qlong lvResult;
			qlong methodID = ECOgetId(pECI);

			oBaseComponent* lvObject;
			if ((pHWND==0) && (pECI->mOmnisInstance==0)) {
				// must be static method call, need to test this
				lvResult = gXCompLib->invokeMethod(methodID, pECI);
			} else {
				if (pHWND==0) {
					lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam); // first try and see if this is an NV object
				} else {
					lvObject = (oBaseComponent *)ECOfindObject( pECI, pHWND ); // No? must be a visual component
				}
				if (lvObject != NULL) { 
					lvResult = lvObject->invokeMethod(methodID, pECI);

					if (pHWND==0) {
						lvObject = (oBaseComponent *)ECOfindNVObject(pECI->mOmnisInstance, lParam);
					};
				} else {
					str255	lvStr(QTEXT("Method call on unknown object"));
					ECOaddTraceLine(&lvStr);
				};
			};
			return lvResult;
		}; break;
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Events
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		case ECM_GETEVENTMETHOD: {
			unsigned int lvID = gXCompLib->eventMethodID(pECI->mCompId);
			if (lvID>0) {
				return ECOreturnEventMethod(gInstLib, pECI, lvID);
			};
		}; break;
			
		case ECM_GETEVENTNAME : {
			qEvents * lvEvents = gXCompLib->events(pECI->mCompId);
			if (lvEvents != NULL) {
				qlong retVal = ECOreturnEvents( gInstLib, pECI, (ECOmethodEvent *) lvEvents->getArray(), (qshort) lvEvents->numberOfElements() );
				
				return retVal; 				
			};
		};  break;
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// (Drop)List object functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		case ECM_PAINTCONTENTS: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
				EXTListLineInfo* lineInfo = (EXTListLineInfo *)lParam;
				
				if (lvObject->ecm_paintcontents(lineInfo, pECI)) {
					return 1L;
				};
			};						
		};

		case ECM_LISTDRAWLINE: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
				EXTListLineInfo* lineInfo = (EXTListLineInfo *)lParam;
				
				if (lvObject->ecm_listdrawline(lineInfo, pECI)) {
					return 1L;
				};
			};			
		}; break;
			
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// window messaging
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		case WM_KEYDOWN:
		case WM_KEYUP: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
				qkey *	keyMessage = (qkey *) lParam;
				if (lvObject->evKeyPressed(keyMessage, pMsg==WM_KEYDOWN, pECI)) {
					return 0L;
				} else {
					return 1L;
				};
			};
		}; break;
			
		// WM_HSCROLL, WM_VSCROLL - scrollbar position has changed
		case WM_HSCROLL:
		case WM_VSCROLL: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
				qdim		min, max, page, oldPos, newPos, otherPos, stepSize;
				
				if (pMsg == WM_HSCROLL) {
					stepSize = lvObject->getHorzStepSize();
					WNDgetScrollPos( pHWND, SB_HORZ, &oldPos ); 
					WNDgetScrollPos( pHWND, SB_VERT, &otherPos ); 
					WNDgetScrollRange( pHWND, SB_HORZ, &min, &max, &page );					
				} else {
					stepSize = lvObject->getVertStepSize();
					WNDgetScrollPos( pHWND, SB_VERT, &oldPos ); 
					WNDgetScrollPos( pHWND, SB_HORZ, &otherPos ); 
					WNDgetScrollRange( pHWND, SB_VERT, &min, &max, &page );
				};
				
				switch ( wParam ) {
					case SB_LINEDOWN: 
						newPos = oldPos + stepSize; 
						break;
					case SB_LINEUP: 
						newPos = oldPos - stepSize; 
						break;
					case SB_PAGEDOWN: 
						newPos = oldPos + page; 
						break;
					case SB_PAGEUP: 
						newPos = oldPos - page; 
						break;
					case SB_TOP:	
						newPos = min;
						break;
					case SB_BOTTOM: 
						newPos = max; 
						break;
					case SB_THUMBPOSITION: 
					case SB_THUMBTRACK: {
						// handle sign extension correctly
						qshort shortNewPos = LOWORD( lParam ); 
						newPos = shortNewPos; 
						break;
					};
					case SB_ENDSCROLL:
					default:
						newPos = oldPos;
						break;
				};
				
				if (newPos!=oldPos) {	
					if (newPos < min) newPos=min;
					if (newPos > max) newPos=max;
					
					if (pMsg == WM_HSCROLL) {
						lvObject->evWindowScrolled(newPos, otherPos);
					} else {
						lvObject->evWindowScrolled(otherPos, newPos);						
					};
				};
				
				return 1L;
			};			
		}; break;
		
		// WM_SETCURSOR - gets sent when the mouse is above our control and Omnis wants to know what cursor to show
		case WM_SETCURSOR: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
				qword2	hittest = LOWORD(lParam);
				qpoint	pt;
				
				// get our mouse position and map it to our control so 0,0 is the left-top of our control
				WNDgetCursorPos(&pt);
				WNDmapWindowPoint(HWND_DESKTOP, pHWND, &pt);
				
				HCURSOR	cursor = lvObject->getCursor(pt, hittest);
				if (cursor != WND_CURS_DEFAULT) {
					WNDsetCursor(cursor);
					
					return 1L;					
				} else {
					// let omnis do its thing...
				};
			};
		}; break;
			
		// WM_LBUTTONDOWN - standard left mouse button down event
		case WM_LBUTTONDOWN: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			// and if its good, call the function
			if (lvObject!=NULL) {
				// let our object know that we have pressed our mouse down.
				qpoint pt;
                
                // get some info about our mouse
				WNDmakePoint( lParam, &pt );
                
                if (!lvObject->mouseIsOver()) {
                    // our mouse is not over our component, we must have captured it..
				} else if (lvObject->wm_lbutton(pt, true, pECI)) { // only if we return true do we round this off, if false we assume default logic for Omnis.
					// capture our mouse
					if (!WNDhasCapture(pHWND, WND_CAPTURE_MOUSE)) {
//                        oBaseComponent::addToTraceLog("Mouse captured on mouse down");
						WNDsetCapture(pHWND, WND_CAPTURE_MOUSE);
					};
					return 0L;
				};
			};		
		} break;
			
		// WM_LBUTTONUP - standard left mouse button up event
		case WM_LBUTTONUP: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			// and if its good, call the function
			if (lvObject!=NULL) {
				qpoint pt; 
                
                // get some info about our mouse
				WNDmakePoint( lParam, &pt );
                
				if (lvObject->wm_lbutton(pt, false, pECI)) { // only if we return true do we round this off, if false we assume default logic for Omnis.
					return 0L;
				};
                
                // note, if we've captured the mouse on our mouse down, our next call to mouse move will clean things up nicely....
			};				
		} break;
			
		// WM_LBUTTONDBLCLK - user double clicked
		case WM_LBUTTONDBLCLK: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			// and if its good, call the function
			if (lvObject!=NULL) {
				qpoint pt; 
				WNDmakePoint( lParam, &pt );
				
				if (lvObject->wm_lbDblClick(pt, pECI)) {
					return 0L;					
				};
			}
		}; break;

		// WM_RBUTTONDOWN - standard right mouse button down event
		case WM_RBUTTONDOWN:
		// WM_RBUTTONUP - standard right mouse button up event
		case WM_RBUTTONUP: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			// and if its good, call the function
			if (lvObject!=NULL) {
				
				// let our object know that we have pressed our mouse down.
				qpoint pt; 
				WNDmakePoint( lParam, &pt );
				
				if (lvObject->wm_rbutton(pt, pMsg == WM_RBUTTONDOWN, pECI)) {
					return 0L;					
				};
			};		
		} break;
			
			
		// WM_MOUSEMOVE - mouse has been moved while it is above us or we have captured the mouse
		case WM_MOUSEMOVE: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
            
			// and if its good, call the function
			if (lvObject!=NULL) {
                HWND   mouse_over = NULL;
				qpoint pt, pt_desktop;
                qword2 hittest;
                bool   captured = WNDhasCapture(pHWND, WND_CAPTURE_MOUSE);
                
                // get some info about our mouse
				WNDmakePoint( lParam, &pt );
                pt_desktop = pt;
                WNDmapWindowPoint(pHWND, HWND_DESKTOP, &pt_desktop);
                WNDgetWindowFromPt(&mouse_over, &hittest, &pt_desktop);
                
                bool mouseIsOver = (mouse_over == pHWND) && (hittest != HTNOWHERE);
                
                // mouse moved
                lvObject->wm_mousemove(pt, pECI, mouseIsOver);

                // check if we need to keep our capture
				if (captured && !mouseIsOver && !WNDmouseLeftButtonDown()) {
					WNDreleaseCapture(WND_CAPTURE_MOUSE);
//                    oBaseComponent::addToTraceLog("Mouse released on mouse moved out");
				} else if (!captured && mouseIsOver) {
                    WNDsetCapture(pHWND, WND_CAPTURE_MOUSE);
//                    oBaseComponent::addToTraceLog("Mouse captured on mouse moved over");
                };
				
				return 0L;
			};	
		} break;
			
		// WM_DRAGDROP - events related to dragging and dropping
		case WM_DRAGDROP: {
			// only handle if we're not in design mode, else leave it up to Omnis.
			if (!ECOisDesign(pHWND)) {
				oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
				if (lvObject!=NULL) {
					qlong retval = lvObject->wm_dragdrop(wParam, lParam, pECI);
					if (retval != -1) {
						return retval;
					};
				};
			};
		}; break;
        
        // WM_GETERASEINFO -  get info for drawing non client area background
        case WM_GETERASEINFO: {
			 // This should only be called on visual object
			 oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			 if (lvObject!=NULL) {
				 if (lvObject->wm_geteraseinfo((WNDeraseInfoStruct*) lParam, pECI)) {
					 return 1L;					 
				 };
			 };
        } break;

		// WM_ERASEBKGND - erase the background
		case WM_ERASEBKGND: {
			 // This should only be called on visual object
			 oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			 // and if its good, call the erase function
			 if (lvObject!=NULL) {
				 if (lvObject->wm_erasebkgnd(pECI)) {
					 return 1L;					 
				 };
			 };
		} break;
			
		// WM_PAINT - standard paint message
		case WM_PAINT: {
			 // This should only be called on visual object
			 oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			 // and if its good, call the paint function
			 if (lvObject!=NULL) {
				 if (lvObject->wm_paint(pECI)) {
					 return 1L;
				 };
			 } ;
		} break;
			
		// WM_WINDOWPOSCHANGED - inform that the position or size of our visual component has changed
		case WM_WINDOWPOSCHANGED: {
			// This should only be called on visual object
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			// and if its good, call the paint function
			if (lvObject!=NULL) {
				lvObject->wm_windowposchanged(pECI, (WNDwindowPosStruct *) lParam); 
				return 1L;
			} 			
		} break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// undocumented but handy!
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // 1026 - $enabled or $active set to true in runtime INCLUDING if this action is done on a container!
        case 1026: {
            // must do our standard logic..
			qlong result = WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);

            /* Can't do anything with this yet, Omnis is not telling us whether $enabled or $active is being set so we have no reliable way of knowing how to react here...
            
			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
                oBaseComponent::addToTraceLog("%li: active/enabled changed to true w: %li, l: %li", pHWND, wParam, lParam);
            };
            
            */
            
            // do call a redraw on our object just in case it wants to change the way it draws...
            WNDinvalidateRect(pHWND, NULL);

            return result;
        } break;

        // 1027 - $enabled or $active set to false in runtime INCLUDING if this action is done on a container!
        case 1027: {
            // must do our standard logic..
			qlong result = WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);

            /* Can't do anything with this yet, Omnis is not telling us whether $enabled or $active is being set so we have no reliable way of knowing how to react here...

			oBaseVisComponent* lvObject = (oBaseVisComponent*)ECOfindObject( pECI, pHWND );
			if (lvObject!=NULL) {
                oBaseComponent::addToTraceLog("%li: active/enabled changed to false w: %li, l: %li", pHWND, wParam, lParam);
            };
            
            */

            // do call a redraw on our object just in case it wants to change the way it draws...
            WNDinvalidateRect(pHWND, NULL);
        
            return result;
        } break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// place holders for later implementation..
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // WM_CREATE - create an object
        case WM_CREATE: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;
        
        // WM_DESTROY - destroy an object
        case WM_DESTROY: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;
        
        // WM_SHOWWINDOW - make object visible
        case WM_SHOWWINDOW: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;

        // WM_WINDOWPOSCHANGING - moving component around (WM_WINDOWPOSCHANGED is called once finished)
        case WM_WINDOWPOSCHANGING: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;

        // ECM_OBJINITIALIZE - Sent during object construction (before set property WPARAM=0; after WPARAM=1)
        case ECM_OBJINITIALIZE: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;
        
        // ECM_PROPERTYCALCTYPE - Request for the Calculation type of an attribute // mt40455
        case ECM_PROPERTYCALCTYPE: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;

        // ECM_INBUILT_OVERRIDE - sent by Omnis to find out if our object wants to maintain certain build in properties itself
        case ECM_INBUILT_OVERRIDE: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unknowns worth looking into
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        case 1037: { // 0x40D seems to be called when the mouse moves in design time
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;
        
        case 1051: { // 0x41B seems to be called regularly
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;
        
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// omnis internal calls that we need to pass through..
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // WM_NULL - no message?
        case WM_NULL: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break ;

        // ECM_OBJECTDATABLOCK - Setting object properties with a data block
        case ECM_OBJECTDATABLOCK: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
        } break;

		// ECM_MEMORYDELETION tells our xcomp to free up memory
		case ECM_MEMORYDELETION: {
			return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);			
		} break;

		default: {
/* uncomment for debugging
			qstring		lvString;
			lvString.appendFormattedString("Call not implemented: %li", pMsg);
			str255		lvStr(lvString.cString());
			ECOaddTraceLine(&lvStr); */
		}; break;	
	};
	
	// As a final result, if we haven't handled our message this must be called. It handles all other messages that this component
	// decided to ignore.
	return WNDdefWindowProc(pHWND,pMsg,wParam,lParam,pECI);
}
