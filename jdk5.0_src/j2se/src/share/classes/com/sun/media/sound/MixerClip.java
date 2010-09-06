/*
 * @(#)MixerClip.java	1.57 04/05/18
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
import javax.sound.sampled.BooleanControl;
import javax.sound.sampled.Control;
import javax.sound.sampled.Clip;
import javax.sound.sampled.DataLine;
import javax.sound.sampled.FloatControl;
import javax.sound.sampled.Line;
import javax.sound.sampled.LineEvent;
import javax.sound.sampled.LineUnavailableException;


/**
 * Represents a pre-loaded clip on the Headspace mixer.
 *
 * @version 1.57, 04/05/18
 * @author Kara Kytle
 * @author Florian Bomers
 */
public class MixerClip extends AbstractDataLine implements Clip, AutoClosingClip {


    // PROPERTIES

    /**
     * Engine identifier for this clip.  This is the sample voice identifier
     * in the engine.  Note that this is *only* non-0 when we are playing sound;
     * loading data just sets up the waveform / data variables.
     */
    private int id;

    /**
     * Engine waveform identifier for this clip data.  This is the GM_Waveform,
     * not the sample voice identifier.
     */
    private long waveformId;

    /**
     * Length in sample frames.  Starts out AudioSystem.NOT_SPECIFIED and
     * gets set in open.
     */
    private int frameLength = AudioSystem.NOT_SPECIFIED;

    /**
     * Duration in microseconds.  Starts out AudioSystem.NOT_SPECIFIED and
     * gets set in open.
     */
    private long microsecondLength = AudioSystem.NOT_SPECIFIED;



    // DEFAULT STATE


    // CURRENT STATE

    long position = 0;				// current position in frames

    int loopCount = 0;
    int currentLoop = 0;
    int loopStart;
    int loopEnd;

    /**
     * This is false except when we're setting the frame position.
     * In this case, we get a callback to callbackSampleEnd(), but
     * we should not change the active state and generate a STOP event.
     */
    private volatile boolean resettingFramePosition = false;

    /**
     * true during implStop() so we can distinguish between STOP and EOM.
     */
    private volatile boolean stopping = false;

    // we need this for pause/resume
    MixerClipSampleRateControl rateControl;

    // we need these for starting the sample with the correct values
    MixerClipGainControl gainControl;
    MixerClipMuteControl muteControl;
    MixerClipPanControl panControl;
    //MixerClipApplyReverbControl applyReverbControl;

    // auto closing clip support
    private boolean autoclosing = false;

    // CONSTRUCTOR


    /**
     * Constructor for pre-loaded clips for the HeadspaceMixer.
     * These should only be created by the HeadspaceMixer; we may want
     * to consider making this an inner class to the HeadspaceMixer to
     * guarantee this.
     *
     * Note that buffer size is in bytes!!
     */
    MixerClip(DataLine.Info info, HeadspaceMixer mixer, AudioFormat format, int bufferSize) throws LineUnavailableException {

	super(info, mixer, new Control[4], format, bufferSize);

	if (Printer.trace) Printer.trace("> MixerClip: constructor: format: " + format +  " bufferSize: " + bufferSize);

	// initialize the controls

	gainControl = new MixerClipGainControl();
	controls[0] = gainControl;

	muteControl = new MixerClipMuteControl();
	controls[1] = muteControl;

	panControl = new MixerClipPanControl();
	controls[2] = panControl;

	rateControl = new MixerClipSampleRateControl();
	controls[3] = rateControl;

	//$$fb 2001-10-09: this isn't implemented!
	//applyReverbControl = new MixerClipApplyReverbControl();
	//controls[4] = applyReverbControl;

	if (Printer.trace) Printer.trace("< MixerClip: constructor completed");
    }


    // CLIP METHODS

