/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oUTF8.h
 *  Helper functions to convert macroman/win-1252 strings to utf8 for non-unicode Omnis
 *
 *  Bastiaan Olij
*
 *  https://github.com/BastiaanOlij/omnis.xcomp.framework
 */

#ifndef outf8
#define outf8

#include "xCompStandardIncludes.h"
#include <string>

class oUTF8 {
public:
	static std::string	convertToUTF8(const qchar * pString);
	static std::string	convertToUTF8(const char * pString, bool pSkipNewLines = false);
	static std::string	convertFromUTF8(const char * pString);
	// may add this one later:
	// static	qString		convertFromUTF8(const char * pString);
};


#endif