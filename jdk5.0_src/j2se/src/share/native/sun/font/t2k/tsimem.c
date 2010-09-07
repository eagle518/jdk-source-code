/*
 * @(#)tsimem.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * File:		TSIMEM.C
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
#include "tsimem.h"
#include "config.h"


#define MAGIC1 0xab1500ff
#define MAGIC2 0xa5a55a5a
#define MAGIC3 0xaa53C5aa
#define MAGIC4 0x5a
#define MAGIC5 0xf0

/* This is *SLOW*. Only define if you are debugging or testing. */
/* #define ZAP_MEMORY */

void tsi_Error( tsiMemObject *t, int errcode )
{
	t->state = T2K_STATE_DEAD;
	longjmp( t->env, errcode );
}

tsiMemObject *tsi_NewMemhandler( int *errCode )
{
	tsiMemObject *t = NULL;
	register tt_int32 i;
	
	assert( errCode != NULL );
	*errCode = 0;
	t = (tsiMemObject *) CLIENT_MALLOC( sizeof( tsiMemObject ) );
	if ( t != NULL ) {
		t->stamp1 = MAGIC1;
		t->state  = T2K_STATE_ALIVE;
		t->stamp2 = MAGIC2;
		
		t->numPointers = 0;
		t->maxPointers = 256;
		t->base = (void **) CLIENT_MALLOC( sizeof( void *) * t->maxPointers );
		
		if ( t->base != NULL ) {
			for ( i = 0; i < t->maxPointers; i++ ) {
				t->base[i] = NULL;
			}
		} else {
			CLIENT_FREE( t ); t = NULL;
			*errCode = T2K_ERR_MEM_MALLOC_FAILED;
		}
	} else {
		*errCode = T2K_ERR_MEM_MALLOC_FAILED;
	}
	
#ifdef TRACK_RAM
	t->totRAM = sizeof( tsiMemObject ) + sizeof( void *) * t->maxPointers;
	t->maxRAM = t->totRAM;
#endif
	return t; /*****/
}

void tsi_DeleteMemhandler( tsiMemObject *t )
{
#ifdef OLD
	tsi_Assert( t, t->stamp1 == MAGIC1 && t->stamp2 == MAGIC2 , T2K_ERR_BAD_MEM_STAMP );
	tsi_Assert( t, t->numPointers == 0, T2K_ERR_MEM_LEAK ); /* Check for dangling pointers */
#endif
	assert( t->stamp1 == MAGIC1 && t->stamp2 == MAGIC2 );
	assert( t->numPointers == 0 ); /* Check for dangling pointers */
	
#ifdef TRACK_RAM
	t->totRAM -= (sizeof( tsiMemObject ) + sizeof( void *) * t->maxPointers);
	printf("********************\n" );
	printf("t->totRAM = %d\n", t->totRAM );
	printf("t->maxRAM = %d\n", t->maxRAM );
	printf("********************\n" );
#endif
	CLIENT_FREE( t->base );
	CLIENT_FREE( t );
}

void tsi_EmergencyShutDown( tsiMemObject *t )
{
	if ( t != NULL ) {
		register tt_int32 i, maxPointers = t->maxPointers;
		register void **base = t->base;
		for ( i = 0; i < maxPointers; i++ ) {
			if ( base[i] != NULL ) {
				CLIENT_FREE( base[i] );
			}
		}
		CLIENT_FREE( base );
		CLIENT_FREE( t );
	}
} 

/* MEMORY LAYOUT:
 *
 * tt_uint32:	MAGIC3
 * tt_uint32:	n
 * byte[]		:	n bytes of allocated data
 *
 * byte:			MAGIC4
 * byte:			MAGIC5
 *
 */
#define headerSize (sizeof(tt_uint32)+sizeof(tt_uint32))
#define tailSize (sizeof(char) + sizeof(char))


/*
 * Description:		Allocates a chunk of memory, and returns a pointer to it.
 * How used:		Just call with the size in bytes.
 * Side Effects: 	None.
 * Return value: 	A pointer to the memory.
 */