    /**
     * Note that offset and bufferSize are in bytes!!
     */
    public void open(AudioFormat format, byte[] data, int offset, int bufferSize) throws LineUnavailableException {
	if (Printer.trace) Printer.trace("> MixerClip.open");

	// $$fb part of fix for 4679187: Clip.open() throws unexpected Exceptions
	Toolkit.isFullySpecifiedAudioFormat(format);

	//$$fb 2001-10-09: Bug #4517739: avoiding deadlock by synchronizing to mixer !
	synchronized (mixer) {
	    // if the line is not currently open, try to open it with this format and buffer size
	    if (!isOpen()) {
		if (Printer.debug) Printer.debug("  MixerClip.open: now open the mixer");

		// reserve mixer resources for this line
		//mixer.open(this, format, bufferSize);
		mixer.open(this);
		try {
		    // open the data line.  may throw LineUnavailableException.
		    implOpen(format, data, offset, bufferSize);

		    // if we succeeded, set the open state to true and send events
		    setOpen(true);
		} catch (LineUnavailableException e) {
		    // release mixer resources for this line and then throw the exception
		    mixer.close(this);
		    throw e;
		}
	    } else {
		// if the line is already open, throw an IllegalStateException
		throw new IllegalStateException("Line is already open with " + getBufferSize() +
						" bytes of " + getFormat() + " data");
	    }
	}
	if (isAutoClosing()) {
	    getEventDispatcher().autoClosingClipOpened(this);
	}
	if (Printer.trace) Printer.trace("< MixerClip.open completed");
    }

    public void open(AudioInputStream stream) throws LineUnavailableException, IOException {
	byte[] streamData;
	int bytesRead;
	int bytesRemaining;
	int temp;

	int lengthInFrames = (int)stream.getFrameLength();
	int lengthInBytes = lengthInFrames * stream.getFormat().getFrameSize();
	if (Printer.debug) Printer.debug("MixerClip: open(AIS): lengthInFrames: " + lengthInFrames);

	// $$fb part of fix for 4679187: Clip.open() throws unexpected Exceptions
	Toolkit.isFullySpecifiedAudioFormat(stream.getFormat());

	if (lengthInFrames != AudioSystem.NOT_SPECIFIED) {
	    // calculate the length in bytes and read the data from the stream into an array
	    // in one fell swoop.
	    streamData = new byte[(int)lengthInBytes];

	    bytesRemaining = lengthInBytes;
	    bytesRead = 0;
	    while( bytesRemaining > 0 ) {
		temp = stream.read(streamData, bytesRead, bytesRemaining);  // can throw IOException
		if( temp > 0 ) {
		    bytesRead += temp;
		    bytesRemaining -= temp;
		} else if( temp==0 ) {
		    Thread.yield();
		} else {
		    // can't read any more
		    break;
		}
	    }
	    if (bytesRead != lengthInBytes) {
		// $$jb: 07.01.99:  The stream's length field may be invalid.
		// If this happens, and we can't read as much data as we expected
		// to, we still want to load as much as we can read, without
		// throwing an exception.  So, recalculate lengthInBytes and
		// lengthInFrames (they're needed later) and use what we've got.
		lengthInBytes = bytesRead;
		lengthInFrames = bytesRead / stream.getFormat().getFrameSize();
	    }
	} else {
	    // read data from the stream until we reach the end of the stream
	    lengthInBytes = 0;
	    bytesRead = 0;

	    // $$kk: 12.18.98: this should be common code in the utility class....
	    int MAX_READ_LIMIT = 2048;
	    ByteArrayOutputStream ba  = new ByteArrayOutputStream();
	    DataOutputStream      dos = new DataOutputStream(ba);
	    byte tmp[] = new byte[MAX_READ_LIMIT];
	    while (true) {
		bytesRead = stream.read(tmp, 0, tmp.length); // can throw IOException
		if (bytesRead == -1) {
		    if (lengthInBytes == 0) {
			throw new IOException("No data found in stream");
		    } else {
			break;
		    }
		}
		dos.write(tmp, 0, bytesRead);
		lengthInBytes += bytesRead;

		Thread.currentThread().yield();
	    } // while
	    streamData = ba.toByteArray();

	    lengthInFrames = streamData.length / stream.getFormat().getFrameSize();
	    if (Printer.debug) Printer.debug("Read to end of stream.  lengthInBytes: " + lengthInBytes + " streamData.length: " + streamData.length + " lengthInFrames: " + lengthInFrames);
	}
	open(stream.getFormat(), streamData, 0, streamData.length);
    }


    public int getFrameLength() {
	return frameLength;
    }


    public long getMicrosecondLength() {
	return microsecondLength;
    }


