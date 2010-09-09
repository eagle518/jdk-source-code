/*
 * @(#)InvalidAudioFormatException.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
