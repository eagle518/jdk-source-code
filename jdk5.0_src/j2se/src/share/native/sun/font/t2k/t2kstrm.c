/*
 * @(#)t2kstrm.c	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * T2KSTRM.C
 * Copyright (C) 1989-1998 all rights reserved by Type Solutions, Inc. Plaistow, NH, USA.
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
#include "syshead.h"

#include "dtypes.h"
#include "config.h"
#include "tsimem.h"
#include "t2kstrm.h"



#ifdef USE_PRE_CACHING
void PreLoadT2KInputStream( InputStream *t, tt_int32 requestedByteCount )
{
	if ( t->ReadToRamFunc != NULL ) {
		tt_int32 byteCount;
		
		assert( t->ReadToRamFunc != NULL );
		
		byteCount 				= PRE_CACHE_SIZE;
		if ( requestedByteCount < PRE_CACHE_SIZE ) {
			byteCount = requestedByteCount;
		}
		
		t->bytesLeftToPreLoad 	= requestedByteCount - byteCount;
		
		
		t->privateBase			= t->cacheBase;
		t->ReadToRamFunc( t->nonRamID, t->privateBase, t->pos, byteCount );
		t->cachePosition 		= t->pos;
		t->cacheCount			= byteCount;
	}
}

int PrimeT2KInputStream(InputStream *t )
{
	tt_int32 n = t->maxPos - t->pos;
	if ( n > 8 ) n = 8;
	if ( t->bytesLeftToPreLoad > n ) n = t->bytesLeftToPreLoad;
	PreLoadT2KInputStream( t, n );
	return 0; /*****/
}
#endif /* USE_PRE_CACHING */



tt_int32 ReadInt32( InputStream *t )
{
	register unsigned char *ptr = t->privateBase;
	tt_uint32 pos = t->pos;
	register tt_uint32 lword;
#ifdef ENABLE_NON_RAM_STREAM
	unsigned char base[4];
	
	if ( ptr != NULL ) {		/* ptr == t->privateBase */
		ptr += pos; 			/* ptr == &t->privateBase[pos] */
		#ifdef USE_PRE_CACHING
			if ( t->ReadToRamFunc != NULL ) {
				EnsureWeHaveDataInT2KInputStream( t, 4 );
				ptr -= t->cachePosition; /* ptr = &t->privateBase[pos - t->cachePosition]; */
			}
		#endif
	} else {
		ptr = base;
		t->ReadToRamFunc( t->nonRamID, ptr, pos, 4 );
	}
#else
	ptr += pos;
#endif
	
	pos = pos + 4;
	assert( pos <= t->maxPos ); 
	t->pos = pos;
	
	lword = *ptr++;
	lword <<= 8;
	lword |= *ptr++;
	lword <<= 8;
	lword |= *ptr++;
	lword <<= 8;
	lword |= *ptr;
	
	return (tt_int32)lword; /*****/
}



tt_int16 ReadInt16( InputStream *t )
{
	register unsigned char *ptr = t->privateBase;
	tt_uint32 pos = t->pos;
	register tt_uint16 word;
#ifdef ENABLE_NON_RAM_STREAM
	unsigned char base[2];
	
	if ( ptr != NULL ) {		/* ptr == t->privateBase */
		ptr += pos; 			/* ptr == &t->privateBase[pos] */
		#ifdef USE_PRE_CACHING
			if ( t->ReadToRamFunc != NULL ) {
				EnsureWeHaveDataInT2KInputStream( t, 2 );
				ptr -= t->cachePosition; /* ptr = &t->privateBase[pos - t->cachePosition]; */
			}
		#endif
	} else {
		ptr = base;
		t->ReadToRamFunc( t->nonRamID, ptr, pos, 2 );
	}
#else
	ptr += pos;
#endif
	
	pos = pos + 2;
	assert( pos <= t->maxPos ); 
	t->pos = pos;
	
	word = *ptr++;
	word <<= 8;
	word |= *ptr;
	
	return (tt_int16)word; /*****/
}

#ifdef OBSOLETE
/* Obsolete since we rely on ReadUnsignedByteMacro and ReadUnsignedByteMacro2 in T2KSTRM.H */ 
tt_uint8 ReadUnsignedByteSlow( InputStream *stream );

tt_uint8 ReadUnsignedByteSlow( InputStream *t )
{
	tt_uint32 pos = t->pos;
	tt_uint8 byte;
	register unsigned char *ptr;
#ifdef ENABLE_NON_RAM_STREAM
	unsigned char base[1];
	if ( t->privateBase == NULL ) {
		ptr = base;
		t->ReadToRamFunc( t->nonRamID, ptr, pos, 1 );
	} else {
		ptr = &t->privateBase[pos];
	}
#else
	ptr = &t->privateBase[pos];
#endif
	
	assert( pos < t->maxPos ); 
	pos = pos + 1;
	byte = *ptr;
	t->pos = pos;
	return byte; /*****/
}
#endif