    /**
     * Sets the media position in sample frames.
     * @param frames desired new media position in sample frames.
     * @return new media position set.
     */
    public synchronized void setFramePosition(int frames) {
	if (Printer.trace) Printer.trace("> MixerClip: setFramePosition: " + frames);

	int lengthInFrames = bufferSize / format.getFrameSize();

	if (Printer.debug) Printer.debug("    MixerClip: bufferSize: " + bufferSize + " lengthInFrames: " + lengthInFrames);
	if (Printer.debug) Printer.debug("    getLongFramePosition(): " + getLongFramePosition());

	// all set!
	if ( (!isStartedRunning()) && (frames == position) ) {
	    return;
	}

	// set this flag so that we do not end up generating a STOP event
	// when callBackSampleEnd() is called.
	resettingFramePosition = true;

	boolean wasRunning = isStartedRunning();

	// if the id is not 0, we will have to stop this sample
	if (id != 0) {
	    implStop();
	}

	// if negative, set the value to 0.
	// $$kk: 04.19.99: or should we throw an IllegalArgumentException??
	if (frames < 0) {
	    frames = 0;
	}

	// otherwise if longer than the sample data, set the value
	// to the last frame.
	// $$kk: 04.19.99: or should we throw an IllegalArgumentException??
	else if (frames > lengthInFrames) {
	    frames = lengthInFrames;
	}

	// set the position value
	position = frames;


	// adjust the loop offset values if necessary
	// $$kk: 04.19.99: need to deal with this!!
	if (loopStart < position) {

	    if (Printer.debug) Printer.debug("Adjusting loopStart to new position value " + position);
	    loopStart = (int) position;
	}
	if (loopEnd < position) {

	    if (Printer.debug) Printer.debug("Adjusting loopEnd to end of sample " + lengthInFrames);
	    loopStart = lengthInFrames;
	}

	// if we should be playing, restart at the new position
	if (wasRunning == true) {
	    implStart();
	}

	// reset this flag.
	resettingFramePosition = false;
	if (Printer.trace) Printer.trace("< MixerClip: setFramePosition: set position to " + position);
    }



    public synchronized void setMicrosecondPosition(long microseconds) {
	if (Printer.trace) Printer.trace("> MixerClip: setMicrosecondPosition: " + microseconds);

	int frames = (int)(microseconds * getFormat().getFrameRate() / 1000000.0);

	if (Printer.debug) Printer.debug("    frame rate: " + format.getFrameRate());
	if (Printer.debug) Printer.debug("    frames: " + frames);

	setFramePosition(frames);
    }


    /**
     * Sets the loop points.  The end point must be greater than
     * or equal to the start point, and both must fall within the
     * duration of the loaded media.  A start value of 0 indicates
     * the beginning of the loaded media.  An end value of -1
     * indicates the end of the loaded media.
     * @param start loop start position in sample frames
     * @param end loop end position in sample frames
     * @throws IllegalArgumentException thrown if the requesed
     * loop points cannot be set, usually because they do no fall within
     * the length of the media posiiton or because the end time falls
     * before the start time.
     */
    public void setLoopPoints(int start, int end) {
	if (Printer.trace) Printer.trace("> MixerClip: setLoopPoints: start: " + start + " end: " + end);

	int lengthInFrames = bufferSize / format.getFrameSize();
	if (Printer.debug) Printer.debug("    > MixerClip: setLoopPoints: lengthInFrames: " + lengthInFrames);

	// if start position is negative, set the position to 0.
	// $$kk: 04.19.99: or should we throw an IllegalArgumentException??
	if (start < 0) {
	    start = 0;
	}

	// if end position is longer than the sample data, set the position
	// to the last frame.
	// $$kk: 04.19.99: or should we throw an IllegalArgumentException??
	if (end > lengthInFrames) {
	    end = lengthInFrames;
	}

	//$$fb 2001-07-20: fix for bug 4385928:
	//Clip.setLoopPoints returns IllegalArgumentException when endpoint is -1.
	if (end==-1) {
	    end = lengthInFrames;
	}

	// if the end position is less than the start position, throw IllegalArgumentException??
	if (end < start) {
	    throw new IllegalArgumentException("End position " + end + "  preceeds start position " + start);
	}

	// now set constraints based on the start position
	// $$kk: 04.19.99: need to deal with this!!
	if (start < position) {

	    if (Printer.debug) Printer.debug("Adjusting start to new position value " + position);
	    start = (int) position;
	}
	if (end < position) {

	    if (Printer.debug) Printer.debug("Adjusting end to end of sample " + lengthInFrames);
	    end = lengthInFrames;
	}

	// i don't think we should allow this in this release: too many ways to screw up!
	/*
	  //		// if we have a sample voice, update it
	  //		if (id != 0) {
	  //			// can throw IllegalArgumentException
	  //			// $$: 04.19.99: this should only happen if the loop size is too small
	  //			nSetLoopPoints(id, (int)start, (int)end);
	  //		}
	*/

	loopStart = (int)start;
	loopEnd = (int)end;

	// need to re-setup the sample voice; can throw LineUnavailableException
	/*if (id != 0) {
	    try {
		// close voice, but not wave
		nClose(id, 0);
		id = 0; // getValidVoiceId will do nothing if id!=0
		id = getValidVoiceId();
	    } catch (LineUnavailableException e) {
		id = 0;
	    }
	    if (id == 0) {
		if (Printer.err) Printer.err("Could not re-open clip in MixerClip.java.setLoopPoints!");
		// we are screwed... re-open failed!
		// sorry....
		implClose();
	    }
	}*/

	if (Printer.trace) Printer.trace("< MixerClip: setLoopPoints completed: loopStart: " + loopStart + " loopEnd: " + loopEnd);
    }


