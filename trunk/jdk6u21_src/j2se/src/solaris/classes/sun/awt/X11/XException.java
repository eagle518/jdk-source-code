/*
 * @(#)XException.java	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
