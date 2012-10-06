/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  omnis.xcomp.framework.h
 *  This include file simply ensures all required header files are imported into our files
 *
 *  Bastiaan Olij
 */

#ifndef omnisxcompframeworkh
#define omnisxcompframeworkh

#ifdef ismach_o
// For MACOSX, Omnis uses some deprectated APIS, ignore for now...
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// some general declares
#define RES_LIBNAME		1000									/* Resource for library name */

// add in our SDK headers
#include <anums.he>
#include <extcomp.he>
#include <extfval.he>
#include <hwnd.he>
#include <gdi.he>

// add in some useful standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


// add in some useful framework headers
#include "qArray.h"

#define qProperties	qArray<ECOproperty>
#define qMethods	qArray<ECOmethodEvent>

// add in our base component class
#include "oBaseComponent.h"

// add in our magic for our framework to work
struct OXFcomponent {
	int					componentType;						/* Type of component */
	unsigned int		componentID;						/* Resource ID for this component, also the object ID */
	unsigned int		bitmapID;							/* Resource ID for this components bitmap */
	void *				(*newObjectFunction) (void);		/* Function pointer to the function that returns a new instance */
};

#define qComponents	qArray<OXFcomponent>

// these methods should be implemented within our
extern "C" qlong LoadLibrary(qComponents *pComponents);
extern "C" qbool UnloadLibrary(void);

#endif