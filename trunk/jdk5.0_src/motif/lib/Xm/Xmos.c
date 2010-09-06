/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* 
 * HISTORY
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: Xmos.c /main/29 1996/11/21 20:04:31 drk $"
#endif
#endif
/* (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */

#include <stdio.h>

#ifdef __cplusplus
extern "C" { /* some 'locale.h' do not have prototypes (sun) */
#endif
#include <X11/Xlocale.h>
#ifdef __cplusplus
} /* Close scope of 'extern "C"' declaration */
#endif /* __cplusplus */

#include <X11/Xos.h>

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#include <unistd.h>
#endif

#include <ctype.h>		/* for isspace() */

#include <sys/time.h>  /* For declaration of select(). */

#if defined(NO_REGCOMP) && !defined(NO_REGEX)
# ifdef __sgi
extern char *regcmp();
extern int regex();
# elif defined(SUN_MOTIF)  /* fix for bug 4132654 leob */
#  include "regexpI.h"
# elif defined(SVR4)
#  include <libgen.h>
# elif defined(SYSV)
extern char *regcmp();
extern int regex();
# endif
#endif /* NO_REGEX */

#ifndef NO_REGCOMP
# include <regex.h>
#endif /* NO_REGCOMP */

#include <sys/stat.h>

/* Solaris 2.7 bugfix # 4072756 - 1 lines */
#define X_INCLUDE_PWD_H
#define X_INCLUDE_DIRENT_H
#define XOS_USE_XT_LOCKING
#include <Xm/Xmos_r.h>

#include "XmosI.h"
#include "XmI.h"

#ifdef USE_GETWD
# include <sys/param.h>
# define MAX_DIR_PATH_LEN    MAXPATHLEN
# define getcwd(buf, len)   ((char *) getwd(buf))
#else
# define MAX_DIR_PATH_LEN    1024
#endif
#define MAX_USER_NAME_LEN   256

#ifndef S_ISDIR
# define S_ISDIR(m) ((m & S_IFMT)==S_IFDIR)
#endif

#ifndef S_ISREG
# define S_ISREG(m) ((m & S_IFMT)==S_IFREG)
#endif

#define FILE_LIST_BLOCK 64

typedef struct {   
  unsigned char type ; 
  char file_name[1] ;		/* Must be last entry in structure. */
} XmDirCacheRec, **XmDirCache ;


/********
 * Set defaults for resources that are implementation dependant
 * and may be modified.
 ********/ 

externaldef(xmos) char _XmSDEFAULT_FONT[] = "fixed";
externaldef(xmos) char _XmSDEFAULT_BACKGROUND[] = "#c4c4c4";

/**************** end of vendor dependant defaults ********/

/********    Static Function Declarations    ********/

static String GetCurrentDir(String buf);
static String GetQualifiedDir(String dirSpec);
static String GetFixedMatchPattern(String pattern);
static void FreeDirCache(void);
static void ResetCache(char *qDirName);
static unsigned char AddEntryToCache(char *entryName, unsigned entryNameLen);
static int Wcslen(wchar_t *wcs);

/********    End Static Function Declarations    ********/

static char *dirCacheName; 
static size_t dirCacheNameLen;  /* Wyoming 64-bit fix */ 
static XmDirCache dirCache;
static unsigned numCacheAlloc;
static unsigned numCacheEntries;
 
static void
FreeDirCache(void)
{   
  if (dirCacheName != NULL)
    {   
      XtFree(dirCacheName);
      dirCacheName = NULL;
      dirCacheNameLen = 0;
      
      while (numCacheEntries)
	XtFree((char *) dirCache[--numCacheEntries]);
    } 
} 

static void
ResetCache(char *qDirName)
{   
  FreeDirCache();

  dirCacheNameLen = strlen(qDirName);
  dirCacheName = XtMalloc(dirCacheNameLen + MAX_USER_NAME_LEN + 1);
  strcpy(dirCacheName, qDirName);
} 

static unsigned char
AddEntryToCache(char *entryName,
		unsigned entryNameLen)
{   
  struct stat statBuf;
  unsigned char result = 0;

  if (numCacheEntries == numCacheAlloc)
    {
      numCacheAlloc += FILE_LIST_BLOCK;
      dirCache = (XmDirCache) 
	XtRealloc((char *) dirCache, numCacheAlloc * sizeof(XmDirCacheRec *));
    } 

  dirCache[numCacheEntries] = (XmDirCacheRec *) 
    XtMalloc(sizeof(XmDirCacheRec) + entryNameLen);
  strcpy(dirCache[numCacheEntries]->file_name, entryName);

  /* Use dirCacheName character array as temporary buffer for full file name.*/
  strcpy(&dirCacheName[dirCacheNameLen], entryName);

  if (!stat(dirCacheName, &statBuf))
    {   
      if (S_ISREG(statBuf.st_mode))
	result = XmFILE_REGULAR;
      else if (S_ISDIR(statBuf.st_mode))
	result = XmFILE_DIRECTORY;
    } 

  /* Restore to dir path only. */
  dirCacheName[dirCacheNameLen] = '\0';

  dirCache[numCacheEntries++]->type = result;
  return result;
}

/****************************************************************/
static String
GetQualifiedDir(String dirSpec)
/*************GENERAL:
 * dirSpec is a directory name, that can contain relative 
 *   as well as logical reference. This routine resolves all these
 *   references, so that dirSpec is now suitable for open().
 * The routine allocates memory for the result, which is guaranteed to be
 *   of length >= 1.  This memory should eventually be freed using XtFree().
 ****************/

