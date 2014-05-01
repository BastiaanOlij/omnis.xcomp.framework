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

// initialise members
void	qstring::init() {
	mBuffer=0;
	mMaxSize=0;
#ifdef isunicode
	mReturnStr=0;
#endif
};

// Initialize as empty string
qstring::qstring() {
	init();
};

// Initialize as empty string with a buffer of size pSize
qstring::qstring(qlong pSize) {
	init();
	
	redim(pSize);
};

// Initialize and copy from another qstring
qstring::qstring(const qstring& pCopy) {
	init();
	
	*this = pCopy;
};

#ifdef isunicode
// On non-unicode qchar and char are the same thing so these are not needed

// Initialize and copy from a UTF-8 string (zero terminated)
qstring::qstring(const char *pString) {
	init();
	
	copy(pString);
};

// Initialize and copy from a native platform string (usually UTF-16, zero terminated)
qstring::qstring(const qoschar *pString) {
	init();
	
	copy(pString);
};

#endif

// Initialize and copy from an omnis string (zero terminated)
qstring::qstring(const qchar *pString) {
	init();
	
	str255 msg(QTEXT("string"));
	ECOaddTraceLine(&msg);
	
	copy(pString);
};

// Initialize and copy from an omnis string with specific length
qstring::qstring(const qchar *pString, qlong pSize) {
	init();
	
	copy(pString, pSize);
};
// Initialize and copy an omnis field value
qstring::qstring(const EXTfldval &pExtFld) {
	init();
	
	copy(pExtFld);	
};

// Free up memory and destruct
qstring::~qstring() {
	if (mBuffer!=0) {
		MEMfree(mBuffer);
		
		mBuffer=0;
		mMaxSize=0;
	};
	
#ifdef isunicode
	if (mReturnStr!=0) {
		MEMfree(mReturnStr);
		mReturnStr = 0;
	};
#endif
	
};

// Create a new qstring instance based on a formatted string
qstring * qstring::newStringFromFormat(const char *pFormat, ...)
{
	qstring *   retString;
	va_list		arglist;

	va_start( arglist, pFormat );

#ifdef isunicode
	qstring		format(pFormat);

	retString = new qstring();
	qstring::vAppendFormattedString(*retString, format, arglist);
#else
	char		tmpBuffer[2048]; // hopefully 2048 is large enough...
	vsprintf( tmpBuffer, pFormat, arglist );
	retString = new qstring((qchar *)tmpBuffer); // qchar = char
#endif
	va_end( arglist );
	
	return retString;
};

#ifdef isunicode

