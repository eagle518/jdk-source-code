/*
 * @(#)REException.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * A class to signal exception from the RegexpPool class.
 * @author  James Gosling
 */

public class REException extends Exception {
    REException (String s) {
	super(s);
    }
}
