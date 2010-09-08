/*
 * @(#)HAEMidiExternal.h	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*
** "HAEMidiExternal.h"
**
**	Generalized Audio Synthesis package presented in an oop fashion
**
** Modification History:
**
**	2/9/97		Created
**	2/10/98		Added virtual to ~HAEMidiExternal
**
*/
/*****************************************************************************/


#ifndef HAE_AUDIO
#include "HAE.h"
#endif

#ifndef HAE_MIDI_EXTERNAL
#define HAE_MIDI_EXTERNAL
enum
{
    HAE_MIDI_MANAGER_FAILED = 20000,
    HAE_OMS_FAILED,
    HAE_MIDI_MANGER_NOT_THERE
};

enum HAEConnection
{
    HAE_OMS_CONNECT = 0,				// connect with just OMS virtual device
    HAE_OMS_NAME_CONNECT,				// connect with OMS and the OMS name manager
    HAE_MIDI_MANAGER_CONNECT			// connect with MIDI Manager
};

// Midi class for direct midi input from external platform devices
class HAEMidiExternal : public HAEMidiDirect
{
    public:
    HAEMidiExternal(HAEAudioMixer *pHAEAudioMixer,
		    char *pName = 0L, void * userReference = 0);
    virtual			~HAEMidiExternal();

    HAEErr			Start(HAE_BOOL loadInstruments,
				      HAEConnection connect,
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

#endif	// HAE_MIDI_EXTERNAL
