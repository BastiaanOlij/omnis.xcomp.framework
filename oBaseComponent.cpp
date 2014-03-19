/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseComponent.cpp
 *  Base class to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "oBaseComponent.h"

/********************************************************************************************************************************************
 oBaseComponent
 ********************************************************************************************************************************************/

// constructor
oBaseComponent::oBaseComponent(void) {
	// nothing to do here just yet
};

// destructor
oBaseComponent::~oBaseComponent(void) {
	// nothing to do here just yet
};

// initialize object
qbool	oBaseComponent::init(qapp pApp) {
	mApp = pApp;
};

/*** some nice support function ***/

// Add formatted string to trace log
void oBaseComponent::addToTraceLog(const char *pData, ...) {
	char		tmpOutLine[2048];
	uint		tmpLen=0;
	char		lvBuffer[2048]; // hopefully 2048 is large enough to parse our buffer, note that our trace log doesn't support strings longer then 255 characters...
	uint		tmpBufLen;
	va_list		lvArgList;
	
	va_start( lvArgList, pData );
	vsprintf( lvBuffer, pData, lvArgList );
	va_end( lvArgList );
	
	tmpBufLen = strlen(lvBuffer);
	
	// remove trailing newline
	while ((tmpBufLen > 0) && (lvBuffer[tmpBufLen-1]=='\r' || lvBuffer[tmpBufLen-1]=='\n')) {
		tmpBufLen--;
	};
	
	// and output..
	for (uint tmpIndex = 0; tmpIndex<tmpBufLen; tmpIndex++) {
		if (lvBuffer[tmpIndex]=='\r') {
			// ignore...
		} else if ((lvBuffer[tmpIndex]=='\n') || (tmpLen==250)) {
			// output line..
			tmpOutLine[tmpLen]='\0';
			
			// need to convert line to qchar if on unicode!
#ifdef isunicode
			qstring	lvString(tmpOutLine);
			str255 lvAddText(lvString.cString());
#else
			str255 lvAddText((qchar *)tmpOutLine);
#endif
			ECOaddTraceLine(&lvAddText);

			tmpLen=0;
		} else {
			tmpOutLine[tmpLen]=lvBuffer[tmpIndex];
			tmpLen++;
		};
	};
	tmpOutLine[tmpLen]='\0';
		
	// need to convert line to qchar if on unicode!
#ifdef isunicode
	qstring	lvString(tmpOutLine);
	str255 lvAddText(lvString.cString());
#else
	str255 lvAddText((qchar *)tmpOutLine);
#endif
	ECOaddTraceLine(&lvAddText);	
};

// Copy the contents of one EXTFLDVAL into another EXTFLDVAL
qbool	oBaseComponent::copyFldVal(EXTfldval &pSource, EXTfldval &pDest) {
	// no idea why this isn't a standard function in Omnis.. or maybe I'm overlooking it...
	
	ffttype valuetype;
	qshort  valuesubtype;
	
	pSource.getType(valuetype, &valuesubtype);
	
	if (pSource.isNull()) {
		pDest.setNull(valuetype, valuesubtype);
	} else if (pSource.isEmpty()) {
		pDest.setEmpty(valuetype, valuesubtype);
	} else {
		switch (valuetype) {
			case fftCharacter: {
				qlong	len			= pSource.getCharLen();
				qlong	reallen		= 0;
				qchar	*buffer		= (qchar *)MEMmalloc(sizeof(qchar) * (len+1));
				
				if (buffer==NULL) {
					return qfalse;
				}
				
				// and copy!
				pSource.getChar(len+1, buffer, reallen, qfalse);
				pDest.setChar(buffer, reallen);
				
				MEMfree(buffer);				
			}; break;
			case fftInteger: {
				pDest.setLong(pSource.getLong());				
			}; break;
			case fftNumber: {
				qreal	number;
				qbool	error;
				pSource.getNum(number, valuesubtype, &error);
				if (error!=qtrue) {
					return qfalse;
				};
				pDest.setNum(number, valuesubtype);
			}; break;
			case fftBinary:
			case fftPicture: {
				qlong	len			= pSource.getBinLen();
				qlong	reallen		= 0;
				qbyte	*buffer		= (qbyte *)MEMmalloc(sizeof(qbyte) * (len+1));
				
				if (buffer == NULL) {
					return qfalse;
				};
				
				pSource.getBinary(len, buffer, reallen);
				pDest.setBinary(valuetype, buffer, reallen, valuesubtype);
				
				MEMfree(buffer);
			}; break;
			default:
				// for now we do not support the other data types
				return qfalse;
				break;
		}
	};
	
	return qtrue;
};

