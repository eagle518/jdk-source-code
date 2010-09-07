/*
 * @(#)Resource.java	1.8 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.security;

import java.net.URL;
import java.io.IOException;
import java.io.InputStream;
import java.util.jar.Manifest;
import java.util.jar.Attributes;

/**
 * This class is used to represent a Resource that has been loaded
 * from the JNLP class path (adapted from JRE code originally written
 * by David Connelly)
 */
public abstract class Resource {
    /**
     * Returns the name of the Resource.
     */
    public abstract String getName();

    /**
     * Returns the URL of the Resource.
     */
    public abstract URL getURL();

    /**
     * Returns the CodeSource URL for the Resource.
     */
    public abstract URL getCodeSourceURL();

    /**
     * Returns an InputStream for reading the Resource data.
     */
    public abstract InputStream getInputStream() throws IOException;

    /**
     * Returns the length of the Resource data, or -1 if unknown.
     */
    public abstract int getContentLength() throws IOException;

    /**
     * Returns the Resource data as an array of bytes.
     */
    public byte[] getBytes() throws IOException {
        byte[] b;
        // Get stream before content length so that a FileNotFoundException
        // can propagate upwards without being caught too early
        InputStream in = getInputStream();
        int len = getContentLength();
        try {
            if (len != -1) {
                // Read exactly len bytes from the input stream
                b = new byte[len];
                while (len > 0) {
                    int n = in.read(b, b.length - len, len);
                    if (n == -1) {
                        throw new IOException("unexpected EOF");
                    }
                    len -= n;
                }
            } else {
                // Read until end of stream is reached
                b = new byte[1024];
                int total = 0;
                while ((len = in.read(b, total, b.length - total)) != -1) {
                    total += len;
                    if (total >= b.length) {
                        byte[] tmp = new byte[total * 2];
                        System.arraycopy(b, 0, tmp, 0, total);
                        b = tmp;
                    }
                }
                // Trim array to correct size, if necessary
                if (total != b.length) {
                    byte[] tmp = new byte[total];
                    System.arraycopy(b, 0, tmp, 0, total);
                    b = tmp;
                }
            }
        } finally {
            in.close();
        }
        return b;
    }
        
    /**
     * Returns the Manifest for the Resource, or null if none.
     */
    public Manifest getManifest() throws IOException {
        return null;
    }

    /**
     * Returns theCertificates for the Resource, or null if none.
     */
    public java.security.cert.Certificate[] getCertificates() {
        return null;
    }

    /**
     * Returns the CodeSigners for the Resource, or null if none.
     */  
    public java.security.CodeSigner[] getCodeSigners() {
	return null;
    }
}


