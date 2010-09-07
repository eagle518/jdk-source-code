/*
 * @(#)tsimem.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * File:		TSIMEM.H
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
#ifndef __TSIMEM__
#define __TSIMEM__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

#include <setjmp.h>

/* #define TRACK_RAM */


typedef struct {
	/* private */
	tt_uint32 stamp1; 		/* == MAGIC1 */
	tt_int32 numPointers;			/* Number of allocated memory pointers */
	tt_int32 maxPointers;			/* current maximum limit on number of pointers */
	void	 **base;
	/* semi-private */
	jmp_buf env;				/* Use the 	tsi_Assert() below */		
#ifdef TRACK_RAM
	tt_int32 totRAM;
	tt_int32 maxRAM;
#endif
	/* private */
	tt_uint32 state;
	tt_uint32 stamp2; 		/* == MAGIC2 */
} tsiMemObject;

/* Normally returns 0 in *errCode */
tsiMemObject *tsi_NewMemhandler( int *errCode );
void tsi_DeleteMemhandler( tsiMemObject *t );

void *tsi_AllocMem( register tsiMemObject *t, size_t size );
void *tsi_ReAllocMem( register tsiMemObject *t, void *p, size_t size );
void tsi_DeAllocMem( register tsiMemObject *t, void *p );


void tsi_EmergencyShutDown( tsiMemObject *t );

/* only for use by tsi_Assert */
void tsi_Error( tsiMemObject *t, int errcode );
/* only for internal T2K use */
#define T2K_STATE_ALIVE 0xaa005501
#define T2K_STATE_DEAD	0x5500aaff

/*
#define tsi_Assert( t, cond, errcode ) assert( cond )
*/
#define tsi_Assert( t, cond, errcode ) if ( !(cond) ) tsi_Error( t, errcode )


#define T2K_ERR_MEM_IS_NULL			10000
#define T2K_ERR_TRANS_IS_NULL		10001
#define T2K_ERR_RES_IS_NOT_POS		10002
#define T2K_ERR_BAD_GRAY_CMD		10003
#define T2K_ERR_BAD_FRAC_PEN		10004
#define T2K_ERR_GOT_NULL_GLYPH		10005
#define T2K_ERR_TOO_MANY_POINTS		10006
#define T2K_ERR_BAD_T2K_STAMP		10007
#define T2K_ERR_MEM_MALLOC_FAILED	10008
#define T2K_ERR_BAD_MEM_STAMP		10009
#define T2K_ERR_MEM_LEAK			10010
#define T2K_ERR_NULL_MEM			10011
#define T2K_ERR_MEM_TOO_MANY_PTRS	10012
#define T2K_ERR_BAD_PTR_COUNT		10013
#define T2K_ERR_MEM_REALLOC_FAILED	10014
#define T2K_ERR_MEM_BAD_PTR			10015
#define T2K_ERR_MEM_INVALID_PTR		10016
#define T2K_ERR_MEM_BAD_LOGIC		10017
#define T2K_ERR_INTERNAL_LOGIC		10018
#define T2K_ERR_USE_PAST_DEATH		10019
#define T2K_ERR_NEG_MEM_REQUEST		10020
#define T2K_BAD_CMAP				10021
#define T2K_UNKNOWN_CFF_VERSION		10022



#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __TSIMEM__ */
