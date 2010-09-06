/*
 * @(#)MixerSequencer.java	1.62 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;

import java.util.Vector;

import javax.sound.midi.*;
import javax.sound.sampled.LineUnavailableException;


/**
 * Sequencer using the Headspace Mixer.  This Sequencer implementation
 * extends MixerSynth, reflecting the current structure in the engine.
 * In the future, we want to modify this to allow selection of the
 * Receiver....
 *
 * @version 1.62, 03/12/19
 * @author Kara Kytle
 * @author Florian Bomers
 */
class MixerSequencer extends AbstractPlayer implements Sequencer{

    // STATIC VARIABLES

    /**
     * Global event thread
     * $$kk: 06.22.99: we are not using this now....
     * $$kk: 10.13.99: MixerSequencer is using this
     * for meta anc controller events!
     */
    private static final EventDispatcher eventDispatcher;

    private static final int MIDI_TYPE_0 = 0;
    private static final int MIDI_TYPE_1 = 1;
    private static final int MIDI_TYPE_2 = 2;

    /**
     * All MixerSequencers share this info object.
     */
    static final MixerSequencerInfo info	= new MixerSequencerInfo();


    private static Sequencer.SyncMode[] masterSyncModes	= { Sequencer.SyncMode.INTERNAL_CLOCK };
    private static Sequencer.SyncMode[] slaveSyncModes	= { Sequencer.SyncMode.NO_SYNC };

    private static Sequencer.SyncMode masterSyncMode	= Sequencer.SyncMode.INTERNAL_CLOCK;
    private static Sequencer.SyncMode slaveSyncMode	= Sequencer.SyncMode.NO_SYNC;



    /**
     * Sequence on which this sequencer is operating.
     */
    private Sequence sequence = null;


    /**
     * Local copy of sequence data
     */
    private byte midiData[] = null;


    /**
     * True whenever the song has been started, until the
     * song end callback.
     */
    private boolean runningInEngine = false;


    /**
     * Marked to false each time we close the native sequencer object,
     * in implClose.  it then gets started with GM_BeginSong.  Subsequently,
     * it gets started with GM_ResumeSong.
     */
    private boolean newSequenceStarted = false;



    // some of the methods to the engine do not function under certain state
    // conditions.  we cache the requested values in these variables and
    // make the calls when they actually work.

    /**
     * If setTickPosition gets called while the sequencer is stopped, we
     * record the value here and set it when the sequencer is next
     * started.
     * -1 means not set.
     */
    private long tickCache = -1;


    /**
     * If setMicrosecondPosition gets called while the sequencer is stopped, we
     * record the value here and set it when the sequencer is next
     * started.
     * -1 means not set.
     */
    private long microsecondCache = -1;


    /**
     * If setTempoInBPM gets called before playback of a particular sequence
     * starts the first time, the value gets overridden by the value in the
     * sequence when it starts.  We record the value here and set it when
     * the sequencer is next started.
     * -1 means not set.
     */
    private float tempoInBPMCache = -1;


    /**
     * Same for setTempoInMPQ...
     * -1 means not set.
     */
    private float tempoInMPQCache = -1;


    //$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
    /**
     * cache value for tempo factor until dequence is set
     * -1 means not set.
     */
    private float tempoFactorCache = -1;


    /**
     * True if the sequence is running.
     */
    private boolean running = false;

    /**
     * True if we are recording
     */
    private boolean recording = false;

    /**
     * True if we have finished recording and need to
     * reset the sequence
     */
    private boolean sequenceChanged = false;

    private long loopStart = 0;
    private long loopEnd = -1;
    private int loopCount = 0;

    /**
     * Used for record timing
     */
    private long  startTime            = 0;
    private long  startMillisecondTime = 0;
    private long  lastTempoChangeTime  = 0;
    private long  lastTempoChangeTick  = 0;
    private long  recordTempoInMPQ     = 500000; // 120 bpm in mpq
    private long  startTick            = 0;
    private float divisionType         = 0.0f;
    private int   resolution           = 0;

    /**
     * Receiver instance for this sequencer
     */
    // $$mp 2003-08-18: disable recording
    // private SequencerReceiver sequencerReceiver = null;

    /**
     * Vector of tracks to which we're recording
     */
    // $$mp 2003-08-18: disable recording
    //private Vector recordingTracks = new Vector();


    /**
     * Meta event listeners
     */
    private Vector metaEventListeners = new Vector();


    /**
     * Control change listeners
     */
    private Vector controllerEventListeners = new Vector();



    static {

	// create and start the global event thread

	// $$kk: 12.21.98:
	// 1) probably don't want a single global event queue
	// 2) need a way to stop this thread when the engine is done

	eventDispatcher = new EventDispatcher();
	eventDispatcher.start();
    }



    // CONSTRUCTOR


    protected MixerSequencer() throws MidiUnavailableException {

	super(info);

	if(Printer.trace) Printer.trace(">> MixerSequencer CONSTRUCTOR");
	if(Printer.trace) Printer.trace("<< MixerSequencer CONSTRUCTOR completed");
    }


    // SEQUENCER METHODS


    public synchronized void setSequence(Sequence sequence) throws InvalidMidiDataException {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setSequence(" + sequence +")");

	// check for file type support
	int midiFileTypes[] = MidiSystem.getMidiFileTypes( sequence );
	if( midiFileTypes.length == 0 ) {
	    throw new InvalidMidiDataException("Unsupported sequence: " + sequence);
	}

	// write the file data
	ByteArrayOutputStream baos = new ByteArrayOutputStream();
	try {
	    MidiSystem.write(sequence, midiFileTypes[0], baos);
	    baos.close();
	} catch( IOException e ) {
	    throw new InvalidMidiDataException("Unable to get file stream from sequence: " + sequence);
	}
	byte[] fileByteArray = baos.toByteArray();
	ByteArrayInputStream fileStream = new ByteArrayInputStream(fileByteArray);

	// set and store the sequence
	try {
	    setSequence(fileStream);
	    this.sequence = sequence;
	} catch(IOException e) {
	    throw new InvalidMidiDataException("Failed to load sequence: " + sequence);
	}

	if(Printer.trace) Printer.trace("<< MixerSequencer: setSequence(" + sequence +") completed");
    }


