/*
 * @(#)IgorMidiInputDevice.h	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*****************************************************************************/
/*
** "IgorMidiInputDevice.h"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**
**
** Modification History:
**
**	2/9/97		Created
**
*/
/*****************************************************************************/


#ifndef IGOR_AUDIO
#include "IgorAudio.h"
#endif

enum
{
    IGOR_MIDI_MANAGER_FAILED = 20000,
    IGOR_OMS_FAILED,
    IGOR_MIDI_MANGER_NOT_THERE
};

typedef enum
{
    IGOR_OMS_CONNECT = 0,				// connect with just OMS virtual device
    IGOR_OMS_NAME_CONNECT,				// connect with OMS and the OMS name manager
    IGOR_MIDI_MANAGER_CONNECT			// connect with MIDI Manager
} IgorConnection;

// Midi Device class for direct midi input from external platform devices
class IgorMidiInputDevice : public IgorMidiDirect
{
    public:
    IgorMidiInputDevice(IgorAudioMixer *pIgorAudioMixer,
			char *pName = 0L, void * userReference = 0);
    ~IgorMidiInputDevice();

    IgorErr			Start(IGOR_BOOL loadInstruments,
				      IgorConnection connect,
				      long midiReference);
    void			Stop(void);

    long			GetMidiReference(void);
    long			GetTimeBaseOffset(void);
    void			SetTimeBaseOffset(long newTimeBase);

    private:
	long			timeBaseOffset;
    long			reference;
    void			*controlVariables;
};

