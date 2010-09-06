/*
 * @(#)ConstructorWriter.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing constructor output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface ConstructorWriter {
    
    /**
     * Write the header for the constructor documentation.
     *
     * @param classDoc the class that the constructors belong to.
     * @param header the header to write.
     */
    public void writeHeader(ClassDoc classDoc, String header);
    
    /**
     * Write the constructor header for the given constructor.
     *
     * @param constructor the constructor being documented.
     * @param isFirst the flag to indicate whether or not the constructor is the 
     *        first to be documented.
     */
    public void writeConstructorHeader(ConstructorDoc constructor, boolean isFirst);
    
    /**
     * Write the signature for the given constructor.
     *
     * @param constructor the constructor being documented.
     */
    public void writeSignature(ConstructorDoc constructor);
    
    /**
     * Write the deprecated output for the given constructor.
     *
     * @param constructor the constructor being documented.
     */
    public void writeDeprecated(ConstructorDoc constructor);
    
    /**
     * Write the comments for the given constructor.
     *
     * @param constructor the constructor being documented.
     */
    public void writeComments(ConstructorDoc constructor);
    
    /**
     * Write the tag output for the given constructor.
     *
     * @param constructor the constructor being documented.
     */
    public void writeTags(ConstructorDoc constructor);
    
    /**
     * Write the constructor footer.
     */
    public void writeConstructorFooter();
    
    /**
     * Write the footer for the constructor documentation.
     *
     * @param classDoc the class that the constructors belong to.
     */
    public void writeFooter(ClassDoc classDoc);
    
    /**
     * Let the writer know whether a non public constructor was found.
     *
     * @param foundNonPubConstructor true if we found a non public constructor.
     */
    public void setFoundNonPubConstructor(boolean foundNonPubConstructor);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

