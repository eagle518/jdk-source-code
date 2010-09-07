/* $XConsortium: IconFile.c /main/12 1996/11/21 20:03:53 drk $ */
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
/* This module references:
     XmeGetHomeDirName, _XmOSFindPathParts, _XmOSAbsolutePathName (new in Xmos)
     _XmOSInitPath (new version that uses absolutepath)
     _XmInImageCache (in ImageCache)
     XmeGetIconControlInfo (in ColorObj)
     _XmHash API
   and it exports:
     XmeFlushIconFileCache  (used by CDE)
     XmGetIconFileName      (used by ImageCache)
   It is still Dt centric for the PATH variables and the local variables.
*/

#include <stdio.h>
#include <locale.h>
#include <X11/Xos.h>
#include <Xm/IconFileP.h>
#include <Xm/ColorObjP.h>
#include "XmI.h"
#include "HashI.h"
#include "XmosI.h"
#include "ImageCachI.h"

/**************** vendor dependant defaults ********/
/* All this stuff (cached dir) should be moved and possibly merged
   in Xmos.c, where it belongs */


#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#include <unistd.h>
#endif


#include <sys/types.h>


/* Solaris 2.7 bugfix # 4072756 - 1 lines */
#define X_INCLUDE_DIRENT_H
#define XOS_USE_XT_LOCKING
#include <Xm/Xmos_r.h>

#include <fcntl.h>
#include <sys/stat.h>


#ifdef USE_GETWD
#include <sys/param.h>
#define MAX_DIR_PATH_LEN    MAXPATHLEN
#define getcwd(buf, len)   ((char *) getwd(buf))
#else
#define MAX_DIR_PATH_LEN    1024
#endif
#define MAX_USER_NAME_LEN   256

#ifndef S_ISDIR
#define S_ISDIR(m) ((m & S_IFMT)==S_IFDIR)
#endif

/**************** end of vendor dependant defaults ********/




/**************** Icon PATH defines ********/


/* static XmConst char ABSOLUTE_IPATH[] = "19 Oct 1995B"; */
static _XmConst char ABSOLUTE_IPATH[] = {'%', 'H', '%', 'B', '\0'};
static XmConst char ABSOLUTE_PATH[] = "\
%P\
%S";


/******------------------------------------------********/

typedef union _DtCachedDirStruct *DtCachedDir;

typedef struct _DtCommonCachedDirStruct{
    ushort		cachedDirType;
    ushort		dirNameLen;
    String		dirName;
}DtCommonCachedDirStruct, *DtCommonCachedDir;

typedef struct _DtValidCachedDirStruct{     
    ushort		cachedDirType;
    ushort		dirNameLen;
    String		dirName;
    int			numFiles;
    /*
     * we allocate both the offsets array and the names themselves in
     * a heap hanging off the end of this struct
     */
    ushort		nameOffsets[1];
    /*
    String		names
    */
}DtValidCachedDirStruct, *DtValidCachedDir;

#define DtVALID_CACHED_DIR	0
#define DtINVALID_CACHED_DIR	1
#define DtUNCACHED_DIR		2

#define MAX_CACHE_DIR_SIZE	(1L << 16)

typedef union _DtCachedDirStruct{
    DtCommonCachedDirStruct	common;
    DtValidCachedDirStruct 	valid_dir;
}DtCachedDirStruct;

typedef struct _DtCachedDirListStruct{
    int			numDirs;
    int			maxDirs;
    DtCachedDir	*dirs;
}DtCachedDirListStruct;


/********    Static Function Declarations    ********/

static DtCachedDir MakeCachedDirEntry( 
                        String dirName) ;
static int CheckDirCache( 
                        String path);
static Boolean TestIconFile( 
                        String path) ;
static Boolean CompareIconNames (XmHashKey key_1, XmHashKey key_2);
static XmHashValue HashIconName (XmHashKey key);

/********    End Static Function Declarations    ********/






