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
	qchar			*mBuffer;
	qlong			mMaxSize;
#ifdef isunicode
	qbyte			*mReturnStr;
#endif
	
	void			init();												/* initialise members */
	void			redim(qlong pSize, qbool pKeepData = qfalse);		/* resize the buffer */
#ifdef isunicode
	void			copy(const char *pString);							/* copy an 8bit string */
#endif
	void			copy(const qchar *pString);							/* copy zero terminated omnis string */
	void			copy(const qchar *pString, qlong pSize);			/* copy an omnis string */
	void			copy(const EXTfldval &pExtFld);						/* copy an omnis field value */
	
protected:
	
public:
	qstring();
	qstring(qlong pSize);
	qstring(const qstring& pCopy);
#ifdef isunicode
	qstring(const char* pCopy);
#endif
	qstring(const qchar* pCopy);
	qstring(const qchar *pString, qlong pSize);
	qstring(const EXTfldval &pExtFld);
	~qstring();

	static qstring * newStringFromFormat(const char *pFormat, ...);
	static qlong	qstrlen(const qchar *pString);
	static qshort	qstrcmp(const qchar *pA, const qchar *pB);

	const qchar*	cString() const;
	const char *	c_str();
	qlong			length() const;

	qstring&		setFormattedString(const char *pFormat, ...);
	
	qstring&		appendStyle(qchar pStyle, qulong pValue);
	qstring&		appendFormattedString(const char *pFormat, ...);
	qstring&		appendBinary(const qbyte *pBuffer, qlong pLen);
	qstring&		appendFldVal(const EXTfldval &value);
	
	qchar*			operator[](qlong pIndex);

	qstring&		operator=(const qstring& pCopy);
	qstring&		operator=(const qchar* pCopy);
	qstring&		operator=(const EXTfldval& pCopy);
	
	qstring&		operator+=(const qstring& pAdd);
	qstring&		operator+=(const qchar pAdd);
	qstring&		operator+=(const qchar* pAdd);
	qstring&		operator+=(const EXTfldval& pAdd);

	/* Some inline operators */
	const qstring	operator+(const qstring& pAdd) const {
		qstring result = *this;
		result += pAdd;
		return result;
	};
	
	bool			operator==(const qchar * pCompare) const;	
	bool			operator==(const qstring& pCompare) const;	
	bool			operator!=(const qstring& pCompare) const;	
	bool			operator<=(const qstring& pCompare) const;
	bool			operator>=(const qstring& pCompare) const;
	bool			operator<(const qstring& pCompare) const;
	bool			operator>(const qstring& pCompare) const;
};

#endif