void ReadSegment( InputStream *t, tt_uint8 *dest, tt_int32 numBytes )
{
	if ( numBytes > 0 ) {
		tt_uint32 pos = t->pos;
		unsigned char *ptr;
		
#ifdef ENABLE_NON_RAM_STREAM
		if ( t->ReadToRamFunc != NULL ) {   /* prior to the USE_PRE_CACHING option the test was (t->privateBase == NULL) */
			t->ReadToRamFunc( t->nonRamID, dest, pos, numBytes );
		} else {
			ptr = &t->privateBase[pos];
			memcpy( dest, ptr, numBytes );
		}
#else
		ptr = &t->privateBase[pos];	
		memcpy( dest, ptr, numBytes );
#endif
		pos = pos + numBytes;
		if ( pos > t->maxPos ) {
			assert( pos <= t->maxPos );
		}
		t->pos = pos;
	}
}


tt_int32 SizeInStream( InputStream *stream )
{
	return stream->maxPos - stream->posZero; /*****/
}

unsigned char *GetEntireStreamIntoMemory( InputStream *stream  )
{
#ifdef MAYBE_SOON
	if ( stream->privateBase != NULL && stream->ReadToRamFunc == NULL ) {
		; /* OK */
	} if ( stream->privateBase == NULL && stream->ReadToRamFunc != NULL ) {
		t->constructorType 	= 1;
		t->privateBase		= tsi_AllocMem( stream->mem, t->maxPos );
		t->ReadToRamFunc( t->nonRamID, t->privateBase, 0, t->maxPos );
	} else {
		assert( false  );
	}
#else
	assert( stream->privateBase != NULL ); /* Only used for Type 1, does not work for non-RAM fonts */
	#ifdef ENABLE_NON_RAM_STREAM
		assert( stream->ReadToRamFunc == NULL );
	#endif
#endif

	return &stream->privateBase[stream->posZero]; /*****/
}

/*
 *
 */
InputStream *New_InputStream( tsiMemObject *mem, unsigned char *data, tt_uint32 length, int *errCode )
{
	InputStream *t;
	
	if ( errCode == NULL || (*errCode = setjmp( mem->env)) == 0 ) {
		/* try */
		t  = (InputStream*) tsi_AllocMem( mem, sizeof( InputStream ) );
		
		t->mem				= mem;
		t->privateBase		= data;
#ifdef ENABLE_NON_RAM_STREAM
		t->ReadToRamFunc	= NULL;
		t->nonRamID			= 0;
		t->cacheCount		= 0;
		t->cachePosition	= 0;
#endif
		t->pos				= 0;
		t->posZero			= 0;
		t->maxPos			= length;
		t->bitBufferIn  	= 0;
		t->bitCountIn		= 0;
		t->constructorType 	= 1;
	} else {
		/* catch */
		t = NULL;
		tsi_EmergencyShutDown( mem );
	}
	
	return t; /*****/
}

/*
 *
 */
InputStream *New_InputStream2( tsiMemObject *mem, InputStream *in, tt_uint32 offset, tt_uint32 length, int *errCode )
{
	InputStream *t;
	
	if ( errCode == NULL || (*errCode = setjmp( mem->env)) == 0 ) {
		/* try */
		t = (InputStream*) tsi_AllocMem( mem, sizeof( InputStream ) );
		
		t->mem				= mem;
		t->privateBase		= in->privateBase;
#ifdef ENABLE_NON_RAM_STREAM
		t->ReadToRamFunc	= in->ReadToRamFunc;
		t->nonRamID			= in->nonRamID;
		t->cacheCount		= 0;
		t->cachePosition	= 0;
#endif
		t->pos				= offset;
		t->posZero			= offset;
		t->maxPos			= offset + length;
		t->bitBufferIn  	= 0;
		t->bitCountIn		= 0;
		t->constructorType 	= 2;
#ifdef USE_PRE_CACHING
		PreLoadT2KInputStream( t, length );
#endif			
	} else {
		/* catch */
		t = NULL;
		tsi_EmergencyShutDown( mem );
	}
	
	return t; /*****/
}

