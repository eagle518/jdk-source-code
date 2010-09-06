/*
 * @(#)MixerSynthProvider.java	1.16 04/03/29
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.spi.MidiDeviceProvider;

/**
 * Provider for MixerSynth objects.
 *
 * @version 1.16, 04/03/29
 * @author Kara Kytle
 */
public class MixerSynthProvider extends MidiDeviceProvider {


    public MidiDevice.Info[] getDeviceInfo() {

	HeadspaceMixer mixer = HeadspaceMixer.getMixerInstance();
	if ((mixer != null) && mixer.audioDeviceExists()) {
	    MidiDevice.Info[] localInfo = { MixerSynth.info };
	    return localInfo;
	}
	return new MidiDevice.Info[0];
    }


    public MidiDevice getDevice(MidiDevice.Info info) {

	HeadspaceMixer mixer = HeadspaceMixer.getMixerInstance();
	if ((mixer == null) || !mixer.audioDeviceExists()) {
	    return null;
	}

	if ( (info != null) && (!info.equals(MixerSynth.info)) ) {
	    return null;
	}

	try {
	    return new MixerSynth();
	} catch (MidiUnavailableException e) {
	    return null;
	}
    }
}
