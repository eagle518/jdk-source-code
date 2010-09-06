/*
 * @(#)MethodWriter.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing method output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface MethodWriter {
    
    /**
     * Write the header for the method documentation.
     *
     * @param classDoc the class that the methods belong to.
     * @param header the header to write.
     */
    public void writeHeader(ClassDoc classDoc, String header);
    
    /**
     * Write the method header for the given method.
     *
     * @param method the method being documented.
     * @param isFirst the flag to indicate whether or not the method is the 
     *        first to be documented.
     */
    public void writeMethodHeader(MethodDoc method, boolean isFirst);
    
    /**
     * Write the signature for the given method.
     *
     * @param method the method being documented.
     */
    public void writeSignature(MethodDoc method);
    
    /**
     * Write the deprecated output for the given method.
     *
     * @param method the method being documented.
     */
    public void writeDeprecated(MethodDoc method);
    
    /**
     * Write the comments for the given method.
     *
     * @param holder the holder type (not erasure) of the method.
     * @param method the method being documented.
     */
    public void writeComments(Type holder, MethodDoc method);
    
    /**
     * Write the tag output for the given method.
     *
     * @param method the method being documented.
     */
    public void writeTags(MethodDoc method);
    
    /**
     * Write the method footer.
     */
    public void writeMethodFooter();
    
    /**
     * Write the footer for the method documentation.
     *
     * @param classDoc the class that the methods belong to.
     */
    public void writeFooter(ClassDoc classDoc);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

