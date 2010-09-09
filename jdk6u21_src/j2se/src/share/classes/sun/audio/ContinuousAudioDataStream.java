/*
 * @(#)ContinuousAudioDataStream.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

/**
 * Create a continuous audio stream. This wraps a stream
 * around an AudioData object, the stream is restarted
 * at the beginning everytime the end is reached, thus
 * creating continuous sound.<p>
 * For example:
 * <pre>
 *   AudioData data = AudioData.getAudioData(url);
 *   ContinuousAudioDataStream audiostream = new ContinuousAudioDataStream(data);
 *   AudioPlayer.player.start(audiostream);
 * </pre>
 *
 * @see AudioPlayer
 * @see AudioData
 * @author Arthur van Hoff
 * @version 1.9 10/03/23
 */

public
    class ContinuousAudioDataStream extends AudioDataStream {
    
    
	/**
	 * Create a continuous stream of audio.
	 */
	public ContinuousAudioDataStream(AudioData data) {
	
	    super(data);
	}
    
    
	public int read() {
	
	    int i = super.read();
	
	    if (i == -1) {
		reset();
		i = super.read();
	    }
	
	    return i;
	}
    
    
	public int read(byte ab[], int i1, int j) {
	
	    int k;
	
	    for (k = 0; k < j; ) {
		int i2 = super.read(ab, i1 + k, j - k);
		if (i2 >= 0) k += i2;
		else reset();
	    }
	
	    return k;
	}
    }
