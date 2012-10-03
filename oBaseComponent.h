/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseComponent.h
 *  Base classes to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Bastiaan Olij
 */

#ifndef obasecomponenth
#define obasecomponenth

#include "omnis.xcomp.framework.h"

/* our base class for all component objects */
class oBaseComponent {
	
}

/* baseclass for non visual components */
class oBaseNVComponent : public oBaseComponent {
	
};

/* baseclass for visual components */
class oBaseVisComponent : public oBaseComponent {
	
};

#endif