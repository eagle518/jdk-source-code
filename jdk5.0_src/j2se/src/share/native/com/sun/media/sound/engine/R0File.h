/*
 * @(#)R0File.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__R0FILE_H__)

#define __R0FILE_H__

#if defined(__cplusplus)
extern "C"
{
#endif



    int R0FILE_open( const char *filename, int oflag, int pmode );

    // Valid values for oflag
    //
    // Notes:
    //   R0FILE_O_RDONLY cannot be specified with R0FILE_RDWR or R0FILE_WRONLY
    //   R0FILE_O_WRONLY cannot be specified with R0FILE_RDWR or R0FILE_RDONLY
    //   R0FILE_O_RDWR   cannot be specified with R0FILE_RDONLY or R0FILE_WRONLY
#define R0FILE_O_CREAT    (1<<1)
#define R0FILE_O_RDONLY   (1<<2)
#define R0FILE_O_RDWR     (1<<3)
#define R0FILE_O_WRONLY   (1<<4)
#define R0FILE_O_TRUNC    (1<<5)
#define R0FILE_O_APPEND   (1<<6)
#define R0FILE_O_EXCL     (1<<7)

    // Valid values for pmode
    //
    // Notes:
    //   To open with Read/Write access, specify R0FILE_S_IREAD | R0FILE_S_IWRITE
#define R0FILE_S_IREAD      (1<<1)
#define R0FILE_S_IWRITE     (1<<2)
#define R0FILE_S_IREADWRITE ( R0FILE_S_IREAD | R0FILE_S_IWRITE )

    int R0FILE_creat( const char *filename, int pmode );

    int R0FILE_close( int handle );

    int R0FILE_read( int handle, void *buffer, unsigned int count );

    int R0FILE_write( int handle, const void *buffer, unsigned int count );

    long R0FILE_lseek( int handle, long offset, int origin );

    int R0FILE_chsize( int handle, unsigned newSize);

    // Valid values for origin
#define R0FILE_SEEK_SET (0)
#define R0FILE_SEEK_CUR (1)
#define R0FILE_SEEK_END (2)


    // Return Codes
#define R0FILE_ERROR   (-1)
#define R0FILE_SUCCESS  (0)



#if defined(__cplusplus)
}
#endif


#endif

