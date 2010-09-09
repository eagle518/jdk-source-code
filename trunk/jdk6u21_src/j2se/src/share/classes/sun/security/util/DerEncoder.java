/*
 * @(#)DerEncoder.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.util;

import java.io.IOException;
import java.io.OutputStream;

/**
 * Interface to an object that knows how to write its own DER 
 * encoding to an output stream.
 *
 * @version 1.12 03/23/10
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
