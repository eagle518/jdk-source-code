/*
 * @(#)InvalidAudioFormatException.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.audio;
import java.io.IOException;

/**
 * Signals an invalid audio stream for the stream handler.
 */
class InvalidAudioFormatException extends IOException {


    /**
     * Constructor.
     */
    public InvalidAudioFormatException() {
	super();
    }

    /**
     * Constructor with a detail message.
     */
    public InvalidAudioFormatException(String s) {
	super(s);
    }
}
