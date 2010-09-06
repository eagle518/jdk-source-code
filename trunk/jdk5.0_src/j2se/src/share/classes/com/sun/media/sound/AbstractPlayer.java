/*
 * @(#)AbstractPlayer.java	1.48 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.util.Vector;
import java.util.StringTokenizer;	// for parsing classpath

import java.net.URL;
import java.net.MalformedURLException;

import java.io.File;
import java.io.IOException;

import javax.sound.midi.*;



/**
 * Abstract AbstractPlayer class representing functionality shared by the
 * Sequencer and Synthesizer components of the Headspace Mixer.  This
 * class is extended by the concrete classes MixerSynth and MixerSequencer.
 *
 * @version 1.48, 03/12/19
 * @author David Rivas
 * @author Kara Kytle
 */
abstract class AbstractPlayer extends AbstractMidiDevice {


    // STATIC VARIABLES

    // soundbank names in descending quality order.
    // i am leaving "soundbank.gm" in for historical reasons:
    // in JDK 1.2 releases, this is identical to "soundbank-mid.gm."
    // in early JDK 1.3 releases, this is identical to "soundbank-min.gm."
    private static final String[] defaultSoundbankNames = { "soundbank-deluxe.gm", "soundbank-mid.gm", "soundbank.gm", "soundbank-min.gm" };

    // $$jb: 04.21.99: is this getable somewhere?
    // $$kk: 04.222.99: put this variable in Platform.java with the other
    // magic strings.
    private static final String soundJarName = "sound.jar";
    private static final String jmfJarName = "jmf.jar";

    static HeadspaceMixer mixer = HeadspaceMixer.getMixerInstance();

    /**
     * Default soundbank for this player.  Might as well be static
     * right now, but theoretically could differ between instances.
     */
    // $$jb: 07.08.99: making this static so there's only one instance
    // of the soundbank for all sequences.
    static private Soundbank defaultSoundbank = null;


    // INSTANCE VARIABLES

    /**
     * The internal synthesis unit.  I think of it like the built-in sound
     * generating circuitry; it doesn't go on the list of receivers.
     */
    AbstractMidiDevice internalSynth = null;


    /**
     * Set of channels for the synthesizer.
     */
    protected MixerMidiChannel[] channels;

    /**
     * List of loaded instruments
     */
    private Vector instruments = new Vector();




    // CONSTRUCTOR

    protected AbstractPlayer(MidiDevice.Info info) {

	super(info);

	if(Printer.trace) Printer.trace(">> AbstractPlayer CONSTRUCTOR: " + this);

	// create and initialize the MidiChannel objects
	channels = new MixerMidiChannel[16];			
	for (int i = 0; i < channels.length; i++) {
	    channels[i] = new MixerMidiChannel(this, i);
	}

	if(Printer.trace) Printer.trace("<< AbstractPlayer CONSTRUCTOR completed: " + this);
    }


    // SYNTHESIZER METHODS


    public int getMaxPolyphony() {
	return (HeadspaceMixer.getMixerInstance().getMidiVoices());
    }


    public MidiChannel[] getChannels() {

	MidiChannel[] returnedChannels = new MidiChannel[channels.length];
	System.arraycopy(channels, 0, returnedChannels, 0, channels.length);
	return returnedChannels;
    }


    public VoiceStatus[] getVoiceStatus() {
	return new VoiceStatus[0];
    }


    public boolean isSoundbankSupported(Soundbank soundbank) {
		return (soundbank instanceof HeadspaceSoundbank) ? true : false;

	// $$kk: 11.03.99: TODO: native midi case
    }


    public boolean loadInstrument(Instrument instrument) {

	if (instruments.contains(instrument)) {
	    return true;
	}

	try {
	    if (nLoadInstrument(id, ((HeadspaceInstrument)instrument).getId())) {
		instruments.addElement(instrument);
		return true;
	    } else {
		return false;
	    }
	} catch (ClassCastException e) {			
	    throw new IllegalArgumentException("Unsupported soundbank: " + instrument.getSoundbank());
	}

	// $$kk: 11.03.99: TODO: native midi case
    }


    public void unloadInstrument(Instrument instrument) {

	try {
	    if (nUnloadInstrument(id, ((HeadspaceInstrument)instrument).getId())) {
		instruments.removeElement(instrument);
	    }
	} catch (ClassCastException e) {			
	    throw new IllegalArgumentException("Unsupported soundbank: " + instrument.getSoundbank());
	}
    }


    public boolean remapInstrument(Instrument from, Instrument to) {

	try {
	    return nRemapInstrument(id, ((HeadspaceInstrument)from).getId(), ((HeadspaceInstrument)to).getId());
	} catch (ClassCastException e) {			
	    throw new IllegalArgumentException("Unsupported soundbank: " + from.getSoundbank() + " or " + to.getSoundbank());
	}
    }