/*************UNIX:
 * Builds directory name showing descriptive path components.  The result
 *   is a directory path beginning at the root directory and terminated
 *   with a '/'.  The path will not contain ".", "..", or "~" components.  
 ****************/
{
  size_t             dirSpecLen; /* Wyoming 64-bit fix */ 
  _Xgetpwparams	  pwd_buf;
  struct passwd * pwd_value;

  char *          userDir;
  size_t             userDirLen; /* Wyoming 64-bit fix */ 
  int             userNameLen;
  char *          outputBuf;
  char *          destPtr;
  char *          srcPtr;
  char *          scanPtr;
  char            nameBuf[MAX_USER_NAME_LEN];
  char            dirbuf[MAX_DIR_PATH_LEN];
  
  dirSpecLen = strlen(dirSpec);
  outputBuf = NULL;
  
  switch (*dirSpec)
    {   
    case '~':
      if (!(dirSpec[1])  ||  (dirSpec[1] == '/'))
	{
	  userDir = XmeGetHomeDirName();
	  if (*userDir)
	    {   
	      userDirLen = strlen(userDir);
	      outputBuf = XtMalloc(userDirLen + dirSpecLen + 2);
	      strcpy(outputBuf, userDir);
	      strcpy(&outputBuf[userDirLen], (dirSpec + 1));
	    }
	}
      else
	{
	  destPtr = nameBuf;
	  userNameLen = 0;
	  srcPtr = dirSpec + 1;
	  while (*srcPtr  &&  (*srcPtr != '/') &&
		 (++userNameLen < MAX_USER_NAME_LEN))
	    { 
	      *destPtr++ = *srcPtr++;
	    } 
	  *destPtr = '\0';
	  
	  pwd_value = _XGetpwnam(nameBuf, pwd_buf);
	  if (pwd_value != NULL)
	    {   
	      userDirLen = strlen(pwd_value->pw_dir);
	      dirSpecLen = strlen(srcPtr);
	      outputBuf = XtMalloc(userDirLen + dirSpecLen + 2);
	      strcpy(outputBuf, pwd_value->pw_dir);
	      strcpy(&outputBuf[userDirLen], srcPtr);
	    } 
	}
      break;

    case '/':
      outputBuf = XtMalloc(dirSpecLen + 2);
      strcpy(outputBuf, dirSpec);
      break;

    default:
      if ((destPtr = GetCurrentDir(dirbuf)) != NULL)
	{
	  userDirLen = strlen(destPtr);
	  outputBuf = XtMalloc(userDirLen + dirSpecLen + 3);
	  strcpy(outputBuf, destPtr);
	  outputBuf[userDirLen++] = '/';
	  strcpy(&outputBuf[userDirLen], dirSpec);
	} 
      break;
    } 

  if (!outputBuf)
    { 
      outputBuf = XtMalloc(2);
      outputBuf[0] = '/';
      outputBuf[1] = '\0';
    } 
  else
    { 
      userDirLen = strlen(outputBuf);
      if (outputBuf[userDirLen - 1]  !=  '/')
        { 
	  outputBuf[userDirLen] = '/';
	  outputBuf[++userDirLen] = '\0';
	} 
      /* The string in outputBuf is assumed to begin and end with a '/'. */
      scanPtr = outputBuf;
      while (*++scanPtr)               /* Skip past '/'. */
        { 
	  /* scanPtr now points to non-NULL character following '/'. */
	  if (scanPtr[0] == '.')
            {   
	      if (scanPtr[1] == '/')
                { 
		  /* Have "./", so just erase (overwrite with shift).
		   */
		  destPtr = scanPtr;
		  srcPtr = &scanPtr[2];
		  while ((*destPtr++ = *srcPtr++) != '\0')
		    /*EMPTY*/;
		  --scanPtr;     /* Leave scanPtr at preceding '/'. */
		  continue;
		} 
	      else
                { 
		  if ((scanPtr[1] == '.')  &&  (scanPtr[2] == '/'))
                    { 
		      /* Have "../", so back up one directory. */
		      srcPtr = &scanPtr[2];
		      --scanPtr;      /* Move scanPtr to preceding '/'.*/
		      if (scanPtr != outputBuf)
                        { 
			  while ((*--scanPtr != '/'))
			    /*EMPTY*/;        /* Now move to previous '/'.*/
			} 
		      destPtr = scanPtr;
		      while ((*++destPtr = *++srcPtr) != '\0')
			/*EMPTY*/;		/* Overwrite "../" with shift.*/
		      continue;
		    } 
		} 
	    } 
	  else
            { 
	      /* Check for embedded "//".  Posix allows a leading double
	       *   slash (and Apollos require it).
	       */
	      if (*scanPtr == '/')
                {   
		  if ((scanPtr > (outputBuf + 1)) ||
		      (scanPtr[1] == '/'))
                    {
		      /* Have embedded "//" (other than root specification),
		       *   so erase with shift and reset scanPtr.
		       */
		      srcPtr = scanPtr;
		      --scanPtr;
		      destPtr = scanPtr;
		      while ((*++destPtr = *++srcPtr) != '\0')
			/*EMPTY*/;
		    }
		  continue;
		}
	    } 
	  while (*++scanPtr != '/')
	    /*EMPTY*/;
	} 
    } 

  return outputBuf;
}

/****************************************************************/
String
_XmOSFindPatternPart(String fileSpec)
/****************GENERAL:
 * fileSpec is made of a directory part and a pattern part.
 * Returns the pointer to the first character of the pattern part
 ****************/

