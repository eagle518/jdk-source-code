/*
 * @(#)ReferenceCountingDevice.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.MidiUnavailableException;
import javax.sound.midi.Receiver;
import javax.sound.midi.Transmitter;



/** MidiDevice that can use reference counting for open/close.
 * This interface is intended to be used by MidiSystem.getTransmitter() and
 * MidiSystem.getReceiver().
 * 
 * @version 1.4, 03/12/19
 * @author Matthias Pfisterer
 */
public interface ReferenceCountingDevice {
    /** Retrieve a Receiver that opens the device implicitly.
     * This method is similar to MidiDevice.getReceiver(). However, by calling this one,
     * the device is opened implicitly. This is needed by MidiSystem.getReceiver().
     */
    public Receiver getReceiverReferenceCounting() throws MidiUnavailableException;

    /** Retrieve a Transmitter that opens the device implicitly.
     * This method is similar to MidiDevice.getTransmitter(). However, by calling this one,
     * the device is opened implicitly. This is needed by MidiSystem.getTransmitter().
     */
    public Transmitter getTransmitterReferenceCounting() throws MidiUnavailableException;
}
