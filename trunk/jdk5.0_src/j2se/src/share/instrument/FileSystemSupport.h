/*
 * @(#)FileSystemSupport.h	1.2 04/06/09 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "FileSystemSupport_md.h"

/**
 * Return the local filesystem's path-separator character.
 */
char pathSeparator();

/**
 * Compare two filenames represent and tell if they represent the same file
 * or not.
 */
int filenameStrcmp(const char* s1, const char* s2);

/**
 * Post-process the given URI path string if necessary.  This is used on
 * win32, e.g., to transform "/c:/foo" into "c:/foo".  The path string
 * still has slash separators; code in the File class will translate them
 * after this method returns.
 */
char* fromURIPath(const char* path);

/**
 * Return the basen path of the given pathname. If the string is already
 * the base path then it is simply returned.
 */
char* basePath(const char* path);

/**
 * Convert the given pathname string to normal form.  If the string is
 * already in normal form then it is simply returned.
 */
char* normalize(const char* path);

/**
 * Tell whether or not the given abstract pathname is absolute.
 */
int isAbsolute(const char * path);

/**
 * Resolve the child pathname string against the parent.
 */
char* resolve(const char* parent, const char* child);

/**
 * Convert a pathname to canonical form. 
 * -- compiled in from src/<platform>/native/java/io/canonicalize_md.c
 */
int canonicalize(char *original, char *resolved, int len);