/****************UNIX:
 * Returns the pointer to the character following the '/' of the name segment
 *   which contains a wildcard or which is not followed by a '/'.
 ****************/
{
  char *          lookAheadPtr = fileSpec;
  char *          maskPtr;
  Boolean         hasWildcards;
  char            prevChar;
  char            prev2Char ;
  
  /* Stop at final name segment or if wildcards were found. */
  do {
    maskPtr = lookAheadPtr;
    hasWildcards = FALSE;
    prevChar = '\0';
    prev2Char = '\0';
    while ((*lookAheadPtr != '/') && !hasWildcards && *lookAheadPtr) 
      {   
	switch (*lookAheadPtr) 
	  {   
	  case '*': 
	  case '?': 
	  case '[': 
	    if ((prevChar != '\\')  ||  (prev2Char == '\\')) 
	      {   
		hasWildcards = TRUE;
		break;
	      } 
	  }
	prev2Char = prevChar;
	prevChar = *lookAheadPtr;
	lookAheadPtr += ((MB_CUR_MAX > 1) ? 
			 abs(mblen(lookAheadPtr, MB_CUR_MAX)) : 1);
      } 
  } while (!hasWildcards  &&  *lookAheadPtr++);
  
  if (*maskPtr == '/') 
    ++maskPtr;
  
  return(maskPtr);
}

/****************************************************************/
void
_XmOSQualifyFileSpec(String  dirSpec,
		     String  filterSpec,
		     String *pQualifiedDir,      /* Cannot be NULL.*/
		     String *pQualifiedPattern)  /* Cannot be NULL.*/
/************GENERAL:
 * dirSpec, filterSpec can contain relative or logical reference.
 * dirSpec cannot contain pattern characters.
 * if filterSpec does not specify all for its last segment, a pattern 
 * for 'all' is added.
 * Use GetQualifiedDir() for dirSpec.
 ****************/

/************UNIX:
 * 'all' is '*' and '/' is the delimiter.
 ****************/
{
  size_t filterLen; /* Wyoming 64-bit fix */ 
  size_t dirLen; /* Wyoming 64-bit fix */ 
  char *fSpec;
  char *remFSpec;
  char *maskPtr;
  char *dSpec;
  char *dPtr;
  
  if (!dirSpec) 
    dirSpec = "";
  if (!filterSpec) 
    filterSpec = "";
  
  filterLen = strlen(filterSpec);
  
  /* Allocate extra for NULL character and for the appended '*' (as needed). */
  fSpec = XtMalloc(filterLen + 2);
  strcpy(fSpec, filterSpec);
  
  /* If fSpec ends with a '/' or is a null string, add '*' since this is
   *   the interpretation.
   */
  if (!filterLen  ||  (fSpec[filterLen - 1] == '/'))
    {   
      fSpec[filterLen] = '*';
      fSpec[filterLen + 1] = '\0';
    } 
  
  /* Some parts of fSpec may be copied to dSpec, so allocate "filterLen" 
   *   extra, plus some for added literals.
   */
  dirLen = strlen(dirSpec);
  dSpec = XtMalloc(filterLen + dirLen + 4);
  strcpy(dSpec, dirSpec);
  dPtr = dSpec + dirLen;
  
  /* Check for cases when the specified filter overrides anything
   *   in the dirSpec.
   */
  remFSpec = fSpec;
  switch(*fSpec) 
    {   
    case '/':
      dSpec[0] = '/';
      dSpec[1] = '\0';
      dPtr = dSpec + 1;
      ++remFSpec;
      break;

    case '~':
      dPtr = dSpec;
      while ((*dPtr = *remFSpec)  &&  (*remFSpec++ != '/')) 
	++dPtr;
      *dPtr = '\0';
      break;
    } 
  
  /* If directory spec. is not null, then make sure that it has a
   *   trailing '/', to be prepared for appending from filter spec.
   */
  if (*dSpec  &&  (*(dPtr - 1) != '/')) 
    {   
      *dPtr++ = '/';
      *dPtr = '\0';
    } 
  
  maskPtr = _XmOSFindPatternPart(remFSpec);
  
  if (maskPtr != remFSpec) 
    {  
      do {   
	*dPtr++ = *remFSpec++;
      } while (remFSpec != maskPtr);
      *dPtr = '\0';
    } 
  
  if (remFSpec != fSpec) 
    {   
      /* Shift remaining filter spec. to the beginning of the buffer. */
      remFSpec = fSpec;
      while ((*remFSpec++ = *maskPtr++) != '\0') 
	/*EMPTY*/;
    } 
  
  *pQualifiedDir = GetQualifiedDir(dSpec);
  *pQualifiedPattern = fSpec;
  XtFree(dSpec);
}

/****************************************************************/
static String
GetFixedMatchPattern(String pattern)
/**********GENERAL:
 * The pattern parameter is converted to the format required of the
 *   the regular expression library routines.
 * Memory is allocated and returned with the result.  This memory
 *   should eventually be freed by a call to XtFree().
 ****************/

/**********UNIX:
 * '/' is used as a delimiter for the pattern.
 ****************/
{
  register char *bufPtr;
  char *outputBuf;
  char lastchar = '\0';
  int len;
  
  outputBuf = XtCalloc(2, strlen(pattern) + 4);
  
  bufPtr = outputBuf;
  *bufPtr++ = '^';
  
  while ((len = mblen(pattern, MB_CUR_MAX)) > 0)
    {
      if (len <= 1)
	{
	  if (*pattern == '/')
	    break;

	  if (lastchar == '\\')
	    *bufPtr++ = *pattern;
	  else
	    {
	      switch(*pattern)
		{   
		case '.':
		  *bufPtr++ = '\\';
		  *bufPtr++ = '.';
		  break;

		case '?':
		  *bufPtr++ = '.';
		  break;

		case '*':
		  *bufPtr++ = '.';
		  *bufPtr++ = '*';
		  break;

		default:
		  *bufPtr++ = *pattern;
		  break;
		} 
	    }
	  lastchar = *pattern;
	  ++pattern;
	} 
      else
	{
	  strncpy(bufPtr, pattern, len);
	  bufPtr += len;
	  pattern += len;
	  lastchar = '\0';
	} 
    }
  
  *bufPtr++ = '$';
  *bufPtr = '\0';
  
  return outputBuf;
}

