/*
 * @(#)PlainTextInputStream.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.content.text;
import java.io.InputStream;
import java.io.FilterInputStream;

/**
 * PlainTextInputStream class extends the FilterInputStream class. 
 * Currently all calls to the PlainTextInputStream object will call 
 * the corresponding methods in the FilterInputStream class.  Hence 
 * for now its use is more semantic.
 *
 * @author Sunita Mani
 * @version 1.12, 12/19/03
 */ 
public class PlainTextInputStream extends FilterInputStream {

    /**
     * Calls FilterInputStream's constructor.
     * @param an InputStream
     */
    PlainTextInputStream(InputStream is) {
	super(is);
    }
}