    public void loop(int count) {

	// if it's playing, loop(0) should cause it to play to the end and complete.
	// if it's not, loop(0) should cause it to play once from the current position to the end.

	// if count is any other value, we call stop and then loop count times.
	// this is sort of weird because the clip won't actually loop back if
	// the position is already past the loop start....

	if (count != 0) {
	    stop();
	}
	loopCount = count;
	start();
    }

    //$$fb 2002-10-29: fix for 4732218: Clip.drain does not actually block until all I/O is complete as documented.
    public void drain() {
	if (Printer.trace) Printer.trace("> MixerClip: drain");
	// bad implementation, since native drain is not implemented...
	try {
	    // wait max. length of this clip
	    long endTime = System.currentTimeMillis() + (getMicrosecondLength() / 1000);
	    boolean wasActive = (id != 0);
	    while (id != 0 && System.currentTimeMillis() < endTime) {
		synchronized(lock) {
		    lock.wait(1000);
		}
	    }
	    if (wasActive && (id == 0)) {
		// STOP event is sent a little bit too early
		Thread.sleep(20);
	    }
	} catch (InterruptedException ie) { }
	if (Printer.trace) Printer.trace("< MixerClip: drain");
    }


    // ABSTRACT METHOD IMPLEMENTATIONS

    // ABSTRACT LINE

    //void implOpen() throws LineUnavailableException {
    void implOpen(AudioFormat format, int bufferSize) throws LineUnavailableException {
	//$$fb 2001-07-12: fixed bug 4479444. changed text of error message to be more meaningful
	throw new IllegalArgumentException("illegal call to open() in interface Clip");
    }