/****************************************************************/
void
     _XmOSGetDirEntries(String          qualifiedDir,
			String          matchPattern,
#if NeedWidePrototypes
			unsigned int fileType,
			int matchDotsLiterally,
			int listWithFullPath,
#else
			unsigned char fileType,
			Boolean matchDotsLiterally,
			Boolean listWithFullPath,
#endif /* NeedWidePrototypes */
			String * *      pEntries, /* Cannot be NULL. */
			unsigned int *  pNumEntries, /* Cannot be NULL. */
			unsigned int *  pNumAlloc) /* Cannot be NULL. */

/***********GENERAL:
 * This routine opens the specified directory and builds a buffer containing
 * a series of strings containing the full path of each file in the directory 
 * The memory allocated should eventually be freed using XtFree.
 * The 'qualifiedDir' parameter must be a fully qualified directory path 
 * The matchPattern parameter must be in the proper form for a regular 
 * expression parsing.
 * If the location pointed to by pEntries is NULL, this routine allocates
 *   and returns a list to *pEntries, though the list may have no entries.
 *   pEntries, pEndIndex, pNumAlloc are updated as required for memory 
 *   management.
 ****************/

/***********UNIX:
 * Fully qualified directory means begins with '/', does not have 
 * embedded "." or "..", but does not need trailing '/'.
 * Regular expression parsing is regcmp or re_comp.
 * Directory entries are also Unix dependent.
 ****************/
 
