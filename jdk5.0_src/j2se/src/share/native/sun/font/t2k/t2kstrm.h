/*
 * @(#)t2kstrm.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * T2KSTRM.H
 * Copyright (C) 1989-1997 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
 * Author: Sampo Kaasila
 *
 * This software is the property of Type Solutions, Inc. and it is furnished
 * under a license and may be used and copied only in accordance with the
 * terms of such license and with the inclusion of the above copyright notice.
 * This software or any other copies thereof may not be provided or otherwise
 * made available to any other person or entity except as allowed under license.
 * No title to and ownership of the software or intellectual property
 * therewithin is hereby transferred.
 *
 * This information in this software is subject to change without notice
 */
#ifndef __T2K_STREAM__
#define __T2K_STREAM__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#ifdef ENABLE_NON_RAM_STREAM
typedef void (*PF_READ_TO_RAM) ( void *id, tt_uint8 *dest_ram, tt_int32 offset, tt_int32 numBytes );
#endif

#ifdef JUST_AN_EXAMPLE_OF_PF_READ_TO_RAM
void ReadFileDataFunc( void *id, tt_uint8 *dest_ram, tt_int32 offset, tt_int32 numBytes )
{
	int error;
	size_t count;
	FILE *fp = (FILE *)id;

	assert( fp != NULL );
	/* A real version of this function should only for instance call fseek if there is a need */
	error	= fseek( fp, offset, SEEK_SET ); assert( error == 0 );
	count	= fread( dest_ram, sizeof( char ), numBytes, fp );
	assert( ferror(fp) == 0 && count == (size_t)numBytes );
}
#endif


#ifdef ENABLE_NON_RAM_STREAM
/* #define USE_PRE_CACHING */
#define USE_PRE_CACHING
#endif /* ENABLE_NON_RAM_STREAM */


typedef struct {
	/* private */
	unsigned char *privateBase;
#ifdef ENABLE_NON_RAM_STREAM
	PF_READ_TO_RAM 	ReadToRamFunc;
	void 			*nonRamID;
	tt_uint8			tmp_ch;
#ifdef USE_PRE_CACHING
  /* Increased this from 512 to 8192 to minimise the number of I/O calls
   * which are java up-calls in JDK 1.5.
   */
	#define PRE_CACHE_SIZE 8192
	tt_uint8 cacheBase[ PRE_CACHE_SIZE ];
	tt_int32 bytesLeftToPreLoad;
#endif
	tt_uint32 cacheCount;
	tt_uint32 cachePosition; /* set to >= 0 when we set privateBase == cacheBase */
#endif
	tt_uint32 pos;
	tt_uint32 maxPos; /* one past the last legal position */
	tt_uint32 posZero;
	char constructorType;
	tsiMemObject *mem;
	
	tt_uint32 bitBufferIn,  bitCountIn;  /* "left"  aligned. */
	/* public */

} InputStream;

#ifdef USE_PRE_CACHING
void PreLoadT2KInputStream( InputStream *t, tt_int32 requestedByteCount );
int PrimeT2KInputStream(InputStream *t );
#define EnsureWeHaveDataInT2KInputStream( stream, n ) \
	 ( ( stream->pos - stream->cachePosition + (n) > stream->cacheCount ) ? \
     PrimeT2KInputStream( stream ) : 0 )
#endif /* USE_PRE_CACHING */

/* ALL external clients (top level call to scaler) need to set errCode pointer,
   ALL INTERNAL clients neet to set the errCode pointer == NULL, so that
   we only do setjmp for the top-most external call
   This applies to the 4 constructors and the one destructor
*/
/* Does free data */
InputStream *New_InputStream( tsiMemObject *mem, unsigned char *data, tt_uint32 length, int *errCode );
InputStream *New_InputStream2( tsiMemObject *mem, InputStream *in, tt_uint32 offset, tt_uint32 length, int *errCode );
/* Does not free data */
InputStream *New_InputStream3( tsiMemObject *mem, unsigned char *data, tt_uint32 length, int *errCode );

