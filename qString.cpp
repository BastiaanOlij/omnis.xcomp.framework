/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  qString.cpp
 *  Our own version of a string object based on qchar, variable lenght NOT limited to 256 characters like Omnis' own str255..
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "qString.h"

/********************************************************************************************************************************
 * Construct/copy/delete
 ********************************************************************************************************************************/

qstring::qstring() {
	mBuffer=0;
	mMaxSize=0;
};

qstring::qstring(qlong pSize) {
	mBuffer=0;
	mMaxSize=0;
	
	redim(pSize);
};

qstring::qstring(const qstring& pCopy) {
	mBuffer=0;
	mMaxSize=0;
	
	*this = pCopy;
};

qstring::qstring(const char *pString) {
	mBuffer=0;
	mMaxSize=0;
	
	copy(pString);
};

#ifdef isunicode
// On non-unicode qchar and char are the same thing so this is not needed
qstring::qstring(const qchar *pString) {
	mBuffer=0;
	mMaxSize=0;
	
	copy(pString);
};
#endif

qstring::qstring(const qchar *pString, qlong pSize) {
	mBuffer=0;
	mMaxSize=0;
	
	copy(pString, pSize);
};

qstring::qstring(const EXTfldval &pExtFld) {
	mBuffer=0;
	mMaxSize=0;
	
	copy(pExtFld);	
};


qstring * qstring::newStringFromFromat(const char *pFormat, ...)
{
	qstring *   retString;
	char		tmpBuffer[2048]; // hopefully 2048 is large enough...
	va_list		arglist;

	va_start( arglist, pFormat );
	vsprintf( tmpBuffer, pFormat, arglist );
	va_end( arglist );
	
	retString = new qstring(tmpBuffer);
	
	return retString;
};

qstring::~qstring() {
	if (mBuffer!=0) {
		MEMfree(mBuffer);

		mBuffer=0;
		mMaxSize=0;
	};
};

qlong qstring::qstrlen(const qchar *pString){
	qlong len = 0;
	
	if (pString != 0) { // ignore NULL pointers
		while (pString[len]!=0x00) {
			len++;
		};		
	};
	
	return len;
};

qshort qstring::qstrcmp(const qchar *pA, const qchar *pB) {
	qlong pos = 0;
	
	// protect against NULL pointers
	if ((pA==0) && (pB==0)){
		return 0;
	} else if (pA==0) {
		return -1;
	} else if (pB==0) {
		return 1;
	}
	
	while (pA[pos]==pB[pos]) {
		if (pA[pos]==0x00) {
			return 0; // strings match
		};
		pos++;
	};
	
	// note, if one of the strings is shorter 0x00 will be smaller then whatever is in the other string:)
	if (pA[pos] < pB[pos]) {
		return -1;
	} else {
		return 1;
	}
	
};


/********************************************************************************************************************************
 * Info
 ********************************************************************************************************************************/

const qchar*	qstring::cString() const {
	if (mBuffer!=0) {
		return mBuffer;
	} else {
		static qchar	emptyString[1];
		
		emptyString[0] = 0;
		
		return emptyString;
	};
};

const char *	qstring::c_str() const {
	if (mBuffer!=0) {
#ifdef isunicode
		static qbyte	lvReturnStr[UTF8_MAX_BYTES_PER_CHAR*16000];		

		long	tmpLen = length();
		if (tmpLen>16000) {
			tmpLen = 16000;
		};
		
		// a UTF8 string can be up to 6 bytes per character so we may be allocating WAY to much memory here...
		long	tmpRealLen = CHRunicode::charToUtf8(mBuffer, tmpLen, lvReturnStr);			
	
		lvReturnStr[tmpRealLen]='\0'; // Make sure we zero terminate the string!
	
		return (char *) lvReturnStr;
#else	
		return (char *) mBuffer; // it's already a c string...
#endif
	} else {
		static char	emptyString[] = "";
		
		return emptyString;
	};	
};