/*** Properties ***/

// Static function, return array of property meta data
// Each subclass of our component must implement this and add their properties into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qProperties * oBaseComponent::properties(void) {
	qProperties * lvProperties = new qProperties();

	// Our base class has not properties
	
	return lvProperties;
};

// return true/false if a property can be written too
qbool oBaseComponent::canAssign(qlong pPropID) {
	return true; // assume assignable
};

// set the value of a property
qbool oBaseComponent::setProperty(qlong pPropID,EXTfldval &pNewValue,EXTCompInfo* pECI) {
	return false;
};

// get the value of a property
void oBaseComponent::getProperty(qlong pPropID,EXTfldval &pGetValue,EXTCompInfo* pECI) {
	// nothing to do here, base class does not have any properties...
};

/*** Methods ***/

// Static function, return array of method meta data
// Each subclass of our component must implement this and add their methods into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qMethods * oBaseComponent::methods(void) {
	qMethods * lvMethods = new qMethods();
	
	// our base class has no methods

	return lvMethods;
};

// invoke a method
int oBaseComponent::invokeMethod(qlong pMethodId,EXTCompInfo* eci) {
	// nothing to do here by default
	
	return 1L;
};

/*** Events ***/

// Static function, return array of event meta data
// Each subclass of our component must implement this and add their events into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qEvents *	oBaseComponent::events(void) {
	qEvents * lvEvents = new qEvents();
	
	// our base class has no events
	
	return lvEvents;
};


/*** Parameters ***/

// get string from parameter, call needs to delete returned object
qstring	* oBaseComponent::newStringFromParam(int pParamNo, EXTCompInfo* pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
		qstring	*			tmpNewString = new qstring(tmpFldVal);
		
		return tmpNewString;		
	} else {
		qstring	*			tmpNewString = new qstring("");
		
		return tmpNewString;
	};
};

// get long from parameter
long oBaseComponent::getLongFromParam(int pParamNo, EXTCompInfo* pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
		
		return tmpFldVal.getLong();
	} else {
		return 0;
	}
};

// get qlist from parameter, caller needs to delete return object
EXTqlist *	oBaseComponent::newQListFromParam(int pParamNo, EXTCompInfo* pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);
		EXTqlist *			tmpList = tmpFldVal.getList(qfalse);
		
		return tmpList;
	} else {
		return 0;
	};
};

// get binary buffer, returns NULL if the buffer is empty
char * oBaseComponent::newBinfromParam(int pParamNo, size_t *pLen, EXTCompInfo* pECI) {
	char *			tmpBuffer = NULL;
	qlong			tmpLen = 0;

	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo*		tmpParam = ECOfindParamNum( pECI, pParamNo );
		EXTfldval			tmpFldVal((qfldval) tmpParam->mData);

		tmpLen = tmpFldVal.getBinLen();
		if (tmpLen>0) {
			tmpBuffer = (char *) malloc(tmpLen);
			if (tmpBuffer!=NULL) {
				qlong	tmpRealLen;
				memset(tmpBuffer, 0, tmpLen); // JIC
				
				tmpFldVal.getBinary(tmpLen, (qbyte *) tmpBuffer, tmpRealLen);
			} else {
				tmpLen = 0;
			};
		};
	};

	*pLen = tmpLen;
	return tmpBuffer;
};



