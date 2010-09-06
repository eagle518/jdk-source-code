/*
 * @(#)HeadspaceMixer.java	1.68 04/04/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import java.util.Vector;

import javax.sound.sampled.AudioFormat;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.Control;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.EnumControl;
import javax.sound.sampled.Line;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.Mixer;
import javax.sound.sampled.ReverbType;
import javax.sound.sampled.SourceDataLine;
import javax.sound.midi.MidiDevice;
import javax.sound.midi.MidiUnavailableException;


/**
 * Represents the Headspace mixer.  This mixer supports the
 * Java Sound Mixer interface, and a set of mixer-specific
 * configuration methods.
 *
 * @version 1.68, 04/04/02
 * @author Kara Kytle
 */
class HeadspaceMixer extends AbstractMixer {

    static {
	// initialize platform-specific values, and load the native library
	Platform.initialize();
    }

    // cache for "device exists"
    // -1: never checked
    // 0: did not exist
    // 1: did exist
    private static int oldDeviceExists = -1;

    // INTERPOLATION DEFINES
    // (note that these match the headspace TerpMode values.)

    /**
     * Represents drop-sample interpolation.
     */
    public static final int INTERPOLATION_DROP_SAMPLE = 0;


    /**
     * Represents two-point interpolation.
     */
    public static final int INTERPOLATION_2_POINT = 1;


    /**
     * Represents linear interpolation.
     */
    public static final int INTERPOLATION_LINEAR = 2;


    // FINAL PROPERTIES

    /**
     * Total voices (midi + sampled)
     * $$kk: 03.06.98: should maybe get this value from the engine via a native method
     */
    private static int TOTAL_VOICES;

    /**
     * This is the default (and for now unchangeable) buffer size in SAMPLE FRAMES
     * for the mixer.  it's the (slice size) * (number of slices).
     */
    private static int DEFAULT_BUFFER_SIZE = AudioSystem.NOT_SPECIFIED;

    /**
     * This is the maximum length of the data we can handle for a Clip, in sample frames
     */
    static final int MAX_SAMPLES = 1048576;

    /**
     * max bytes per frame
     */
    static final int MAX_FRAME_SIZE = 4;


    /**
     * 8-bit encoding for output
     */
    private static final AudioFormat.Encoding encoding8 = (Platform.isSigned8() ? AudioFormat.Encoding.PCM_SIGNED : AudioFormat.Encoding.PCM_UNSIGNED);