static DtCachedDir 
MakeCachedDirEntry(String dirName)
{
    DIR * 		fileDesc = NULL;
    struct dirent	*currDirect;
    DtCachedDir 	cachedDir;
    int			cachedDirType;

    if ((fileDesc = opendir (dirName)) == NULL)  {
	/* invalid directory */
	cachedDirType = DtINVALID_CACHED_DIR;
    }
    else {
	int	bufLen, oldBufLen = 0;
	char	stackBuf[MAX_CACHE_DIR_SIZE];
	int	numFiles = 0;
	int	nameHeapSize = 0;
	_Xreaddirparams dirEntryBuf;

        cachedDirType = DtVALID_CACHED_DIR;
	while ((currDirect = _XReaddir(fileDesc, dirEntryBuf)) != NULL) {
	  bufLen = currDirect->d_reclen;
	  if (bufLen + oldBufLen >= MAX_CACHE_DIR_SIZE) {
	    /*
	     * don't cache this one
	     */
	    cachedDirType = DtUNCACHED_DIR;
	    break;
	  } else {
	    (void) memcpy(&(stackBuf[oldBufLen]), (void *)currDirect, bufLen);
	    oldBufLen += bufLen;
	  }
	}
	if (oldBufLen == 0) {
	  /* invalid entry */
	  cachedDirType = DtINVALID_CACHED_DIR;
	}

        if( cachedDirType == DtVALID_CACHED_DIR)
        {
	    DtValidCachedDir	validDir;
	    String		nameHeap;
	    Cardinal		i;

	    for (currDirect = (struct dirent *)stackBuf;
		 ((char *)currDirect - stackBuf) < oldBufLen;
		 currDirect = (struct dirent *) 
		 (((char *)currDirect) + currDirect->d_reclen)) {

		numFiles++;
		nameHeapSize += strlen (currDirect->d_name);
	    }
	    /*
	     * we allocate an extra nameOffset to track the length of
	     * the last name
	     */
	    validDir = (DtValidCachedDir) 
	      XtMalloc((sizeof(DtValidCachedDirStruct)) +
		       (sizeof(short) * numFiles) +
		       (nameHeapSize));

	    validDir->dirNameLen = strlen(dirName);
	    validDir->dirName = dirName;
	    validDir->numFiles = numFiles;
	    cachedDirType = 
	      validDir->cachedDirType = 
		DtVALID_CACHED_DIR;
	    validDir->nameOffsets[0] = 0;
	    nameHeap = (String)
	      &(validDir->nameOffsets[numFiles + 1]);

	    for (i = 0, currDirect = (struct dirent *)stackBuf;
		 i < validDir->numFiles;
		 i++,
		 currDirect = (struct dirent *) 
		 (((char *)currDirect) + currDirect->d_reclen)) {

		validDir->nameOffsets[i + 1] = 
		  validDir->nameOffsets[i] + (short)strlen (currDirect->d_name); /* Wyoming 64-bit Fix */
		memcpy(&(nameHeap[validDir->nameOffsets[i]]),
		       &(currDirect->d_name[0]),
		       strlen (currDirect->d_name));
	    }
          cachedDir = (DtCachedDir)validDir ;
	}
    }
    switch (cachedDirType) {
      case DtINVALID_CACHED_DIR:
      case DtUNCACHED_DIR:
	cachedDir = (DtCachedDir)
	  XtMalloc(sizeof(DtCommonCachedDirStruct));
	cachedDir->common.cachedDirType =
	  cachedDirType;
	cachedDir->common.dirNameLen = strlen(dirName);
	cachedDir->common.dirName = dirName;
	break;
      case DtVALID_CACHED_DIR:
	break;
    }
    if (fileDesc != NULL)
      closedir(fileDesc);
    return cachedDir;
}

static 	DtCachedDirListStruct cacheList;



