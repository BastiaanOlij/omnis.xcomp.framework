/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oBaseComponent.cpp
 *  Base class to subclass components from. These classes implement alot of the basics that each component shares
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - Complete this implementation
 *  - Make addToTraceLog work with qchar (or make a version thereof)
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

	return qtrue;
};

/*** some nice support function ***/

// Add string to trace log
void	oBaseComponent::addToTraceLog(qstring & pData) {
	qstring lvOut;
	
	for (uint lvIndex = 0; lvIndex < pData.length(); lvIndex++) {
		qchar lvChar = pData[lvIndex];
		if (lvChar == '\r') {
			// ignore.
		} else if (lvChar == '\n') {
			if (lvOut.length() > 0) {
				str255 lvAddText(lvOut.cString());
				ECOaddTraceLine(&lvAddText);
				lvOut = "";
			};
		} else  {
			lvOut += lvChar;
			
			if (lvOut.length()==250) {
				str255 lvAddText(lvOut.cString());
				ECOaddTraceLine(&lvAddText);
				lvOut = "";				
			};
		};
	};

	if (lvOut.length()>0) {
		str255 lvAddText(lvOut.cString());
		ECOaddTraceLine(&lvAddText);
		lvOut = "";
	};
};

// Add formatted string to trace log
void oBaseComponent::addToTraceLog(const char *pData, ...) {
	qstring		lvTrace;
	qstring		lvFormat(pData);
	va_list		lvArgList;
	
	va_start( lvArgList, pData );
	qstring::vAppendFormattedString(lvTrace, lvFormat, lvArgList);
	va_end( lvArgList );
	
	oBaseComponent::addToTraceLog(lvTrace);
};

#ifdef isunicode
// Add formatted string to trace log
void	oBaseComponent::addToTraceLog(const qoschar *pData, ...) {
	qstring		lvTrace;
	qstring		lvFormat(pData);
	va_list		lvArgList;
	
	va_start( lvArgList, pData );
	qstring::vAppendFormattedString(lvTrace, lvFormat, lvArgList);
	va_end( lvArgList );
	
	oBaseComponent::addToTraceLog(lvTrace);
};
#endif

// Convert a string of up to 8 characters from hex to qlong
qlong	oBaseComponent::HexToLong(const qchar *pText) {
	qlong	retVal = 0;
	qshort	pos = 0;
	
	while (pText[pos]!=0 && pos<8) {
		qchar	hex = pText[pos];
		qlong	addVal = 0;
		
		if (hex>='0' && hex<='9') {
			addVal = hex - '0';
		} else if (hex>='a' && hex<='f') {
			addVal = hex - 'a' + 10;
		} else if (hex>='A' && hex<='F') {
			addVal = hex - 'A' + 10;			
		} else {
		  // ignore...
		};
		
		retVal = (retVal << 4) + addVal;
	
		pos++;
	};
	
	return retVal;
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
			case fftRow: {
				EXTqlist *copyList = pSource.getList(qtrue); // create a copy
				if (copyList==NULL) {
					pDest.setEmpty(valuetype, valuesubtype);					
				} else {
					pDest.setList(copyList, qtrue, qtrue);
					delete copyList; // destruct our object BUT do not assign qnil as our EXTfldval now owns it
				};				
			}; break;
			case fftList: {
				EXTqlist *copyList = pSource.getList(qtrue); // create a copy
				if (copyList==NULL) {
					pDest.setEmpty(valuetype, valuesubtype);					
				} else {
					pDest.setList(copyList, qtrue, qfalse);
					delete copyList; // destruct our object BUT do not assign qnil as our EXTfldval now owns it
				};				
			}; break;
			case fftItemref: {
//				addToTraceLog("Using an item reference as $dataname is not yet implemented, coming to an external near you soon!");
				
				// !BAS! need to implement this soon and figure out the best way to do so. 
				
				// hopefully this works..
				pDest.setFldVal(pSource.getFldVal());
				pDest.setOmnisData(pSource.isOmnisData());
				pDest.setReadOnly(pSource.isReadOnly());				
			}; break;
			default:
				// for now we do not support the other data types
				return qfalse;
				break;
		}
	};
	
	return qtrue;
};

// get our type
const qoschar *	oBaseComponent::fldTypeName(ffttype pType) {
		switch (pType) {
			case fftNone:
				return QTEXT("None");
				break;
			case fftCharacter:
				return QTEXT("Character");
				break;
			case fftBoolean:
				return QTEXT("Boolean");
				break;
			case fftDate:
				return QTEXT("Date");
				break;
			case fftNumber:
				return QTEXT("Number");
				break;
			case fftInteger:
				return QTEXT("Integer");
				break;
			case fftPicture:
				return QTEXT("Picture");
				break;
			case fftBinary:
				return QTEXT("Binary");
				break;
			case fftList:
				return QTEXT("List");
				break;				
			case fftCrb:
				return QTEXT("CRB");
				break;
			case fftCalc:
				return QTEXT("Calculation");
				break;
			case fftConstant:
				return QTEXT("Constant");
				break;
			case fftRow:
				return QTEXT("Row");
				break;
			case fftObject:
				return QTEXT("Object");
				break;
			case fftObjref:
				return QTEXT("Object reference");
				break;
			// THE FOLLOWING TYPES ARE FOR INTERNAL USE ONLY.
			case fftLong:
				return QTEXT("Long");
				break;				
			case fftSequence:
				return QTEXT("Sequence");
				break;				
			case fftRdef:
				return QTEXT("Record definition");
				break;				
			case fftFieldname:
				return QTEXT("Fieldname");
				break;				
			case fftItemref:
				return QTEXT("Item reference");
				break;
			case fftUnknown:
				return QTEXT("Unknown");
				break;				
			case fftQuickest:
				return QTEXT("Quickest");
				break;				
			case fftNegative:
				return QTEXT("Negative");
				break;
			default:
				return QTEXT("???");
				break;
		};
};

// initialize calculation
EXTfldval *	oBaseComponent::newCalculation(qstring &pCalculation, EXTCompInfo *pECI) {
	qlong		error1,error2;
	EXTfldval *	calcFld = new EXTfldval;
	
	qret ret = calcFld->setCalculation(pECI->mLocLocp, ctyCalculation, (qchar *)pCalculation.cString(), pCalculation.length(), &error1, &error2);
	if (ret != 0) {
		// error1 => error2 will be the substring of the part of the calculation that is wrong. 
		
		char errorstr[16000];
		strcpy(errorstr, pCalculation.c_str());
		errorstr[error2+1]=0x00;
		addToTraceLog("Error in calculation : %s",&errorstr[error1]);
			
		delete calcFld;
		return NULL;
	} else {
		return calcFld;
	};
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
		qstring	*			tmpNewString = new qstring(QTEXT(""));
		
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