    void implOpen(AudioFormat format, byte[] data, int offset, int lengthInBytes) throws LineUnavailableException {
	// note that bufferSize is in BYTES now

	// update the initial value for the sample rate control
	rateControl.update(format.getFrameRate());

	// $$fb 2001-09-26: workaround for bug#4507837: Problems with applets: buzzing sound
	// add silence at end of clip, if clip is shorter than 15ms
	// side effects: looping will not be as smooth. This problem is neglectible,
	// as 15ms clips are REALLY short anyway.
	int bytes15Millis = ((int) (0.015f*format.getFrameRate()))*format.getFrameSize();
	//$$fb 2001-09-26, for workaround above
	if (lengthInBytes<bytes15Millis) {
	    boolean unsigned8 = format.getEncoding().equals(AudioFormat.Encoding.PCM_UNSIGNED)
		&& (format.getSampleSizeInBits()==8);
	    byte fillByte = 0;
	    if (unsigned8) {
		// 8bit unsigned has a different value for silence
		fillByte = -128;
	    }
	    byte[] newData = new byte[bytes15Millis];
	    System.arraycopy(data, 0, newData, 0, lengthInBytes);
	    for (int i = lengthInBytes; i<bytes15Millis; i++) {
		newData[i]=fillByte;
	    }
	    data = newData;
	    // this is not strictly correct, because the length is effectively changed...
	    lengthInBytes = bytes15Millis;
	}

	// determine whether we need to convert signed 8-bit data to unsigned,
	// or swap the byte order.
	// $$kk: 04.16.99: should share some of this initialization code with MixerSourceLine!!

	boolean convertSign = false;
	boolean convertByteOrder = false;

	if ( (format.getSampleSizeInBits() == 8) && (AudioFormat.Encoding.PCM_SIGNED.equals(format.getEncoding())) ) {
	    convertSign = true;
	}

	if ( (format.getSampleSizeInBits() > 8) && (format.isBigEndian() != Platform.isBigEndian()) ) {
	    convertByteOrder = true;
	}

	// do any sign and byte order conversions
	if (convertSign) {
	    Toolkit.getUnsigned8(data, offset, lengthInBytes);

	} else if (convertByteOrder) {
	    Toolkit.getByteSwapped(data, offset, lengthInBytes);
	}


	// create the native clip and set the waveform  identifier (*not* the sample voice identifier).
	// note that the nOpen method takes the offset in *bytes* and the length in *frames*
	int lengthInFrames = lengthInBytes / format.getFrameSize();

	// $$jb: 11.09.99:  Patch for 4288683: We have a 1Meg sample limit in the engine,
	// but the native code is not throwing any error conditions if the buffer size is
	// too large.  Calculate the size (in samples) of the buffer here, and throw a
	// LineUnavailableException if it is too large.  We should revisit this if the
	// 1Meg limit is fixed in the engine.

	// $$kk: 11.15.99: why are you multiplying by the number of bytes here??
	//  plus: i moved this hard-coded value to the DataLine.Info class for MixerClip.

	if (lengthInFrames > ((HeadspaceMixer)mixer).MAX_SAMPLES) {
	    throw new LineUnavailableException("Failed to allocate clip data: Requested buffer too large.");
	}

	// get the waveformId
	waveformId = nOpen(format.getSampleSizeInBits(), format.getChannels(), format.getSampleRate(), data, offset, lengthInFrames);

	// convert the array back - it might come from outside!
	if (convertSign) {
	    Toolkit.getUnsigned8(data, offset, lengthInBytes);

	} else if (convertByteOrder) {
	    Toolkit.getByteSwapped(data, offset, lengthInBytes);
	}

	if (waveformId == 0) {
	    throw new LineUnavailableException("Failed to allocate clip data.");
	}

	// set the format and buffer size values
	this.format = format;
	this.bufferSize = lengthInBytes;

	// update the frame and microsecond length variables
	frameLength = bufferSize / format.getFrameSize();
	microsecondLength = (long)(((double)frameLength) / format.getFrameRate() * 1000000);

	// set the inital loop points to correspond to the first and last frame of the clip
	loopStart = 0;
	loopEnd = lengthInFrames;

	// set the initial position to 0.
	position = 0;

	// setup the sample voice; can throw LineUnavailableException
	try {
	    id = getValidVoiceId();
	} catch (LineUnavailableException e) {
	    // $$fb 2002-07-18: part of fix for 4716412:
	    //      Memory leak when Clip.open(stream) throws LineUnavailableException
	    implClose();
	    throw e;
	}
    }


    synchronized int getValidVoiceId() throws LineUnavailableException {
	if (id != 0) {
	    return id;
	}

	float currentGain = muteControl.getValue() ? 0.0f : Toolkit.dBToLinear(gainControl.getValue());
	float currentPan = 	panControl.getValue();
	int currentRate = (int)rateControl.getValue();

	// get the sample voice id
	// can throw LineUnavailableException, IllegalArgumentException
	// have to switch the sign on the pan value for the engine!
	int newId = nSetup(waveformId, (int) position, loopStart, loopEnd, currentGain, (currentPan * -1), currentRate);

	if (newId == 0) {
	    throw new LineUnavailableException("Failed to allocate sample voice");
	}
	return newId;
    }


    void implClose() {
	nClose(id, waveformId);
	id = 0;
	waveformId = 0;
	getEventDispatcher().autoClosingClipClosed(this);
    }


    // ABSTRACT DATA LINE