void
XmeFlushIconFileCache(String	path)
{
    size_t  	dirNameLen; /* Wyoming 64-bit Fix */
    Cardinal 	i;

    _XmProcessLock();

    /*
     * loop thru the dir list. if no path was specified then flush the
     * entire cache.
     */
    if (path) 
      dirNameLen = strlen(path);
    else
      dirNameLen = 0;
    for (i = 0; i < cacheList.numDirs; i++) {
	DtValidCachedDir	currDir;

	currDir = (DtValidCachedDir)cacheList.dirs[i];
	if (!path ||
	    ((currDir->dirNameLen == dirNameLen) &&
	     (strncmp(currDir->dirName, path, dirNameLen) == 0))) {
            /* bug fix for 4108377 - a little hack will cause mem leak */
	    XtFree((char *)currDir);

	    if (path) {
		/* ripple down the dir array */
		for (; i < cacheList.numDirs - 1; i++)
		  cacheList.dirs[i] = cacheList.dirs[i+1];
		cacheList.numDirs--;
		_XmProcessUnlock();
		return;
	    }
	}
    }
    if (path && (i == cacheList.numDirs)) {
	_XmProcessUnlock();
	return;
    }
    cacheList.numDirs = 0;
    /* don't free the dirList itself */
    _XmProcessUnlock();
}


#ifndef XTHREADS
static String GdirName;
static String GleafName;
#endif

static int
CheckDirCache(String	path)

{
    String	dirName;
    String	filePtr;
    String	suffixPtr;
    int		numDirs;
	long    dirNameLen, fileNameLen; /* Wyoming 64-bit Fix */
    Cardinal	i, j;
    char   	stackString[MAX_DIR_PATH_LEN];
    
    (void) _XmOSAbsolutePathName(path, &path, stackString);
    _XmOSFindPathParts(path, &filePtr, &suffixPtr);
    
    if (path == filePtr) {
	dirNameLen = 0;
	fileNameLen = strlen(path);
    }
    else {
	/* take the slash into account */
	dirNameLen = filePtr - path - 1;
	fileNameLen = strlen(path) - dirNameLen - 1;
    }

    /*
     * set global variable for later use
     */
#ifndef XTHREADS
    GleafName = filePtr;
#endif

    if (dirNameLen == 0) {
	return DtINVALID_CACHED_DIR;
    }

    /*
     * loop thru the dir list. on the last pass create the new cached
     * dir and process it.  
     */
    _XmProcessLock();

    numDirs = cacheList.numDirs;
    for (i = 0; i <= numDirs; i++) {
	String			currName;
	int			currNameLen;
	String			nameHeap;
	DtValidCachedDir	currDir;
	
	if (i == cacheList.numDirs) {

	    /*
	     * we didn't get a hit on the directory list so create a new one
	     */
	    if (cacheList.numDirs == cacheList.maxDirs) {
		cacheList.maxDirs += 16;
		cacheList.dirs = (DtCachedDir *)
		  XtRealloc((char *)cacheList.dirs,
			    cacheList.maxDirs * sizeof (DtCachedDir));
	    }
	    dirName = strncpy(XtMalloc(dirNameLen+1), path, dirNameLen);
	    dirName[dirNameLen] = '\0';
	    cacheList.dirs[cacheList.numDirs++] = MakeCachedDirEntry(dirName);
	}
	currDir = (DtValidCachedDir)cacheList.dirs[i];

	/*
	 * set global variable
	 */
#ifndef XTHREADS
	GdirName = currDir->dirName;
#endif

	if ((currDir->dirNameLen == dirNameLen) &&
	    (strncmp(currDir->dirName, path, dirNameLen) == 0)) {

	    switch(currDir->cachedDirType) {
	      case DtINVALID_CACHED_DIR:
	      case DtUNCACHED_DIR:
		_XmProcessUnlock();
		return currDir->cachedDirType;
		break;
	      case DtVALID_CACHED_DIR:
		nameHeap = (String)
		  &(currDir->nameOffsets[currDir->numFiles + 1]);
		for (j = 0; j < currDir->numFiles; j++) {
		    /*
		     * nameOffsets has an extra offset to indicate the
		     * end of the last name (to handle border condition
		     */
		    currNameLen = (currDir->nameOffsets[j + 1] -
				   currDir->nameOffsets[j]);
		    if (currNameLen == fileNameLen) {
			currName =  &(nameHeap[currDir->nameOffsets[j]]);
			if (strncmp(currName, filePtr, currNameLen) == 0) {
			    _XmProcessUnlock();
			    return DtVALID_CACHED_DIR;
			}
		    }
		}
		_XmProcessUnlock();
		return DtINVALID_CACHED_DIR;
	    }
	}
    }
    _XmProcessUnlock();
    return DtINVALID_CACHED_DIR;
}