    public synchronized void setSequence(InputStream stream) throws IOException, InvalidMidiDataException {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setSequence(" + stream +")");

	// first make sure the mixer is loaded.
	//if ( !isOpen() ) {
	//	throw new IllegalStateException("Cannot set sequence until sequencer has been opened");
	//}

	// now make sure that stream is a supported file type.
	// if not, and if a sequence is currently playing, we
	// can avoid interrupting it.
	MidiFileFormat fileFormat = MidiSystem.getMidiFileFormat(stream); // can throw IOException, InvalidMidiDataException
	int type = fileFormat.getType();
	int resolution = fileFormat.getResolution();


	// we don't support MIDI type 2 files
	if (type == MIDI_TYPE_2) {
	    throw new InvalidMidiDataException("Unsupported file type: " + type + ". Only type 0 and type 1 MIDI files are supported.");
	}

	// okay, we support it.  if we're currently playing,
	// stop and destroy the current native sequencer object
	if (isOpen() && id != 0) {

	    stop();

	    // this is a bit excessive!!  it will close and reopen the external synth
	    // if it's being used.
	    implClose();
	    try { doImplOpen(); } catch (MidiUnavailableException mue) { }
	}

	// get the midi data into a byte array
	midiData = getBytesFromFileStream(stream,fileFormat);
	if ( (midiData == null) || (midiData.length == 0) ) {
	    throw new IOException("Failed to read data from stream.");
	}

	// make sure we remove old instances of sequence
	this.sequence = null;

	if (!isOpen()) {
	    // just leave it as it is, let implOpen do the rest
	    return;
	}

	// create a MIDI sequencer if it's a MIDI type 0 or type 1 file
	if ( (type == MIDI_TYPE_0) || (type == MIDI_TYPE_1) ) {
	    id = 0;
	    if (resolution == MidiFileFormat.UNKNOWN_LENGTH) {
		// seems to be RMF
		id = nOpenRmfSequencer(midiData,midiData.length);
	    }
	    if (id == 0) {
		id = nOpenMidiSequencer(midiData, midiData.length);
	    }
	}

	// failure!
	if (id == 0) {
	    throw new InvalidMidiDataException("Failed to load sequence");
	}


	int i;

	// update the channels with the new id
	for (i = 0; i < channels.length; i++) {
	    channels[i].setId(id);
	}

	// connect to the internal synth (with the new id value)
	connectToInternalSynth();

	// $$fb propagate caches
	propagateCaches();


	// if we have any MIDI OUT receivers that are AbstractMidiDevices,
	// add them now!

	// $$kk: 09.27.99 need to implement additional receivers here
	/*
	  Receiver[] currentOutReceivers = getOutReceivers();

	  for (i = 0; i < currentOutReceivers.length; i++) {

	  if (currentOutReceivers[i] instanceof AbstractMidiDevice) {
	  addOutReceiver(currentOutReceivers[i]);
	  }
	  }
	*/

	// $$jb: if we have any ControllerEventListeners, add them now
	// because they were probably added before we had a native
	// sequencer object to add the listeners to.

	for (i = 0; i < controllerEventListeners.size(); i++) {

	    ControllerVectorElement cve = (ControllerVectorElement)controllerEventListeners.elementAt(i);
	    for (int z = 0; z < cve.controllers.length; z++) {
		nAddControllerEventCallback(id, cve.controllers[z]);
	    }
	}

	// $$jb: 06.02.99: do we want an event for sequence loaded?
	if(Printer.trace) Printer.trace("<< MixerSequencer: setSequence(" + stream +") completed");

    }


    public Sequence getSequence() {

	// $$kk: 04.14.99: right now this may be null if the sequencer
	// was opened with a FileStream; should decide how to handle
	// this!!
	// $$jb: 06.02.99: we can get it from MidiSystem.getSequence.
	// this call is expensive, so it makes sense to call it here,
	// rather than building the local sequence object every time
	// a new sequence is set.

	if( (sequence == null) && (midiData != null) && (midiData.length > 0) ) {

	    ByteArrayInputStream bais = new ByteArrayInputStream( midiData );

	    try {
		sequence = MidiSystem.getSequence(bais);
	    } catch (InvalidMidiDataException imde) {
	    } catch (IOException ioe) {
	    }
	}

	// if we failed to get the sequence, this will be null
	return sequence;
    }


    public synchronized void start() {


	if(Printer.trace) Printer.trace(">> MixerSequencer: start()");

	// sequencer not open: throw an exception
	if ( !isOpen() ) {
	    throw new IllegalStateException("sequencer not open");
	}

	// sequence not set: fail quietly
	if (id == 0) {
	    // $$fb should throw IllegalStateException
	    // throw new IllegalStateException("sequence not set");
	    return;
	}

	// already running: return quietly
	if (running == true) {
	    return;
	}

	// $$jb:11.03.99: If we have recorded, we need to re-set the
	// sequence before we play again.
	if (sequenceChanged == true) {
	    try {
		setSequence( sequence );
	    } catch(InvalidMidiDataException e) {
		// should never happen
	    }
	    sequenceChanged = false;
	}

	// mark the sequencer running
	running = true;

	if (!newSequenceStarted) {

	    nStartSequencer(id);
	    newSequenceStarted = true;

	    propagateCaches();

	} else {
	    // otherwise just resume.
	    nResumeSequencer(id);
	}

	runningInEngine = true;

	// this isn't a real callback!
	callbackSongStart();
	if(Printer.trace) Printer.trace("<< MixerSequencer: start() completed");
    }


