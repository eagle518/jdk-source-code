/*
 * @(#)HAE_Host.h	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "HAE_Host.h"
**
**	Generalized Audio API to host services
**
**	© Copyright 1998 Headspace, Inc, All Rights Reserved.
**	Written by Steve Hales
**
**	Headspace products contain certain trade secrets and confidential and
**	proprietary information of Headspace.  Use, reproduction, disclosure
**	and distribution by any means are prohibited, except pursuant to
**	a written license from Headspace. Use of copyright notice is
**	precautionary and does not imply publication or disclosure.
**
**	Restricted Rights Legend:
**	Use, duplication, or disclosure by the Government is subject to
**	restrictions as set forth in subparagraph (c)(1)(ii) of The
**	Rights in Technical Data and Computer Software clause in DFARS
**	252.227-7013 or subparagraphs (c)(1) and (2) of the Commercial
**	Computer Software--Restricted Rights at 48 CFR 52.227-19, as
**	applicable.
**
** Modification History:
**
**	6/29/98		Created
*/
/*****************************************************************************/

#ifndef HAE_HOST_API
#define HAE_HOST_API

// *** MIDI Support ***

// Creation/deletion/management

int HostOpenMidi(void *context, long *pReference, int device);

int HostCloseMidi(void *context, long reference);

int HostLoadMidiPatch(void *context, long reference, long patch, long bank);
int HostUnloadMidiPatch(void *context, long reference, long patch, long bank);

// sequencer

// given a midi stream, parse it out to the various midi functions
// for example:
// 0x92			0x50		0x7F		0x00
// comandByte	data1Byte	data2Byte	data3Byte
// Note 80 on with a velocity of 127 on channel 2
int HostParseMidiData(void *context, long reference, unsigned char commandByte, unsigned char data1Byte, 
		      unsigned char data2Byte, unsigned char data3Byte,
		      unsigned long time = 0);

// if you pass 0 for time the current time will be passed
// The channel variable is 0 to 15. Channel 9 is percussion for example.
// The programNumber variable is a number from 0-127
int HostNoteOff(void *context, long reference, unsigned char channel, 
		unsigned char note, 
		unsigned char velocity,
		unsigned long time = 0);

// note on that checks to see if an instrument needs to be loaded. DO NOT call this
// during an interrupt, as it might load memory. This only works when queuing is enabled
int HostNoteOnWithLoad(void *context, long reference, unsigned char channel, 
		       unsigned char note, 
		       unsigned char velocity,
		       unsigned long time = 0);

int HostNoteOn(void *context, long reference, unsigned char channel, 
	       unsigned char note, 
	       unsigned char velocity,
	       unsigned long time = 0);

int HostKeyPressure(void *context, long reference, unsigned char channel, 
		    unsigned char note, 
		    unsigned char pressure,
		    unsigned long time = 0);

int HostControlChange(void *context, long reference, unsigned char channel, 
		      unsigned char controlNumber,
		      unsigned char controlValue, 
		      unsigned long time = 0);

int HostProgramBankChange(void *context, long reference, unsigned char channel,
			  unsigned char programNumber,
			  unsigned char bankNumber,
			  unsigned long time = 0);

int HostProgramChange(void *context, long reference, unsigned char channel, 
		      unsigned char programNumber,
		      unsigned long time = 0);

int HostChannelPressure(void *context, long reference, unsigned char channel, 
			unsigned char pressure, 
			unsigned long time = 0);

int HostPitchBend(void *context, long reference, unsigned char channel, 
		  unsigned char lsb, 
		  unsigned char msb,
		  unsigned long time = 0);

int HostAllNotesOff(void *context, long reference, unsigned long time = 0);


// *** PCM Audio support ***

int HostOpenAudio(void *context, long *pReference, int device, 
		  int rate, int channels, int bits, int volume);
int HostCloseAudio(void *context, long reference);

// must match open device format
int HostPlayAudio(void *context, long reference, void *pcmAudio, unsigned long pcmAudioLength);

int HostSetAudioRate(void *context, long reference, int newRate);
int HostSetAudioVolume(void *context, long reference, int newVolume);
int HostSetAudioStereoPan(void *context, long reference, int newPan);


#endif	// HAE_HOST_API

