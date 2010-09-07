/*
 * @(#)JSUtil.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.javascript;

import java.io.ByteArrayOutputStream;
import java.io.PrintWriter;

public class JSUtil {

    /* Return the stack trace of an exception or error as a String */
    public static String getStackTrace(Throwable t) {
	ByteArrayOutputStream captureStream;
	PrintWriter p;
	
	captureStream = new ByteArrayOutputStream();
	p = new PrintWriter(captureStream);

	t.printStackTrace(p);
	p.flush();

	return captureStream.toString();
    }
}