qlong	qstring::length() const {
	if (mBuffer!=0) {
		return qstring::qstrlen(mBuffer); 
	} else {
		return 0;
	};
};

/********************************************************************************************************************************
 * Modifications
 ********************************************************************************************************************************/

void	qstring::redim(qlong pSize, qbool pKeepData) {
	if (pSize==0) {
		// Nothing to store?? free up our memory!
		if (mBuffer!=0) {
			MEMfree(mBuffer);			
			mBuffer = 0;
			mMaxSize = 0;
		};
	} else {
		qchar	*newBuffer;
		qulong	bufferSize = sizeof(qchar) * pSize;

		newBuffer = (qchar *) MEMmalloc(bufferSize);
		if (newBuffer!=0) { // valid new buffer? Only then we'll recycle our old!
			memset(newBuffer, 0, bufferSize); /* zero out the buffer, lets play nice */
			
			if (mBuffer!=0) {
				// Do we need to copy our old data?
				if (pKeepData) {
					qlong len = this->length() + 1; /* +1, include zero terminator */
					len = len > pSize ? pSize : len;
					memcpy(newBuffer, mBuffer, sizeof(qchar) * len); 
					
					newBuffer[len-1]=0; /* make sure we have a zero terminator, this only applies if we made our buffer smaller */
				};
				
				// Now its time to throw away our old buffer...
				MEMfree(mBuffer);
			};
			
			mBuffer = newBuffer;
			mMaxSize = pSize;
		};
	};
};

void	qstring::copy(const char *pString){
	qlong		len = strlen(pString);
		
	if (len==0) {
		if (mBuffer!=0) {
			mBuffer[0]=0;
		};
	} else {
#ifdef isunicode
		/* 
		 on unicode we need to convert our 8bit string!
		 Use CHRconvFromBytes, assumes UTF-8 content 
		*/
		CHRconvFromBytes	newString((qbyte *)pString, strlen(pString));
		
		copy(newString.dataPtr(), newString.len());
#else
		// on non-unicode we can just copy it old style!
		if ((len+1)>mMaxSize) { /* not enough space? Redim! */
			redim(len+1);
		};
		
		if ((len+1)<=mMaxSize) { /* double check just in case the redim failed... */
			strcpy((char *)mBuffer, pString);
		};
#endif
	};
};

#ifdef isunicode
// on non-unicode char and qchar are the same so we don't need this...
void	qstring::copy(const qchar *pString){
	qlong		len = qstring::qstrlen(pString);
	
	if (len==0) {
		if (mBuffer!=0) {
			mBuffer[0]=0;
		};
	} else {
		OMstrcpy(mBuffer, pString);
	};
};
#endif

void	qstring::copy(const qchar *pString, qlong pSize) {
	if (pString==mBuffer) {
		// Copying ourselves into ourselves? ignore!
	} else if (pSize==0) {
		if (mBuffer!=0) {
			mBuffer[0]=0;
		};
	} else {
		if ((pSize+1)>mMaxSize) { /* not enough space? Redim! */
			redim(pSize+1);
		};
		
		if ((pSize+1)<=mMaxSize) { /* double check just in case the redim failed... */
			memcpy(mBuffer, pString, pSize * sizeof(qchar));
			mBuffer[pSize] = 0; // zero terminate
		};
	};
};

void	qstring::copy(const EXTfldval &pExtFld) {
	qlong				tmpLen, tmpRealLen;

	// ignore const warnings on value, we are not doing any modifications, need to fix this..
	// note, we assume our fldval contains a character string

	// create a buffer large enough for our data
	tmpLen = const_cast<EXTfldval &>(pExtFld).getCharLen() + 12; // getBinLen won't modify our object, Tigerlogic should mark this as a const function
	if (tmpLen>mMaxSize) {
		redim(tmpLen, qtrue);
	};
	
	if (tmpLen<=mMaxSize) { // always double check in case redim failed
		const_cast<EXTfldval &>(pExtFld).getChar(tmpLen, mBuffer, tmpRealLen, qtrue); // getChar won't modify our object, Tigerlogic should mark this as a const function
		tmpLen = tmpRealLen;
		mBuffer[tmpLen]=0; // zero terminate
	};
};