#ifdef ENABLE_NON_RAM_STREAM
InputStream *New_NonRamInputStream( tsiMemObject *mem, void *nonRamID, PF_READ_TO_RAM readFunc, tt_uint32 length, int *errCode );
#endif

void Delete_InputStream( InputStream *t, int *errCode );

tt_int32 SizeInStream( InputStream *stream );

tt_int32 ReadInt32( InputStream *stream );
tt_int16 ReadInt16( InputStream *stream );

#define EOF_STREAM -1

#ifdef ENABLE_NON_RAM_STREAM

#ifdef USE_PRE_CACHING
#define ReadUnsignedByteMacro( stream ) \
( (tt_uint8) (stream->privateBase != NULL ? \
    ( stream->ReadToRamFunc != NULL ? \
    	EnsureWeHaveDataInT2KInputStream( stream, 1 ), stream->privateBase[(stream->pos)++ - stream->cachePosition] : \
       stream->privateBase[(stream->pos)++] ) : \
    ( stream->ReadToRamFunc( stream->nonRamID, &(stream->tmp_ch), (stream->pos)++, 1 ), stream->tmp_ch))  ) 
#else
	#define ReadUnsignedByteMacro( stream ) ( (tt_uint8) (stream->privateBase != NULL ? (stream->privateBase[(stream->pos)++]) : ( stream->ReadToRamFunc( stream->nonRamID, &(stream->tmp_ch), (stream->pos)++, 1 ), stream->tmp_ch))  ) 
#endif

#define ReadUnsignedByteMacro2( stream ) ( (int)(stream->pos >= stream->maxPos ? EOF_STREAM : (ReadUnsignedByteMacro(stream)) ) ) 

#else /* ENABLE_NON_RAM_STREAM */

#define ReadUnsignedByteMacro( stream ) ( (tt_uint8)(stream->privateBase[(stream->pos)++]) ) 
#define ReadUnsignedByteMacro2( stream ) ( (int)(stream->pos >= stream->maxPos ? EOF_STREAM : stream->privateBase[(stream->pos)++]) ) 

#endif /* ENABLE_NON_RAM_STREAM */

unsigned char *GetEntireStreamIntoMemory( InputStream *stream  );
void ReadSegment( InputStream *stream, tt_uint8 *dest, tt_int32 numBytes );

void Rewind_InputStream( InputStream *t );
void Seek_InputStream( InputStream *t, tt_uint32 offset );
tt_uint32 Tell_InputStream( InputStream *t );


typedef struct {
	/* private */
	unsigned char *base;
	tt_uint32 maxPos;
	tt_uint32 pos;
	tt_uint32 maxLength;
	tsiMemObject *mem;

	tt_uint32 bitBufferOut, bitCountOut; /* "left"  aligned. */
	/* public */

} OutputStream;

#define Tell_OutputStream( out ) (out->pos)

#define GET_POINTER( out ) ( out->base )
OutputStream *New_OutputStream( tsiMemObject *mem, tt_int32 initialSize );

void WriteBitsToStream( OutputStream *out, tt_uint32 bits, tt_uint32 count );
/* When done with all calls to WriteBitsToStream call this to flush remaining
  data to the stream  */
void FlushOutStream( OutputStream *out );

void WriteInt32( OutputStream *stream, tt_int32 value );
void WriteInt16( OutputStream *stream, tt_int16 value );
void WriteUnsignedByte( OutputStream *stream, tt_uint8 value );
void Write( OutputStream *stream, tt_uint8 *src, tt_int32 numBytes );
tt_int32 SizeOutStream( OutputStream *stream ); /* max size/position seen */
tt_int32 OutStreamPos( OutputStream *stream );  /* current size/position */
void Rewind_OutputStream( OutputStream *t );

void Delete_OutputStream( OutputStream *t );

/* uses a variable number of bytes */
void WriteUnsignedNumber( OutputStream *out, tt_uint32 n );
tt_uint32 ReadUnsignedNumber( InputStream *in );

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_STREAM__ */
