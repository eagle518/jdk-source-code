/*
 * @(#)MixerMidiChannel.java	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.midi.MidiChannel;


/**
 * MixerMidiChannel.
 *
 * @version 1.22, 03/12/19
 * @author David Rivas
 * @author Kara Kytle
 */
class MixerMidiChannel implements MidiChannel {

    // DEFAULTS

    //protected static final float defaultLinearGain			= Util.dBToLinear(0.0f);

    protected static final boolean defaultMute				= false;
    protected static final boolean defaultLocal				= true;
    protected static final boolean defaultMono				= false;
    protected static final boolean defaultOmni				= false;
    protected static final boolean defaultSolo				= false;
    protected static final int     defaultProgram			= 0;


    // STATE VARIABLES

    private int channelNumber;

    //protected float linearGain;

    protected boolean mute;
    protected boolean local;
    protected boolean mono;
    protected boolean omni;
    protected boolean solo;
    protected int     program;


    // IMPLEMENTATION VARIABLES

    protected AbstractPlayer player;
    private long id							= 0;


    MixerMidiChannel(AbstractPlayer player, int channelNumber) {

	// set the player
	this.player = player;

	// setup defaults
	mute = defaultMute;
	this.local = defaultLocal;
	this.mono = defaultMono;
	this.omni = defaultOmni;
	this.solo = defaultSolo;
	this.program = defaultProgram;

	// set the channel id
	this.channelNumber = channelNumber;
    }


    // MIDI CHANNEL METHODS


    public void noteOn(int noteNumber, int velocity) {
	if (id != 0) { nNoteOn(id, channelNumber, noteNumber, velocity, player.getTimeStamp()); }
    }


    public void noteOff(int noteNumber, int velocity) {
	if (id != 0) { nNoteOff(id, channelNumber, noteNumber, velocity, player.getTimeStamp()); }
    }


    public void noteOff(int noteNumber) {
	noteOff(noteNumber, 0);
    }


    public void setPolyPressure(int noteNumber, int pressure) {
	// $$kk: 11.08.99: the engine does not implement this
    }


    public int getPolyPressure(int noteNumber) {
	// $$kk: 11.08.99: the engine does not implement this
	return 0;
    }



    public void setChannelPressure(int pressure) {
	// $$kk: 11.08.99: the engine does not implement this
    }


    public int getChannelPressure() {
	// $$kk: 11.08.99: the engine does not implement this
	return 0;
    }


    public void controlChange(int controller, int value) {
	if (id != 0) { nControlChange(id, channelNumber, controller, value, player.getTimeStamp()); }
    }


    public int getController(int controller) {

	if (id != 0) { return nGetController(id, channelNumber, controller); }
	return 0;
    }


    public void programChange(int program) {

	if (id != 0) {
	    nProgramChange(id, channelNumber, program, player.getTimeStamp());
	    this.program = program;
	}
    }


    public void programChange(int bank, int program) {

	if (id != 0) {
	    nProgramChange(id, channelNumber, bank, program, player.getTimeStamp());
	    this.program = program;
	}
    }


    public int getProgram() {

	// $$jb: 07.14.99: is there a way to get this from the engine?
	//                 what about banks?

	// $$kk: 11.09.99: yes:	((GM_Song *)id)->channelProgram[channel]
	//						((GM_Song *)id)->channelBank[channel]

	return program;
    }


    public void setPitchBend(int bend) {
	if (id != 0) { nSetPitchBend(id, channelNumber, (bend / 128), (bend % 128), player.getTimeStamp()); }
    }


    public int getPitchBend() {

	if (id != 0) {return nGetPitchBend(id, channelNumber); }
	return 0;
    }


    public void resetAllControllers() {
	if (id != 0) { nResetAllControllers(id, channelNumber); }
    }


    public void allNotesOff() {
	if (id != 0) { nAllNotesOff(id, channelNumber, player.getTimeStamp()); }
    }


    public void allSoundOff() {
	// $$kk: 04.07.99: need to implement!
    }


    public boolean localControl(boolean on) {
	// $$kk: 04.07.99: need to implement!
	return local;
    }


    public void setMono(boolean on) {
	// $$kk: 04.07.99: need to implement!
    }


    public boolean getMono() {
	return mono;
    }


    public void setOmni(boolean on) {
	// $$kk: 10.12.99: need to implement!
    }


    public boolean getOmni() {
	return omni;
    }


    public void setMute(boolean muteState) {

	if ( (id != 0) && (mute != muteState) ) {
	    mute = nSetMute(id, channelNumber, muteState);
	}
    }


    public boolean getMute() {
	return mute;
    }


    public void setSolo(boolean soloState) {

	// $$jb:07.14.99: look at this!  the native code is
	// setting the opposite of what we're requesting...

	if ( (id != 0) && (solo != soloState) ) {
	    solo = nSetSolo(id, channelNumber, soloState);
	}
    }


    public boolean getSolo() {
	return solo;
    }


    // ADDITIONAL METHODS

    void setId(long id) {
	this.id = id;
    }



    // NATIVE METHODS

    // GM_SoloChannel or GM_UnsoloChannel
    protected native boolean nSetSolo(long id, int channelNumber, boolean soloState);

    // GM_GetChannelSoloStatus
    protected native boolean nGetSolo(long id, int channelNumber);

    //protected native int nGetPolyPressure(long id, int channelNumber, int noteNumber );
    //protected native int nGetChannelPressure(long id, int channelNumber);
    protected native int nGetController(long id, int channelNumber, int controller);
    protected native int nGetPitchBend(long id, int channelNumber);

    protected native void nNoteOn(long id, int channelNumber, int noteNumber, int  velocity, long timeStamp);
    protected native void nNoteOff(long id, int channelNumber, int noteNumber, int  velocity, long timeStamp);
    //protected native void nSetPolyPressure(long id, int channelNumber, int noteNumber, int pressure, long timeStamp);
    //protected native void nSetChannelPressure(long id, int channelNumber, int pressure, long timeStamp);
    protected native void nControlChange(long id, int channelNumber, int controller, int value, long timeStamp);
    protected native void nProgramChange(long id, int channelNumber, int program, long timeStamp);
    protected native void nProgramChange(long id, int channelNumber, int bank, int program, long timeStamp);
    protected native void nSetPitchBend(long id, int channelNumber, int bendHigh, int bendLow, long timeStamp);

    // GM_AllNotesOff or QGM_AllNotesOff
    protected native void nAllNotesOff(long id, int channelNumber, long timeStamp);

    // $$kk: 09.21.98: in the engine, can use a boolean for complete or semi-complete reset???
    // PV_ResetControlers
    protected native void nResetAllControllers(long id, int channelNumber);

    protected native boolean nSetMute(long id, int channelNumber, boolean muteState);
    //protected native boolean nGetMute(long id, int channelNumber);
    //protected native float nSetLinearGain(long id, int channelNumber, float linearGain);
    //protected native float nGetLinearGain(long id, int channelNumber);
}

