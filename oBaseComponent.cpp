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

/**********************************************************************************
 * oBaseComponent
 **********************************************************************************/

// constructor
oBaseComponent::oBaseComponent(void){
	// nothing to do here just yet
};

// destructor
oBaseComponent::~oBaseComponent(void){
	// nothing to do here just yet
};

// initialize object
qbool oBaseComponent::init(qapp pApp) {
	mApp = pApp;

	return qtrue;
};

/*** some nice support function ***/

// Add string to trace log
void oBaseComponent::addToTraceLog(qstring &pData) {
	qstring lvOut;

	for (qulong lvIndex = 0; lvIndex < pData.length(); lvIndex++) {
		qchar lvChar = pData[lvIndex];
		if (lvChar == '\r') {
			// ignore.
		} else if (lvChar == '\n') {
			if (lvOut.length() > 0) {
				str255 lvAddText(lvOut.cString());
				ECOaddTraceLine(&lvAddText);
				lvOut = "";
			};
		} else {
			lvOut += lvChar;

			if (lvOut.length() == 250) {
				str255 lvAddText(lvOut.cString());
				ECOaddTraceLine(&lvAddText);
				lvOut = "";
			};
		};
	};

	if (lvOut.length() > 0) {
		str255 lvAddText(lvOut.cString());
		ECOaddTraceLine(&lvAddText);
		lvOut = "";
	};
};

// Add formatted string to trace log
void oBaseComponent::vAddToTraceLog(const char *pData, va_list pArgList) {
	qstring lvTrace;
	qstring lvFormat(pData);

	qstring::vAppendFormattedString(lvTrace, &lvFormat, pArgList);

	oBaseComponent::addToTraceLog(lvTrace);
};

// Add formatted string to trace log
void oBaseComponent::addToTraceLog(const char *pData, ...) {
	va_list lvArgList;

	va_start(lvArgList, pData);
	vAddToTraceLog(pData, lvArgList);
	va_end(lvArgList);
};

#ifdef isunicode
// Add formatted string to trace log
void oBaseComponent::addToTraceLog(const qoschar *pData, ...) {
	qstring lvTrace;
	qstring lvFormat(pData);
	va_list lvArgList;

	va_start(lvArgList, pData);
	qstring::vAppendFormattedString(lvTrace, &lvFormat, lvArgList);
	va_end(lvArgList);

	oBaseComponent::addToTraceLog(lvTrace);
};
#endif

