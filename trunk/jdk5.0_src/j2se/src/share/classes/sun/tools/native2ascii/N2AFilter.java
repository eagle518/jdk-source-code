/*
 * @(#)N2AFilter.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/**
 * This FilterWriter class takes an existing Writer and uses
 * the 'back-tick U' escape notation to escape characters which are
 * encountered within the input character based stream which
 * are outside the 7-bit ASCII range. The native platforms linefeed 
 * character is emitted for each line of processed input 
 */

package sun.tools.native2ascii;
import java.io.*;
import java.nio.BufferOverflowException;

class N2AFilter extends FilterWriter {

    public N2AFilter(Writer out) { super(out); }

    public void write(char b) throws IOException { 
	char[] buf = new char[1];
	buf[0] = (char)b;
	write(buf, 0, 1);
    }

    public void write(char[] buf, int off, int len) throws IOException {

	String lineBreak = System.getProperty("line.separator");

	//System.err.println ("xx Out buffer length is " + buf.length );
	for (int i = 0; i < len; i++) {
	    if ((buf[i] > '\u007f')) {
		// write \udddd
		out.write('\\');
		out.write('u');
		String hex =
		    Integer.toHexString(buf[i]);
		StringBuffer hex4 = new StringBuffer(hex);
		hex4.reverse();
		int length = 4 - hex4.length();
		for (int j = 0; j < length; j++) {
		    hex4.append('0');
		}
		for (int j = 0; j < 4; j++) {
		    out.write(hex4.charAt(3 - j));
		}
	    } else
		out.write(buf[i]);
	}
    }
}

