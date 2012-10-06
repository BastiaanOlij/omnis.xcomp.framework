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

#endif