qstring& qstring::setFormattedString(const char *pFormat, ...) {
	char		tmpBuffer[2048]; // hopefully 2048 is large enough...
	va_list		arglist;
	
	va_start( arglist, pFormat );
	vsprintf( tmpBuffer, pFormat, arglist );
	va_end( arglist );
	
	*this = tmpBuffer;
	
	return *this; // return ourselves
};


qstring&	qstring::appendStyle(qchar pStyle, qulong pValue) {
	qlong	len = this->length();
	
	if ((len+12)>mMaxSize) {
		// redim our buffer but copy the existing contents!
		redim(len+12, qtrue);
	};
	
	if ((len+12)<=mMaxSize) { // always double check in case redim failed
		// Code based on sample code in Omnis SDK, note that we start at index 0 not index 1 as in the sample code!
		
		mBuffer[len] = txtEsc;
		mBuffer[len+1] = pStyle;
		
		register qchar lookup[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' }; 
		register qchar* add = &mBuffer[ len + 9 ];
		for ( qlong cnt = 1 ; cnt <= 8 ; cnt++ ) {
			*add = lookup[ pValue & 0x0000000F ]; 
			add--; 
			pValue = pValue >> 4;
		};
		
		mBuffer[len+10] = txtAsciiEnd;
		mBuffer[len+11] = 0; // zero terminate
	};
	
	return *this; // return ourselves
};

qstring& qstring::appendFormattedString(const char *pFormat, ...) {
	char		tmpBuffer[2048]; // hopefully 2048 is large enough...
	va_list		arglist;
	
	va_start( arglist, pFormat );
	vsprintf( tmpBuffer, pFormat, arglist );
	va_end( arglist );
	
	*this += tmpBuffer;
	
	return *this; // return ourselves
};

qstring& qstring::appendBinary(const qbyte *pBuffer, qlong pLen) {
	char hex[10];
	
	for (qlong i=0; i<pLen;i++) {
		qbyte byte = pBuffer[i];
		
		sprintf(hex, "%02X", byte);
		
		*this += hex;
	};

	return *this;
};

qstring& qstring::appendFldVal(const EXTfldval &value){
	qbyte *	tmpBuffer;
	qchar	data[2048];
	qlong	len, intvalue;
	ffttype	valueType;
	qshort	valueSubtype;
	
	// ignore const warnings on value, we are not doing any modifications, need to fix this..
	
	const_cast<EXTfldval &>(value).getType(valueType, &valueSubtype); // getType won't modify our object, Tigerlogic should mark this as a const function
	
	switch (valueType) {
		case fftCharacter:
			const_cast<EXTfldval &>(value).getChar(sizeof(data), data, len, qfalse);  // getChar won't modify our object, Tigerlogic should mark this as a const function
			data[len / sizeof(qchar)]=0x00;
			
			*this += data;
			
			break;
		case fftInteger:
			intvalue = const_cast<EXTfldval &>(value).getLong();   // getLong won't modify our object, Tigerlogic should mark this as a const function
			this->appendFormattedString("%i",intvalue);
			break;
		case fftBinary:
			len = const_cast<EXTfldval &>(value).getBinLen();    // getBinLen won't modify our object, Tigerlogic should mark this as a const function
			if (len>0) {
				tmpBuffer = (qbyte *) MEMmalloc(len+1);
				if (tmpBuffer!=0) {					
					const_cast<EXTfldval &>(value).getBinary(len, tmpBuffer, len);   // getBinary won't modify our object, Tigerlogic should mark this as a const function
					
					this->appendBinary(tmpBuffer, len);
					
					MEMfree(tmpBuffer);				
				} else {
					*this += "???";	
				};
			};
			break;
		default:
			// need to implement!!
			break;
	};

	return *this;
};


/********************************************************************************************************************************
 * Operators
 ********************************************************************************************************************************/

qchar*	qstring::operator[](qlong pIndex) {
	static qchar returnChar;		// we return this only if our index is out of bounds. Protects against crashes, does nothing against bugs :)
	
	if (mBuffer==0) {
		returnChar=0;
		return &returnChar;
	} else {
		// We allow access to any character within our buffer, even those after the end of our string
		if (pIndex < mMaxSize) {
			return &mBuffer[pIndex];
		} else {
			returnChar=0;
			return &returnChar;			
		};
	};
};


qstring&	qstring::operator=(const qstring& pCopy) {
	if (this!=&pCopy) {
		// no need to copy ourselves...
		copy(pCopy.cString(), pCopy.length());		
	};
	
	return *this;
};

qstring&	qstring::operator=(const qchar* pCopy) {
	copy(pCopy, qstring::qstrlen(pCopy));		
	
	return *this;
};

qstring&	qstring::operator=(const EXTfldval& pCopy) {
	copy(pCopy);
	
	return *this;
};


qstring&	qstring::operator+=(const qstring& pAdd) {
	qlong	ourLen = this->length();
	qlong	addLen = pAdd.length();
		
	if (addLen==0) {
		// ignore, there is nothing to add..
	} else if (ourLen==0) {
		// just copy...
		copy(pAdd.cString(), addLen);		
	} else {
		// Append together
		
		if ((ourLen+addLen+1)>mMaxSize) {
			// make space for our complete buffer
			redim(ourLen+addLen+1,qtrue);
		};
		
		if ((ourLen+addLen+1)<=mMaxSize) { /* always verify if our redim succeeded */
			// we use copy mem, this should also be safe if we're adding ourselves to ourselves..
			memcpy(&mBuffer[ourLen], pAdd.cString(), (addLen+1) * sizeof(qchar));
		};		
	};
	
	return *this;
};

qstring&	qstring::operator+=(const qchar* pAdd) {
	qlong	ourLen = this->length();
	qlong	addLen = qstring::qstrlen(pAdd);
	
	if (addLen==0) {
		// ignore, there is nothing to add..
	} else if (ourLen==0) {
		// just copy...
		copy(pAdd, addLen);		
	} else {
		// Append together
		
		if ((ourLen+addLen+1)>mMaxSize) {
			// make space for our complete buffer
			redim(ourLen+addLen+1,qtrue);
		};
		
		if ((ourLen+addLen+1)<=mMaxSize) { // always verify if our redim succeeded
			// we use copy mem, this should also be safe if we're adding ourselves to ourselves..
			memcpy(&mBuffer[ourLen], pAdd, (addLen+1) * sizeof(qchar));
		};
	};
	
	return *this;
};

qstring&	qstring::operator+=(const EXTfldval& pAdd) {
	appendFldVal(pAdd);
	
	return *this;
};


bool	qstring::operator==(const qstring& pCompare) const {
	const qchar	*strA = this->cString();
	const qchar	*strB = pCompare.cString();
	qlong cmp;
	
	if ((strA==0) && (strB==0)) {
		cmp = 0;
	} else if (strA==0) {
		cmp = 1;
	} else if (strB==0) {
		cmp = -1;
	} else {
		qlong cmp = qstring::qstrcmp(strA, strB);		
	};
	
	return cmp==0;
};

bool	qstring::operator!=(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp!=0;
};

bool	qstring::operator<=(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp<=0;	
};

bool	qstring::operator>=(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp<0;	
};

bool	qstring::operator<(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp>=0;		
};

bool	qstring::operator>(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp>0;	
};
