/*
 * @(#)AudioDataStream.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

import java.io.*;
import javax.sound.sampled.*;
import javax.sound.midi.*;

/**
 * An input stream to play AudioData.
 *
 * @see AudioPlayer
 * @see AudioData
 * @author Arthur van Hoff
 * @author Kara Kytle
 * @version 1.9 03/12/19
 */
public class AudioDataStream extends ByteArrayInputStream {
    
    AudioData ad;
    
    /**
     * Constructor
     */
    public AudioDataStream(AudioData data) {
	
	super(data.buffer);
	this.ad = data;
    }
    
    AudioData getAudioData() {
	return ad;
    }
}
