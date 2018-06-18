/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  xCompStandardIncludes.h
 *  This include file includes all the header files from our SDK and such
 *
 *  Bastiaan Olij
 *
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#ifndef xcompstandardincludesh
#define xcompstandardincludesh

#ifdef ismach_o
// For MACOSX, Omnis uses some deprectated APIS, ignore for now...
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#ifdef iswin32
#define uint unsigned int
#endif

// make sure we can mark functions as deprecated
#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

// add in our SDK headers
#include <anums.he>
#include <dmconst.he>
#include <extcomp.he>
#include <extfval.he>
#include <gdi.he>
#include <hwnd.he>

// add in some useful standard libraries
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

#define PI 3.14159265

#endif
