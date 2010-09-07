/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)R0CARD.H	1.4 03/12/19
 */

#if !defined(__R0CARD_H__)
#define __R0CARD_H__

#if defined(__cplusplus)
extern "C"
{
#endif


int R0CARD_AcquireSoundCard( int SampleRate, int Channels, int SampleWidth,
                             int BufferTime, int *pBufferSize,
                             void (*pProc)(void *) );

int R0CARD_ReleaseSoundCard( int handle );

unsigned long R0CARD_Microseconds( void );


#if defined(__cplusplus)
}
#endif


#endif

