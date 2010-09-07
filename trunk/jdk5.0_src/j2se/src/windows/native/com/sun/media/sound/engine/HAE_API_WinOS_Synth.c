/*
 * @(#)HAE_API_WinOS_Synth.c	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
**	HAE_API_WinOS_Synth.c
**
**	This provides platform specfic MIDI Synthesizer functions for Windows 95/NT.
**  This interface for HAE is for Windows 95/NT and uses the midiOut API to send
**	MIDI messages through the multimedia system.
**
**	Overview:
**
**	History	-
**	06.21.99	Created
*/
/*****************************************************************************/


#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>

#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include "GenSnd.h"


#if USE_EXTERNAL_SYNTH

//$$fb 2002-02-01: remove compiler warning of undeclared external
#if USE_PLATFORM_MIDI_OUT == TRUE
#include "PlatformMidi.h"
#endif

/*
GM_Synth * PV_OpenExternalSynth(struct GM_Song *pSong, INT32 deviceID)
{
	MMRESULT		theErr;
	//UINT			deviceID = MIDI_MAPPER;
	HMIDIOUT		hMidiOut;
	GM_Synth *		pSynth;

	theErr = midiOutOpen(&hMidiOut, (UINT)deviceID, 0L, (DWORD)0L,  (DWORD)0L);

	if ( (theErr != MMSYSERR_NOERROR) || (hMidiOut == NULL) )
	{
		fprintf(stderr, "ERROR: PV_OpenExternalSynth: failed to open midi out device\n");
		return NULL;
	}

	fprintf(stderr, "SUCCESS: PV_OpenExternalSynth: opened midi out device\n");

	pSynth = (GM_Synth *)XNewPtr((INT32)sizeof(GM_Synth));

	pSynth->deviceHandle = (INT32)hMidiOut;
	pSynth->pProgramChangeProcPtr = PV_ProcessExternalSynthProgramChange;
	pSynth->pNoteOffProcPtr = PV_ProcessExternalSynthNoteOff;
	pSynth->pNoteOnProcPtr = PV_ProcessExternalSynthNoteOn;
	pSynth->pPitchBendProcPtr = PV_ProcessExternalSynthPitchBend;
	pSynth->pProcessControllerProcPtr = PV_ProcessExternalSynthController;
	pSynth->pNext = NULL;

	return pSynth;
}

void PV_CloseExternalSynth(struct GM_Song *pSong, INT32 deviceHandle)
{
	MMRESULT		theErr;

	theErr = midiOutClose((HMIDIOUT)deviceHandle);

	if (theErr != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "ERROR: PV_CloseExternalSynth: failed to close midi out device\n");
	}
	else
	{
		fprintf(stderr, "SUCCESS: PV_CloseExternalSynth: closed midi out device\n");
	}
}
*/


UINT sendMIDIEvent(UINT_PTR deviceHandle, BYTE bStatus, BYTE bData1, BYTE bData2)
{

#if USE_PLATFORM_MIDI_OUT == TRUE
    return (UINT)MIDI_OUT_SendShortMessage((MidiDeviceHandle*) deviceHandle, bStatus | (bData1 << 8) | (bData2 << 16), -1);
#endif // USE_PLATFORM_MIDI_OUT
}


// this is going to kill sound for the whole synth, not just this song....
// we send the "All Sound Off" controller (controller #120) to each channel.
// unlike the AllNotesOff controller, this overrides the sustain controller.
// $$kk: 07.20.99: stone-age MIDI sound cards like the sbpro do not seem to
// respond to the AllSoundOff controller, so i'm hitting this over the head
// with 1) sustain off 2) all notes off and 3) all sound off for each channel.
void PV_ProcessExternalSynthSoundOff(struct GM_Song *pSong)
{
    if (pSong)
	{
	    // $$kk: 07.12.99: this is a temporary solution!  i am setting the song userReference
	    // to the GM_Synth pointer....
	    GM_Synth *pSynth = (GM_Synth *)pSong->userReference;

	    if (pSynth)
		{
		    INT16 channelNumber;

		    for (channelNumber = 0; channelNumber < 16; channelNumber++)
			{
				// $$fb 2002-02-01: itanium port: widen handle to ptr size
				// sustain off
			    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0xB0 | channelNumber), (BYTE)64, (BYTE)0);

				// all notes off
			    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0xB0 | channelNumber), (BYTE)123, (BYTE)0);

				// all sound off
			    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0xB0 | channelNumber), (BYTE)120, (BYTE)0);
			}
		}
	}
}


GM_Synth * PV_CreateExternalSynthForDevice(struct GM_Song *pSong, void* deviceHandle)
{
    GM_Synth *		pSynth;

    pSynth = (GM_Synth *)XNewPtr((INT32)sizeof(GM_Synth));

    if (pSynth) {

	pSynth->deviceHandle = deviceHandle;
	pSynth->pProgramChangeProcPtr = PV_ProcessExternalSynthProgramChange;
	pSynth->pNoteOffProcPtr = PV_ProcessExternalSynthNoteOff;
	pSynth->pNoteOnProcPtr = PV_ProcessExternalSynthNoteOn;
	pSynth->pPitchBendProcPtr = PV_ProcessExternalSynthPitchBend;
	pSynth->pProcessControllerProcPtr = PV_ProcessExternalSynthController;
	pSynth->pProcessSongSoundOffProcPtr = PV_ProcessExternalSynthSoundOff;
	pSynth->pNext = NULL;
    }

    return pSynth;
}