// This is a simplified version of vprintf but then for qstring/qchar
// We'll build on this eventually
void	qstring::vAppendFormattedString(qstring &appendTo, qstring &pFormat, va_list & pArgList) {
	qlong	lvPos = 0;
	qlong	lvLen = pFormat.length();
	
	while (lvPos < lvLen) {
		qchar	lvChar = pFormat[lvPos];
		if (lvChar == '%') {
			lvPos++;
			lvChar = pFormat[lvPos];
			
			if ((lvChar=='%') || (lvChar==' ') || (lvPos==lvLen)) {
				appendTo += lvChar;
				lvPos++;
			} else {
				qstring	lvFormat("%");
				
				// should really change this to a regex or something.. poor mans implementation to detect %[flags][width][.precision][length]specifier
				// we do NOT support * (multi parameter formatted strings)!
				bool	lvIsPrefix	= true;	
				bool	lvIsFirst	= true;
				
				while (lvPos < lvLen) {
					if (lvIsFirst && ((lvChar=='-') || (lvChar=='+') || (lvChar==' ') || (lvChar == '#'))) {
						// first character is a flag? that is allowed!
						lvFormat += lvChar;
					} else if (lvIsPrefix && lvChar == '.') {
						lvFormat += lvChar;
					} else if (lvIsPrefix && lvChar >= '0' && lvChar <= '9') {
						lvFormat += lvChar;
					} else if (lvChar >= 'a' && lvChar <= 'z') {
						lvFormat += lvChar;
						lvIsPrefix = false;
					} else if (lvChar >= 'A' && lvChar <= 'Z') {
						lvFormat += lvChar;
						lvIsPrefix = false;
					} else {
						// we found the end
						break;
					};
					
					// on to the next character
					lvIsFirst = false;
					lvPos++;
					lvChar = pFormat[lvPos];
				};
				
				switch (lvFormat[lvFormat.length()-1]) {
/*					case 'i': {
						char	lvBuffer[256];
						qlong	lvVal = va_arg(pArgList, qlong);
						sprintf(lvBuffer, lvFormat.c_str(), lvVal); // cheat, use sprintf..
						appendTo += lvBuffer;
					}; break;
					case 'f': {
						char	lvBuffer[256];
						double	lvVal = va_arg(pArgList, double);
						sprintf(lvBuffer, lvFormat.c_str(), lvVal); // cheat, use sprintf..
						appendTo += lvBuffer;
					}; break;
 */
					case 'c': { // no need for any overhead for characters
						char	lvVal = va_arg(pArgList, int);
						if (lvVal!=0) {							
							appendTo += lvVal;
						};
					}; break;
					case 's': { // and we can handle strings just fine too
						if (lvFormat[lvFormat.length()-2]=='q') {
							qstring * lvVal = va_arg(pArgList, qstring *);
							if (lvVal!=0) {
								appendTo += *lvVal;								
							};
						} else {
							char *	lvVal = va_arg(pArgList, char *);
							if (lvVal!=0) {
								appendTo += lvVal;
							};
						};
					}; break;
					default: { // anything else, leave it up to vsprintf to handle this properly...
						char	lvBuffer[1024*16];
						
						vsprintf(lvBuffer, lvFormat.c_str(), pArgList);						
						appendTo += lvBuffer;
						
						// and discard it for ourselves
						int discard = va_arg(pArgList, int); 
					}; break;
				};
				
			};			
		} else {
			// just add
			appendTo += lvChar;
			lvPos++;
		};		
	};
};

// Create a new qstring instance based on a formatted string
qstring * qstring::newStringFromFormat(const qoschar *pFormat, ...) {
	qstring *   retString = new qstring();
	qstring		format(pFormat);
	va_list		arglist;
	
	va_start( arglist, pFormat );	
	qstring::vAppendFormattedString(*retString, format, arglist);
	va_end( arglist );
	
	return retString;
};

// Create a new qstring instance based on a formatted string
qstring * newStringFromFormat(qstring &pFormat, ...) {
	qstring *   retString = new qstring();
	va_list		arglist;
	
	va_start( arglist, pFormat );	
	qstring::vAppendFormattedString(*retString, pFormat, arglist);
	va_end( arglist );
	
	return retString;
	
};
#endif

// Get the length of a qoschar string (UTF-16)
qlong qstring::qosstrlen(const qoschar *pString) {
	qlong len = 0;
	
	if (pString != 0) { // ignore NULL pointers
		while (pString[len]!=0x00) {
			len++;
		};		
	};
	
	return len;	
};

// Get the length of an omnis string
qlong qstring::qstrlen(const qchar *pString){
	qlong len = 0;
	
	if (pString != 0) { // ignore NULL pointers
		while (pString[len]!=0x00) {
			len++;
		};		
	};
	
	return len;
};

// Compare two omnis strings
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
	};
};


/********************************************************************************************************************************
 * Info
 ********************************************************************************************************************************/

// return a pointer to our string (UTF-32)
const qchar*	qstring::cString() const {
	if (mBuffer!=0) {
		return mBuffer;
	} else {
		static qchar	emptyString[1];
		
		emptyString[0] = 0;
		
		return emptyString;
	};
};

