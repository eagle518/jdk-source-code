/*
 * @(#)AudioData.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

import java.io.*;
import javax.sound.sampled.*;


/**
 * A clip of audio data. This data can be used to construct an
 * AudioDataStream, which can be played. <p>
 *
 * @author  Arthur van Hoff
 * @author  Kara Kytle
 * @version 1.12 03/12/19
 * @see     AudioDataStream
 * @see     AudioPlayer
 */

 /*
  * the idea here is that the AudioData object encapsulates the
  * data you need to play an audio clip based on a defined set
  * of data.  to do this, you require the audio data (a byte
  * array rather than an arbitrary input stream) and a format
  * object.
  */


public class AudioData {
    
    private static final AudioFormat DEFAULT_FORMAT = 
	new AudioFormat(AudioFormat.Encoding.ULAW,
	                8000,   // sample rate
	                8,      // sample size in bits
	                1,      // channels
	                1,      // frame size in bytes
	                8000,   // frame rate
	                true ); // bigendian (irrelevant for 8-bit data)
    
    AudioFormat format;   // carry forth the format array amusement
    byte buffer[];
    
    /**
     * Constructor
     */
    public AudioData(byte buffer[]) {
	
	this.buffer = buffer;
	// if we cannot extract valid format information, we resort to assuming the data will be 8k mono u-law
	// in order to provide maximal backwards compatibility....
	this.format = DEFAULT_FORMAT;
	
	// okay, we need to extract the format and the byte buffer of data
	try {
	    AudioInputStream ais = AudioSystem.getAudioInputStream(new ByteArrayInputStream(buffer));
	    this.format = ais.getFormat();
	    ais.close();
	    // $$fb 2002-10-27: buffer contains the file header now!
	} catch (IOException e) {
	    // use default format
	} catch (UnsupportedAudioFileException e1 ) {
	    // use default format
	}
    }


    /**
     * Non-public constructor; this is the one we use in ADS and CADS
     * constructors.
     */
    AudioData(AudioFormat format, byte[] buffer) {

	this.format = format;
	this.buffer = buffer;
    }
}