// Convert a string of up to 8 characters from hex to qlong
qlong oBaseComponent::HexToLong(const qchar *pText) {
	qlong retVal = 0;
	qshort pos = 0;

	while (pText[pos] != 0 && pos < 8) {
		qchar hex = pText[pos];
		qlong addVal = 0;

		if (hex >= '0' && hex <= '9') {
			addVal = hex - '0';
		} else if (hex >= 'a' && hex <= 'f') {
			addVal = hex - 'a' + 10;
		} else if (hex >= 'A' && hex <= 'F') {
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
qbool oBaseComponent::copyFldVal(EXTfldval &pSource, EXTfldval &pDest) {
	// no idea why this isn't a standard function in Omnis.. or maybe I'm overlooking it...

	ffttype valuetype;
	qshort valuesubtype;

	pSource.getType(valuetype, &valuesubtype);

	if (pSource.isNull()) {
		pDest.setNull(valuetype, valuesubtype);
	} else if (pSource.isEmpty()) {
		pDest.setEmpty(valuetype, valuesubtype);
	} else {
		switch (valuetype) {
			case fftCharacter: {
				qlong len = pSource.getCharLen();
				qlong reallen = 0;
				qchar *buffer = (qchar *)MEMmalloc(sizeof(qchar) * (len + 1));

				if (buffer == NULL) {
					return qfalse;
				}

				// and copy!
				pSource.getChar(len + 1, buffer, reallen, qfalse);
				pDest.setChar(buffer, reallen);

				MEMfree(buffer);
			}; break;
			case fftInteger: {
				pDest.setLong(pSource.getLong());
			}; break;
			case fftNumber: {
				qreal number;
				qbool error;
				pSource.getNum(number, valuesubtype, &error);
				if (error != qtrue) {
					return qfalse;
				};
				pDest.setNum(number, valuesubtype);
			}; break;
			case fftBinary:
			case fftPicture: {
				qlong len = pSource.getBinLen();
				qlong reallen = 0;
				qbyte *buffer = (qbyte *)MEMmalloc(sizeof(qbyte) * (len + 1));

				if (buffer == NULL) {
					return qfalse;
				};

				pSource.getBinary(len, buffer, reallen);
				pDest.setBinary(valuetype, buffer, reallen, valuesubtype);

				MEMfree(buffer);
			}; break;
			case fftRow: {
				EXTqlist *copyList = pSource.getList(qtrue); // create a copy
				if (copyList == NULL) {
					pDest.setEmpty(valuetype, valuesubtype);
				} else {
					pDest.setList(copyList, qtrue, qtrue);
					delete copyList; // destruct our object BUT do not assign qnil as our EXTfldval now owns it
				};
			}; break;
			case fftList: {
				EXTqlist *copyList = pSource.getList(qtrue); // create a copy
				if (copyList == NULL) {
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
const qoschar *oBaseComponent::fldTypeName(ffttype pType) {
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
EXTfldval *oBaseComponent::newCalculation(qstring &pCalculation, EXTCompInfo *pECI) {
	qlong error1, error2;
	EXTfldval *calcFld = new EXTfldval;

	qret ret = calcFld->setCalculation(pECI->mLocLocp, ctyCalculation, (qchar *)pCalculation.cString(), pCalculation.length(), &error1, &error2);
	if (ret != 0) {
		// error1 => error2 will be the substring of the part of the calculation that is wrong.

		char errorstr[16000];
		strcpy(errorstr, pCalculation.c_str());
		errorstr[error2 + 1] = 0x00;
		addToTraceLog("Error in calculation : %s", &errorstr[error1]);

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
qProperties *oBaseComponent::properties(void) {
	qProperties *lvProperties = new qProperties();

	// Our base class has not properties

	return lvProperties;
};

// return true/false if a property can be written too
qbool oBaseComponent::canAssign(qlong pPropID) {
	return true; // assume assignable
};

// set the value of a property (return true if property was handled, false if Omnis needs to do its thing..)
qbool oBaseComponent::setProperty(qlong pPropID, EXTfldval &pNewValue, EXTCompInfo *pECI) {
	return false;
};

// get the value of a property (return true if property was handled, false if Omnis needs to do its thing..)
qbool oBaseComponent::getProperty(qlong pPropID, EXTfldval &pGetValue, EXTCompInfo *pECI) {
	return false;
};

/*** Methods ***/

// Static function, return array of method meta data
// Each subclass of our component must implement this and add their methods into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qMethods *oBaseComponent::methods(void) {
	qMethods *lvMethods = new qMethods();

	// our base class has no methods

	return lvMethods;
};

// invoke a method
int oBaseComponent::invokeMethod(qlong pMethodId, EXTCompInfo *eci) {
	// nothing to do here by default

	return 1L;
};

/*** Events ***/

// Static function, return array of event meta data
// Each subclass of our component must implement this and add their events into this definition
// See oBaseNVComponent and oBaseVisComponent for examples
qEvents *oBaseComponent::events(void) {
	qEvents *lvEvents = new qEvents();

	// our base class has no events

	return lvEvents;
};

/*** Parameters ***/

// get c-string from parameter, call needs to delete returned object
char *oBaseComponent::newCStringFromParam(int pParamNo, EXTCompInfo *pECI, char *pBuffer, int pBufLen) {
	static char buffer[1024];
	int maxlen = pBuffer == NULL ? 1024 : pBufLen;
	char *result = pBuffer == NULL ? buffer : pBuffer;

	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *tmpParam = ECOfindParamNum(pECI, pParamNo);
		EXTfldval tmpFldVal((qfldval)tmpParam->mData);
		qstring *tmpNewString = new qstring(tmpFldVal);

		if (tmpNewString->length() < maxlen - 1) {
			strcpy(result, tmpNewString->c_str());
		} else {
			memcpy(result, tmpNewString->c_str(), maxlen - 1);
			result[maxlen - 1] = 0;
		}
		delete tmpNewString;
	} else {
		result[0] = 0;
	};

	return result;
};

const std::string oBaseComponent::getStringFromParam(int pParamNo, EXTCompInfo *pECI) {
	std::string new_string;

	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *param_info = ECOfindParamNum(pECI, pParamNo);
		EXTfldval fld_val((qfldval)param_info->mData);

		qlong len = fld_val.getCharLen();
		if (len > 0) {
			// resize our string so we don't get to many reallocs, we assume there won't be many higher character that take up more then 1 character.
			new_string.reserve(len + 15);

			// create a big enough buffer to hold our data
			qchar *omnis_string = (qchar *)MEMmalloc(sizeof(qchar) * (len + 15));
			if (omnis_string != NULL) {
				char utf8[15];

				qlong real_len;
				fld_val.getChar(len + 1, omnis_string, real_len);
				omnis_string[real_len] = 0;

				for (long i = 0; i < real_len; i++) {
#if OMNISSDK >= 81
					len = CHRunicode::charToUtf8(&omnis_string[i], 1, (qbyte *)utf8);
#else
					len = CHRunicode::charToEncodedCharacters(qtrue, &omnis_string[i], 1, (qbyte *)utf8);
#endif
					utf8[len] = 0;
					new_string += utf8;
				}

				MEMfree(omnis_string);
			}
		}
	}

	return new_string;
}

const std::vector<qbyte> oBaseComponent::getBinaryFromParam(int pParamNo, EXTCompInfo *pECI) {
	std::vector<qbyte> data;

	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *param_info = ECOfindParamNum(pECI, pParamNo);
		EXTfldval fld_val((qfldval)param_info->mData);

		qlong len = fld_val.getBinLen();
		if (len > 0) {
			data.resize(len);

			fld_val.getBinary(len, data.data(), len);
		}
	}

	return data;
}

// get string from parameter, call needs to delete returned object
qstring *oBaseComponent::newStringFromParam(int pParamNo, EXTCompInfo *pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *tmpParam = ECOfindParamNum(pECI, pParamNo);
		EXTfldval tmpFldVal((qfldval)tmpParam->mData);
		qstring *tmpNewString = new qstring(tmpFldVal);

		return tmpNewString;
	} else {
		qstring *tmpNewString = new qstring(QTEXT(""));

		return tmpNewString;
	};
};

// get long from parameter
long oBaseComponent::getLongFromParam(int pParamNo, EXTCompInfo *pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *tmpParam = ECOfindParamNum(pECI, pParamNo);
		EXTfldval tmpFldVal((qfldval)tmpParam->mData);

		return tmpFldVal.getLong();
	} else {
		return 0;
	}
};

// get boolean from parameter
bool oBaseComponent::getBoolFromParam(int pParamNo, EXTCompInfo *pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *tmpParam = ECOfindParamNum(pECI, pParamNo);
		EXTfldval tmpFldVal((qfldval)tmpParam->mData);

		return tmpFldVal.getBool() == 2;
	} else {
		return 0;
	}
};

// get qlist from parameter, caller needs to delete return object
EXTqlist *oBaseComponent::newQListFromParam(int pParamNo, EXTCompInfo *pECI) {
	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *tmpParam = ECOfindParamNum(pECI, pParamNo);
		EXTfldval tmpFldVal((qfldval)tmpParam->mData);
		EXTqlist *tmpList = tmpFldVal.getList(qfalse);

		return tmpList;
	} else {
		return 0;
	};
};

// get binary buffer, returns NULL if the buffer is empty
char *oBaseComponent::newBinfromParam(int pParamNo, size_t *pLen, EXTCompInfo *pECI) {
	char *tmpBuffer = NULL;
	qlong tmpLen = 0;

	if (ECOgetParamCount(pECI) >= pParamNo) {
		EXTParamInfo *tmpParam = ECOfindParamNum(pECI, pParamNo);
		EXTfldval tmpFldVal((qfldval)tmpParam->mData);

		tmpLen = tmpFldVal.getBinLen();
		if (tmpLen > 0) {
			tmpBuffer = (char *)malloc(tmpLen);
			if (tmpBuffer != NULL) {
				qlong tmpRealLen;
				memset(tmpBuffer, 0, tmpLen); // JIC

				tmpFldVal.getBinary(tmpLen, (qbyte *)tmpBuffer, tmpRealLen);
			} else {
				tmpLen = 0;
			};
		};
	};

	*pLen = tmpLen;
	return tmpBuffer;
};

// get list variable by name
EXTqlist *oBaseComponent::getNamedList(qstring &pName, EXTCompInfo *pECI) {
	str255 listName;
	ffttype datatype;
	qshort datasubtype;

	if (pName.length() == 0) {
		addToTraceLog("No list name specified");
		return NULL;
	};

	/* copy our name */
	listName = pName.cString();

	/* and now get the field related to this.. */
	EXTfldval dataField(listName, qfalse, pECI->mLocLocp);

	/* check the type of our variable */
	dataField.getType(datatype, &datasubtype);
	if (datatype == fftItemref) {
		/* this is an item reference, lets parse the item reference, thanks to TL tech support */
		EXTfldval calc, result;

		/* Add .$fullname to our reference */
		listName.concat(str255(QTEXT(".$fullname")));

		/* execute this as a calculation to get the name of the variable our reference points at */
		calc.setCalculation(pECI->mInstLocp, ctyCalculation, &listName[1], listName[0]);
		calc.evalCalculation(result, pECI->mInstLocp);
		listName = result.getChar(); /* this will return something like $root.$iwindows.myWindow.$objs.mySubWindow.$ivars.ivList */

		/* now our variable could be an instance variable for a subwindow. Our $fullname reference actually doesn't work then.. So lets check for this situation.. */
		qshort ivarspos = listName.pos(str255(QTEXT(".$ivars.")));
		qshort objspos = listName.pos(str255(QTEXT(".$objs.")));
		if ((ivarspos != 0) && (objspos != 0)) {
			/*
			 if we're dealing with an instance variable and we have $objs in our dataname this must be a subwindow, we need to add $subinst..
			 
			 so it becomes $root.$iwindows.myWindow.$objs.mySubWindow.$subinst().$ivars.ivList
			 */

			listName.insert(str255(QTEXT(".$subinst()")), ivarspos);
		};

		/* and now get our real data variable */
		EXTfldval referencedField(listName, qfalse, pECI->mLocLocp);
		referencedField.getType(datatype, &datasubtype);
		if ((datatype == fftList) || (datatype == fftRow)) {
			/* list or row? return the list */
			return referencedField.getList(qfalse);
		} else {
			addToTraceLog("Item reference isn't a list or row");
			return NULL;
		};
	} else if ((datatype == fftList) || (datatype == fftRow)) {
		/* list or row? return the list */
		return dataField.getList(qfalse);
	} else {
		addToTraceLog("%qs isn't a list or row", &pName);
		return NULL;
	};
};
