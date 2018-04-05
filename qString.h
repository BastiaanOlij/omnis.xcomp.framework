/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  qString.h
 *  Our own version of a string object based on qchar, variable lenght NOT limited to 256 characters like Omnis' own str255..
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#include "xCompStandardIncludes.h"

#ifndef qstringh
#define qstringh

class qstring {
private:
	qchar *mBuffer; // Our buffer in native Omnis char (UTF-32)
	qlong mMaxSize; // Size of our buffer
#ifdef isunicode
	qbyte *mReturnStr; // Buffer for return string when converting
#endif

	void init(); // initialise members
	void redim(qlong pSize, qbool pKeepData = qfalse); // resize the buffer
#ifdef isunicode
	void copy(const char *pString); // copy a UTF-8 string (zero terminated)
	void copy(const qoschar *pString); // copy a native platform string (usually UTF-16, zero terminated)
#endif
	void copy(const qchar *pString); // copy an omnis string (zero terminated)
	void copy(const qchar *pString, qlong pSize); // copy an omnis string of specific length
	void copy(const EXTfldval &pExtFld); // copy an omnis field value

protected:
public:
	qstring(); // Initialize as empty string
	qstring(qlong pSize); // Initialize as empty string with a buffer of size pSize
	qstring(const qstring &pCopy); // Initialize and copy from another qstring
#ifdef isunicode
	qstring(const char *pCopy); // Initialize and copy from a UTF-8 string (zero terminated)
	qstring(const qoschar *pCopy); // Initialize and copy from a native platform string (usually UTF-16, zero terminated)
#endif
	qstring(const qchar *pCopy); // Initialize and copy from an omnis string (zero terminated)
	qstring(const qchar *pString, qlong pSize); // Initialize and copy from an omnis string with specific length
	qstring(const EXTfldval &pExtFld); // Initialize and copy an omnis field value
	~qstring(); // Free up memory and destruct

	static qstring *newStringFromFormat(const char *pFormat, ...); // Create a new qstring instance based on a formatted string
#ifdef isunicode
	static void vAppendFormattedString(qstring &appendTo, qstring &pFormat, va_list pArgList);
	static qstring *newStringFromFormat(const qoschar *pFormat, ...); // Create a new qstring instance based on a formatted string
	static qstring *newStringFromFormat(qstring &pFormat, ...); // Create a new qstring instance based on a formatted string
#endif
	static qlong qosstrlen(const qoschar *pString); // Get the length of a qoschar string
	static qlong qstrlen(const qchar *pString); // Get the length of an omnis string
	static qshort qstrcmp(const qchar *pA, const qchar *pB); // Compare two omnis strings

	const qchar *cString() const; // return a pointer to our string (UTF-32)
	const char *c_str(); // return a pointer to our string (UTF-8)
	qulong length() const; // return the length our our string in characters

	qlong pos(qchar pChar) const; // find position of character within string (returns -1 if not found)
	qstring mid(qlong pFrom, qlong pLen = 0) const; // get the substring (pLen <= 0 is from end)
	void replace(const char &pWhat, char &pWith); // replace one string with another
	void replace(const qstring &pWhat, const qstring &pWith); // replace one string with another

	qstring &setFormattedString(const char *pFormat, ...); // Sets the contents of our string to a formatted string

	qstring &appendString(const qchar *pString, qlong pSize); // Append an omnis string of specific length
	qstring &appendStyle(qchar pStyle, qulong pValue); // Adds an Omnis style character into our string (like style(...) in omnis)
	qstring &appendFormattedString(const char *pFormat, ...); // Append a formatted string to our string
#ifdef isunicode
	qstring &appendFormattedString(const qoschar *pFormat, ...); // Append a formatted string to our string
	qstring &appendFormattedString(qstring &pFormat, ...); // Append a formatted string to our string
#endif
	qstring &appendBinary(const qbyte *pBuffer, qlong pLen); // Append a binary to our string (as 0x0123456789ABCDEF)
	qstring &appendFldVal(const EXTfldval &value); // Append an omnis field value to our string

	qchar &operator[](qlong pIndex); // Get a pointer to character at a specific location

	qstring &operator=(const qstring &pCopy); // Copy a string into our string
#ifdef isunicode
	qstring &operator=(const char *pCopy); // Copy a UTF-8 string into our string
	qstring &operator=(const qoschar *pCopy); // Copy a platform string into our string
#endif
	qstring &operator=(const qchar *pCopy); // Copy an Omnis string into our string
	qstring &operator=(const EXTfldval &pCopy); // Copy an Omnis field value to our string

	qstring &operator+=(const qstring &pAdd); // Append a string to our string
	qstring &operator+=(const qchar pAdd); // Append a character to our string
#ifdef isunicode
	qstring &operator+=(const char *pAdd); // Append a UTF-8 string to our string
	qstring &operator+=(const qoschar *pAdd); // Append a platform string to our string
#endif
	qstring &operator+=(const qchar *pAdd); // Append an omnis string to our string
	qstring &operator+=(const EXTfldval &pAdd); // Append an omnis field value to our string

	/* Some inline operators */
	const qstring operator+(const qstring &pAdd) const {
		qstring result = *this;
		result += pAdd;
		return result;
	};

	bool operator==(const qchar *pCompare) const; // Compare if our string is the same as an omnis string
	bool operator==(const qstring &pCompare) const; // Compare if our string is the same as another string
	bool operator!=(const qstring &pCompare) const; // Compare if our string isn't the same as another string
	bool operator<=(const qstring &pCompare) const; // Compare if our string is "smaller" then or the same as another string
	bool operator>=(const qstring &pCompare) const; // Compare if our string is "bigger" then or the same as another string
	bool operator<(const qstring &pCompare) const; // Compare if our string is "smaller" then another string
	bool operator>(const qstring &pCompare) const; // Compare if our string is "bigger" then another string
};

#endif