{
  char *          fixedMatchPattern;
  String          entryPtr;
  DIR *           dirStream = NULL;
  struct stat     statBuf;
  Boolean         entryTypeOK;
  size_t    dirLen = strlen(qualifiedDir); /* Wyoming 64-bit fix */ 
  Boolean         useCache = FALSE;
  Boolean         loadCache = FALSE;
  unsigned        readCacheIndex;
  unsigned char   dirFileType;
#ifndef NO_REGCOMP
  regex_t         preg;
  int             comp_status;
#elif !defined(NO_REGEX)
# ifdef SUN_MOTIF /* fix for bug 4132654 leob */
  XmRegexpRec *   compiledRE = NULL;
# else
  char *          compiledRE = NULL;
# endif
#endif /* NO_REGCOMP */
/****************/

  _XmProcessLock();

  if (!*pEntries)
    {
      *pNumEntries = 0;
      *pNumAlloc = FILE_LIST_BLOCK;
      *pEntries = (String *) XtMalloc(FILE_LIST_BLOCK * sizeof(char *));
    } 
  fixedMatchPattern = GetFixedMatchPattern(matchPattern);
  
  if (fixedMatchPattern)
    {   
      if (!*fixedMatchPattern)
	{   
	  XtFree(fixedMatchPattern);
	  fixedMatchPattern = NULL;
	} 
      else
	{   
#ifndef NO_REGCOMP
	  comp_status = regcomp(&preg, fixedMatchPattern, REG_EXTENDED|REG_NOSUB);
	  if (comp_status)
#elif !defined(NO_REGEX)
# ifdef SUN_MOTIF /* fix for bug 4132654 leob */
            compiledRE = _XmRegcomp(fixedMatchPattern);
# else
            compiledRE = (char *)regcmp(fixedMatchPattern, (char *) NULL);
# endif
	  if (!compiledRE)
#else
          if (re_comp(fixedMatchPattern))
#endif
	    {
	      XtFree(fixedMatchPattern);
	      fixedMatchPattern = NULL;
	    } 
	}
    }

  if ((dirCacheName != NULL) &&
      !strcmp(qualifiedDir, dirCacheName))
    {   
      useCache = TRUE;
      readCacheIndex = 0;
    } 
  else
    {   
      if (!strcmp(matchPattern, "*") &&
	  (fileType == XmFILE_DIRECTORY) &&
	  !matchDotsLiterally)
	{   
	  /* This test is a incestual way of knowing that we are searching
	   * a directory to fill the directory list.  We can thereby conclude
	   * that a subsequent call will be made to search the same directory
	   * to fill the file list.  Since a "stat" of every file is very
	   * performance-expensive, we will cache the directory used for
	   * a directory list search and subsequently use the results for
	   * the file list search.
	   */
	  loadCache = TRUE;
	}
      dirStream = opendir(qualifiedDir);
    }

  if (dirStream || useCache)
    {   
      unsigned loopCount = 0;
      _Xreaddirparams dirEntryBuf;
      
      if (loadCache)
	ResetCache(qualifiedDir);
      
      /* The POSIX specification for the "readdir" routine makes
       *  it OPTIONAL to return the "." and ".." entries in a
       *  directory.  The algorithm used here depends on these
       *  entries being included in the directory list.  So, we
       *  will first handle "." and ".." explicitly, then ignore
       *  them later if they happen to be returned by "readdir".
       */
      while (TRUE)
	{   
	  char *dirName;
	  size_t dirNameLen = 0; /* Wyoming 64-bit fix */ 
	  
	  if (loopCount < 2)
	    {   
	      if (loopCount == 0)
		{   
		  /* Do current directory the first time through. */
		  dirName = ".";
		  dirNameLen = 1;
		} 
	      else
		{   
		  /* Do parent directory the second time through. */
		  dirName = "..";
		  dirNameLen = 2;
		} 
	      ++loopCount;
	      
	      if (useCache || loadCache)
		dirFileType = XmFILE_DIRECTORY;
	    } 
	  else
	    {   
	      struct dirent * dirEntry;

	      do {   
		if (useCache)
		  {   
		    if (readCacheIndex == numCacheEntries)
		      {   
			dirName = NULL; 
			break;
		      } 
		    else
		      {
			dirFileType = dirCache[readCacheIndex]->type;
			dirName = dirCache[readCacheIndex++]->file_name;
			dirNameLen = strlen(dirName);
		      }
		  }
		else
		  {   
		    if ((dirEntry = _XReaddir(dirStream, dirEntryBuf)) == NULL)
		      {
			dirName = NULL;
			break;
		      } 
		    dirName = dirEntry->d_name;
		    dirNameLen = strlen(dirName);
		    if (loadCache)
		      dirFileType = AddEntryToCache(dirName, (int)dirNameLen); /* Wyoming 64-bit fix */ 
		  }
		/* Check to see if directory entry is "." or "..",
		 *  since these have already been processed.
		 *  So if/when readdir returns these directories,
		 *  we just ignore them.
		 */
	      } while (((dirNameLen == 1) && (dirName[0] == '.')) ||
		       ((dirNameLen == 2) && 
			(dirName[0] == '.') && (dirName[1] == '.')));
	      
	      if (dirName == NULL)
		break;             /* Exit from outer loop. */
	    }
	  if (fixedMatchPattern)
	    {   
#ifndef NO_REGCOMP
	      if (regexec(&preg, dirName, 0, NULL, 0))
#else /* NO_REGCOMP */
#  ifndef NO_REGEX
#    ifdef SUN_MOTIF /* fix for bug 4132654 leob */
              if (!_XmRegexec(compiledRE, dirName))
#    else
              if (!regex(compiledRE, dirName))
#    endif
#  else
              if (!re_exec(dirName))
#  endif
#endif /* NO_REGCOMP */
		continue;
	    } 
	  if (matchDotsLiterally &&
	      (dirName[0] == '.') &&
	      (*matchPattern != '.'))
	    continue;
	  if (*pNumEntries == *pNumAlloc)
	    {
	      *pNumAlloc += FILE_LIST_BLOCK;
	      *pEntries = (String *) 
		XtRealloc((char*) *pEntries, (*pNumAlloc* sizeof(char *)));
	    } 
	  entryPtr = XtMalloc(dirNameLen + dirLen + 1);
	  strcpy(entryPtr, qualifiedDir);
	  strcpy(&entryPtr[dirLen], dirName);
	  
	  /* Now screen entry according to type. */
	  entryTypeOK = FALSE;
	  
	  if (fileType == XmFILE_ANY_TYPE)
	    {
	      entryTypeOK = TRUE;
	    }
	  else if (useCache || loadCache)
	    {   
	      if (dirFileType == fileType)
		entryTypeOK = TRUE;
	    } 
	  else
	    {   
	      if (!stat(entryPtr, &statBuf))
		{   
		  switch (fileType)
		    {   
		    case XmFILE_REGULAR:
		      if (S_ISREG(statBuf.st_mode))
			entryTypeOK = TRUE;
		      break;
		      
		    case XmFILE_DIRECTORY:
		      if (S_ISDIR(statBuf.st_mode))
			entryTypeOK = TRUE;
		      break;
		    }
		}
	    }
	  if (entryTypeOK)
	    {   
	      if (listWithFullPath)
		{   
		  (*pEntries)[(*pNumEntries)++] = entryPtr;
		} 
	      else
		{
		  /* This is ONLY for BC in a (apparently unused) API, (w/o
		   *  full path) so don't worry too much about efficiency.
		   */
		  XtFree(entryPtr);
		  entryPtr = XtMalloc(dirNameLen + 1);
		  strcpy(entryPtr, dirName);
		  (*pEntries)[(*pNumEntries)++] = entryPtr;
		} 
	    } 
	  else
	    XtFree(entryPtr);
	}
      if (!useCache)
	closedir(dirStream);
    }
#ifndef NO_REGCOMP
  if (!comp_status)
    regfree(&preg);
#else /* NO_REGCOMP */
#  ifndef NO_REGEX
  if (compiledRE)
    {
      /* Use free instead of XtFree since malloc is inside of regex(). */
      free(compiledRE); 
    } 
#  endif
#endif /* NO_REGCOMP */
  XtFree(fixedMatchPattern);

  if (!loadCache)
    FreeDirCache();
  _XmProcessUnlock();
}

/****************************************************************
 * _XmOSBuildFileList:
 *
 * GENERAL:
 * The 'dirPath' parameter must be a qualified directory path.
 * The 'pattern' parameter must be valid as a suffix to dirPath.
 * typeMask is an Xm constant coming from Xm.h.
 *
 * UNIX:
 * Qualified directory path means no match characters, with '/'
 * at end.
 ****************************************************************/
void
_XmOSBuildFileList(String          dirPath,
		   String          pattern,
#if NeedWidePrototypes
		   unsigned int typeMask,
#else
		   unsigned char typeMask,
#endif /* NeedWidePrototypes */
		   String * *      pEntries,       /* Cannot be NULL. */
		   unsigned int *  pNumEntries,    /* Cannot be NULL. */
		   unsigned int *  pNumAlloc)      /* Cannot be NULL. */
{  
  String          qualifiedDir;
  String          nextPatternPtr;
  String *        localEntries;
  unsigned int    localNumEntries;
  unsigned int    localNumAlloc;
  unsigned int    entryIndex;
  
  qualifiedDir = GetQualifiedDir(dirPath);
  nextPatternPtr = pattern;
  while (*nextPatternPtr  &&  (*nextPatternPtr != '/')) 
    ++nextPatternPtr;
  
  if (!*nextPatternPtr) 
    {   
      /* At lowest level directory, so simply return matching entries.*/
      _XmOSGetDirEntries(qualifiedDir, pattern, typeMask, FALSE, TRUE, 
			 pEntries, pNumEntries, pNumAlloc);
    } 
  else
    {   
      ++nextPatternPtr;               /* Move past '/' character.*/
      localEntries = NULL;
      _XmOSGetDirEntries(qualifiedDir, pattern, XmFILE_DIRECTORY, TRUE, TRUE, 
			 &localEntries, &localNumEntries, &localNumAlloc);
      entryIndex = 0;
      while (entryIndex < localNumEntries) 
	{   
	  _XmOSBuildFileList(localEntries[entryIndex], nextPatternPtr, 
			     typeMask, pEntries, pNumEntries, pNumAlloc);
	  XtFree(localEntries[entryIndex]);
	  ++entryIndex;
	} 
      XtFree((char*)localEntries);
    }
  XtFree(qualifiedDir);
}

