/*
 * @(#)XException.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

/**
 * Signals that some Xlib routine failed.
 * 
 * @since 1.5
 */
public class XException extends RuntimeException {
    public XException() {
	super();
    }
    public XException(String message) {
	super(message);
    }
}