    public Soundbank getDefaultSoundbank() {
	return defaultSoundbank;
    }


    public Instrument[] getAvailableInstruments() {
		
	if (defaultSoundbank != null) {
	    return defaultSoundbank.getInstruments();
	} else {
	    return new Instrument[0];
	}
    }


    public Instrument[] getLoadedInstruments() {

	Instrument[] returnedArray;

	synchronized(instruments) {
	    returnedArray = new Instrument[instruments.size()];
	    for (int i = 0; i < returnedArray.length; i++) {
		returnedArray[i] = (Instrument)instruments.elementAt(i);
	    }
	}

	return returnedArray;
    }


    public boolean loadAllInstruments(Soundbank soundbank) {

	boolean allOk = true;
	Instrument[] instrumentArray = soundbank.getInstruments();

	for (int i = 0; i < instrumentArray.length; i++) {

	    // may throw IllegalArgumentException
	    if (!loadInstrument(instrumentArray[i])) {
		allOk = false;
	    }
	}

	return allOk;
    }


    public void unloadAllInstruments(Soundbank soundbank) {

	Instrument[] instrumentArray = soundbank.getInstruments();

	for (int i = 0; i < instrumentArray.length; i++) {				
	    // may throw IllegalArgumentException
	    unloadInstrument(instrumentArray[i]);
	}
    }


    public boolean loadInstruments(Soundbank soundbank, Patch[] patchList) {

	boolean allOk = true;

	for (int i = 0; i < patchList.length; i++) {

	    Instrument instrument = soundbank.getInstrument(patchList[i]);

	    if (instrument != null) {
		// may throw IllegalArgumentException
		if (!loadInstrument(instrument)) {
		    allOk = false;
		}
	    } else {
		allOk = false;
	    }
	}

	return allOk;
    }


    public void unloadInstruments(Soundbank soundbank, Patch[] patchList) {

	for (int i = 0; i < patchList.length; i++) {

	    Instrument instrument = soundbank.getInstrument(patchList[i]);

	    if (instrument != null) {
		// may throw IllegalArgumentException
		unloadInstrument(instrument);
	    }
	}
    }



    // HELPER METHODS

    /**
     * Return the programmed time-stamp in microseconds.
     * MixerSynth always returns -1.
     */
    protected abstract long getTimeStamp();


    /**
     * This method gets the default synth for the player.
     * This now always is the softsynth; HW synths (via MidiOut) are
     * no longer supported.
     * this operates as the built-in sound generating unit
     * for the synth/sequencer. 
     *
     * We try to load the default soundbank, but silently fail if this is
     * not possible. Applications can find out if the default soundbank is
     * loaded by calling getDefaultSoundbank(). It returns null if there is
     * no default soundbank. Applications can then load a soundbank via
     * MidiSystem and use the loadInstrument methods of the Synthesizer
     * to use the soundbank.
     */
    protected void openInternalSynth() throws MidiUnavailableException {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: openInternalSynth: " + this);

	// if the internalSynth is already set, just return
	if (internalSynth != null) {
	    if (Printer.trace) Printer.trace("<< AbstractPlayer: openInternalSynth: internalSynth already set: " + internalSynth);
	    return;
	}

	if (Printer.debug) Printer.debug("AbstractPlayer: openInternalSynth: trying software midi");

	// try to load the default soundbank
	if (defaultSoundbank == null) {
	    defaultSoundbank = loadDefaultSoundbank();
	}

	internalSynth = openSoftwareSynth();
	if (Printer.trace) Printer.trace("< AbstractPlayer: openInternalSynth: internalSynth");
    }



    /**
     * remove and close the default synth
     */
    protected void closeInternalSynth() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: closeInternalSynth");

	if (internalSynth != this) {
	    if (Printer.err) Printer.err("AbstractPlayer.connectToInternalSynth: external synths no longer supported!");
	} else {
	    nRemoveReceiver(id, 0);
	}

