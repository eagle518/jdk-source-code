/*
 * @(#)Patcher.java	1.1 05/01/20
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.jardiff;

import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;

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