/****************************************************************
 * GENERAL:
 * The routine must return an integer less than, equal to, or
 * greater than 0 according as the first argument is to be
 * considered less than, equal to, or greater than the second.
 ****************************************************************/

int
_XmOSFileCompare(XmConst void *sp1,
		 XmConst void *sp2)
{
  return strcoll(*((String *) sp1), *((String *) sp2));
}

/*************************************************************************
 *
 *   Path code, used in Mwm and Xm.
 *   Returned pointer should not be freed!
 *
 *************************************************************************/

String
XmeGetHomeDirName(void)
{
  uid_t uid;
  _Xgetpwparams	pwd_buf;
  struct passwd * pwd_value;

  char *ptr = NULL;
  static char empty = '\0';
  static char *homeDir = NULL;
  /* Solaris 2.6 Motif diff bug 4034689, 1 lines */
  char tmp_ptr[MAX_DIR_PATH_LEN];
  
  _XmProcessLock();
  if (homeDir == NULL) 
    {
      if ((ptr = (char *)getenv("HOME")) == NULL) 
	{
	  if ((ptr = (char *)getenv(USER_VAR)) != NULL)
	    pwd_value = _XGetpwnam(ptr, pwd_buf);
	  else 
	    {
	      uid = getuid();
	      pwd_value = _XGetpwuid(uid, pwd_buf);
            }

	  if (pwd_value != NULL)
	    ptr = pwd_value->pw_dir;
	  else
	    ptr = NULL;
        }

      if (ptr != NULL) 
	{
          /* Solaris 2.6 Motif diff bug 4034689, 3 lines */
	  strncpy(tmp_ptr, ptr, MAX_DIR_PATH_LEN -1);
	  tmp_ptr[MAX_DIR_PATH_LEN-1] = '\0';
	  ptr = tmp_ptr;
	  homeDir = XtMalloc (strlen(ptr) + 1);
	  strcpy (homeDir, ptr);
	}
      else 
	{
	  homeDir = &empty;
	}
    }
  
  _XmProcessUnlock();
  return homeDir;
}

#ifndef LIBDIR
#define LIBDIR "/usr/lib/X11"
#endif
#ifndef INCDIR
#define INCDIR "/usr/include/X11"
#endif

static XmConst char libdir[] = LIBDIR;
static XmConst char incdir[] = INCDIR;

/*************************************************************************
 *
 *   When the locale contains a codeset, the Toolkit's default path fail
 *   to search for the directory with only language and territory.
 *
 *   For example, when locale is "zh_TW.dechanyu", directories searched 
 *   should be :
 *
 *   1. zh_TW.dechanyu       (%L)
 *   2. zh_TW                (%l_%t)
 *   3. zh                   (%l)
 *
 *************************************************************************/

static XmConst char XAPPLRES_DEFAULT[] = "\
%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l_%%t/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l_%%t/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l_%%t/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l_%%t/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S";

static XmConst char PATH_DEFAULT[] = "\
%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l_%%t/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l_%%t/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%P\
%%S:\
%s/%%L/%%T/%%N/%%P\
%%S:\
%s/%%l_%%t/%%T/%%N/%%P\
%%S:\
%s/%%l/%%T/%%N/%%P\
%%S:\
%s/%%T/%%N/%%P\
%%S:\
%s/%%L/%%T/%%P\
%%S:\
%s/%%l_%%t/%%T/%%P\
%%S:\
%s/%%l/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S:\
%s/%%T/%%P\
%%S";

static XmConst char ABSOLUTE_PATH[] = "\
%P\
%S";

/*
 * buf must be of length MAX_DIR_PATH_LEN
 */
static String
GetCurrentDir(String 	buf)
{
    String pwd = getenv ("PWD");
    struct stat stat1, stat2;

    if (pwd
	&& (stat(pwd, &stat1) == 0)
	&& (stat(".", &stat2) == 0)
	&& (stat1.st_dev == stat2.st_dev)) {
/* Solaris 2.7 bugfix # 4072243 - 1 lines */
/* Inode compare is not really needed
	&& stat1.st_ino == stat2.st_ino) {
*/
	/* Use PWD environment variable */
	strncpy(buf, pwd, (MAX_DIR_PATH_LEN-1));
	buf[MAX_DIR_PATH_LEN-1] = '\0';
	return pwd ;
    } 

    return getcwd(buf, MAX_DIR_PATH_LEN) ;
}

#ifdef notdef
    /* old way */
    String pwd = NULL;
    
    if ((pwd = getenv("PWD")) != NULL)
	strcpy(buf, pwd);
    if (!pwd) pwd = getcwd(buf, MAX_DIR_PATH_LEN)
    return pwd ;
#endif


/*
 * buf must be of length MAX_DIR_PATH_LEN
 */
