/*
 * @(#)RealTimeSequencerProvider.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.spi.MidiDeviceProvider;

/**
 * Provider for RealTimeSequencer objects.
 *
 * @version 1.2, 03/12/19
 * @author Florian Bomers
 */
public class RealTimeSequencerProvider extends MidiDeviceProvider {


    public MidiDevice.Info[] getDeviceInfo() {

	MidiDevice.Info[] localArray = { RealTimeSequencer.info };
	return localArray;
    }


    public MidiDevice getDevice(MidiDevice.Info info) {
	if ((info != null) && (!info.equals(RealTimeSequencer.info))) {
	    return null;
	}

	try {
	    return new RealTimeSequencer();
	} catch (MidiUnavailableException e) {
	    return null;
	}
    }
}
