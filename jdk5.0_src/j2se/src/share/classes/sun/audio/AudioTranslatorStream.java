/*
 * @(#)AudioTranslatorStream.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;

import java.io.InputStream;
import java.io.DataInputStream;
import java.io.FilterInputStream;
import java.io.IOException;

/**
 * Translator for native audio formats (not implemented in this release).
 *
 * @version 1.7 03/12/19
 */
public
    class AudioTranslatorStream extends NativeAudioStream {
    
	private int length = 0;
    
	public AudioTranslatorStream(InputStream in) throws IOException {
	    super(in);
	    // No translators supported yet.
	    throw new InvalidAudioFormatException();
	}
    
	public int getLength() {
	    return length;
	}
    }
