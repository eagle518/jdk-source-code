/*
 * @(#)CodingMethod.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.util.jar.pack;

import java.io.*;

/**
 * Interface for encoding and decoding int arrays using bytewise codes.
 * @author John Rose
 * @version 1.6, 03/23/10
 */
interface CodingMethod {
    // Read and write an array of ints from/to a stream.
    public void readArrayFrom(InputStream in, int[] a, int start, int end) throws IOException;
    public void writeArrayTo(OutputStream out, int[] a, int start, int end) throws IOException;

    // how to express me in a band header?
    public byte[] getMetaCoding(Coding dflt);
}