    void implStart() {
	int lengthInFrames = bufferSize / format.getFrameSize();

	if (position >= lengthInFrames) {
	    if (Printer.debug) Printer.debug("MixerClip: implStart: already at end of sample; returning.");
	    return;
	}

	// setup the sample voice (can throw LineUnavailableException) and start playback
	try {
	    id = getValidVoiceId();
	    if (nStart(id)) {
		// $$kk: 05.31.99: we don't have a real callback for this
		setActive(true);
		setStarted(true);
	    }
	} catch (LineUnavailableException e) {
	    if (Printer.err) Printer.err("ERROR: LineUnavailableException in implStart: " + e);
	}
    }


    void implStop() {
	if (Printer.debug) Printer.debug("> MixerClip: implStop: id: " + id);

	// record the current position
	position = getLongFramePosition();

	// reset the loop params
	loopCount = 0;
	currentLoop = 0;

	// stopping!
	stopping = true;

	// stop the sample.  this invalidates the sample voice id.
	nStop(id);

	// wait for the callback
	synchronized(lock) {
	    if (id!=0) {
		try {
		    //long time=System.currentTimeMillis();
		    lock.wait(3000);
		    //if (System.currentTimeMillis()-time > 2500) {
		    //System.out.println(" WAITING TIMED OUT!"); System.out.flush();
		    //id=0;
		    //}
		} catch (InterruptedException e) { }
	    }
	}

	// send STOP event
	stopping = false;
	setStarted(false);

	if (Printer.debug) Printer.debug("< MixerClip: implStop: id: " + id + " position: " + position);
    }


    // METHOD OVERRIDES

    // ABSTRACT DATA LINE


    /**
     * Determines the current volume level for the channel.
     * The range is 0 to 1.
     * @return current level
     *
     * <p>
     * <b>ISSUE: What are the units?  Can we / should we always support this?
     * do we want to be able to provide the "waveform" or per-channel (left/right)
     * level as well?  Can we / should we provide getBalance() as well?
     * Should this be a separate control?</b>
     */
    /*
      public float getLevel() {

      return (id != 0) ? nGetLevel(id) : (float)AudioSystem.NOT_SPECIFIED;
      }
    */

    /**
     * Obtains the current position in sample frames.
     * @return sample frame position.
     */
    // replacement for getFramePosition (see AbstractDataLine)
    public long getLongFramePosition() {
	return (id != 0) ? (position + nGetPosition(id)) : position;
    }


    // CALLBACKS


    // called by the engine when it reaches the sample loop-end point.
    // return 'true' to loop back to the loop-start point, or false
    // to play out to the end of the sample.
    boolean callbackSampleLoop() {
	if (Printer.trace) Printer.trace(">> MixerClip: callbackSampleLoop()");

	// $$jb: 05.12.99: adding (loopCount==-1) for continuous loop.
	//       This should be updated when we add a field in Clip for this value.

	boolean keepLooping = (loopCount==-1) || (currentLoop < loopCount) ? true : false;
	if (Printer.debug) Printer.debug("currentLoop: " + currentLoop + " loopCount: " + loopCount + " keepLooping: " + keepLooping);
	currentLoop++;

	if (!keepLooping) {
	    loopCount = 0;
	    currentLoop = 0;
	}

	if (Printer.trace) Printer.trace("<< MixerClip: callbackSampleLoop() returning: " + keepLooping);
	return keepLooping;
    }


    // called by the engine when the sample voice in the engine is destroyed.
    // the sample void id becomes invalid.
    void callbackSampleEnd() {
	if (Printer.trace) Printer.trace(">> MixerClip: callbackSampleEnd()");

	// we're ending, so our position must be "length."
	// $$kk: 04.19.99: getFramePosition does *not* work now!!
	// we're doing this below in callbackSampleFramePosition now....
	// position = length;

	// set the sample voice id to 0
	synchronized (lock) {
	    id = 0;
	    lock.notifyAll();
	}

	// update the state, *unless* we are currently just setting the
	// frame position, in which case we should not send the STOP event
	// generated by setActive(false).
	if (!resettingFramePosition) {
	    setActive(false);

	    // $$kk: 11.10.99: need to verify this
	    //running = false;
	    if (!stopping) {
		setEOM();
	    }
	}

	// $$kk: 06.09.99: i think this is *wrong*.  however, people really
	// screw up if i don't do this.
	// position = 0;

	if (Printer.trace) Printer.trace("<< MixerClip: callbackSampleEnd() completed");
    }