// return a pointer to our string (UTF-8)
const char *	qstring::c_str() {
	if (mBuffer!=0) {
#ifdef isunicode
		if (mReturnStr!=0) {
			MEMfree(mReturnStr);
			mReturnStr = 0;
		};
		
		long	tmpLen = length();

		mReturnStr = (qbyte *) MEMmalloc(UTF8_MAX_BYTES_PER_CHAR * (tmpLen+1));		
		if (mReturnStr == 0) {
			static char	emptyString[] = "";		
			return emptyString;			
		} else if (tmpLen == 0) {
			mReturnStr[0]='\0'; // Make sure we zero terminate the string!
		} else {
			// a UTF8 string can be up to 6 bytes per character so we may be allocating WAY to much memory here...
			long	tmpRealLen = CHRunicode::charToUtf8(mBuffer, tmpLen, mReturnStr);						
			mReturnStr[tmpRealLen]='\0'; // Make sure we zero terminate the string!
		};
	
		return (char *) mReturnStr;
#else	
		return (char *) mBuffer; // it's already a c string...
#endif
	} else {
		static char	emptyString[] = "";		
		return emptyString;
	};	
};

// return the length our our string in characters
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

// resize the buffer
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

#ifdef isunicode
// on non-unicode char and qchar are the same so we don't need this...

// copy a UTF-8 string (zero terminated)
void	qstring::copy(const char *pString){
	qlong		len = strlen(pString);
		
	if (len==0) {
		if (mBuffer!=0) {
			mBuffer[0]=0;
		};
	} else {
		/* 
		 on unicode we need to convert our 8bit string!
		 Use CHRconvFromBytes, assumes UTF-8 content 
		*/
		CHRconvFromBytes	newString((qbyte *)pString, len);
		
		copy(newString.dataPtr(), newString.len());
	};
};

// copy a native platform string (usually UTF-16, zero terminated)
void	qstring::copy(const qoschar *pString){
	qlong		len = qstring::qosstrlen(pString);
	
	if (len==0) {
		if (mBuffer!=0) {
			mBuffer[0]=0;
		};
	} else {
		/* 
		 on unicode we need to convert our platform string!
		 Use CHRconvFromOs, assumes UTF-16 content 
		 */
		CHRconvFromOs	newString((qoschar *)pString, len);
		
		copy(newString.dataPtr(), newString.len());
	};
};
#endif

// copy an omnis string (zero terminated)
void	qstring::copy(const qchar *pString){
	qlong		len = qstring::qstrlen(pString); // could have used OMstrlen...
	
	if (len==0) {
		if (mBuffer!=0) {
			mBuffer[0]=0;
		};
	} else {
		if ((len+1)>mMaxSize) { /* not enough space? Redim! */
			redim(len+1);
		};
		
		OMstrcpy(mBuffer, pString);
	};
};

// copy an omnis string of specific length
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

// copy an omnis field value
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

// find position of character within string (returns -1 if not found)
qlong	qstring::pos(qchar pChar) const {
	qlong	pos = 0;

	if (mBuffer!=0) {
		while (mBuffer[pos]!=0) {
			if (mBuffer[pos] == pChar) {
				return pos;
			};
			pos++;
		};
	};
	
	return -1;
};

// get the substring (pLen <= 0 is from end)
qstring qstring::mid(qlong pFrom, qlong pLen) const {
	qstring	newstring;
	
	if (mBuffer!=0) {
		if (pLen <= 0) {
			pLen = length() - pFrom - pLen;
		};

		if (pLen>0) {
			newstring.redim(pLen+1, false);
			
			while ((mBuffer[pFrom]!=0) && (pLen > 0)) {
				newstring += mBuffer[pFrom];
				
				pFrom++;
				pLen--;
			};			
		};
	};
	
	return newstring;
};

// replace one string with another
void	qstring::replace(const char & pWhat, char & pWith) {
	replace(qstring(pWhat), qstring(pWith));
};

