/*
 * @(#)MixerSynth.java	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.*;
import javax.sound.sampled.LineUnavailableException;


/**
 * Synthesizer using the Headspace Mixer.
 *
 * @version 1.28, 03/12/19
 * @author David Rivas
 * @author Kara Kytle
 * @author Florian Bomers
 */
class MixerSynth extends AbstractPlayer implements Synthesizer {

    /**
     * All MixerSynths share this info object.
     */
    static final MixerSynthInfo info = new MixerSynthInfo();
    
    /** Debugging flag */
    private static final boolean TRACE_MESSAGES = false;


    // CONSTRUCTOR

    MixerSynth() throws MidiUnavailableException {

	super(info);
	if(Printer.trace) Printer.trace(">> MixerSynth CONSTRUCTOR");
	if(Printer.trace) Printer.trace("<< MixerSynth CONSTRUCTOR completed");
    }


    // SYNTHESIZER PUBLIC METHODS

    public long getLatency() {
	return nGetLatency();
    }


    // OVERRIDES OF ABSTRACT MIDI DEVICE METHODS


    /** Returns if this device supports Receivers.
	This implementation always returns true.
	@return true, if the device supports Receivers, false otherwise.
    */
    protected boolean hasReceivers() {
	return true;
    }


    protected Receiver createReceiver() {
	return new SynthReceiver();
    }


    // INNER CLASSES

    class SynthReceiver extends AbstractReceiver {

	protected void implSend(MidiMessage message, long timeStamp) {
	    if (TRACE_MESSAGES) Printer.println("> implSend");
	    // if the device is not open, throw an exception
	    if (id == 0) {
		throw new IllegalStateException("Synthesizer is not open.");
	    }
	    // send the received message to the internal synth
	    if ( ! (message instanceof ShortMessage) ) {
		if(Printer.err) Printer.err("Unsupported message type: " + message);
		return;
	    }
	    ShortMessage shortMessage = (ShortMessage)message;
	    parse(shortMessage.getCommand(), shortMessage.getChannel(), shortMessage.getData1(), shortMessage.getData2());
	    if (TRACE_MESSAGES) Printer.println("< implSend");
	}

	// shortcut for optimization. timeStamp is ignored (always realtime)
	void sendPackedMidiMessage(int packedMsg, long timeStamp) {
	    if (TRACE_MESSAGES) Printer.println("> sendPackedMidiMessage");
	    // if the device is not open, ignore
	    if (id == 0) {
		return;
	    }
	    parse(packedMsg & 0xF0,
		  packedMsg & 0x0F,
		  (packedMsg & 0x7F00) >> 8,
		  (packedMsg & 0x7F0000) >> 16);
	    if (TRACE_MESSAGES) Printer.println("< sendPackedMidiMessage");
	}
    } // class SynthReceiver


    // returns true if successfully parsed, otherwise false
    private boolean parse(int command, int channel, int data1, int data2) {
	if (((command & 0x0F) != 0) || (command < 0x80) || (command > 0xE0)) {
	    // invalid status
	    return false;
	}

	switch (command) {
	case 0x80:	// Note off
	    channels[channel].noteOff(data1, data2);
	    break;
	case 0x90: 	// Note on
	    channels[channel].noteOn(data1, data2);
	    break;
	case 0xA0:	// key pressure (aftertouch)
	    channels[channel].setPolyPressure(data1, data2);
	    break;
	case 0xB0:	// controllers
	    // $$kk: 11.06.98: what about 14-bit controllers??
	    channels[channel].controlChange(data1, data2);
	    break;
	case 0xC0:	// Program change
	    //$$fb 2002-10-30: fix for 4425843: MIDI Program Change messages sent to the synthesizer are ignored
	    channels[channel].programChange(data1);
	    break;
	case 0xD0:	// channel pressure (aftertouch)
	    channels[channel].setChannelPressure(data1);
	    break;
	case 0xE0:	// SetPitchBend
	    channels[channel].setPitchBend(((data2 & 0x7F) << 7) | (data1 & 0x7F));
	    break;
	}
	return true;
    }


    // HELPER METHODS


    /**
     * MixerSynth always returns -1.
     */
    protected long getTimeStamp() {
	return -1;
    }


    protected void implOpen() throws MidiUnavailableException {
	try {
	    mixer.open(this);
	} catch (LineUnavailableException lue) {
	    throw new MidiUnavailableException(lue.getMessage());
	}

	id = nCreateSynthesizer();
	if (id == 0) {
	    throw new MidiUnavailableException("Failed to initialize synthesizer");
	}

	// open the default synth
	openInternalSynth(); // can throw MidiUnavailableException

	// connect to the internal synth (with the new id value)
	connectToInternalSynth();

	// $$kk: 04.07.99: can this fail??
	if (!(nStartSynthesizer(id))) {
	    id = 0;
	    throw new MidiUnavailableException("Failed to start synthesizer");
	}

	// update the channels with the new id
	for (int i = 0; i < channels.length; i++) {
	    channels[i].setId(id);
	}
    }


    protected void implClose() {
	if (Printer.trace) Printer.trace(">> MixerSynth.implClose");
	super.implClose();
	mixer.close(this);
	if (Printer.trace) Printer.trace("<< MixerSynth.implClose");
    }


    // INNER CLASSES

    private static class MixerSynthInfo extends MidiDevice.Info {

	private static final String name = "Java Sound Synthesizer";
	private static final String vendor = "Sun Microsystems";
	private static final String description = "Software wavetable synthesizer and receiver";
	private static final String version = "Version 1.0";

	private MixerSynthInfo() {
	    super(name, vendor, description, version);
	}
    }


    // NATIVE METHODS

    // SYNTH CREATION AND DESTRUCTION

    // GM_CreateLiveSong
    private native long nCreateSynthesizer();

    // GM_StartLiveSong
    private native boolean nStartSynthesizer(long id);

    // HAE_GetMaxSamplePerSlice() * HAE_GetAudioBufferCount());
    private native long nGetLatency();
}


