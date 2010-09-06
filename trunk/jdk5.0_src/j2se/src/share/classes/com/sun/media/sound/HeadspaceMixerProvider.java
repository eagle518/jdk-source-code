/*
 * @(#)HeadspaceMixerProvider.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;	  

import javax.sound.sampled.Mixer;
import javax.sound.sampled.spi.MixerProvider;


/**
 * Provider for the Headspace Mixer.
 *
 * @version 1.11 03/12/19
 * @author Kara Kytle
 */
public class HeadspaceMixerProvider extends MixerProvider {

    /**
     * Headspace mixer instance
     */
    private static HeadspaceMixer theMixer = HeadspaceMixer.getMixerInstance();


    /**
     * Headspace mixer description.
     */
    private static final Mixer.Info info = theMixer.getMixerInfo();
    
    public Mixer.Info[] getMixerInfo() {

	Mixer.Info[] localInfo = { info };
	if ((theMixer != null) && theMixer.audioDeviceExists()) {
	    return localInfo;
	}
	return new Mixer.Info[0];
    }

    public Mixer getMixer(Mixer.Info info) {

	if ( (info == null) || (info.equals(this.info)) ) {
	    if ((theMixer != null) && theMixer.audioDeviceExists()) {
		return theMixer;
	    }
	} 

	throw new IllegalArgumentException("Mixer " + info.toString() + " not supported by this provider.");
    }
}