	internalSynth = null;
	if (Printer.trace) Printer.trace("<< AbstractPlayer: closeInternalSynth completed");
    }


    /**
     * add the software synth (always succeeds)
     */
    protected AbstractMidiDevice openSoftwareSynth() {
	if (Printer.trace) Printer.trace(">> AbstractPlayer: openSoftwareSynth");
	if (Printer.trace) Printer.trace("<< AbstractPlayer: openSoftwareSynth returning: " + this);
	return this;
    }


    /** 
     * Set the internal synth as a receiver.  Note
     * that the order of events matters here:  with
     * ths sofware synth, you need the soundbank loaded
     * before the sequence gets set.  But you need to
     * connect to the internal synth *after* setting the
     * sequence because the id value gets set there!!
     */
    protected void connectToInternalSynth() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: connectToInternalSynth");

	if (internalSynth == null) {
	    if(Printer.err) Printer.err("AbstractPlayer.connectToInternalSynth: internalSynth is null!");
	    return;
	}

	if (internalSynth == this) {			
	    // add the software synth
	    synchronized(this) { nAddReceiver(id, 0); }
		
	} else {
	    if (Printer.err) Printer.err("AbstractPlayer.connectToInternalSynth: external synths no longer supported!");
	}

	if (Printer.trace) Printer.trace("<< AbstractPlayer: connectToInternalSynth completed");
    }


    /**
     * this method closes the native resources but does not change the state.
     *
     * we need this because MixerSequencer.setSequence() actually needs to
     * close and reopen the native resources without changing the Sequencer
     * open/close state.
     */
    protected synchronized void implClose() {

	if (Printer.trace) Printer.trace(">> AbstractPlayer: implClose");

	// remove and close the default synth
	closeInternalSynth();
	
	long oldId = id;
	id = 0;

	// update the channels with the new id
	for (int i = 0; i < channels.length; i++) {
	    channels[i].setId(id);
	}

	// $$kk: 04.14.99: concrete subclasses should call this
	// method and then do their own cleanup.
	// $$kk: 04.14.99: wait, we actually use the same method
	// for closing sequencers and synths, so might as well
	// put it here.
	nClose(oldId);
	super.implClose();
	if (Printer.trace) Printer.trace("<< AbstractPlayer: implClose completed");
    }



    /**
     * This method currently looks in the current working directory and then
     * relative to javahome for each element in the hard-coded set of
     * default soundbank names.  This allows us, for instance, to ship
     * different soundbanks on different platforms, and look for them in
     * quality order.
     *
     * $$kk: 04.16.99: we may want to add support for a special property name
     * and / or searching the classpath....
     */
    private Soundbank loadDefaultSoundbank() {

	String soundbankName;
	URL url;
	String path;
	String classpath;
	String classpathElement;
	String issoundjar;

	Soundbank bank = null;

	for (int i = 0; i < defaultSoundbankNames.length; i++) {

	    soundbankName = defaultSoundbankNames[i];

	    // look locally
	    try {

		path = "file:" + soundbankName;
		url = new URL(path);

		if(Printer.debug) Printer.debug("Looking for soundbank: " + url);

		bank = new HeadspaceSoundbank(url);

		if (bank != null) {

		    break;
		}

	    } catch (MalformedURLException e) {
	    } catch (IOException e2) {
	    } catch (IllegalArgumentException e3) {
	    }			

	    // look where sound.jar is
	    classpath = Platform.getClasspath();
	    StringTokenizer st = new StringTokenizer(classpath, File.pathSeparator);

	    while (st.hasMoreTokens() && (bank==null) ) {
			
		// this is the next element in the classpath
		classpathElement = st.nextToken();
		// find sound.jar in classpath, replace with defaultSoundbankNames
		issoundjar = classpathElement.substring(classpathElement.lastIndexOf(File.separatorChar)+1,classpathElement.length());
		if( issoundjar.equals(soundJarName) || issoundjar.equals(jmfJarName) ){
		    path = classpathElement.substring(0, classpathElement.lastIndexOf(File.separatorChar));
		    path += File.separatorChar + soundbankName;
		    if(Printer.debug) Printer.debug("Looking for soundbank: " + path);
		    try {
			bank = new HeadspaceSoundbank(path);
		    } catch (IllegalArgumentException e) {
		    }
		}
	    }

	    if( bank != null ) break;

	    // look relative to java.home.
	    path = Platform.getJavahome();
	    path += File.separatorChar + "lib" + File.separatorChar + "audio" + File.separatorChar + soundbankName;

	    try  {

		if(Printer.debug) Printer.debug("Looking for soundbank: " + path);

		bank = new HeadspaceSoundbank(path);

		if (bank != null) {

		    break;
		}

	    } catch (IllegalArgumentException e2) {
	    }
	}

	if(Printer.debug) Printer.debug("Default soundbank: " + bank);
	return bank;
    }



    // NATIVE METHODS

    // SYNTHESIZER / SEQUENCER MANAGEMENT

    // GM_FreeSong
    protected native void nClose(long id);


    // MIDI OUT / SYNTH MANAGEMENT

    // GM_AddSongSynth
    protected native void nAddReceiver(long id, long receiverId);

    // GM_RemoveSongSynth
    protected native void nRemoveReceiver(long id, long receiverId);


    // INSTRUMENT MANAGEMENT

    // GM_LoadInstrument
    private native boolean nLoadInstrument(long id, int instrumentId);

    // GM_UnloadInstrument
    private native boolean nUnloadInstrument(long id, int patchIdentifier);

    // GM_RemapInstrument
    private native boolean nRemapInstrument(long id, int from, int to);
}
