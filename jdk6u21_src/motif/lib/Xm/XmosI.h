/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */
/* $XConsortium: XmosI.h /main/6 1995/07/13 18:28:56 drk $ */
#ifndef _XmosI_h
#define _XmosI_h

#include <Xm/XmosP.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Vendor dependent macro for XmCvtXmStringToCT */
/* Sample implementation treats unmapped charsets as locale encoded text. */
#define _XmOSProcessUnmappedCharsetAndText(tag, ctext, sep, outc, outl, prev) \
	processCharsetAndText(XmFONTLIST_DEFAULT_TAG, (ctext), (sep), \
			      (outc), (outl), (prev))


/********    Private Function Declarations    ********/

extern String _XmOSFindPatternPart( 
                        String fileSpec) ;
extern void _XmOSQualifyFileSpec( 
                        String dirSpec,
                        String filterSpec,
                        String *pQualifiedDir,
                        String *pQualifiedPattern) ;
extern void _XmOSGetDirEntries( 
                        String qualifiedDir,
                        String matchPattern,
#if NeedWidePrototypes
                        unsigned int fileType,
                        int matchDotsLiterally,
                        int listWithFullPath,
#else
                        unsigned char fileType,
                        Boolean matchDotsLiterally,
                        Boolean listWithFullPath,
#endif /* NeedWidePrototypes */
                        String **pEntries,
                        unsigned int *pNumEntries,
                        unsigned int *pNumAlloc) ;
extern void _XmOSBuildFileList( 
                        String dirPath,
                        String pattern,
#if NeedWidePrototypes
                        unsigned int typeMask,
#else
                        unsigned char typeMask,
#endif /* NeedWidePrototypes */
                        String **pEntries,
                        unsigned int *pNumEntries,
                        unsigned int *pNumAlloc) ;
extern int _XmOSFileCompare( 
                        XmConst void *sp1,
                        XmConst void *sp2) ;
extern String _XmOSInitPath( 
                        String file_name,
                        String env_pathname,
                        Boolean *user_path) ;
extern String _XmOSBuildFileName(
			String file,
			String path) ;
extern int _XmOSPutenv(
		       char *string);
extern void _XmOSGenerateMaskName( 
				  String imageName,
				  String	maskNameBuf) ;

extern Status _XmOSGetInitialCharsDirection(XtPointer     characters,
					    XmTextType    type,
					    XmStringTag   locale,
					    unsigned int *num_bytes,
					    XmDirection  *direction) ;

extern XmDirection _XmOSGetCharDirection(XtPointer   character,
					 XmTextType  type,
					 XmStringTag locale) ;

extern int _XmOSKeySymToCharacter(KeySym keysym,
				  char	 *locale,
				  char	 *buffer);
extern void _XmOSFindPathParts(String path, 
			       String *filenameRtn, 
			       String *suffixRtn);
extern Boolean _XmOSAbsolutePathName( 
                        String path,
                        String *pathRtn,
                        String buf) ;

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmosI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
