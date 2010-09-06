/*
 * @(#)Patcher.java	1.9 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.cache;

import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;
import com.sun.javaws.exceptions.JNLPException;

/**
 * Patcher describes the necessary method to apply and create deltas.
 *
 * @version 1.9, 12/19/03
 */
public interface Patcher {
    /**
     * Applies a patch previously created with <code>createPatch</code>.
     * Pass in a delegate to be notified of the status of the patch.
     */
    public void applyPatch(PatchDelegate delegate, String oldJarPath,
                           String deltaPath, OutputStream result) throws IOException;
        
    /**
     * Callback used when patching a file.
     */
    public interface PatchDelegate {
        public void patching(int percentDone);
    }
}