static String
find_slash(String str)
{
  int n;
  if (MB_CUR_MAX == 1) {
      return strchr(str, '/');
  } else {
      while ((n = mblen(str, MB_CUR_MAX)) != 0) {
        if (n == 1 && *str == '/')
            return str;
        str += n == -1 ? 1 : n;
      }
      return NULL;
  }
}

static Boolean
TestIconFile(String	path)
{
    struct stat status;
    int	dirCacheType;

    if (!path || !*path)
      return False;

    /* if there is no directory information in the name, it's
       a local file, check here or CheckDirCache will fail */
    
    if (!find_slash(path)) {
	dirCacheType = DtUNCACHED_DIR ;
#ifndef XTHREADS
	GleafName = path ;
	GdirName = "." ;
#endif
    } else
	dirCacheType = CheckDirCache(path);

    switch(dirCacheType) {
      case DtVALID_CACHED_DIR:
	return True;

      case DtINVALID_CACHED_DIR:
	return False;

      case DtUNCACHED_DIR:
	return (access(path, R_OK) == 0 &&	/* exists and is readable */
		stat(path, &status) == 0 &&	/* get the status */
		S_ISDIR(status.st_mode) == 0	/* not a directory */
		);
    }

    return False ;
}


/*********** Hash table stuff  */


typedef struct _DtIconNameEntryRec{
    String	dirName;
    String	leafName;
    String	key_name;  
}DtIconNameEntryRec, *DtIconNameEntry;


/* Compare two icon names from icon entry rec */
static Boolean 
CompareIconNames (XmHashKey key_1, 
		XmHashKey key_2)
{
  DtIconNameEntry data_1 = (DtIconNameEntry) key_1;
  DtIconNameEntry data_2 = (DtIconNameEntry) key_2;

  return ((data_1->key_name == data_2->key_name) || 
	  (strcmp(data_1->key_name, data_2->key_name) == 0));
}


/* Hash an icon name . */
static XmHashValue 
HashIconName (XmHashKey key)
{
  DtIconNameEntry data = (DtIconNameEntry) key;
  size_t len = strlen(data->key_name); /* Wyoming 64-bit Fix */

  return (((len << 8) | data->key_name[0]) << 8) | data->key_name[len];
}


