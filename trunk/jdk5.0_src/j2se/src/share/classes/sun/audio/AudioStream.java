/*
 * @(#)AudioStream.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

import java.io.InputStream;
import java.io.DataInputStream;
import java.io.FilterInputStream;
import java.io.ByteArrayInputStream;
import java.io.BufferedInputStream;
import java.io.IOException;

import javax.sound.sampled.*;
import javax.sound.midi.*;

/**
 * Convert an InputStream to an AudioStream.
 *
 * @version 1.12 03/12/19
 */


public class AudioStream extends FilterInputStream {
    
    // AudioContainerInputStream acis;
    protected AudioInputStream ais = null;
    protected AudioFormat format = null;
    protected MidiFileFormat midiformat = null;
    protected InputStream stream = null;
    
    
    /*
     * create the AudioStream; if we survive without throwing
     * an exception, we should now have some subclass of
     * ACIS with all the header info already read
     */
    
    public AudioStream(InputStream in) throws IOException {
	
	super(in);
	
	stream = in;
	
	if( in.markSupported() == false ) {
	    
	    stream = new BufferedInputStream( in, 1024 );
	}
	
	try {
	    ais = AudioSystem.getAudioInputStream( stream );
	    format = ais.getFormat();
	    this.in = ais;
	    
	} catch (UnsupportedAudioFileException e ) {
	    
	    // not an audio file, see if it's midi...
	    try {
		midiformat = MidiSystem.getMidiFileFormat( stream );
		
	    } catch (InvalidMidiDataException e1) {
		throw new IOException("could not create audio stream from input stream");
	    }
	}
    }
    
    
    
    
    /**
     * A blocking read.
     */
    /*    public int read(byte buf[], int pos, int len) throws IOException {
 
	  return(acis.readFully(buf, pos, len));
	  }
    */
    
    /**
     * Get the data.
     */
    public AudioData getData() throws IOException {
	int length = getLength();
	
	//limit the memory to 1M, so too large au file won't load
	if (length < 1024*1024) {
	    byte [] buffer = new byte[length];
	    try {
		ais.read(buffer, 0, length);
	    } catch (IOException ex) {
		throw new IOException("Could not create AudioData Object");
	    }
	    return new AudioData(format, buffer);
	}
	
	/* 		acis.setData();
 
			if (acis.stream instanceof ByteArrayInputStream) {
			Format[] format = acis.getFormat();
			byte[] bytes = acis.getBytes();
			if (bytes == null)
			throw new IOException("could not create AudioData object: no data received");
			return new AudioData((AudioFormat)format[0], bytes);
			}
	*/
	
	throw new IOException("could not create AudioData object");
    }
    
    
    public int getLength() {
	
	if( ais != null && format != null ) {
	    return (int) (ais.getFrameLength() *
			  ais.getFormat().getFrameSize() );
	    
	} else if ( midiformat != null ) {
	    return (int) midiformat.getByteLength();
	    
	} else {
	    return -1;
	}
    }
}
