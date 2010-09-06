/*
 * @(#)DerEncoder.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Interface to an object that knows how to write its own DER 
 * encoding to an output stream.
 *
 * @version 1.10 12/19/03
 * @author D. N. Hoover
 */
public interface DerEncoder {
    
    /**
     * DER encode this object and write the results to a stream.
     *
     * @param out  the stream on which the DER encoding is written.
     */
    public void derEncode(OutputStream out) 
	throws IOException;

}