String
XmGetIconFileName(
    Screen	*screen,		     
    String	imageInstanceName,
    String	imageClassName,
    String	hostPrefix,
    unsigned int size)
{
    Display		*display = DisplayOfScreen(screen);
    String		fileName = NULL;
    String		names[2];
    String		names_w_size[2] = {0,0};
    XmConst char       *bPath, *iPath;
    Cardinal		i;
    Boolean		useColor;
    Boolean		useMask;
    Boolean		useIconFileCache;
    Boolean		absolute;
    XtFilePredicate	testFileFunc;
    String		homedir = NULL ;
    static String	iconPath = NULL;
    static String	bmPath = NULL;
    static XmHashTable iconNameCache = NULL;
    char 		stackString[MAX_DIR_PATH_LEN];
    
#define B_SUB	0
#define P_SUB	1
#define M_SUB	2
#define H_SUB	3

    SubstitutionRec iconSubs[] = {
	{'B', NULL},	/* bitmap name */
	{'P', NULL},	/* alternate bitmap name BC */
	{'M', NULL},	/* magnitude */
	{'H', NULL},	/* host prefix */
    };

    XtAppContext app;

    app = XtDisplayToApplicationContext(display);
    
    _XmAppLock(app);

    /* start by asking some screen state */
    (void)XmeGetIconControlInfo(screen, 
				&useMask,  /* not used here */
				&useColor,
				&useIconFileCache);

    _XmProcessLock();

    /* generate the icon paths once per application: iconPath and bmPath */
    if (!iconNameCache) {
	String 		tmpPath;
	Boolean		junkBoolean;

/* Solaris 2.6 Motif diff bug 4085003 1 line */

	iconNameCache =  _Xm21AllocHashTable(100, 
				   CompareIconNames, HashIconName);
    
	cacheList.numDirs =
	  cacheList.maxDirs = 0;
	cacheList.dirs = NULL;

	homedir = XmeGetHomeDirName();
	strcpy(stackString, homedir) ;

	if (useColor) {
	    tmpPath = getenv("XMICONSEARCHPATH");
	}
	else {
	    tmpPath = getenv("XMICONBMSEARCHPATH");
	}
	if (tmpPath) iconPath = XtNewString(tmpPath);
	else iconPath = XtNewString(ABSOLUTE_IPATH);

	/* 1.2 path as a fallback */
	bmPath = _XmOSInitPath(NULL, "XBMLANGPATH", &junkBoolean);

    }

    switch (size) {
      case XmTINY_ICON_SIZE:
	iconSubs[M_SUB].substitution = ".t";
	break;
      case XmSMALL_ICON_SIZE:
	iconSubs[M_SUB].substitution = ".s";
	break;
      case XmMEDIUM_ICON_SIZE:
	iconSubs[M_SUB].substitution = ".m";
	break;
      case XmLARGE_ICON_SIZE:
	iconSubs[M_SUB].substitution = ".l";
	break;
      case XmUNSPECIFIED_ICON_SIZE:
	iconSubs[M_SUB].substitution = NULL;
	break;
    }

    iconSubs[H_SUB].substitution = hostPrefix;
 
    if (useIconFileCache)
      testFileFunc = TestIconFile;
    else
      testFileFunc = NULL;

    names[0] 	    = imageInstanceName;
    names[1] 	    = imageClassName;
    names_w_size[0] = names_w_size[1] = (String)NULL;

    /** loop over the two names */
    for (i = 0; i < 2; i++) {

	if (names[i] == NULL)
	  continue;

	if ((absolute = _XmOSAbsolutePathName(names[i], &names[i], 
					      stackString)) != FALSE) {
	    iPath = ABSOLUTE_IPATH;
	    bPath = ABSOLUTE_PATH;
	}
	else {
	    iPath = iconPath;
	    bPath = bmPath;
	}

	iconSubs[B_SUB].substitution = names[i];
	iconSubs[P_SUB].substitution = names[i];

       /* need to add size suffix if size is specified */
        if (size != XmUNSPECIFIED_ICON_SIZE) {
           long basenameLen = strlen(names[i]); /* Wyoming 64-bit Fix */
           long sizeLen = strlen(iconSubs[M_SUB].substitution); /* Wyoming 64-bit Fix */
	   char * ext_name = XtMalloc(basenameLen + sizeLen + 1);
	   /* XmosP.h takes care of bcopy translation */
	   memmove(&ext_name[0], names[i], basenameLen);
	   memmove(&ext_name[basenameLen],
	           iconSubs[M_SUB].substitution, sizeLen);
	   ext_name[basenameLen + sizeLen] = '\0';

           names_w_size[i] = ext_name;

        } else
           names_w_size[i] = NULL;

       /*
        * try to see if its already in the image cache
	*/
/* Solaris 2.6 Motif diff bug 4085003 1 line */

	if (_Xm21InImageCache(names[i]))
	  fileName = XtNewString(names[i]);


	/*
	 * optimization to check all expansions in cache
	 */
	if (!fileName) {
	    DtIconNameEntry iNameEntry;
	    DtIconNameEntryRec  iNameData ;

	    iNameData.key_name = (names_w_size[i])?names_w_size[i]:names[i];

	    iNameEntry =  (DtIconNameEntry) 
		_XmGetHashEntry(iconNameCache, (XmHashKey)&iNameData);

	    if (iNameEntry) {
		long dirLen, leafLen; /* Wyoming 64-bit Fix */

		dirLen = strlen(iNameEntry->dirName);
		leafLen = strlen(iNameEntry->leafName);
		fileName = XtMalloc(dirLen + leafLen + 2);

		memmove(&fileName[0],
	      		iNameEntry->dirName,
			dirLen);
		fileName[dirLen] = '/';
		memmove(&fileName[dirLen + 1],
	      		iNameEntry->leafName,
			leafLen);

		fileName[dirLen + leafLen + 1] = '\0';
	    }
	}

	if (fileName) {
	  /*
	   * CDExc20823 (memory leak): free names_w_size[i]
	   * if it is not NULL.
	   * NOTE: This code could be reorganized to do
	   *   _Xm21InImageCache() at the top of this loop
	   *   so we could avoid unnecessary malloc's for
	   *   names_w_size, but I wanted to minimize the
	   *   code impact of this defect for now.
	   */
	  for (i = 0; i < 2; i++)
	  {
	    if (names_w_size[i] != (String)NULL)
	      XtFree(names_w_size[i]);
	  }

	  _XmProcessUnlock();
	  _XmAppUnlock(app);
	  return fileName;
        }

	/*******************************
	 * first try XPM and then XBM
	 ******************************/
	fileName = 
	  XtResolvePathname(display, "icons", NULL,
			    NULL, iPath, iconSubs, 
			    XtNumber(iconSubs),
			    (XtFilePredicate) testFileFunc);
	
	if (fileName == NULL) {
	    fileName = 
	      XtResolvePathname(display, "bitmaps", NULL,
				NULL, bPath, iconSubs, 
				XtNumber(iconSubs),
				(XtFilePredicate) testFileFunc);
	}

	if (fileName)
	  break;
    }
    _XmProcessUnlock();

    if (fileName && !absolute) {
	/* register it in name cache */
	DtIconNameEntry 	iNameEntry;
	String name_used = (names_w_size[i])? names_w_size[i] : names[i] ;

	/** alloc a icon cache entry **/
	iNameEntry = (DtIconNameEntry) XtMalloc(sizeof(DtIconNameEntryRec));
	iNameEntry->key_name = XtNewString(name_used);
      
#ifndef XTHREADS
        if (useIconFileCache)
        {
	   iNameEntry->dirName = GdirName;
	   iNameEntry->leafName = XtNewString(GleafName);
        }
        else
#endif
        {
           String	dirName;
           String	filePtr;
           String 	suffixPtr;
           long	dirNameLen; /* Wyoming 64-bit Fix */

           _XmOSFindPathParts(fileName, &filePtr, &suffixPtr);

           if (fileName == filePtr)
	       dirNameLen = 0;
           else {
	       /* take the slash into account */
	       dirNameLen = filePtr - fileName - 1;
           }

           dirName = (String)XtMalloc(dirNameLen + 1);
	   strncpy(dirName, fileName, dirNameLen);
	   dirName[dirNameLen] = '\0';

	   iNameEntry->dirName = dirName;
	   iNameEntry->leafName = XtNewString(filePtr);
        }

	_XmProcessLock();
        _XmAddHashEntry(iconNameCache, (XmHashKey)iNameEntry, 
			(XtPointer)iNameEntry);
	_XmProcessUnlock();
    }

    /*
     * CDExc20823 (memory leak): free names_w_size[i] if not NULL.
     */
    for (i = 0; i < 2; i++)
    {
	if (names_w_size[i] != (String)NULL)
	    XtFree(names_w_size[i]);
    }

    _XmAppUnlock(app);
    return fileName;
}
    
      