    /**
     * Supported source line (source data line and clip) formats
     */
    private static final AudioFormat[] sourceLineFormats = {
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 8,  1, true, false)),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 8,  1, false, false)),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 8,  2, true, false)),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 8,  2, false, false)),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 16, 1, true, !Platform.isBigEndian())),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 16, 1, true, Platform.isBigEndian())),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 16, 2, true, !Platform.isBigEndian())),
	(new AudioFormat((float)AudioSystem.NOT_SPECIFIED, 16, 2, true, Platform.isBigEndian()))
    };


    private DataLine.Info sourceDataLineInfo =
	new DataLine.Info(SourceDataLine.class, sourceLineFormats, 0,
			  AudioSystem.NOT_SPECIFIED);

    // the engine supports clips with up to 1M *samples* (not bytes) of data.
    // our buffer size is measured in bytes.  i am using a max buffer size of
    // ((MAX_SAMPLES == 1048576) * 2 channels * 2 bytes) because this is the
    // largest buffer size we can ever support.  unfortunately, we can generate
    // IllegalArgumentExceptions if the buffer has too many *samples* but not
    // too many *bytes*!!
    // (when we update code in the engine, we will get away from this restriction.)
    private final DataLine.Info clipInfo = new DataLine.Info(Clip.class,
							     sourceLineFormats,
							     0,
							     (MAX_SAMPLES * MAX_FRAME_SIZE));

    // DEFAULT STATE

    // $$fb 2002-07-26 apparently, the engine ALWAYS operated at CD quality. set it directly...
    static final AudioFormat defaultFormat = new AudioFormat(44100.0f, 16, 2, true, Platform.isBigEndian());

    private static final int defaultMixLevel = 6;

    // $$kk: 03.11.99: check this default
    private static final int defaultInterpolationMode = INTERPOLATION_LINEAR;


    // CURRENT STATE

    // number of voices allocated to sampled audio streams
    private int sampledVoices;

    // number of voices allocated to MIDI voices
    private int midiVoices;

    // mix level
    private int mixLevel;

    // interpolation mode
    private int terpMode;

    // DEFAULT MIXER INSTANCE (right now we only support one instance)

    private static final HeadspaceMixer mixerInstance = new HeadspaceMixer();

    /**
     * Private constructor.
     */
    private HeadspaceMixer() {

	// $$kk: 05.31.99: we unfortunately cannot fill the mixer and
	// control values in until after calling the superclass constructor....
	super(new MixerInfo(), // Mixer.Info
	      new Control[1],  // controls
	      null,            // SourceLine infos
	      null);           // TargetLine infos

	if (Printer.trace) Printer.trace("HeadspaceMixer: constructor");
	init();
	if (Printer.trace) Printer.trace("HeadspaceMixer: constructor completed");
    }


    void init() {
	//$$fb check if there is an audio device at all
	// re-using nSetMixerFormat for this task
	// in order to not add a new native method
    	boolean devExists = nSetMixerFormat(0, 0, 0);

	if (((oldDeviceExists == 0) && !devExists)
	    || ((oldDeviceExists == 1) && devExists)) {
	    // if device existence has not changed, don't setup
	    return;
	}
	if (!devExists) {
	    controls = new Control[0];
	    sourceLineInfo = new Line.Info[0];
	    targetLineInfo = new DataLine.Info[0];
	    oldDeviceExists = 0;
	    if (Printer.debug) Printer.debug("no headspace mixer device installed!");
	} else {
	    targetLineInfo = new DataLine.Info[0];

	    TOTAL_VOICES = nGetTotalVoices();

	    // $$kk: 06.05.99: this won't work 'cause we haven't initialized
	    // the mixer yet.
	    DEFAULT_BUFFER_SIZE = nGetDefaultBufferSize();
	    if (DEFAULT_BUFFER_SIZE == 0) {
		DEFAULT_BUFFER_SIZE = AudioSystem.NOT_SPECIFIED;
	    }

	    // set up the controls
	    controls = new Control[1];
	    controls[0] = new MixerReverbControl();

	    // set up the mixer source line info
	    sourceLineInfo = new Line.Info[2];
	    sourceLineInfo[0] = sourceDataLineInfo;
	    sourceLineInfo[1] = clipInfo;

	    targetLineInfo = new DataLine.Info[0];

	    sampledVoices = TOTAL_VOICES / 2;
	    midiVoices = TOTAL_VOICES / 2;
	    mixLevel = defaultMixLevel;
	    terpMode = defaultInterpolationMode;

	    oldDeviceExists = 1;
	    if (Printer.debug) Printer.debug("headspace mixer device installed!");
	}
    }


    // ABSTRACT MIXER: ABSTRACT METHOD IMPLEMENTATIONS

    public Line getLine(Line.Info info) throws LineUnavailableException{

	Line.Info fullInfo = getLineInfo(info);

	if (fullInfo == null) {
	    throw new IllegalArgumentException("Line unsupported: " + info);
	}

	if (fullInfo instanceof DataLine.Info) {

	    DataLine.Info dataLineInfo = (DataLine.Info)fullInfo;

	    AudioFormat lineFormat;
	    int lineBufferSize = AudioSystem.NOT_SPECIFIED;

	    // if a format is specified by the info class passed in, use it.
	    // otherwise use a format from fullInfo.

	    AudioFormat[] supportedFormats = null;

	    if (info instanceof DataLine.Info) {
		supportedFormats = ((DataLine.Info)info).getFormats();
		lineBufferSize = ((DataLine.Info)info).getMaxBufferSize();
	    }

	    if ((supportedFormats == null) || (supportedFormats.length == 0)) {
		// use the default format
		lineFormat = defaultFormat;
	    } else {
		// use the last format specified in the line.info object passed
		// in by the app
		lineFormat = supportedFormats[supportedFormats.length-1];

		// if a rate is not specified, match the mixer sample rate
		boolean isSigned = lineFormat.getEncoding().equals(AudioFormat.Encoding.PCM_SIGNED);
		boolean isUnsigned = lineFormat.getEncoding().equals(AudioFormat.Encoding.PCM_UNSIGNED);
		if ((isSigned || isUnsigned)
		    && (lineFormat.getSampleRate() == (float)AudioSystem.NOT_SPECIFIED)) {

		    lineFormat = new AudioFormat(defaultFormat.getSampleRate(),
						 lineFormat.getSampleSizeInBits(),
						 lineFormat.getChannels(),
						 isSigned,
						 lineFormat.isBigEndian());
		}
	    }


	    if (lineBufferSize <= AudioSystem.NOT_SPECIFIED) {
		lineBufferSize = dataLineInfo.getMaxBufferSize();
	    }

	    if (dataLineInfo.getLineClass().isAssignableFrom(MixerSourceLine.class)) {
		return new MixerSourceLine(sourceDataLineInfo, this,
					   lineFormat, lineBufferSize);
	    }

	    if (dataLineInfo.getLineClass().isAssignableFrom(MixerClip.class)) {
		return new MixerClip(clipInfo, this, lineFormat,
				     lineBufferSize);
	    }
	}

	throw new SecurityException("Access to line not granted: " + info);
    }

    public int getMaxLines(Line.Info info) {

	Line.Info fullInfo = getLineInfo(info);

	// if it's not a supported line, the number is automatically 0
	if (fullInfo == null) {
	    return 0;
	}

	// this covers both source data lines and clips
	if ( (fullInfo instanceof DataLine.Info) && isSourceLine(fullInfo) ) {
	    return sampledVoices;
	}

	// this is our one target line
	//$$fb which doesn't exist...
	if ( (fullInfo instanceof DataLine.Info) && isTargetLine(fullInfo) ) {
	    return 0;
	}

	// this shouldn't happen, but the answer's still 0....
	return 0;
    }

    // $$fb 2001-10-09: make open/start/stop/close synchronized
    // part of fix for bug #4517739: Using JMF to playback sound clips blocks virtual machine
    protected synchronized void implOpen() throws LineUnavailableException {

	if (Printer.trace) Printer.trace(">> HeadspaceMixer: implOpen");

	if (oldDeviceExists == 0) {
	    // if no device is accessible, do not try to open the engine
	    throw new LineUnavailableException("no audio device available for Java Sound Audio Engine");
	}

	AudioFormat format = defaultFormat;

	// initialize the mixer: InitGeneralSound
	nOpenMixer(format.getSampleSizeInBits(), format.getChannels(),
		   (int)format.getSampleRate(),  terpMode, midiVoices,
		   sampledVoices, mixLevel);
	try {
	    // this reconnects to hardware and starts the engine thread.
	    // $$kk: 06.08.99: we will want to reconsider how to handle this.
	    nResume();
	} catch (LineUnavailableException e) {
	    // $$fb 2002-07-18: part of fix for 4716412:
	    //      Memory leak when Clip.open(stream) throws LineUnavailableException
	    implClose();
	    throw e;
	}
	if (Printer.trace) Printer.trace("<< HeadspaceMixer: implOpen succeeded");
    }

    // $$fb 2001-10-09: make open/start/stop/close synchronized
    // part of fix for bug #4517739: Using JMF to playback sound clips blocks virtual machine
    protected synchronized void implClose() {

	if (Printer.trace) Printer.trace(">> HeadspaceMixer: implClose. Thread="+Thread.currentThread().getName());

	// this disconnects from hardware and stops the engine thread.
	// $$kk: 06.08.99: we will want to reconsider how to handle this.
	nPause();

	// shut down the mixer: FinisGeneralSound
	nCloseMixer();

	if (Printer.trace) Printer.trace("<< HeadspaceMixer: implClose succeeded. Thread="+Thread.currentThread());
    }


    /**
     * Start the mixer
     */
    // $$fb 2001-10-09: make open/start/stop/close synchronized
    // part of fix for bug #4517739: Using JMF to playback sound clips blocks virtual machine
    protected synchronized void implStart() {
	if (Printer.trace) Printer.trace(">> HeadspaceMixer: implStart. Thread="+Thread.currentThread());
	if (Printer.trace) Printer.trace("<< HeadspaceMixer: implStart succeeded. Thread="+Thread.currentThread());
    }


    /**
     * Stop the mixer
     */
    // $$fb 2001-10-09: make open/start/stop/close synchronized
    // part of fix for bug #4517739: Using JMF to playback sound clips blocks virtual machine
    protected synchronized void implStop() {
	if (Printer.trace) Printer.trace(">> HeadspaceMixer: implStop");
	if (Printer.trace) Printer.trace("<< HeadspaceMixer: implStop succeeded");
    }



    // PUBLIC CONFIGURATION METHODS

    /**
     * Obtains the total number of voices available on the system.
     * The voices are allocated between MIDI and sampled voices.
     * @return total number of voices
     */
    public int getSystemVoices() {
	return TOTAL_VOICES;
    }


    /**
     * Obtains the number of voices dedicated to the MIDI synthesizer.
     * @return number of MIDI voices
     */
    public int getMidiVoices() {
	return midiVoices;
    }


    /**
     * Obtains the number of voices dedicated to sampled audio streams and
     * clips.
     * @return number of sampled voices
     */
    public int getSampledVoices() {
	return sampledVoices;
    }


    /**
     * Allocates the system voices between MIDI and sampled voices.
     * @param midi desired number of MIDI voices
     * @param sampled desired number of sampled audio voices
     */
    public boolean allocateVoices(int midi, int sampled) {

	// $$kk: 03.11.99: need error case here!
	if (! (nAllocateVoices(midi, sampled)) ) {
	    return false;
	}

	midiVoices = midi;
	sampledVoices = sampled;
	return true;
    }


    /**
     * Sets the mix level for the system.
     * @param mixLevel desired mix level
     */
    public boolean setMixLevel(int mixLevel) {

	// $$kk: 03.11.99: need error case here!

	if (! (nSetMixLevel(mixLevel)) ) {
	    return false;
	}

	this.mixLevel = mixLevel;
	return true;
    }


    /**
     * Obtains the mix level for the system.
     * @return current mix level
     */
    public int getMixLevel() {
	return mixLevel;
    }


    /**
     * Sets the interpolation mode for the system.
     * @param mode desired interpolation mode
     * @see #INTERPOLATION_DROP_SAMPLE
     * @see #INTERPOLATION_2_POINT
     * @see #INTERPOLATION_LINEAR
     */
    public boolean setInterpolationMode(int terpMode) {

	if (! (nSetInterpolation(terpMode)) ) {
	    return false;
	}

	this.terpMode = terpMode;
	return true;
    }


    /**
     * Obtains the interpolation mode for the system.
     * @return current interpolation mode
     */
    public int getInterpolationMode() {
	return terpMode;
    }


    /**
     * Obtains the current CPU load imposed by the audio system on the
     * machine as a fraction between 0 and 1.
     * @return current load
     */
    public float getCpuLoad() {
	return nGetCpuLoad();
    }



    // METHODS FOR INTERNAL IMPLEMENTATION USE


    /**
     * Called by the midi implementation!
     * This lets us handle MIDI devices on the mixer.
     * Create a special MidiLine instance that holds the MidiDevice
     * and use it to open it like a regular audio line.
     */
    synchronized void open(MidiDevice mididevice) throws LineUnavailableException {
	// don't add this mididevice if it's already open
	if (getMidiLine(mididevice) == null) {
	    MidiLine line = new MidiLine(this, mididevice);
	    line.open();
	    line.start();
	}
    }

    /**
     * Called by the midi implementation!
     * This lets us handle MIDI devices on the mixer.
     */
    synchronized void close(MidiDevice mididevice) {
	if (Printer.trace) Printer.trace("> HeadspaceMixer.close(MidiDevice)");
	MidiLine line = getMidiLine(mididevice);
	if (line != null) {
	    line.stop();
	    line.close();
	}
	if (Printer.trace) Printer.trace("< HeadspaceMixer.close(MidiDevice)");
    }


    private MidiLine getMidiLine(MidiDevice dev) {
	synchronized (sourceLines) {
	    for (int i=0; i<sourceLines.size(); i++) {
		if ((sourceLines.elementAt(i) instanceof MidiLine)
		  && (((MidiLine) sourceLines.elementAt(i)).getMidiDevice() == dev)) {
		    return (MidiLine) sourceLines.elementAt(i);
		}
	    }
	}
	return null;
    }

    /**
     * overriden from AbstractMixer to include MidiLines in it
     */
    boolean isSourceLine(Line.Info info) {
    	return (info instanceof MidiLineInfo) || super.isSourceLine(info);
    }


    /**
     * Returns the static mixer instance.  i want to get rid of this!!
     */
    static HeadspaceMixer getMixerInstance() {
	return mixerInstance;
    }

    /**
     * Returns if there is a hardware audio device present
     * for use with the Java Sound Audio engine
     */
    boolean audioDeviceExists() {
	init();
	return oldDeviceExists == 1;
    }

    AudioFormat getDefaultFormat() {
    	return defaultFormat;
    }


    // INNER CLASSES

    /**
     * Private Mixer.Info subclass.
     */
    private static class MixerInfo extends Mixer.Info {

	/**
	 * Mixer name.
	 */
	private static final String name = "Java Sound Audio Engine";

	/**
	 * Mixer vendor.
	 */
	private static final String vendor = "Sun Microsystems";

	/**
	 * Mixer description.
	 */
	private static final String description	= "Software mixer and synthesizer";

	/**
	 * Mixer version.
	 */
	private static final String version = "1.0";

	/**
	 * Constructs a new device info object.
	 * @param name name of the device
	 */
	private MixerInfo() {
	    super(name, vendor, description, version);
	}

    } // class MixerInfo


    private static class MixerReverbControl extends EnumControl {

	private static final MixerReverbType NO_REVERB =
	    new MixerReverbType("No Reverb", 0, 0.0f, 0, 0.0f, 0, 1);
	private static final MixerReverbType CLOSET =
	    new MixerReverbType("Closet", 600, -7.0f, 2500, -10.0f, 150000, 2);
	private static final MixerReverbType GARAGE =
	    new MixerReverbType("Garage", 3900, -4.0f, 14700, -6.0f, 900000, 3);
	private static final MixerReverbType ACOUSTIC_LAB =
	    new MixerReverbType("Acoustic Lab", 2000, -2.0f, 8000, -3.0f,
				280000, 4);
	private static final MixerReverbType CAVERN =
	    new MixerReverbType("Cavern", 10300, -1.4f, 41300, -2.0f,
				2250000, 5);
	private static final MixerReverbType DUNGEON =
	    new MixerReverbType("Dungeon", 2600, -0.7f, 10300, -1.0f,
				1600000, 6);

	/**
	 * Supported reverb types
	 */
	private static final ReverbType[] types = {
	    NO_REVERB, CLOSET, GARAGE, ACOUSTIC_LAB, CAVERN, DUNGEON };

	/**
	 * Default reverb type
	 */
	private static final ReverbType defaultReverb = ACOUSTIC_LAB;


	// STATE VARIABLES

	/**
	 * Set the engine reverb to the default value.  If this
	 * fails, set the reverb type to ReverbControl.NO_REVERB.
	 */
	private MixerReverbControl() {
	    super(EnumControl.Type.REVERB, types, defaultReverb);
	}


	public void setValue(Object value) {

	    if (Printer.trace)
		Printer.trace("HeadspaceMixer: MixerReverbControl: setValue: " + value.toString());

	    if (!(value instanceof ReverbType)) {
		throw new IllegalArgumentException("Value must be of type ReverbType");
	    }

	    ReverbType reverbType = (ReverbType)value;
	    int reverbMode;

	    if (reverbType instanceof MixerReverbType) {
		reverbMode = ((MixerReverbType)reverbType).getReverbMode();
	    } else if (value == NO_REVERB) {
		reverbMode = 1;	// pass-through
	    } else {
		throw new IllegalArgumentException("Unknown reverb value: " + value);
	    }

	    int reverbSet = nSetReverb(reverbMode);

	    if (reverbSet == reverbMode) {

		if (Printer.trace)
		    Printer.trace("HeadspaceMixer: setReverb succeeded: " +
				  value.toString());
		super.setValue(value);

	    } else {

		if (Printer.err)
		    Printer.err("HeadspaceMixer: MixerReverbControl: " +
				"setValue failed: value: " + value.toString() +
				" set mode: " + reverbMode + " returned mode: " +
				reverbSet);
		if (reverbSet == 1) {
		    /* pass-through */
		} else {

		    for (int i = 0; i < types.length; i++) {

			if (types[i] instanceof MixerReverbType) {
			    if ( ((MixerReverbType)types[i]).getReverbMode() == reverbSet ) {
				super.setValue(types[i]);
			    }
			}
		    }
		}
	    }
	}


	/**
	 * Set the engine reverb to the default value.  If this
	 * fails, set the reverb type to NO_REVERB.
	 */
	private void initializeReverb() {
	    try {
		setValue(defaultReverb);
		if (Printer.trace)
		    Printer.trace("Initialized reverb to: " + getValue());
	    } catch (IllegalArgumentException e) {
		setValue(NO_REVERB);
		if (Printer.err)
		    Printer.err("Failed to initialize reverb to: " +
				defaultReverb + "; set reverb to: " + getValue());
	    }
	}


	private static class MixerReverbType extends ReverbType {

	    // ReverbMode value used in the Headspace engine
	    private final int reverbMode;

	    private MixerReverbType(String name,
				    int earlyReflectionDelay,
				    float earlyReflectionIntensity,
				    int lateReflectionDelay,
				    float lateReflectionIntensity,
				    int decayTime, int reverbMode) {

		super(name, earlyReflectionDelay,
		      earlyReflectionIntensity, lateReflectionDelay,
		      lateReflectionIntensity, decayTime);
		this.reverbMode = reverbMode;
	    }

	    private int getReverbMode() {
		return reverbMode;
	    }
	}

    } // class MixerReverbControl


    private static class MidiLineInfo extends DataLine.Info {
    	private MidiLineInfo() {
	    super(DataLine.class,
	          HeadspaceMixer.defaultFormat,
	          HeadspaceMixer.DEFAULT_BUFFER_SIZE);
	}
    }


    /**
     * Private DataLine for MidiDevices
     */
    private static class MidiLine extends AbstractDataLine {

    	private MidiDevice device;
    	private boolean closed = true;
    	private boolean started = false;

    	private MidiLine(AbstractMixer mixer, MidiDevice device) {
    	    super(new MidiLineInfo(),
    	          mixer,  // AbstractMixer
    	          null,   // Control[]
    	          HeadspaceMixer.defaultFormat,
    	          HeadspaceMixer.DEFAULT_BUFFER_SIZE);
	    this.device = device;
	}

    	private MidiDevice getMidiDevice() {
	    return device;
    	}

    	// implementation of abstract methods in AbstractDataLine

	// nothing to do for open - the midi device is already open!
	protected void implOpen(AudioFormat format, int bufferSize) throws LineUnavailableException {
	    closed = false;
	}

	protected void implClose() {
	    if (!closed) {
	    	// prevent recursive calls
	    	closed = true;
	    	started = false;
		device.close();
	    }
	}

	protected void implStart() {
	    if (!started) {
		// prevent recursive calls
		started = true;
		// don't stop the sequencer!
		// the line is started when the sequencer is opened!
	    }
	}

	protected void implStop() {
	    if (started) {
		// prevent recursive calls
		started = false;
		// don't start the sequencer!
		// the line is started when the sequencer is opened!
	    }
	}

    }


    // NATIVE METHODS

    // GM_InitGeneralSound and GM_SetAudioTask
    private native void nOpenMixer(int sampleSizeInBits, int channels,
				   int sampleRate, int terpMode,
				   int midiVoices, int sampleVoices,
				   int mixLevel) throws LineUnavailableException;

    // GM_SetAudioTask(NULL), GM_StopHardwareSoundManager,
    // and GM_FinisGeneralSound
    private native void nCloseMixer();

    // GM_PauseGeneralSound
    private native void nPause();

    // GM_ResumeGeneralSound
    // $$kk 06.11.99: we are using this for "start" which shouldn't throw this exception.
    // however, the truth is that this method opens the device and can fail!  the native
    // code can throw the exception; this will screw the program.
    private native void nResume() throws LineUnavailableException;

    // not implemented
    private native void nDrain();
    private native void nFlush();
    private native long nGetPosition();
    private native float nGetLevel();


    // GM_ChangeAudioModes
    private native boolean nSetMixerFormat(int sampleSizeInBits, int channels, int sampleRate);

    // GM_ChangeSystemVoices
    private native boolean nAllocateVoices(int midiVoices, int sampledVoices);

    // GM_ChangeSystemVoices
    private native boolean nSetMixLevel(int mixLevel);

    // GM_ChangeAudioModes
    private native boolean nSetInterpolation(int terpMode);

    // not implemented
    private native float nGetCpuLoad();

    // $$kk: 03.15.99: we need to figure out how to handle the
    // device!!!!  for now i am using these methods....
    //private native void nOpenDevice() throws LineUnavailableException;
    //private native void nCloseDevice();

    private static native int nGetTotalVoices();


    // LINKED STREAMS

    private native long nCreateLinkedStreams(long[] idArray);
    private native boolean nStartLinkedStreams(long linkRef);
    private native void nStopLinkedStreams(long linkRef);

    // STATICS

    // in SAMPLE FRAMES.
    // this should be HAE_GetMaxSamplePerSlice() * HAE_GetAudioBufferCount()
    private static native int nGetDefaultBufferSize();

    // GM_SetReverbType
    private static native int nSetReverb(int reverbMode);
}