// replace one string with another
void	qstring::replace(const qstring & pWhat, const qstring & pWith) {
	qlong	ourlen = this->length();
	qstring newstring(ourlen);
	qlong	whatlen = pWhat.length();
	qlong	pos = 0;
	
	while (pos < ourlen) {
		if (pos > ourlen - whatlen) {
			newstring += mBuffer[pos];
			pos++;
		} else if (memcmp(&mBuffer[pos], pWhat.cString(), whatlen * sizeof(qchar))==0) {
			newstring += pWith;
			pos += whatlen;
		} else {
			newstring += mBuffer[pos];
			pos++;
		};
	};
	
	// and copy our new string in place
	*this = newstring;
};

// Sets the contents of our string to a formatted string
qstring& qstring::setFormattedString(const char *pFormat, ...) {
	char		tmpBuffer[2048]; // hopefully 2048 is large enough...
	va_list		arglist;
	
	va_start( arglist, pFormat );
	vsprintf( tmpBuffer, pFormat, arglist );
	va_end( arglist );
	
#ifdef isunicode	
	*this = tmpBuffer;
#else
	*this = (qchar *)tmpBuffer;
#endif

	return *this; // return ourselves
};

// append an omnis string of specific length
qstring&	qstring::appendString(const qchar *pString, qlong pSize) {
	qlong	ourLen = this->length();
	
	if (pSize==0) {
		// ignore, there is nothing to add..
	} else if (ourLen==0) {
		// just copy...
		copy(pString, pSize);		
	} else {
		// Append together
		
		if ((ourLen+pSize+1)>mMaxSize) {
			// make space for our complete buffer
			redim(ourLen+pSize+1,qtrue);
		};
		
		if ((ourLen+pSize+1)<=mMaxSize) { /* always verify if our redim succeeded */
			// we use copy mem, this should also be safe if we're adding ourselves to ourselves..
			memcpy(&mBuffer[ourLen], pString, pSize * sizeof(qchar));
			mBuffer[ourLen+pSize] = 0x00;
		};		
	};
	
	return *this;
};

// Adds an Omnis style character into our string (like style(...) in omnis)
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

// Append a formatted string to our string
qstring& qstring::appendFormattedString(const char *pFormat, ...) {
	qstring		format(pFormat);
	va_list		arglist;
	
	va_start( arglist, pFormat );
	
#ifdef isunicode	
	qstring::vAppendFormattedString(*this, format, arglist);
#else
	char		tmpBuffer[2048]; // hopefully 2048 is large enough...
	vsprintf( tmpBuffer, pFormat, arglist );
	*this += (qchar *)tmpBuffer;
#endif

	va_end( arglist );

	return *this; // return ourselves
};

#ifdef isunicode
// Append a formatted string to our string
qstring&	qstring::appendFormattedString(const qoschar *pFormat, ...) {
	qstring		format(pFormat);
	va_list		arglist;
	
	va_start( arglist, pFormat );
	qstring::vAppendFormattedString(*this, format, arglist);
	va_end( arglist );
	
	return *this; // return ourselves	
};

// Append a formatted string to our string
qstring&	qstring::appendFormattedString(qstring &pFormat, ...) {
	va_list		arglist;
	
	va_start( arglist, pFormat );
	qstring::vAppendFormattedString(*this, pFormat, arglist);
	va_end( arglist );
	
	return *this; // return ourselves		
};
#endif


// Append a binary to our string (as 0x0123456789ABCDEF)
qstring& qstring::appendBinary(const qbyte *pBuffer, qlong pLen) {
	char hex[10];
	
	for (qlong i=0; i<pLen;i++) {
		qbyte byte = pBuffer[i];
		
		sprintf(hex, "%02X", byte);
		
#ifdef isunicode	
		*this += hex;
#else
		*this += (qchar *)hex;
#endif
		
	};

	return *this;
};

