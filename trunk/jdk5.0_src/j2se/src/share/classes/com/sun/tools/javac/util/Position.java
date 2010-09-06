/**
 * @(#)Position.java	1.13 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

/** A class that encodes and decodes source code positions. Source code
 *  positions are internally represented as integers that contain
 *  both column and line number information.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Position {

    /** Source file positions are integers in the format:
     *  line-number << LINESHIFT + column-number
     *  NOPOS represents an undefined position.
     */
    public static final int LINESHIFT    = 10;
    public static final int COLUMNMASK   = (1 << LINESHIFT) - 1;
    public static final int NOPOS        = 0;
    public static final int FIRSTPOS     = (1 << LINESHIFT) + 1;
    public static final int MAXPOS       = Integer.MAX_VALUE;

    /** The line number of the given position.
     */
    public static int line(int pos) {
	return pos >>> LINESHIFT;
    }

    /** The column number of the given position.
     */
    public static int column(int pos) {
	return pos & COLUMNMASK;
    }

    /** Form a position from a line number and a column number.
     */
    public static int make(int line, int col) {
	return (line << LINESHIFT) + col;
    }
}