// Process midi program change
void PV_ProcessExternalSynthProgramChange(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 program)
{
    //fprintf(stderr, "PV_ProcessExternalSynthProgramChange: pSong: %d, MIDIChannel: %d, currentTrack: %d, program: %d\n", pSong, MIDIChannel, currentTrack, program);

    // $$kk: 07.12.99: this is a temporary solution!  i am setting the song userReference
    // to the GM_Synth pointer....
    GM_Synth *pSynth = (GM_Synth *)pSong->userReference;

    if (pSong->AnalyzeMode == SCAN_NORMAL)
	{
	    // $$fb 2002-02-01: itanium port: widen handle to ptr size
	    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0xC0 | MIDIChannel), (BYTE)program, 0);
	}
}

// Process note off
void PV_ProcessExternalSynthNoteOff(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume)
{
    //fprintf(stderr, "PV_ProcessExternalSynthNoteOff: pSong: %d, MIDIChannel: %d, currentTrack: %d, note: %d, volume: %d\n", pSong, MIDIChannel, currentTrack, note, volume);

    // $$kk: 07.12.99: this is a temporary solution!  i am setting the song userReference
    // to the GM_Synth pointer....
    GM_Synth *pSynth = (GM_Synth *)pSong->userReference;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    if (pSong->AnalyzeMode == SCAN_NORMAL)
		{
		    if (XTestBit(&pSong->allowPitchShift, MIDIChannel))
			{
			    note += pSong->songPitchShift;
			}
		    // $$fb 2002-02-01: itanium port: widen handle to ptr size
		    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0x80 | MIDIChannel), (BYTE)note, (BYTE)volume);
		}
	}
}

// Process note on
void PV_ProcessExternalSynthNoteOn(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 note, INT16 volume)
{
    //fprintf(stderr, "PV_ProcessExternalSynthNoteOn: pSong: %d, MIDIChannel: %d, currentTrack: %d, note: %d, volume: %d\n", pSong, MIDIChannel, currentTrack, note, volume);

    // $$kk: 07.12.99: this is a temporary solution!  i am setting the song userReference
    // to the GM_Synth pointer....
    GM_Synth *pSynth = (GM_Synth *)pSong->userReference;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    if (pSong->AnalyzeMode == SCAN_NORMAL)
		{
		    if (XTestBit(&pSong->allowPitchShift, MIDIChannel))
			{
			    note += pSong->songPitchShift;
			}

		    // $$fb 2002-02-01: itanium port: widen handle to ptr size
		    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0x90 | MIDIChannel), (BYTE)note, (BYTE)volume);
		}
	}
}


// Process pitch bend
void PV_ProcessExternalSynthPitchBend(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, UBYTE valueMSB, UBYTE valueLSB)
{
    //fprintf(stderr, "PV_ProcessExternalSynthPitchBend: pSong: %d, MIDIChannel: %d, currentTrack: %d, valueMSB: %d, valueLSB: %d\n", pSong, MIDIChannel, currentTrack, valueMSB, valueLSB);

    // $$kk: 07.12.99: this is a temporary solution!  i am setting the song userReference
    // to the GM_Synth pointer....
    GM_Synth *pSynth = (GM_Synth *)pSong->userReference;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)

	{
	    if (pSong->AnalyzeMode == SCAN_NORMAL)
		{
		    // $$kk: 11.09.99: the order of the data bytes for pitch bend is: data1 = LSB, data2 = MSB!!
		    // $$fb 2002-02-01: itanium port: widen handle to ptr size
		    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0xE0 | MIDIChannel), (BYTE)valueLSB, (BYTE)valueMSB);

		    // $$kk: 11.09.99: the engine stores pitch bend values in the range from -8192 to 8191.
		    // we convert here so that GM_GetPitchBend will work.
		    // (should eventually handle this *differently*!!
		    pSong->channelBend[MIDIChannel] = (((valueMSB * 128) + valueLSB) - 8192);
		}
	}
}

// Process midi controlers
void PV_ProcessExternalSynthController(void *threadContext, struct GM_Song *pSong, INT16 MIDIChannel, INT16 currentTrack, INT16 controler, UINT16 value)
{
    //fprintf(stdout, "PV_ProcessExternalSynthController: pSong: %d, MIDIChannel: %d, currentTrack: %d, controler: %d, valud: %d\n", pSong, MIDIChannel, currentTrack, controler, value);

    // $$kk: 07.12.99: this is a temporary solution!  i am setting the song userReference
    // to the GM_Synth pointer....
    GM_Synth *pSynth = (GM_Synth *)pSong->userReference;

    if (PV_IsMuted(pSong, MIDIChannel, currentTrack) == FALSE)
	{
	    // $$kk: 11.09.99: use this to set the values so that our "get" methods pseudo-work.
	    PV_ProcessController(threadContext, pSong, MIDIChannel, currentTrack, controler, value);

	    if (pSong->AnalyzeMode == SCAN_NORMAL)
		{
		    // $$fb 2002-02-01: itanium port: widen handle to ptr size
		    sendMIDIEvent((UINT_PTR) pSynth->deviceHandle, (BYTE)(0xB0 | MIDIChannel), (BYTE)controler, (BYTE)value);
		}
	}
}

#endif // USE_EXTERNAL_SYNTH