// Append an omnis field value to our string
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
					*this += QTEXT("???");	
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

// Get access to character at a specific location
qchar &	qstring::operator[](qlong pIndex) {
	static qchar returnChar;		// we return this only if our index is out of bounds. Protects against crashes, does nothing against bugs :)
	
	if (mBuffer==0) {
		returnChar=0;
		return returnChar;
	} else {
		// We allow access to any character within our buffer, even those after the end of our string
		if (pIndex < mMaxSize) {
			return mBuffer[pIndex];
		} else {
			returnChar=0;
			return returnChar;			
		};
	};
};

// Copy a string into our string
qstring&	qstring::operator=(const qstring& pCopy) {
	if (this!=&pCopy) {
		// no need to copy ourselves...
		copy(pCopy.cString(), pCopy.length());		
	};
	
	return *this;
};

#ifdef isunicode
// Copy a UTF-8 string into our string
qstring&	qstring::operator=(const char* pCopy) {
	copy(pCopy);
	
	return *this;
};

// Copy a platform string into our string
qstring&	qstring::operator=(const qoschar* pCopy) {
	copy(pCopy);
	
	return *this;
};
#endif

// Copy an Omnis string into our string
qstring&	qstring::operator=(const qchar* pCopy) {
	copy(pCopy, qstring::qstrlen(pCopy));		
	
	return *this;
};

// Copy an Omnis field value to our string
qstring&	qstring::operator=(const EXTfldval& pCopy) {
	copy(pCopy);
	
	return *this;
};

// Append a string to our string
qstring&	qstring::operator+=(const qstring& pAdd) {
	appendString(pAdd.cString(), pAdd.length());
	
	return *this;
};

// Append a character to our string
qstring&	qstring::operator+=(const qchar pAdd) {
	appendString(&pAdd, 1);
	
	return *this;
};

#ifdef isunicode
// Append a UTF-8 string to our string
qstring&	qstring::operator+=(const char* pAdd) {
	qstring	addstring(pAdd);
	
	appendString(addstring.cString(), addstring.length());
	
	return *this;
};

// Append a platform string to our string
qstring&	qstring::operator+=(const qoschar* pAdd) {
	qstring	addstring(pAdd);
	
	appendString(addstring.cString(), addstring.length());
		
	return *this;
};
#endif

// Append an omnis string to our string
qstring&	qstring::operator+=(const qchar* pAdd) {
	qlong	addLen = qstring::qstrlen(pAdd);

	appendString(pAdd, addLen);
	
	return *this;
};

// Append an omnis field value to our string
qstring&	qstring::operator+=(const EXTfldval& pAdd) {
	appendFldVal(pAdd);
	
	return *this;
};

// Compare if our string is the same as an omnis string
bool	qstring::operator==(const qchar * pCompare) const {
	const qchar	*strA = this->cString();
	qlong cmp;
	
	if ((strA==0) && (pCompare==0)) {
		cmp = 0;
	} else if (strA==0) {
		cmp = 1;
	} else if (pCompare==0) {
		cmp = -1;
	} else {
		cmp = qstring::qstrcmp(strA, pCompare);
	};
	
	return cmp==0;
};

// Compare if our string is the same as another string
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
		cmp = qstring::qstrcmp(strA, strB);
	};
	
	return cmp==0;
};

// Compare if our string isn't the same as another string
bool	qstring::operator!=(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp!=0;
};

// Compare if our string is "smaller" then or the same as another string
bool	qstring::operator<=(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp<=0;	
};

// Compare if our string is "bigger" then or the same as another string
bool	qstring::operator>=(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp<0;	
};

// Compare if our string is "smaller" then another string
bool	qstring::operator<(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp>=0;		
};

// Compare if our string is "bigger" then another string
bool	qstring::operator>(const qstring& pCompare) const {
	qlong cmp = qstring::qstrcmp(this->cString(), pCompare.cString());
	
	return cmp>0;	
};