InputStream *New_InputStream3( tsiMemObject *mem, unsigned char *data, tt_uint32 length, int *errCode )
{
	InputStream *t;
	
	if ( errCode == NULL || (*errCode = setjmp( mem->env)) == 0 ) {
		/* try */
		t = (InputStream*) tsi_AllocMem( mem, sizeof( InputStream ) );
		
		t->mem				= mem;
		t->privateBase		= data;
#ifdef ENABLE_NON_RAM_STREAM
		t->ReadToRamFunc	= NULL;
		t->nonRamID			= 0;
		t->cacheCount		= 0;
		t->cachePosition	= 0;
#endif
		t->pos				= 0;
		t->posZero			= 0;
		t->maxPos			= length;
		t->bitBufferIn  	= 0;
		t->bitCountIn		= 0;
		t->constructorType 	= 3;
	} else {
		/* catch */
		t = NULL;
		tsi_EmergencyShutDown( mem );
	}
	
	return t; /*****/
}

#ifdef ENABLE_NON_RAM_STREAM
InputStream *New_NonRamInputStream( tsiMemObject *mem, void *nonRamID, PF_READ_TO_RAM readFunc, tt_uint32 length, int *errCode )
{
	InputStream *t;
	
	if ( errCode == NULL || (*errCode = setjmp( mem->env)) == 0 ) {
		/* try */
		t  = (InputStream*) tsi_AllocMem( mem, sizeof( InputStream ) );
		
		t->mem				= mem;
		t->privateBase		= NULL;
		t->ReadToRamFunc	= readFunc;
		t->nonRamID			= nonRamID;
		t->cacheCount		= 0;
		t->cachePosition	= 0;
		t->pos				= 0;
		t->posZero			= 0;
		t->maxPos			= length;
		t->bitBufferIn  	= 0;
		t->bitCountIn		= 0;
		t->constructorType 	= 4;
	} else {
		/* catch */
		t = NULL;
		tsi_EmergencyShutDown( mem );
	}
	
	return t; /*****/
}
#endif /* ENABLE_NON_RAM_STREAM */


/*
 *
 */
void Rewind_InputStream( InputStream *t )
{
	t->pos = t->posZero;
#ifdef ENABLE_NON_RAM_STREAM
	if ( t->pos < t->cachePosition ) {
		t->cacheCount		= 0; /* "flush" the cache */
		t->cachePosition	= 0;
	}
#endif
}

/*
 *
 */
void Seek_InputStream( InputStream *t, tt_uint32 offset )
{
	t->pos = t->posZero + offset;
#ifdef ENABLE_NON_RAM_STREAM
	if ( t->pos < t->cachePosition ) {
		t->cacheCount		= 0; /* "flush" the cache */
		t->cachePosition	= 0;
	}
#endif
}
/*
 *
 */
tt_uint32 Tell_InputStream( InputStream *t )
{
	return t->pos - t->posZero; /*****/
}



/*
 *
 */
void Delete_InputStream( InputStream *t, int *errCode )
{
	if ( t != NULL ) {
		if ( errCode == NULL || (*errCode = setjmp( t->mem->env)) == 0 ) {
			/* try */
			if ( t->constructorType == 1 ) {
				tsi_DeAllocMem( t->mem, t->privateBase );
			}
			tsi_DeAllocMem( t->mem, t );
		} else {
			/* catch */
			tsi_EmergencyShutDown( t->mem );
		}
	}
}


#ifdef ENABLE_WRITE
void WriteInt32( OutputStream *stream, tt_int32 value )
{
	register tt_uint32 pos = stream->pos;
	register unsigned char *ptr;
	register tt_uint32 lword = (tt_uint32)value;
	
	pos += 4;
	if ( pos > stream->maxLength ) {
		stream->maxLength = pos + (pos>>1);
		stream->base = (tt_uint8 *)tsi_ReAllocMem( stream->mem, stream->base, stream->maxLength );
		assert( stream->base != NULL );
	}
	ptr = &stream->base[stream->pos];
	stream->pos = pos;
	
	*ptr++ = (unsigned char)(lword >> 24);
	*ptr++ = (unsigned char)(lword >> 16);
	*ptr++ = (unsigned char)(lword >> 8);
	*ptr   = (unsigned char)(lword);
}	


void WriteInt16( OutputStream *stream, tt_int16 value )
{
	register tt_uint32 pos = stream->pos;
	register unsigned char *ptr;
	register tt_uint16 word = (tt_uint16)value;
	
	pos += 2;
	if ( pos > stream->maxLength ) {
		stream->maxLength = pos + (pos>>1);
		stream->base = (tt_uint8 *)tsi_ReAllocMem( stream->mem, stream->base, stream->maxLength );
		assert( stream->base != NULL );
	}
	ptr = &stream->base[stream->pos];
	stream->pos = pos;
	
	*ptr++ = (unsigned char)(word >> 8);
	*ptr   = (unsigned char)word;
}	


