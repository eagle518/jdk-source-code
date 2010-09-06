/*
 * @(#)AutoConnectSequencer.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.Receiver;

/**
 * Interface for Sequencers that are able to do the auto-connect
 * as required by MidiSystem.getSequencer()
 *
 * @version 1.2, 03/12/19
 * @author Florian Bomers
 */
public interface AutoConnectSequencer {

    /**
     * Set the receiver that this device is 
     * auto-connected. If non-null, the device
     * needs to re-connect itself to a suitable
     * device in open().
     */
    public void setAutoConnect(Receiver autoConnectReceiver);

}