    //	// called by the engine when a marked sample frame position is reached.
    //	// we're using this to detect EOM.
    //	void callbackSampleFramePosition(int position) {
    //
    //		if (Printer.trace) Printer.trace(">> MixerClip: callbackSampleFramePosition: position: " + position);
    //
    //		// sometimes the exact value is not the exact last-frame value.
    //		// since we're only using this for EOM right now, we can cheat....
    //
    //		// $$kk: 04.19.99: yikes!  but the position value we get back may not
    //		// be this exact value....
    //		this.position = length;
    //
    //		/* send events for EOM. */
    //		// $$kk: 04.19.99: note that this currently generates EOM events for the
    //		// end of every loop if you loop to the end of the sample....
    //		sendEvents(new ChannelEvent(this, ChannelEvent.Type.EOM, getLongFramePosition()));
    //	}


    // AUTO CLOSING CLIP SUPPORT

    public boolean isAutoClosing() {
	return autoclosing;
    }

    public void setAutoClosing(boolean value) {
	if (value != autoclosing) {
	    if (isOpen()) {
		if (value) {
		    getEventDispatcher().autoClosingClipOpened(this);
		} else {
		    getEventDispatcher().autoClosingClipClosed(this);
		}
	    }
	    autoclosing = value;
	}
    }

    // INNER CLASSES


    private class MixerClipGainControl extends FloatControl {

	// STATE VARIABLES
	private float linearGain = 1.0f;

	private MixerClipGainControl() {
	    super(FloatControl.Type.MASTER_GAIN,
		  Toolkit.linearToDB(0.0f),
		  Toolkit.linearToDB(5.0f),
		  //$$fb 2001-07-16: fix for Bug 4385654
		  //Toolkit.linearToDB(1.0f / 128.0f),
		  Math.abs(Toolkit.linearToDB(5.0f)-Toolkit.linearToDB(0.0f))/128.0f,
		  //$$fb: this should better be: AudioSystem.NOT_SPECIFIED
		  -1,
		  0.0f,
		  "dB", "Minimum", "", "Maximum");
	}

	public void setValue(float newValue) {
	    // don't cache the values unless the clip is open.  this is how streams work,
	    // and it seems like a reasonable requirement.
	    if (!isOpen()) {
		return;
	    }

	    // adjust value within range
	    newValue = Math.min(newValue, getMaximum());
	    newValue = Math.max(newValue, getMinimum());

	    float newLinearGain = Toolkit.dBToLinear(newValue);

	    if ( (newLinearGain != linearGain) && (id != 0) ) {
		newLinearGain = nSetLinearGain(id, newLinearGain);
	    }

	    linearGain = newLinearGain;
	    super.setValue(Toolkit.linearToDB(linearGain));
	}
    } // class MixerClipGainControl


    private class MixerClipPanControl extends FloatControl {

	private MixerClipPanControl() {
	    super(FloatControl.Type.PAN,
		  -1.0f,
		  1.0f,
		  (1.0f / 64.0f),
		  -1,
		  0.0f,
		  "", "Left", "Center", "Right");
	}

	public void setValue(float newValue) {
	    // don't cache the values unless the clip is open.  this is how streams work,
	    // and it seems like a reasonable requirement.
	    if (!isOpen()) {
		return;
	    }

	    // adjust value within range
	    newValue = Math.min(newValue, getMaximum());
	    newValue = Math.max(newValue, getMinimum());
	    if ((newValue != getValue()) && (id != 0)) {
		// $$kk: 04.07.99: the headspace docs say that the pan range
		// is -63 (left) to +63 (right), but we are hearing the reverse.
		// i'm throwing a -1 in here to compensate, since we do use -1
		// for left and +1 for right.
		newValue = (-1.0f * nSetPan(id, (-1.0f * newValue)));
	    }
	    super.setValue(newValue);
	}
    } // class MixerClipPanControl


    private class MixerClipSampleRateControl extends FloatControl {

