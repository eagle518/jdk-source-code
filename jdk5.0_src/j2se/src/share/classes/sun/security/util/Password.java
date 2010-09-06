/*
 * @(#)Password.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

import java.io.*;
import java.util.Arrays;

/**
 * A utility class for reading passwords
 *
 * @version 1.2
 */
public class Password {
    /** Reads user password from given input stream. */
    public static char[] readPassword(InputStream in) throws IOException {
	char[] lineBuffer;
	char[] buf;
	int i;

	buf = lineBuffer = new char[128];

	int room = buf.length;
	int offset = 0;
	int c;

	boolean done = false;
	while (!done) {
	    switch (c = in.read()) {
	      case -1:
	      case '\n':
		  done = true;
		  break;

	      case '\r':
		int c2 = in.read();
		if ((c2 != '\n') && (c2 != -1)) {
		    if (!(in instanceof PushbackInputStream)) {
			in = new PushbackInputStream(in);
		    }
		    ((PushbackInputStream)in).unread(c2);
		} else {
		    done = true;
		    break;
		}

	      default:
		if (--room < 0) {
		    buf = new char[offset + 128];
		    room = buf.length - offset - 1;
		    System.arraycopy(lineBuffer, 0, buf, 0, offset);
		    Arrays.fill(lineBuffer, ' ');
		    lineBuffer = buf;
		}
		buf[offset++] = (char) c;
		break;
	    }
	}

	if (offset == 0) {
	    return null;
	}

	char[] ret = new char[offset];
	System.arraycopy(buf, 0, ret, 0, offset);
	Arrays.fill(buf, ' ');

	return ret;
    }
} 

