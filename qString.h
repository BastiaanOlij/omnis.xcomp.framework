/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  qString.h
 *  Our own version of a string object based on qchar, variable lenght NOT limited to 256 characters like Omnis' own str255..
 *
 *  Bastiaan Olij
 */

#ifndef qstringh
#define qstringh

#include "omnis.xcomp.framework.h"

class qstring {
private:
	qchar			*mBuffer;
	qlong			mMaxSize;
	
	void			redim(qlong pSize, qbool pKeepData = qfalse);		/* resize the buffer */
	void			copy(const char *pString);							/* copy an 8bit string */
#ifdef isunicode
	void			copy(const qchar *pString);							/* copy zero terminated omnis string */
#endif
	void			copy(const qchar *pString, qlong pSize);			/* copy an omnis string */
	
protected:
	
public:
	qstring();
	qstring(qlong pSize);
	qstring(const qstring& pCopy);
	qstring(const char* pCopy);
#ifdef isunicode
	qstring(const qchar* pCopy);
#endif
	qstring(const qchar *pString, qlong pSize);
	~qstring();

	static qstring * newStringFromFromat(const char *pFormat, ...);
	static qlong	qstrlen(const qchar *pString);
	static qshort	qstrcmp(const qchar *pA, const qchar *pB);

	const qchar*	cString() const;
	qlong			length() const;
	
	qstring&		appendStyle(qchar pStyle, qulong pValue);
	qstring&		appendFormattedString(const char *pFormat, ...);
	qstring&		appendBinary(const qbyte *pBuffer, qlong pLen);
	qstring&		appendFldVal(EXTfldval &value);
	
	qchar*			operator[](qlong pIndex);

	qstring&		operator=(const qstring& pCopy);
	qstring&		operator=(const qchar* pCopy);
	qstring&		operator+=(const qstring& pAdd);
	qstring&		operator+=(const qchar* pAdd);

	/* Some inline operators */
	const qstring	operator+(const qstring& pAdd) const {
		qstring result = *this;
		result += pAdd;
		return result;
	};
	
	bool			operator==(const qstring& pCompare) const;	
	bool			operator!=(const qstring& pCompare) const;	
	bool			operator<=(const qstring& pCompare) const;
	bool			operator>=(const qstring& pCompare) const;
	bool			operator<(const qstring& pCompare) const;
	bool			operator>(const qstring& pCompare) const;
};

#endif