Boolean
_XmOSAbsolutePathName(String path, String *pathRtn, String buf)
{
    Boolean 	doubleDot = False;

    *pathRtn = path;

    if (path[0] == '/')
      return True;

    if (path[0] == '.') {
	if (path[1] == '/') 
	  doubleDot = False;
	else if ((path[1] == '.') &&
		 (path[2] == '/'))
	  doubleDot = True;

	if (GetCurrentDir(buf) != NULL) {
	    if (doubleDot) {
		String filePart, suffixPart;
		_XmOSFindPathParts(buf, &filePart, &suffixPart);
		(void) strcpy(filePart, &path[2]);
	    }
	    else {
		(void) strcat(buf, &path[1]);
	    }
	    *pathRtn = buf;
	    return True;
	}
	else {
	    XmeWarning(NULL, "Cannot find current dir");
	    return True;
	}
    }
    return False;
}

String
_XmOSInitPath(String   file_name,
	      String   env_pathname,
	      Boolean *user_path)
{
  String path;
  String old_path;
  /* Solaris 2.6 Motif diff bug 4034689, 1 lines */
  char old_path1[MAX_DIR_PATH_LEN];
  char stackString[MAX_DIR_PATH_LEN];
  String homedir = stackString ;
  String local_path;
  
  *user_path = False;
  
  if (file_name && _XmOSAbsolutePathName(file_name, &file_name, homedir)) {
      path = XtNewString(ABSOLUTE_PATH);
  } 
  else
    {
      local_path = (char *)getenv (env_pathname);
      if (local_path  == NULL) 
	{
	  homedir = XmeGetHomeDirName();
	  old_path = (char *)getenv ("XAPPLRESDIR");
	  if (old_path == NULL) 
	    {
	      path = XtCalloc(1, (9*strlen(homedir) + strlen(PATH_DEFAULT) +
				  8*strlen(libdir) + strlen(incdir) + 1));
	      sprintf(path, PATH_DEFAULT, homedir, homedir, homedir,
		      homedir, homedir, homedir, homedir, homedir, homedir,
		      libdir, libdir, libdir, libdir, libdir, libdir, libdir,
		      libdir, incdir);
	    } 
	  else
	    {
  	      /* Solaris 2.6 Motif diff bug 4034689, 3 lines */
	      strncpy(old_path1, old_path, MAX_DIR_PATH_LEN -1);
	      old_path1[MAX_DIR_PATH_LEN -1] = '\0';
	      old_path = old_path1;
	      path = XtCalloc(1, (8*strlen(old_path) + 2*strlen(homedir) +
				  strlen(XAPPLRES_DEFAULT) + 8*strlen(libdir) +
				  strlen(incdir) + 1));
	      sprintf(path, XAPPLRES_DEFAULT, 
		      old_path, old_path, old_path, old_path, old_path, 
		      old_path, old_path, old_path, homedir, homedir,
		      libdir, libdir, libdir, libdir, libdir, libdir, libdir,
		      libdir, incdir);
	    }
	} 
      else
	{
	  path = XtMalloc(strlen(local_path) + 1);
	  strcpy (path, local_path);
	  *user_path = True;
	}
    }

  return path;
}

int
XmeMicroSleep(long usecs)
{
/* Solaris 2.7 bugfix # 4072243 - 8 lines */
/* Removed Alpha, SGI, CRAY dependant stuff : Prabhat */
  struct timeval      timeoutVal;
  
  /* split the micro seconds in seconds and remainder */
  timeoutVal.tv_sec = usecs/1000000;
  timeoutVal.tv_usec = usecs - timeoutVal.tv_sec*1000000;
  
/* Solaris 2.7 bugfix # 4072243 - 1 lines */
  return select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &timeoutVal);
}

/************************************************************************
 * 
 *	XmeGetLocalizedString	Map an X11 R5 XPCS string in a locale
 *				sensitive XmString.
 * 
 *		reserved	Reserved for future use.
 *		widget		The widget id.		
 *		resource	The resource name.	
 *		string		The input 8859-1 value.	
 * 
 ************************************************************************/

/*ARGSUSED*/
XmString
XmeGetLocalizedString(char *reserved,		/* unused */
		      Widget widget,		/* unused */
		      char *resource,		/* unused */
		      String string)
{
  return XmStringCreateLocalized(string);
}

/************************************************************************
 *									*
 *    _XmOSBuildFileName						*
 *									*
 *	Build an absolute file name from a directory and file.		*
 *	Handle case where 'file' is already absolute.			*
 *	Return value should be freed by XtFree()			*
 *									*
 ************************************************************************/

String
_XmOSBuildFileName(String path,
		   String file)
{
  String fileName;
  
  if (file[0] == '/') 
    {
      fileName = XtMalloc (strlen (file) + 1);
      strcpy (fileName, file);
    }
  else 
    {
      fileName = XtMalloc (strlen(path) + strlen (file) + 2);
      strcpy (fileName, path);
      strcat (fileName, "/");
      strcat (fileName, file);
    }
  
  return fileName;
}



/************************************************************
 *
 *  return poiinter to the file and the suffix
 *        /usr/foo/bar.xpm returns &"bar.xpm" and &"xpm"
 *
 ************************************************************/

void
_XmOSFindPathParts(String  path,
	      String *filenameRtn,
	      String *suffixRtn)
{
  String	filename = path, suffix = NULL;
  String	s;
  /*
   * maybe a problem for I18N - probably: filenames may be multibyte!!!
   */
#define FILESEP '/'
#define SUFFIXSEP '.'
  
  s = path;
  while (*s) 
    {
      if (*s == FILESEP) 
	{
	  filename = s++;
	}
      else if (*s == SUFFIXSEP) 
	{
	  suffix = s++;
	}
      else
	s++;
    }

  if (suffix < filename)
    suffix = NULL;

  if ((*filenameRtn = filename) != NULL)
    {
      if (filename != path)
	(*filenameRtn)++;
    }

  if ((*suffixRtn = suffix) != NULL)
    (*suffixRtn)++;
}