void WriteUnsignedByte( OutputStream *stream, tt_uint8 value )
{
	register tt_uint32 pos = stream->pos;
	register unsigned char *ptr;
	
	pos++;
	if ( pos > stream->maxLength ) {
		stream->maxLength = pos + (pos>>1);
		stream->base = (tt_uint8 *)tsi_ReAllocMem( stream->mem, stream->base, stream->maxLength );
		assert( stream->base != NULL );
	}
	ptr = &stream->base[stream->pos];
	stream->pos = pos;
	
	*ptr   = value;
}

void Write( OutputStream *stream, tt_uint8 *src, tt_int32 numBytes )
{
	if ( numBytes > 0 ) {
		register tt_uint32 pos = stream->pos;
		register unsigned char *ptr;
		
		pos += numBytes;
		if ( pos > stream->maxLength ) {
			stream->maxLength = pos + (pos>>1);
			stream->base = (tt_uint8 *)tsi_ReAllocMem( stream->mem, stream->base, stream->maxLength );
			assert( stream->base != NULL );
		}
		ptr = &stream->base[stream->pos];
		stream->pos = pos;
		memcpy( ptr, src, numBytes );
	}
}

tt_int32 SizeOutStream( OutputStream *stream )
{
	if ( stream->pos > stream->maxPos ) {
		stream->maxPos = stream->pos;
	}
	return stream->maxPos; /*****/
}

tt_int32 OutStreamPos( OutputStream *stream )
{
	if ( stream->pos > stream->maxPos ) {
		stream->maxPos = stream->pos;
	}
	return stream->pos; /*****/
}

/*
 *
 */
OutputStream *New_OutputStream( tsiMemObject *mem, tt_int32 initialSize )
{
	OutputStream *t = (OutputStream*) tsi_AllocMem( mem, sizeof( OutputStream ) );
	
	t->mem			= mem;
	t->pos			= 0;
	t->maxPos		= 0;
	if ( initialSize <= 0 ) initialSize = 1024;
	t->maxLength	= initialSize;
	
	t->base 		= (tt_uint8 *)tsi_AllocMem( t->mem, t->maxLength );
	assert( t->base != NULL );
	
	t->bitBufferOut  	= 0;
	t->bitCountOut		= 0;
	return t; /*****/
}

void Rewind_OutputStream( OutputStream *t )
{
	assert( t->pos <= t->maxLength );
	if ( t->pos > t->maxPos ) {
		t->maxPos = t->pos;
	}
	t->pos = 0;
}

/*
 *
 */
void Delete_OutputStream( OutputStream *t )
{
	if ( t != NULL ) {
		assert( t->pos <= t->maxLength );
		tsi_DeAllocMem( t->mem, t->base );
		tsi_DeAllocMem( t->mem, t );
	}
}

#endif /* ENABLE_WRITE */


#ifdef ENABLE_WRITE

void WriteBitsToStream( OutputStream *out, tt_uint32 bits, tt_uint32 count )
{
	/* First we have t->bitCountOut bits, followed by count bits */
	/* We always keep the bits slammed up againts the "left edge" */
	bits       <<= 32 - count - out->bitCountOut;
	out->bitCountOut += count;
	out->bitBufferOut |= bits;

	while ( out->bitCountOut >= 8 ) {
		WriteUnsignedByte( out, (unsigned char)(out->bitBufferOut >> 24) );
		out->bitBufferOut <<= 8;
		out->bitCountOut -= 8;
	}

}

void FlushOutStream( OutputStream *out )
{
	assert( out->bitCountOut < 8 );
	if ( out->bitCountOut > 0 ) {
		WriteUnsignedByte( out, (unsigned char)(out->bitBufferOut >> 24) );
		out->bitCountOut = 0;
	}
	out->bitBufferOut = 0;
}


void WriteUnsignedNumber( OutputStream *out, tt_uint32 n )
{
	unsigned char value;
	
	do {
		value = (unsigned char)(n & 0x7f);
		if ( n > 0x7f ) {
			value |= 0x80;
			n >>= 7;
		}
		WriteUnsignedByte( out, value );
	} while (value & 0x80);
}
#endif /* ENABLE_WRITE */

tt_uint32 ReadUnsignedNumber( InputStream *in )
{
	unsigned char value;
	tt_uint32 n = 0;
	tt_uint32 shift = 0;
	
	do {
		value = ReadUnsignedByteMacro( in );
		n |= ((value & 0x7f) << shift );
		shift += 7;
	} while (value & 0x80);
	return n; /*****/
}