void *tsi_AllocMem( register tsiMemObject *t, size_t size )
{
	register tt_int32 i, maxPointers;
	register unsigned char *p;
	register tt_uint32 *plong;
	register void **base;
	
	tsi_Assert( t, t != NULL, T2K_ERR_NULL_MEM );
	/* 	tsi_ValidateMemory( t ); */
	/*  commented out because the check in the line below it makes no sense
		tsi_Assert( t, size >= 0, T2K_ERR_NEG_MEM_REQUEST );
		*/
	p = (unsigned char *)CLIENT_MALLOC( headerSize + size + tailSize );
	tsi_Assert( t, p != NULL, T2K_ERR_MEM_MALLOC_FAILED );

#ifdef TRACK_RAM
	t->totRAM += headerSize + size + tailSize;
	if ( t->totRAM > t->maxRAM ) t->maxRAM = t->totRAM;
#endif
	
	plong = (tt_uint32 *)p;
	
	plong[0] = MAGIC3;
	plong[1] = (tt_uint32)size;
	p[headerSize + size]	= (unsigned char)MAGIC4;
	p[headerSize + size+1]	= (unsigned char)MAGIC5;
	
	tsi_Assert( t, t->numPointers < t->maxPointers, T2K_ERR_MEM_TOO_MANY_PTRS );
	
	base = t->base;
	maxPointers = t->maxPointers;
	for ( i = 0; i < maxPointers; i++ ) {
		if ( base[i] == NULL ) {
			base[i] = p;
			t->numPointers++;
			break; /*****/
		}
	}
	tsi_Assert( t, i < maxPointers, T2K_ERR_MEM_BAD_LOGIC );
	
	#ifdef ZAP_MEMORY
	{
		for ( i = 0; i < size; i++ )  {
			((char *)p)[i+headerSize] = 0x5a;
		}

	}
	#endif
	
	return (p+headerSize); /*****/
}


/*
 * Description:		reallocs the memory the pointer "p" points at.
 * How used:		Call with the pointer to the memory, received from ag_AllocMem.
 *					It is OK to call this with a NULL pointer.
 * Side Effects: 	None.
 * Return value: 	None.
 */
void *tsi_ReAllocMem( register tsiMemObject *t, void *pIn, size_t size2 )
{
	register tt_int32 i, maxPointers;
	register void **base;
	register tt_uint32 *plong;
	unsigned char *p = (unsigned char *) pIn;
	tt_uint32 size1;
	
	if ( p != NULL ) {
		p -= headerSize;
		plong = (tt_uint32 *)p;
		
		tsi_Assert( t, plong[0] == MAGIC3, T2K_ERR_BAD_MEM_STAMP );

		size1 = plong[1];
		tsi_Assert( t, ((unsigned char *)p)[headerSize + size1] 		== MAGIC4, T2K_ERR_BAD_MEM_STAMP );
		tsi_Assert( t, ((unsigned char *)p)[headerSize + size1 + 1] 	== MAGIC5, T2K_ERR_BAD_MEM_STAMP );
		
		base = t->base;
		maxPointers = t->maxPointers;
		tsi_Assert( t, t->numPointers > 0 && t->numPointers <= maxPointers, T2K_ERR_BAD_PTR_COUNT );
		for ( i = 0; i < maxPointers; i++ ) {
			if ( base[i] == p ) {
				base[i] = CLIENT_REALLOC( p, headerSize + size2 + tailSize );
#ifdef TRACK_RAM
	t->totRAM -= size1;
	t->totRAM += size2;
	if ( t->totRAM > t->maxRAM ) t->maxRAM = t->totRAM;
#endif
				p = (unsigned char *) base[i];
				tsi_Assert( t, p != NULL, T2K_ERR_MEM_REALLOC_FAILED );

				plong = (tt_uint32 *)p;
				tsi_Assert( t, plong[0] == MAGIC3, T2K_ERR_BAD_MEM_STAMP );
				plong[1] = size2;
				p[headerSize + size2]	= (unsigned char)MAGIC4;
				p[headerSize + size2+1]	= (unsigned char)MAGIC5;
				break; /*****/
			}
		}
		tsi_Assert( t, i < t->maxPointers, T2K_ERR_MEM_BAD_PTR );
		return (p+headerSize); /*****/
	}
	return NULL; /*****/
}

