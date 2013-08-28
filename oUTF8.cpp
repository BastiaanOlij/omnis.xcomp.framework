/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oUTF8.cpp
 *  Helper functions to convert macroman/win-1252 strings to utf8 for non-unicode Omnis
 *
 *  Bastiaan Olij
 *
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */


#include "oUTF8.h"

// With thanks to: http://alienryderflex.com/utf-8/

#if iswin32
// convert win-1252
// Derived from the table at:  http://alanwood.net/demos/ansi.html
unsigned long  charsetToUnicode[128] = {
	8364 ,  129, 8218,  402, 8222, 8230, 8224, 8225,  710, 8240,  352, 8249,  338,  141,   381,   143, // 128 - 143
	144  , 8216, 8217, 8220, 8221, 8226, 8211, 8212,  732, 8482,  353, 8250,  339,  157,   382,   376, // 144 - 159
	160  ,  161,  162,  163,  164,  165,  166,  167,  168,  169,  170,  171,  172,  173,   174,   175, // 160 - 175
	176  ,  177,  178,  179,  180,  181,  182,  183,  184,  185,  186,  187,  188,  189,   190,   191, // 176 - 191
	192  ,  193,  194,  195,  196,  197,  198,  199,  200,  201,  202,  203,  204,  205,   206,   207, // 192 - 207
	208  ,  209,  210,  211,  212,  213,  214,  215,  216,  217,  218,  219,  220,  221,   222,   223, // 208 - 223
	224  ,  225,  226,  227,  228,  229,  230,  231,  232,  233,  234,  235,  236,  237,   238,   239, // 224 - 239
	240  ,  241,  242,  243,  244,  245,  246,  247,  248,  249,  250,  251,  252,  253,   254,   255  // 240 - 255
};
#else
// convert macroman
//  Derived from the table at:  http://alanwood.net/demos/macroman.html
unsigned long  charsetToUnicode[128] = {
	196  ,  197,  199,  201,  209,  214,  220,  225,  224,  226,  228,  227,  229,  231,   233,   232, // 128 - 143
	234  ,  235,  237,  236,  238,  239,  241,  243,  242,  244,  246,  245,  250,  249,   251,   252, // 144 - 159
	8224 ,  176,  162,  163,  167, 8226,  182,  223,  174,  169, 8482,  180,  168, 8800,   198,   216, // 160 - 175
	8734 ,  177, 8804, 8805,  165,  181, 8706, 8721, 8719,  960, 8747,  170,  186,  937,   230,   248, // 176 - 191
	191  ,  161,  172, 8730,  402, 8776, 8710,  171,  187, 8230,  160,  192,  195,  213,   338,   339, // 192 - 207
	8211 , 8212, 8220, 8221, 8216, 8217,  247, 9674,  255,  376, 8260, 8364, 8249, 8250, 64257, 64258, // 208 - 223
	8225 ,  183, 8218, 8222, 8240,  194,  202,  193,  203,  200,  205,  206,  207,  204,   211,   212, // 224 - 239
	63743,  210,  218,  219,  217,  305,  710,  732,  175,  728,  729,  730,  184,  733,   731,   711  // 240 - 255
};
#endif

std::string	oUTF8::convertToUTF8(const qchar *pString) {
#ifdef isunicode
	std::string		tmpResult="";
	long			tmpLen = OMstrlen(pString);
	char *			tmpString = (char *) MEMmalloc((tmpLen*UTF8_MAX_BYTES_PER_CHAR)+10);
	if (tmpString != NULL) {
		long			tmpRealLen = CHRunicode::charToUtf8((qchar *)pString, tmpLen, (qbyte *) tmpString);
		
		tmpString[tmpRealLen]='\0'; // Make sure we zero terminate the string!
		
		tmpResult = tmpString;
		
		MEMfree(tmpString);
	};
	
	return tmpResult;
#else 
	return oUTF8::convertToUTF8((char *) pString);
#endif
};