	private MixerClipSampleRateControl() {
	    super(FloatControl.Type.SAMPLE_RATE,
		  0.0f,
		  48000.0f,
		  1.0f,
		  -1,
		  getFormat().getFrameRate(),
		  "FPS", "Minimum", "", "Maximum");
	}

	public void setValue(float newValue) {
	    // don't cache the values unless the clip is open.  this is how streams work,
	    // and it seems like a reasonable requirement.
	    if (!isOpen()) {
		return;
	    }

	    // adjust value within range
	    newValue = Math.min(newValue, getMaximum());
	    newValue = Math.max(newValue, getMinimum());

	    if ( (newValue != getValue()) && (id != 0) ) {
		newValue = (float)nSetSampleRate(id, (int)newValue);
	    }

	    super.setValue(newValue);
	}

	// Update the sample rate to reflect the natural rate.
	// (needs to be done if the format changes.)
	private void update(float frameRate) {
	    super.setValue(frameRate);
	}
    } // class MixerClipSampleRateControl


    private class MixerClipMuteControl extends BooleanControl {

	private MixerClipMuteControl() {
	    super(BooleanControl.Type.MUTE, false, "True", "False");
	}

	public void setValue(boolean newValue) {
	    // don't cache the values unless the clip is open.  this is how streams work,
	    // and it seems like a reasonable requirement.
	    if (!isOpen()) {
		return;
	    }
	    if (newValue && (!getValue()) && (id != 0) ) {
		nSetLinearGain(id, 0.0f);
	    } else if ((!newValue) && (getValue()) && (id != 0)) {
		float linearGain = Toolkit.dBToLinear(gainControl.getValue());
		nSetLinearGain(id, linearGain);
	    }
	    //this.value = value;
	    super.setValue(newValue);
	}
    }  // class MixerClipMuteControl


    private class MixerClipApplyReverbControl extends BooleanControl {

	private MixerClipApplyReverbControl() {
	    super(BooleanControl.Type.APPLY_REVERB, false, "Yes", "No");
	}

	public void setValue(boolean newValue) {
	    // don't cache the values unless the lipsource line is open.  this is how streams work,
	    // and it seems like a reasonable requirement.
	    if (!isOpen()) {
		return;
	    }
	    if ( (newValue != getValue()) && (id != 0) ) {
		/* $$kk: 10.11.99: need to implement! */
	    }
	    super.setValue(newValue);
	}
    }  // class MixerClipApplyReverbControl



    // NATIVE METHODS

    private native boolean nStart(int id);

    // this returns the sample voice identifier (formerly in nStart).
    // throws IllegalArgumentException if the size is too big
    // throws LineUnavailableException if no voices are available
    // $$kk: 04.19.99: note that when the engine sets up the sample, it simply sets the sample data pointer
    // ahead according to the frame offset position, so it is imperative to pass in loop start and end positions
    // which make sense according to the engine.  i am passing in the framePosition, loopStart, and loopEnd as
    // total sample frame positions for the sample.  however, the engine won't be able to handle, for instance,
    // a start offset of 100 and a loopStart of 0, nor can it handle the case where (offset + loopEnd) > lengthInFrames.
    private native int nSetup(long waveformId, int framePosition, int loopStart, int loopEnd, float linearGain, float pan, int rate)
	throws LineUnavailableException, IllegalArgumentException;


    private native void nDrain(int id);
    private native void nFlush(int id);
    private native long nGetPosition(int id);
    private native float nGetLevel(int id);

    // this returns the GM_Waveform identifier for the sample data
    private native long nOpen(int sampleSizeInBits, int channels, float sampleRate, byte[] data, int offset, int lengthInFrames) throws LineUnavailableException;

    // this ends sample playback if the id is non-0, and frees the GM_Waveform
    // if the waveformId is non-0.  it also deletes the global java object ref.
    private native void nClose(int id, long waveformId);


    // this ends sample playback and makes the sample voice identifier invalid
    private native void nStop(int id);

    // set volume using linear scale
    // GM_ChangeSampleVolume
    protected native float nSetLinearGain(int id, float linearGain);

    // GM_ChangeSampleStereoPosition
    protected native float nSetPan(int id, float pan);

    // GM_ChangeSamplePitch
    protected native int nSetSampleRate(int id, int rate);

    // GM_SetSampleLoopPoints
    //private native void nSetLoopPoints(int id, int start, int end);
}