#ifdef BIG_TEST
void tsi_ValidatePointer( register tsiMemObject *t, void *pIn );
void  tsi_ValidatePointer( register tsiMemObject *t, void *pIn )
{
	int err;
	char *p = (char *) pIn;
	tt_uint32 size;
	register tt_uint32 *plong;
	
	tsi_Assert( t, t != NULL, T2K_ERR_NULL_MEM );

	p -= headerSize;
	plong = (tt_uint32 *)p;
	
	err = ( plong[0] != MAGIC3 );
	if ( err == 0 ) {
		size = plong[1];
		err |= ( ((unsigned char *)p)[headerSize + size] 		!= MAGIC4 );
		err |= ( ((unsigned char *)p)[headerSize + size + 1] 	!= MAGIC5 );
	}
	if ( err != 0 ) {
		printf("trouble\n");
	}
	tsi_Assert( t, err == 0, T2K_ERR_MEM_INVALID_PTR );
}

void  tsi_ValidateMemory( register tsiMemObject *t );
void  tsi_ValidateMemory( register tsiMemObject *t )
{
	register tt_int32 i, maxPointers;
	register void **base;
	
	assert( false );
	tsi_Assert( t, t != NULL, T2K_ERR_NULL_MEM );

	
	base = t->base;
	maxPointers = t->maxPointers;
	for ( i = 0; i < maxPointers; i++ ) {
		if ( base[i] != NULL ) {
			tsi_ValidatePointer( t, (char *)base[i] + headerSize );
		}
	}
}
#endif

/*
 * Description:		Free the memory the pointer "p" points at.
 * How used:		Call with the pointer to the memory, received from ag_AllocMem.
 *					It is OK to call this with a NULL pointer.
 * Side Effects: 	None.
 * Return value: 	None.
 */
void tsi_DeAllocMem( register tsiMemObject *t, void *pIn )
{
	register tt_int32 i, maxPointers;
	register void **base;
	register tt_uint32 *plong;
	char *p = (char *) pIn;
	tt_uint32 size;
	

	tsi_Assert( t, t != NULL, T2K_ERR_NULL_MEM );
	if ( p != NULL ) {
		p -= headerSize;
		plong = (tt_uint32 *)p;
		
		tsi_Assert( t, plong[0] == MAGIC3, T2K_ERR_BAD_MEM_STAMP );

		size = plong[1];
		tsi_Assert( t, ((unsigned char *)p)[headerSize + size] 		== MAGIC4, T2K_ERR_BAD_MEM_STAMP );
		tsi_Assert( t, ((unsigned char *)p)[headerSize + size + 1] 	== MAGIC5, T2K_ERR_BAD_MEM_STAMP );

		#ifdef ZAP_MEMORY
		{
			for ( i = 0; i < size; i++ )  {
				((char *)pIn)[i] = 0xa5;
			}

		}
		#endif
		
		base = t->base;
		maxPointers = t->maxPointers;
		tsi_Assert( t, t->numPointers <= maxPointers, T2K_ERR_MEM_TOO_MANY_PTRS );
		for ( i = 0; i < maxPointers; i++ ) {
			if ( base[i] == p ) {
				base[i] = NULL;
				t->numPointers--;
				break; /*****/
			}
		}
		tsi_Assert( t, i < t->maxPointers, T2K_ERR_MEM_BAD_PTR );
		CLIENT_FREE(p);
#ifdef TRACK_RAM
	t->totRAM -= (headerSize+size+tailSize);
#endif
	}
}

/*
void tsi_Assert( register tsiMemObject *t, int cond, int errcode  )
{
	if ( !cond ) {
		longjmp( t->env, errcode );
	}
}
*/