std::string	oUTF8::convertToUTF8(const char *pString, bool pSkipNewLines) {
	std::string		tmpUTF8 = "";
	unsigned long	tmpIdx = 0;
	
#ifdef isunicode
	// would have already been converted if we've handled this through qString..
	tmpUTF8 = pString;
#else
	// convert 8bit charset to utf-8
	while (pString[tmpIdx]!='\0') {
		unsigned char tmpChar = pString[tmpIdx];
		if (((tmpChar == '\n') || (tmpChar == '\r')) && pSkipNewLines) {
			// Skip...
		} else if (tmpChar < 128) {
			tmpUTF8 += tmpChar;
		} else {
			unsigned long tmpUnicode = charsetToUnicode[tmpChar-128];
		
			if (tmpUnicode < 2048) {
				tmpChar = 192 + (tmpUnicode >> 6);
				tmpUTF8 += tmpChar;
				tmpChar = 128 + (tmpUnicode & 0x03F);
				tmpUTF8 += tmpChar;
			} else if (tmpUnicode < 65536) {
				tmpChar = 224 + (tmpUnicode >> 12);
				tmpUTF8 += tmpChar;
				tmpChar = 128 + ((tmpUnicode >> 6) & 0x03F);
				tmpUTF8 += tmpChar;
				tmpChar = 128 + (tmpUnicode & 0x03F);
				tmpUTF8 += tmpChar;
			} else {
				tmpChar = 240 + (tmpUnicode >> 18);
				tmpUTF8 += tmpChar;
				tmpChar = 128 + ((tmpUnicode >> 12) & 0x03F);
				tmpUTF8 += tmpChar;
				tmpChar = 128 + ((tmpUnicode >> 6) & 0x03F);
				tmpUTF8 += tmpChar;
				tmpChar = 128 + (tmpUnicode & 0x03F);
				tmpUTF8 += tmpChar;
			};
		};
		
		tmpIdx++;
	};
#endif
	
	return tmpUTF8;
};	

std::string	oUTF8::convertFromUTF8(const char * pString) {
	std::string		tmpLocal = "";
	unsigned long	tmpIdx = 0;

	// convert 8bit charset to utf-8
	while (pString[tmpIdx]!='\0') {
		unsigned char	tmpChar = pString[tmpIdx];
		
		if (tmpChar < 128) {
			tmpLocal += tmpChar;
		} else {
			unsigned long	tmpUnicode = 0;
			unsigned long	tmpUTF8Bytes = 1;
			bool			tmpIsUTF8 = true;
			
			
			if (tmpChar < 0xC0) {
				// this should not happen, we've not encoded something properly or this is not an UTF-8 character!
				tmpIsUTF8 = false;
			} else if (tmpChar < 0xE0) {
				tmpUnicode = tmpChar && 0x1F;
				tmpUTF8Bytes = 2;
			} else if (tmpChar < 0xF0) {
				tmpUnicode = tmpChar && 0x0F;
				tmpUTF8Bytes = 3;
			} else if (tmpChar < 0xF8) {
				tmpUnicode = tmpChar && 0x07;
				tmpUTF8Bytes = 4;
			} else if (tmpChar < 0xFE) {
				tmpUnicode = tmpChar && 0x03;
				tmpUTF8Bytes = 5;
			} else {
				tmpUnicode = tmpChar && 0x01;
				tmpUTF8Bytes = 6;				
			};
			
			for (unsigned long tmpCount = 1; tmpIsUTF8 & (tmpCount < tmpUTF8Bytes); tmpCount++) {
				// get the next byte
				tmpChar = pString[tmpIdx+tmpCount];
				if ((tmpChar && 0x80) == 0x80) {
					tmpUnicode << 6;
					tmpUnicode += (tmpChar && 0x3F);
				} else {
					// this should not happen, we've not encoded something properly or this is not an UTF-8 character!
					tmpIsUTF8 = false;
				};
			};
			
			if (tmpIsUTF8) {
				tmpChar = '?';
				
				for (unsigned long tmpUTF8Idx = 0; (tmpChar == '?') & (tmpUTF8Idx < 128); tmpUTF8Idx++) {
					if (charsetToUnicode[tmpUTF8Idx] == tmpUnicode) {
						// found it!
						tmpChar = 128 + tmpUTF8Idx;
					};
				};	
			} else {
				while (tmpUTF8Bytes>0) {
					tmpLocal += pString[tmpIdx];
					tmpUTF8Bytes--;
					tmpIdx++;
				};
				tmpIdx--; // we added one to much...
			};
		};
		
		tmpIdx++;
	};
	
	
	return tmpLocal;
};