/************************************************************
 *
 *  Add _m to the imageName: 
 *        transform /usr/foo/bar.xpm in /usr/foo/bar_m.xpm
 *        or        joe in joe_m
 *
 ************************************************************/

void
_XmOSGenerateMaskName(
    String	imageName,
    String	maskNameBuf)
{
    String 	file, suffix;
    long		len; /* Wyoming 64-bit fix */ 

    _XmOSFindPathParts(imageName, &file, &suffix);

    if (suffix) {
	len = (suffix - imageName) - 1; /* Wyoming 64-bit fix */ 
	/* point before the '.' */
	suffix--;
    }
    else
      len = strlen(imageName);

    strncpy(maskNameBuf, imageName, len);
    maskNameBuf += len;
    strcpy(maskNameBuf, "_m");
    if (suffix) 
      strcpy(maskNameBuf+2, suffix);
    else
      maskNameBuf[2] = '\0';
}



/*ARGSUSED*/
Status
_XmOSGetInitialCharsDirection(XtPointer     characters,
			      XmTextType    type,
			      XmStringTag   locale, /* unused */
			      unsigned int *num_bytes,
			      XmDirection  *direction)
{
  /* ??? This is a temporary stub implementation. */
  switch (type)
    {
    case XmWIDECHAR_TEXT:
      *num_bytes = Wcslen((wchar_t*) characters) * sizeof(wchar_t);
      *direction = XmLEFT_TO_RIGHT;
      return Success;
      
    case XmCHARSET_TEXT:
    case XmMULTIBYTE_TEXT:
      *num_bytes = strlen((char*) characters);
      *direction = XmLEFT_TO_RIGHT;
      return Success;
      
    default:
      *num_bytes = 0;
      *direction = XmDEFAULT_DIRECTION;
      return ~Success;
    }
}

/*ARGSUSED*/
XmDirection
_XmOSGetCharDirection(XtPointer   character, /* unused */
		      XmTextType  type,
		      XmStringTag locale) /* unused */
{
  /* ??? This is a temporary stub implementation. */
  switch (type)
    {
    case XmWIDECHAR_TEXT:
    case XmCHARSET_TEXT:
    case XmMULTIBYTE_TEXT:
      return XmLEFT_TO_RIGHT;

    default:
      return XmDEFAULT_DIRECTION;
    }
}

static int
Wcslen(wchar_t *wcs)
{
  /* Count characters, not bytes. */
  wchar_t *ptr = wcs;
  if (ptr != NULL)
    while (*ptr++)
      /*EMPTY*/;

  return (ptr - wcs);
}

typedef struct XmOSMethodEntryRec {
  String    method_id;
  XtPointer method;
  XtPointer os_data;
  XtPointer reserved;   /* for future use - fonts & such?*/
} XmOSMethodEntry;


static XmOSMethodEntry method_table[] = {
  { 
    XmMCharDirection,
    (XtPointer)_XmOSGetCharDirection,
    NULL, NULL
  },

  { 
    XmMInitialCharsDirection,
    (XtPointer)_XmOSGetInitialCharsDirection,
    NULL, NULL
  },

  { NULL, NULL, NULL, NULL}
};

/****************************************************************
 * XmOSGetMethod:
 *   get the function that implements the requested method.
 ****************************************************************/

/*ARGSUSED*/
XmOSMethodStatus
XmOSGetMethod(Widget w,		/* unused */
	      String method_id, 
	      XtPointer *method,
	      XtPointer *os_data)
{
  int i;
  
  if (method == NULL)
    return XmOS_METHOD_NULL;
  
  for (i = 0; method_table[i].method_id; i++)
    if (method_id == method_table[i].method_id) 
      {
	if (*method == NULL || (method_table[i].method != NULL && 
				*method != method_table[i].method)) 
	  {
	    *method = method_table[i].method;
	    if (os_data) *os_data = method_table[i].os_data;
	    return XmOS_METHOD_REPLACED;
	  } 
	else
	  {
	    if (os_data) *os_data = method_table[i].os_data;
	    return XmOS_METHOD_DEFAULTED;
	  }
      }
  
  for (i = 0; method_table[i].method_id; i++)
    if (strcmp(method_id, method_table[i].method_id) == 0) 
      {
	if (*method == NULL || (method_table[i].method != NULL && 
				*method != method_table[i].method)) 
	  {
	    *method = method_table[i].method;
	    if (os_data) *os_data = method_table[i].os_data;
	    return XmOS_METHOD_REPLACED;
	  } 
	else
	  {
	    if (os_data) *os_data = method_table[i].os_data;
	    return XmOS_METHOD_DEFAULTED;
	  }
      }
  
  return XmOS_METHOD_DEFAULTED;
}

/*
 * This routine is used by Label (and LabelG) to determine which
 * character in the label string matches the Mnemonic keysym, and
 * thus should be underlined.
 *
 * Parameters:
 *	keysym - Specifies the keysym to be converted.
 *	locale - Specifies the locale to convert into.  NULL => current locale.
 *	buffer - A buffer allocated by the caller to hold the (at most one)
 *		multibyte character that corresponds to the keysym.
 *
 * Return value:
 *	The number of bytes written into the buffer.
 */

/*ARGSUSED*/
int 
_XmOSKeySymToCharacter(KeySym keysym,
		       char  *locale,
		       char  *buffer)
{
  /* 
   * This implementation is exceptionally stupid, but works in the
   * common case of ISO8859-1 locales and keysyms.
   *
   * Vendors who use more exotic encodings (e.g. roman8) should
   * replace this code with something appropriate.
   */

  /* Maybe we should generate a warning for non-Latin 1 encodings */
  /* outside the range 0..127? */
  *buffer = (keysym & 0xFF);

  return 1;
}
