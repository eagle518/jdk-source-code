/*
 * @(#)plain.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * Plain text file handler.
 * @version 1.19, 12/19/03
 * @author  Steven B. Byrne
 */
package sun.net.www.content.text;
import java.net.*;
import java.io.InputStream;
import java.io.IOException;

public class plain extends ContentHandler {
    /**
     * Returns a PlainTextInputStream object from which data
     * can be read.
     */
    public Object getContent(URLConnection uc) {
	try {
	    InputStream is = uc.getInputStream();
	    return new PlainTextInputStream(uc.getInputStream());
	} catch (IOException e) {
	    return "Error reading document:\n" + e.toString();
	}
    }
}
