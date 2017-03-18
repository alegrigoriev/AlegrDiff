#ifndef ALEGR_DIFF_VERSION_MAJOR
#define ALEGR_DIFF_VERSION_MAJOR 2
#endif

#ifndef ALEGR_DIFF_VERSION_MINOR
#define ALEGR_DIFF_VERSION_MINOR 0
#endif

#ifndef ALEGR_DIFF_VERSION_SUBMINOR
#define ALEGR_DIFF_VERSION_SUBMINOR 0
#endif

#ifndef ALEGR_DIFF_VERSION_FIX_NUMBER
#define ALEGR_DIFF_VERSION_FIX_NUMBER 1
#endif

/* VER_FILETYPE, VER_FILESUBTYPE, VER_FILEDESCRIPTION_STR
* and VER_INTERNALNAME_STR must be defined before including
* COMMON.VER The strings don't need a '\0', since common.ver
* has them.  */

#define _MAKE_STRING_1(s)       #s
#define _MAKE_STRING_2(s)       _MAKE_STRING_1(s)

#undef VER_COMPANYNAME_STR
#undef VER_PRODUCTNAME_STR
#undef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION

//
// These variables are now imported from PropertySheet.props
// Edit only PropertySheet.props to build with different version stamp
//

#define ALEGR_DIFF_VERSION_SUFFIX_STR   ""


#define VER_FILETYPE                VFT_APP
#define VER_FILESUBTYPE             VFT2_UNKNOWN
#if AMD64
#if DBG
#define VER_FILEDESCRIPTION_STR     "AlegrDiff: file comparison and merging application DEBUG 64 bit"
#else
#define VER_FILEDESCRIPTION_STR     "AlegrDiff: file comparison and merging application 64 bit"
#endif
#else
#if DBG
#define VER_FILEDESCRIPTION_STR     "AlegrDiff: file comparison and merging application DEBUG 32 bit"
#else
#define VER_FILEDESCRIPTION_STR     "AlegrDiff: file comparison and merging application 32 bit"
#endif
#endif
#define VER_INTERNALNAME_STR        "AlegrDiff.exe"
#define VER_ORIGINALFILENAME_STR    "AlegrDiff.exe"
#define VER_LEGALCOPYRIGHT_STR      "Copyright (C) Alexandre Grigoriev, 2001-2017"
#define VER_FILEVERSION             ALEGR_DIFF_VERSION_MAJOR,ALEGR_DIFF_VERSION_MINOR,ALEGR_DIFF_VERSION_SUBMINOR,ALEGR_DIFF_VERSION_FIX_NUMBER
#define VER_FILEVERSION_STR         _MAKE_STRING_2(ALEGR_DIFF_VERSION_MAJOR) "." \
									_MAKE_STRING_2(ALEGR_DIFF_VERSION_MINOR) "." \
									_MAKE_STRING_2(ALEGR_DIFF_VERSION_SUBMINOR) "." \
									_MAKE_STRING_2(ALEGR_DIFF_VERSION_FIX_NUMBER) ALEGR_DIFF_VERSION_SUFFIX_STR
#define VER_COMPANYNAME_STR         "AleGr Software"
#define VER_PRODUCTNAME_STR         "AlegrDiff Application"
#define VER_PRODUCTVERSION          VER_FILEVERSION
#define VER_PRODUCTVERSION_STR      VER_FILEVERSION_STR

#define ALEGR_DIFF_VERSION _T(_MAKE_STRING_2(ALEGR_DIFF_VERSION_MAJOR) "." \
									_MAKE_STRING_2(ALEGR_DIFF_VERSION_MINOR) "." \
									_MAKE_STRING_2(ALEGR_DIFF_VERSION_SUBMINOR) "." \
									_MAKE_STRING_2(ALEGR_DIFF_VERSION_FIX_NUMBER))


