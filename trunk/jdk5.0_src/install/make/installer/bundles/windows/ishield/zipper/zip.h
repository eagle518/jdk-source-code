/*
 *  @(#)zip.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifdef _MSC_VER 
#include <windows.h>
#include <winuser.h>
#include <windef.h>
#include <io.h>
#include <string.h>
#include <mapiwin.h>
#include <direct.h>
#define MKDIR(dir) 	mkdir(dir)
#define getpid() 	_getpid()
#define dup2(a,b)	_dup2(a,b)
#define strcasecmp(s1, s2) _stricmp(s1,s2)
#endif


void openZipFileReader(char *fname);
void closeZipFileReader();
void do_read(char *, char *);
void do_write(char *, char *);
void set_remove_input_file();

#define ushort unsigned short
#define uint   unsigned int
#define uchar  unsigned char

#ifdef sparc
#define SWAP_BYTES(a) \
    ((((a) << 8) & 0xff00) | 0x00ff) & (((a) >> 8) | 0xff00)
#else
#define SWAP_BYTES(a)  (a)
#endif
