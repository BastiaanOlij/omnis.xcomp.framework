/*
 *  omnis.xcomp.framework
 *  =====================
 *
 *  oUTF8.h
 *  Helper functions to convert macroman/win-1252 strings to utf8 for non-unicode Omnis
 *
 *  Bastiaan Olij
 *
 *  Todos:
 *  - Also add support for unicode Omnis where UTF-32 is converted to UTF-8 but through methods provided in the SDK
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
};


#endif