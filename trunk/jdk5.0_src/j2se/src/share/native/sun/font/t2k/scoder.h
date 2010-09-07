/*
 * @(#)scoder.h	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * File:		SCODER.H
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
#ifndef __T2K_SCODER__
#define __T2K_SCODER__
#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/* private */
#define No_of_chars 256                 /* Number of character symbols      */

typedef struct {
	/* private */
	tsiMemObject *mem;
	unsigned char *numBitsUsed;
	tt_uint32 numEntries, maxBits;
	unsigned char *LookUpSymbol;	/* maps a bitpattern to a symbol */
	unsigned short *LookUpBits;		/* maps a symbol the bitpattern  */

	/* public */
} SCODER;

/* Private methods. */
/*
 * Internal function used for sequencing the look-up table.
 */
void SCODER_SequenceLookUp( SCODER *t );

/* Public methods. */

/* Two different constructors. */
/* count is 256 entries large array with event counts for all the bytes. */
/* codingCost is an informative output from this constructor. */
#ifdef ENABLE_WRITE
SCODER *New_SCODER( tsiMemObject *mem, tt_int32 *count, tt_int32 *codingCost);

/*
 * This method saves an SCODER model to the stream, so that it can later be
 * recreated with New_SCODER_FromStream().
 */
void SCODER_Save( SCODER *t, OutputStream *out );

/*
 * Write a symbol to the output stream.
 */
int SCODER_EncodeSymbol( SCODER *t, OutputStream *out, unsigned char symbol );

#endif /* ENABLE_WRITE */

/*
 * This standard constructor recreates the SCODER object from a stream.
 */
SCODER *New_SCODER_FromStream( tsiMemObject *mem, InputStream *in );


/*
 * Read a symbol from the input stream.
 */
unsigned char SCODER_ReadSymbol( SCODER *t, InputStream *in );


/*
 * The destructor.
 */
void Delete_SCODER( SCODER *t );

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif /* __T2K_SCODER__ */

/*
 * File:		SCODER.H
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
#ifndef __T2K_SCODER__
#define __T2K_SCODER__

/* private */
#define No_of_chars 256                 /* Number of character symbols      */

typedef struct {
	/* private */
	tsiMemObject *mem;
	unsigned char *numBitsUsed;
	tt_uint32 numEntries, maxBits;
	unsigned char *LookUpSymbol;	/* maps a bitpattern to a symbol */
	unsigned short *LookUpBits;		/* maps a symbol the bitpattern  */

	/* public */
} SCODER;

#ifdef __cplusplus
extern "C"
{
#endif

/* Private methods. */
/*
 * Internal function used for sequencing the look-up table.
 */
void SCODER_SequenceLookUp( SCODER *t );

/* Public methods. */

/* Two different constructors. */
/* count is 256 entries large array with event counts for all the bytes. */
/* codingCost is an informative output from this constructor. */
#ifdef ENABLE_WRITE
SCODER *New_SCODER( tsiMemObject *mem, tt_int32 *count, tt_int32 *codingCost);

/*
 * This method saves an SCODER model to the stream, so that it can later be
 * recreated with New_SCODER_FromStream().
 */
void SCODER_Save( SCODER *t, OutputStream *out );

/*
 * Write a symbol to the output stream.
 */
int SCODER_EncodeSymbol( SCODER *t, OutputStream *out, unsigned char symbol );

#endif /* ENABLE_WRITE */

/*
 * This standard constructor recreates the SCODER object from a stream.
 */
SCODER *New_SCODER_FromStream( tsiMemObject *mem, InputStream *in );


/*
 * Read a symbol from the input stream.
 */
unsigned char SCODER_ReadSymbol( SCODER *t, InputStream *in );


/*
 * The destructor.
 */
void Delete_SCODER( SCODER *t );

#ifdef __cplusplus
}
#endif

#endif /* __T2K_SCODER__ */