    public synchronized void stop() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: stop()");

	if ( !isOpen() ) {
	    throw new IllegalStateException("Sequencer not open");
	}

	stopRecording();
	if (id == 0) {
	    // sequence not set; fail quietly
	    return;
	}

	// not running; just return
	if (running == false) {
	    return;
	}

	// stop playback
	implStop();

	// this isn't a real callback!
	callbackSongStop();

	if(Printer.trace) Printer.trace("<< MixerSequencer: stop() completed");
    }


    public boolean isRunning() {
	return running;
    }


    public void startRecording() {

	if ( !isOpen() ) {
	    throw new IllegalStateException("Sequencer not open");
	}

	// $$mp 2003-08-18: disable recording

	// 		recording = true;

	// 		divisionType         = sequence.getDivisionType();
	// 		resolution           = sequence.getResolution();
	// 		startTime            = System.currentTimeMillis();
	// 		startMillisecondTime = getMicrosecondPosition() / 1000;
	// 		startTick            = 0;
	// 		lastTempoChangeTime  = 0;
	// 		lastTempoChangeTick  = startTick;
	// 		recordTempoInMPQ     = (long) getTempoInMPQ();

	// 		start();
	// 		startMillisecondTime = getMicrosecondPosition() / 1000;

    }


    public void stopRecording() {

	if ( !isOpen() ) {
	    throw new IllegalStateException("Sequencer not open");
	}

	// $$mp 2003-08-18: disable recording
	// 		// if we were recording, we need to re-set the sequence next time we play
	// 		if( recording == true ) {
	// 			sequenceChanged = true;
	// 		}

	// 		recording = false;
    }


    public boolean isRecording() {
	return recording;
    }


    public void recordEnable(Track track, int channel) {

	// $$mp 2003-08-18: disable recording
	// 		if ( ! findTrack(track) ) {
	// 			throw new IllegalArgumentException("Track does not exist in the current sequence");
	// 		}

	// 		synchronized(recordingTracks) {

	// 			RecordingTrack rc = RecordingTrack.get(recordingTracks, track);

	// 			if (rc != null) {
	// 				rc.channel = channel;
	// 			} else {
	// 				recordingTracks.addElement(new RecordingTrack(track, channel));
	// 			}
	// 		}
    }


    public void recordDisable(Track track) {

	// $$mp 2003-08-18: disable recording
	// 		synchronized(recordingTracks) {
	// 			RecordingTrack rc = RecordingTrack.get(recordingTracks, track);
	// 			if (rc != null) {
	// 				recordingTracks.removeElement(rc);
	// 			}
	// 		}
    }


    // $$mp 2003-08-18: disable recording
    //	 private boolean findTrack(Track track) {

    // 		boolean found = false;

    // 		if (sequence != null) {
    // 			Track[] tracks = sequence.getTracks();
    // 			for (int i = 0; i < tracks.length; i++) {
    // 				if (track == tracks[i]) {
    // 					found = true;
    // 					break;
    // 				}
    // 			}
    // 		}
    // 		return found;
    //	 }


    public float getTempoInBPM() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getTempoInBPM() ");

	//$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
	if ((id == 0) || !newSequenceStarted) {
	    // if the sequence has not been started and a tempo
	    // value has been set, return that value.
	    if (tempoInBPMCache != -1) {
		return tempoInBPMCache;
	    }
	    else if (tempoInMPQCache != -1) {
		return 60000000 / tempoInMPQCache;
	    }
	    // if the sequence is not set, return 0.
	    if (id == 0) {
		return 0;
	    }
	}

	return (float)nGetTempoInBPM(id);
    }


    public void setTempoInBPM(float bpm) {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setTempoInBPM() ");

	if (bpm <= 0) {
	    // should throw IllegalArgumentException
	    bpm = 1.0f;
	}

	//$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
	if ((id == 0) || !newSequenceStarted) {
	    // if this sequence has not been started, this tempo value will be
	    // overwritten by the sequence's tempo when it does start.  so we
	    // cache the value.
	    tempoInBPMCache = bpm;
	    // if previously tempoInMPQCache was cached, reset it
	    tempoInMPQCache = -1;
	    // it will be set when the sequence is started
	    return;
	}

	// set the tempo in BPM
	nSetTempoInBPM(id, (int)bpm);

	// reset the tempoInBPMCache and tempoInMPQCache values so we won't use them again
	this.tempoInBPMCache = -1;
	this.tempoInMPQCache = -1;
    }


    public float getTempoInMPQ() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getTempoInMPQ() ");

	//$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
	if ((id == 0) || !newSequenceStarted) {
	    // if the sequence has not been started and a tempo
	    // value has been set, return that value.
	    if (tempoInMPQCache != -1) {
		return tempoInMPQCache;
	    }
	    else if (tempoInBPMCache != -1) {
		return 60000000 / tempoInBPMCache;
	    }
	    // if the sequence is not set, return 0.
	    if (id == 0) {
		return 0;
	    }
	}

	return (float)nGetTempoInMPQ(id);
    }


    public void setTempoInMPQ(float mpq) {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setTempoInMPQ() ");

	if (mpq <= 0) {
	    // should throw IllegalArgumentException
	    mpq = 1.0f;
	}
	//$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
	if ((id == 0) || !newSequenceStarted) {
	    // if this sequence has not been started, this tempo value will be
	    // overwritten by the sequence's tempo when it does start.  so we
	    // cache the value.
	    tempoInMPQCache = mpq;
	    // if previously tempoInBPMCache was cached, reset it
	    tempoInBPMCache = -1;
	    // it will be set when the sequence is started
	    return;
	}
	// set the tempo in MPQ
	nSetTempoInMPQ(id, (int)mpq);

	// reset the tempoInBPMCache and tempoInMPQCache values so we won't use them again
	tempoInBPMCache = -1;
	tempoInMPQCache = -1;
    }


    public void setTempoFactor(float factor) {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setTempoFactor() ");

	if (factor <= 0) {
	    // should throw IllegalArgumentException
	    return;
	}
	if (id == 0) {
	    //$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
	    this.tempoFactorCache = factor;
	    return;
	}

	nSetMasterTempo(id, factor);
	// don't need cache anymore
	this.tempoFactorCache = -1;
    }


    public float getTempoFactor() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getTempoFactor() ");

	if (id == 0) {
	    //$$fb 2002-07-19: fix for 4716740: default sequencer does not set the tempo factor
	    if (tempoFactorCache != -1) {
		return tempoFactorCache;
	    }
	    return 1.0f; // default
	}

	return nGetMasterTempo(id);
    }


    public long getTickLength() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getTickLength() ");

	if (getSequence() != null) {
	    return getSequence().getTickLength();
	}

	if (id == 0) {
	    return 0;
	}

	//$$fb 2002-10-30: fix for 4427890: Sequencer.getTickLength() and Sequence.getTickLength() report the wrong length
	// see below (getTickPosition) for reason
	return nGetSequenceTickLength(id)/64;
    }


    public synchronized long getTickPosition() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getTickPosition() ");

	if (tickCache != -1) {
	    return tickCache;
	}
	if (id == 0) {
	    return 0;
	}

	// $$jb: 11.09.99:  The engine is keeping track of ticks
	// at 64x the normal tick speed.  This is probably better
	// fixed in the engine code, but I'm patching it here for
	// the time being. (bug #4288671)

	return (nGetSequencerTickPosition(id)/64);
    }


    public synchronized void setTickPosition(long tick) {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setTickPosition() ");

	if (tick < 0) {
	    // should throw IllegalArgumentException
	    return;
	}

	if (id == 0) {
	    if (isOpen()) {
		this.tickCache = tick;
		this.microsecondCache = -1;
		return;
	    }
	    // should throw IllegalStateException
	    return;
	}

	// $$kk: 06.08.99: record the requested tick position if not currently running.
	if (running == false) {
	    this.tickCache = tick;
	} else {

	    // $$kk: 06.08.99: note that this will *always* call GM_ResumeSong
	    // at the chosen tick position, even if the song was paused!  i think
	    // maybe this should change.  in the meantime, though, i am just going
	    // to handle this in this class by not calling this method unless we
	    // are not paused.
	    // $$jb: 11.09.99: The engine is keeping track of ticks
	    // at 64x the normal tick speed.  (bug #4288671)
	    nSetSequencerTickPosition(id, tick*64);

	    // reset the tick position so we won't use it again
	    this.tickCache = -1;
	}
	this.microsecondCache = -1;
    }


    public long getMicrosecondLength() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getMicrosecondLength() ");

	if (getSequence() != null) {
	    return getSequence().getMicrosecondLength();
	}
	if (id == 0) {
	    return 0;
	}

	return nGetSequenceMicrosecondLength(id);
    }


    public long getMicrosecondPosition() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: getMicrosecondPosition() ");

	if (microsecondCache != -1) {
	    return microsecondCache;
	}

	if (id == 0) {
	    return 0;
	}

	return (nGetSequencerMicrosecondPosition(id));
    }


    public void setMicrosecondPosition(long microseconds) {

	if(Printer.trace) Printer.trace(">> MixerSequencer: setMicrosecondPosition() ");

	if (microseconds < 0) {
	    // should throw IllegalArgumentException
	    return;
	}

	if (id == 0) {
	    if (isOpen()) {
		this.microsecondCache = microseconds;
		this.tickCache = -1;
		return;
	    }
	    // should throw IllegalStateException
	    return;
	}


	if (running == false) {
	    this.microsecondCache = microseconds;
	} else {

	    // $$kk: 07.19.99: each time we call GM_SetSongMicrosecondPosition (called from
	    // nSetSequencerMicrosecondPosition), it calls PV_ComfigureMusic,
	    // which parses the midi file data and resets the tempo to what's specified there.
	    // i think that is the appropriate behaviour since tempo changes can occur at any
	    // time in a midi sequence; here we're just getting the initial one....
	    nSetSequencerMicrosecondPosition(id, microseconds);
	    this.microsecondCache = -1;
	}

	// $$kk: 06.08.99
	// reset the tick position so we won't use it again
	// note that GM_SetSongTickPosition will resume a song regardless of
	// whether it was originally paused, but GM_SetSongMicrosecondPosition
	// only resumes it if it was not playing before.  a lot of this ugly
	// code will go away if i change GM_SetSongTickPosition to handle the
	// paused case....
	this.tickCache = -1;
    }


    public void setMasterSyncMode(Sequencer.SyncMode sync) {
	// $$kk: 04.07.99: we do not support this now.
    }


    public Sequencer.SyncMode getMasterSyncMode() {
	return masterSyncMode;
    }


    public Sequencer.SyncMode[] getMasterSyncModes() {

	Sequencer.SyncMode[] returnedModes = new Sequencer.SyncMode[masterSyncModes.length];
	System.arraycopy(masterSyncModes, 0, returnedModes, 0, masterSyncModes.length);
	return returnedModes;
    }


    public void setSlaveSyncMode(Sequencer.SyncMode sync) {
	// $$kk: 04.07.99: we do not support this now.
    }


    public Sequencer.SyncMode getSlaveSyncMode() {
	return slaveSyncMode;
    }


    public Sequencer.SyncMode[] getSlaveSyncModes() {

	Sequencer.SyncMode[] returnedModes = new Sequencer.SyncMode[slaveSyncModes.length];
	System.arraycopy(slaveSyncModes, 0, returnedModes, 0, slaveSyncModes.length);
	return returnedModes;
    }

    protected int getTrackCount() {
	Sequence seq = getSequence();
	if (seq != null) {
	    // $$fb wish there was a nicer way to get the number of tracks...
	    return sequence.getTracks().length;
	}
	return 0;
    }



    public void setTrackMute(int track, boolean mute) {
	// fix for 4713900: default Sequencer allows to set Mute for invalid track
	if (id == 0 || track < 0 || track >= getTrackCount()) return;
	nSetTrackMute(id, track, mute);
    }


    public boolean getTrackMute(int track) {
	// fix for 4713900: default Sequencer allows to set Mute for invalid track
	if (id == 0 || track < 0 || track >= getTrackCount()) return false;
	return nGetTrackMute(id, track);
    }


    public void setTrackSolo(int track, boolean solo) {
	// fix for 4713900: default Sequencer allows to set Mute for invalid track
	if (id == 0 || track < 0 || track >= getTrackCount()) return;
	nSetTrackSolo(id, track, solo);
    }


    public boolean getTrackSolo(int track) {
	// fix for 4713900: default Sequencer allows to set Mute for invalid track
	if (id == 0 || track < 0 || track >= getTrackCount()) return false;
	return nGetTrackSolo(id, track);
    }


    public boolean addMetaEventListener(MetaEventListener listener) {

	synchronized( metaEventListeners ) {
	    if (! metaEventListeners.contains(listener)) {

		metaEventListeners.addElement(listener);
	    }
	    return true;
	}
    }


    public void removeMetaEventListener(MetaEventListener listener) {

	synchronized( metaEventListeners ) {
	    metaEventListeners.removeElement(listener);
	}
    }


    public int[] addControllerEventListener(ControllerEventListener listener, int[] controllers) {

	synchronized( controllerEventListeners ) {

	    // first find the listener.  if we have one, add the controllers
	    // if not, create a new element for it.
	    ControllerVectorElement cve = null;
	    boolean flag = false;
	    for( int i=0; i < controllerEventListeners.size(); i++ ) {

		cve = (ControllerVectorElement) controllerEventListeners.elementAt(i);

		if( cve.listener.equals( listener ) ) {
		    cve.addControllers( controllers );
		    flag = true;
		    break;
		}
	    }
	    if( !flag ) {

		cve = new ControllerVectorElement( listener, controllers );
		controllerEventListeners.addElement( cve );
	    }

	    // now make sure the engine knows to notify us on these controllers
	    // this won't get called if the sequencer hasn't been opened yet,
	    // but if we're adding a listener while the song is playing, it will.
	    if( id != 0 ) {
		for(int i=0; i<controllers.length; i++) {
		    nAddControllerEventCallback( id, controllers[i] );
		}
	    }

	    // and return all the controllers this listener is interested in
	    return cve.getControllers();
	}
    }


    public int[] removeControllerEventListener(ControllerEventListener listener, int[] controllers) {

	synchronized(controllerEventListeners) {

	    ControllerVectorElement cve = null;
	    boolean flag = false;
	    for( int i=0; i < controllerEventListeners.size(); i++ ) {

		cve = (ControllerVectorElement) controllerEventListeners.elementAt(i);

		if( cve.listener.equals( listener ) ) {
		    cve.removeControllers( controllers );
		    flag = true;
		    break;
		}
	    }
	    if( !flag ) {
		return new int[0];
	    }
	    if( controllers == null ) {
		controllerEventListeners.removeElement( cve );
		return new int[0];
	    }
	    return cve.getControllers();
	}
    }

    ////////////////// LOOPING (added in 1.5) ///////////////////////


    public void setLoopStartPoint(long tick) {
	if (((sequence != null) && (tick > sequence.getTickLength()))
	    || ((loopEnd != -1) && (loopStart > loopEnd))
	    || (tick < 0)) {
	    throw new IllegalArgumentException("invalid loop start point: "+tick);
	}
	loopStart = tick;
    }

    public long getLoopStartPoint() {
	return loopStart;
    }

    public void setLoopEndPoint(long tick) {
	if (((sequence != null) && (tick > sequence.getTickLength()))
	    || (loopStart > tick)
	    || (tick < -1)) {
	    throw new IllegalArgumentException("invalid loop end point: "+tick);
	}
	loopEnd = tick;
    }

    public long getLoopEndPoint() {
	return loopEnd;
    }

    public void setLoopCount(int count) {
	if (count != LOOP_CONTINUOUSLY
	    && count < 0) {
	    throw new IllegalArgumentException("illegal value for loop count: "+count);
	}
	loopCount = count;
    }

    public int getLoopCount() {
	return loopCount;
    }


    // RECEIVER METHODS


    /**
     * Return the time-stamp in microseconds
     * MixerSynth always returns -1.
     */
    protected long getTimeStamp() {
	return getMicrosecondPosition();
    }


    // OVERRIDES OF ABSTRACT MIDI DEVICE METHODS


    /**
     * this method opens the native resources *and* sets the state to open.
     * implOpen() does everything but change the state.  we need this because
     * MixerSequencer.setSequence() actually needs to close and reopen the
     * native resources without changing the Sequencer open/close state.
     */
    public synchronized void open() throws MidiUnavailableException {

	if(Printer.trace) Printer.trace(">> AbstractPlayer open(): " + this);

	if (isOpen()) {
	    if(Printer.trace) Printer.trace("<< AbstractPlayer open(): already open: " + this);
	    return;
	}

	try {
	    mixer.open(this);
	} catch (LineUnavailableException lue) {
	    throw new MidiUnavailableException(lue.getMessage());
	}
	implOpen();	// can throw MidiUnavailableException
	open = true;
	if(Printer.trace) Printer.trace("<< AbstractPlayer open(): open completed: " + this);
    }


    /**
     * this method closes the native resources *and* sets the state to closed.
     * implClose() does everything but change the state.  we need this because
     * MixerSequencer.setSequence() actually needs to close and reopen the
     * native resources without changing the Sequencer open/close state.
     */
    public synchronized void close() {
	if(Printer.trace) Printer.trace(">> AbstractPlayer close(): " + this);
	if (isOpen()) {
	    // close the player
	    open = false;
	    implClose();
	    mixer.close(this);
	}
	if(Printer.trace) Printer.trace("<< AbstractPlayer close() completed: " + this);
    }


    // OVERRIDES OF ABSTRACT PLAYER METHODS


    /**
     * return ourselves, since we are a Receiver
     */
    // $$mp 2003-08-18: disable recording
    //	 public synchronized Receiver getReceiver() throws MidiUnavailableException {

    // 		if( sequencerReceiver == null ) {
    // 			sequencerReceiver = new SequencerReceiver();
    // 		}
    // 		return sequencerReceiver;
    //	 }

    /**
     * wait for the callbackSongDone callback
     */
    protected synchronized void implClose() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: implClose() ");

	// close native sequencer */

	implStop();
	super.implClose();

	//programmedTick = 0;
	sequence = null;
	running = false;
	newSequenceStarted = false;

	while (runningInEngine) {
	    synchronized(this) {
		try {
		    wait();
		} catch (InterruptedException e) {
		}
	    }
	}

	if(Printer.trace) Printer.trace("<< MixerSequencer: implClose() completed");
    }



    // HELPER METHODS


    private void propagateCaches() {
	if (tempoFactorCache != -1) {
	    setTempoFactor(tempoFactorCache);
	}
	if (tickCache != -1) {
	    setTickPosition(tickCache);
	}
	if (microsecondCache != -1) {
	    setMicrosecondPosition(microsecondCache);
	}
	if (tempoInMPQCache != -1) {
	    setTempoInMPQ(tempoInMPQCache);
	}
	if (tempoInBPMCache != -1) {
	    setTempoInBPM(tempoInBPMCache);
	}
    }

    /*
     * we actually do very little here 'cause we
     * create the native stuff in setSequence.
     */
    protected void implOpen() throws MidiUnavailableException {
	doImplOpen();
	if (id == 0) {
	    try {
		Sequence oldSeq = sequence;
		if (midiData != null) {
		    setSequence(new ByteArrayInputStream(midiData));
		    sequence = oldSeq;
		}
		else if (sequence != null) {
		    setSequence(sequence);
		}
	    } catch (IOException ioe) {
		throw new MidiUnavailableException(ioe.getMessage());
	    } catch (InvalidMidiDataException imde) {
		throw new MidiUnavailableException(imde.getMessage());
	    }
	}
    }

    private void doImplOpen() throws MidiUnavailableException {
	// TODO: remove MidiUnavailableException from openInternalSynth,
	//       and then from this method, and maybe at other methods, too
	openInternalSynth();
    }


    protected void implStop() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: implStop()");

	nPauseSequencer(id);
	running = false;
	//recording = false;
	if(Printer.trace) Printer.trace("<< MixerSequencer: implStop() completed");
    }


    /**
     * Send midi player events.
     */
    protected void sendMetaEvents(MetaMessage message) {

	// $$kk: 04.22.99: i'm using the same method we use
	// for sampled audio channel events 'cause there's no relevant
	// typing....
	if (Printer.debug) Printer.debug("sending a meta event");
	eventDispatcher.sendAudioEvents(message, metaEventListeners);
    }

    /**
     * Send midi player events.
     */
    protected void sendControllerEvents(ShortMessage message,int controller) {

	// $$kk: 04.22.99: i'm using the same method we use
	// for sampled audio channel events 'cause there's no relevant
	// typing....

	if (Printer.debug) Printer.debug("sending a controller event");

	Vector sendToListeners = new Vector();
	for( int i=0; i<controllerEventListeners.size(); i++ ) {

	    ControllerVectorElement cve = (ControllerVectorElement) controllerEventListeners.elementAt(i);
	    for(int j=0; j<cve.controllers.length; j++) {
		if( cve.controllers[j] == controller ) {
		    sendToListeners.addElement( cve.listener );
		    break;
		}
	    }
	}

	eventDispatcher.sendAudioEvents(message, sendToListeners);
    }


    /**
     * Return the entire contents of the FileStream as a byte array.
     */
    protected byte[] getBytesFromFileStream(InputStream stream, MidiFileFormat format) throws IOException {

	// if null, throw an IOException
	if (stream == null) {
	    throw new IOException("Stream is null");
	}


	// load data from the stream into a byte array

	byte[] data;
	long length = format.getByteLength();

	// if we know the length, just read into an array of that size.
	if (length != MidiFileFormat.UNKNOWN_LENGTH) {

	    data = new byte[(int)length];
	    if (stream.read(data) != length) {
		throw new IOException("Read failure: expected " + length + " bytes");
	    }
	} else {

	    // otherwise, read until we hit the end of the stream
	    int MAX_READ_LIMIT = 2048;
	    ByteArrayOutputStream ba  = new ByteArrayOutputStream();
	    DataOutputStream      dos = new DataOutputStream(ba);

	    int readCount = MAX_READ_LIMIT;
	    byte tmp[]    = new byte[readCount];
	    int num       = 0;
	    int readByte  = 0;

	    while (true) {
		readByte = stream.read(tmp, 0, readCount); // can throw IOException
		if (readByte == -1)
		    if (num == 0) {
			throw new IOException("No data found in stream");
		    } else {
			break;
		    }

		dos.write(tmp, 0, readByte);
		num += readByte;
		Thread.currentThread().yield();
	    } // while

	    data = ba.toByteArray();
	}

	return data;
    }


    // CALLBACKS

    // called by the engine when the song ends.
    private synchronized void callbackSongEnd() {

	if(Printer.trace) Printer.trace(">> MixerSequencer: callbackSongEnd()");

	// this will set running to false
	implStop();

	// the engine no longer thinks this song is playing
	runningInEngine = false;

	// $$kk: 07.19.99: this will now get set in implStop()
	//paused = true;

	// this will knock us out of the wait() in implClose()
	synchronized(this) {
	    notifyAll();
	}


	// $$kk: 04.22.99: we should have something equivalent to this for midi!
	// setActive(false);

	// send events
	// $$kk: 04.22.99: need to make the tick position work here.
	// however, right now we have state management problems (sequencer may be closed)
	// and we don't (necessarily) actually have a real Sequence object....
	// $$jb: 06.04.99: no longer sending midi events
	// sendEvents(new MidiDeviceEvent(this, Sequencer.EOM, (long)0));

	MetaMessage message = new MetaMessage();
	try{
	    message.setMessage(47, new byte[0], 0);
	    if (Printer.debug) Printer.debug("new meta message: " + message);
	} catch(InvalidMidiDataException e1) {
	    if (Printer.debug) Printer.debug("invalid midi data: " + e1);
	}

	sendMetaEvents(message);

	if(Printer.trace) Printer.trace("<< MixerSequencer: callbackSongEnd completed()");
    }


    // called by us when we start or resume the sequence.
    // there's no real callback for this
    private void callbackSongStart() {
	if(Printer.trace) Printer.trace("MixerSequencer: callbackSongStart()");
    }


    // called by us when we pause the sequence.
    // there's no real callback for this
    private void callbackSongStop() {
	if(Printer.trace) Printer.trace("MixerSequencer: callbackSongStop()");
    }



    private void callbackMetaEvent( int type, int msgLength, int channel, byte data[]) {

	if (Printer.trace) Printer.trace(">> MixerSequencer: callbackMetaEvent()");

	if (Printer.debug) Printer.debug("    type = " + type +
					 ", msgLength = " + msgLength + ", channel = " + channel );

	MetaMessage newMessage = new MetaMessage();

	try {
	    newMessage.setMessage(type, data, msgLength);
	    if (Printer.debug) Printer.debug("new meta message: " + newMessage);
	    sendMetaEvents(newMessage);

	} catch (InvalidMidiDataException e1) {
	    if (Printer.debug) Printer.debug("invalid midi data: " + e1);
	}


	if(Printer.trace) Printer.trace("<< MixerSequencer: callbackMetaEvent() completed");
    }

    private void callbackControllerEvent( int channel, int track, int controller, int value ) {

	if(Printer.trace) Printer.trace(">> MixerSequencer: callbackControllerEvent()");

	ShortMessage newMessage = new ShortMessage();
	try {
	    // 176 = 0xB0 -- the status byte for controller and mode
	    // events is 0xBc, where c is the channel the event occurs on
	    newMessage.setMessage( (176+channel), controller, value);
	    sendControllerEvents(newMessage, controller);

	} catch(InvalidMidiDataException e1) {
	    if(Printer.debug)Printer.debug("invalid midi data: " + e1);
	}

	if(Printer.trace) Printer.trace("<< MixerSequencer: callbackControllerEvent() completed");
    }


    // INNER CLASSES

    // $$mp 2003-08-18: disable recording
    //	 class SequencerReceiver extends PlayerReceiver {


    // 		public void send(MidiMessage message, long timeStamp) {

    // 			// $$jb: 11.03.99: do we really want to do this here?
    // 			super.send(message, timeStamp);

    // 			// RECORD!!
    // 			if (recording) {

    // 				long tickPos;
    // 				Vector v = null;

    // 				// convert timeStamp to ticks

    // 				if( timeStamp < 0 ) {
    // 					if( running ) {
    // 						tickPos = calculateTickPosition( getMicrosecondPosition()/1000 );
    // 					} else {
    // 						tickPos = calculateTickPosition( startMillisecondTime +
    //     (System.currentTimeMillis() - startTime) );
    // 					}
    // 				} else {
    // 					tickPos = calculateTickPosition( timeStamp );
    // 				}


    // 				MidiEvent me = new MidiEvent(message, tickPos);

    // 				if( message instanceof ShortMessage ) {
    // 					v = RecordingTrack.get( recordingTracks, ((ShortMessage)message).getChannel() );
    // 				} else {
    // 					// $$jb: where to record meta, sysex events?

    // 					v = RecordingTrack.get( recordingTracks, 0 );
    // 				}

    // 				for(int i=0; i<v.size(); i++) {
    // 					((Track)v.elementAt(i)).add( me );
    // 				}
    // 			}
    // 		}
    //	 }


    private static class MixerSequencerInfo extends MidiDevice.Info {

	private static final String name = "Java Sound Sequencer";
	private static final String vendor = "Sun Microsystems";
	private static final String description = "Software sequencer / synthesizer module";
	private static final String version = "Version 1.0";

	private MixerSequencerInfo() {
	    //super(name, vendor, description, version, MixerSequencer.class);
	    super(name, vendor, description, version);
	}
    } // class Info

    private class ControllerVectorElement {

	// $$jb: using an array for controllers b/c its
	//       easier to deal with than turning all the
	//       ints into objects to use a Vector
	int []  controllers;
	ControllerEventListener listener;

	private ControllerVectorElement( ControllerEventListener listener, int[] controllers ) {

	    this.listener = listener;
	    this.controllers = controllers;
	}

	private void addControllers( int[] c ) {

	    if( c==null) {
		return;
	    }
	    int temp[] = new int[ controllers.length + c.length ];
	    int elements;

	    // first add what we have
	    for( int i=0; i<controllers.length; i++) {
		temp[i] = controllers[i];
	    }
	    elements = controllers.length;
	    // now add the new controllers only if we don't already have them
	    for( int i=0; i<c.length; i++ ) {
		boolean flag = false;

		for( int j=0; j<controllers.length; j++ ) {
		    if( c[i] == controllers[j] ) {
			flag = true;
			break;
		    }
		}
		if( !flag ) {
		    temp[elements++] = c[i];
		}
	    }
	    // now keep only the elements we need
	    int newc[] = new int[ elements ];
	    for( int i=0; i<elements; i++ ){
		newc[i] = temp[i];
	    }
	    controllers = newc;

	}

	private void removeControllers( int[] c ) {

	    if( c==null ) {
		controllers = new int[0];
	    } else {
		int temp[] = new int[ controllers.length ];
		int elements = 0;


		for(int i=0; i<controllers.length; i++){
		    boolean flag = false;
		    for(int j=0; j<c.length; j++) {
			if( controllers[i] == c[j] ) {
			    flag = true;
			    break;
			}
		    }
		    if( !flag ){
			temp[elements++] = controllers[i];
		    }
		}
		// now keep only the elements remaining
		int newc[] = new int[ elements ];
		for( int i=0; i<elements; i++ ) {
		    newc[i] = temp[i];
		}
		controllers = newc;

	    }
	}

	private int[] getControllers() {

	    // return a copy of our array of controllers,
	    // so others can't mess with it

	    int c[] = new int[controllers.length];

	    for(int i=0; i<controllers.length; i++){
		c[i] = controllers[i];
	    }
	    return c;
	}

    } // class ControllerVectorElement


    static class RecordingTrack {

	Track track;
	int channel;

	RecordingTrack(Track track, int channel) {
	    this.track = track;
	    this.channel = channel;
	}

	static RecordingTrack get(Vector vector, Track track) {

	    synchronized(vector) {

		int size = vector.size();
		RecordingTrack current;

		for (int i = 0; i < size; i++ ) {

		    current = (RecordingTrack)vector.elementAt(i);
		    if (current.track == track) {
			return current;
		    }
		}
	    }

	    return null;
	}
	static Vector get(Vector vector, int channel) {

	    Vector newV = new Vector();

	    synchronized(vector) {

		int size = vector.size();
		RecordingTrack current;

		for (int i = 0; i < size; i++ ) {

		    current = (RecordingTrack)vector.elementAt(i);
		    if( (current.channel == channel) || (current.channel == -1) ) {
			newV.addElement( current.track );
		    }
		}

	    }
	    return newV;
	}


    }


    // NATIVE METHODS

    // GM_LoadSong
    protected native long nOpenMidiSequencer(byte[] midiData, int length);

    // GM_LoadSong
    protected native long nOpenRmfSequencer(byte[] rmfData, int length);

    // GM_BeginSong
    protected native void nStartSequencer(long id);

    // GM_PauseSong
    protected native void nPauseSequencer(long id);

    // GM_ResumeSong
    protected native void nResumeSequencer(long id);


    // GM_AddControllerEventCallback
    protected native void nAddControllerEventCallback(long id, int controller);


    // GM_SongTicks
    protected native long nGetSequencerTickPosition(long id);

    // GM_SetSongTickPosition
    protected native long nSetSequencerTickPosition(long id, long tick);

    // GM_SongMicroseconds
    protected native long nGetSequencerMicrosecondPosition(long id);

    // GM_SetSongMicrosecondPosition
    protected native long nSetSequencerMicrosecondPosition(long id, long microseconds);

    // GM_GetSongTempoInBeatsPerMinute
    protected native int nGetTempoInBPM(long id);

    // GM_SetSongTempoInBeatsPerMinute
    protected native int nSetTempoInBPM(long id, int bpm);

    // GM_GetSongTempo
    protected native int nGetTempoInMPQ(long id);

    // GM_SetSongTempo
    protected native int nSetTempoInMPQ(long id, int mpq);

    // GM_GetMasterSongTempo
    protected native float nGetMasterTempo(long id);

    // GM_SetMasterSongTempo
    protected native float nSetMasterTempo(long id, float factor);

    // GM_MuteTrack or GM_UnmuteTrack
    protected native void nSetTrackMute(long id, int track, boolean mute);

    // GM_GetTrackMuteStatus
    protected native boolean nGetTrackMute(long id, int track);

    // GM_SoloTrack or GM_UnsoloTrack
    protected native void nSetTrackSolo(long id, int track, boolean mute);

    // GM_GetTrackSoloStatus
    protected native boolean nGetTrackSolo(long id, int track);


    // WE'RE CHEATING WITH THESE:

    // GM_GetSongTickLength
    protected native long nGetSequenceTickLength(long id);

    // GM_GetSongMicrosecondLength
    protected native long nGetSequenceMicrosecondLength(long id);
}
