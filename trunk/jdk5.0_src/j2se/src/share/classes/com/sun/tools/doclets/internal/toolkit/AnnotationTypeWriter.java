/*
 * @(#)AnnotationTypeWriter.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing annotation type output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API.
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface AnnotationTypeWriter {
    
    /**
     * Write the header of the page.
     * @param header the header to write.
     */
    public void writeHeader(String header);
    
    /**
     * Write the signature of the current annotation type.
     *
     * @param modifiers the modifiers for the signature.
     */
    public void writeAnnotationTypeSignature(String modifiers);  
    
    /**
     * Build the annotation type description.
     */
    public void writeAnnotationTypeDescription();
    
    /**
     * Write the tag information for the current annotation type.
     */
    public void writeAnnotationTypeTagInfo();
    
    /**
     * If this annotation type is deprecated, write the appropriate information.
     */
    public void writeAnnotationTypeDeprecationInfo();
    
    /**
     * Write the footer of the page.
     */
    public void writeFooter();
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
    
    /**
     * Return the {@link AnnotationTypeDoc} being documented.
     *
     * @return the AnnotationTypeDoc being documented.
     */
    public AnnotationTypeDoc getAnnotationTypeDoc();
    
    /**
     * Perform any operations that are necessary when the member summary
     * finished building.
     */
    public void completeMemberSummaryBuild();
}

