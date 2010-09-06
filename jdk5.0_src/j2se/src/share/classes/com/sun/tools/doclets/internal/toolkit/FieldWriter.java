/*
 * @(#)FieldWriter.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing field output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface FieldWriter {
    
    /**
     * Write the header for the field documentation.
     *
     * @param classDoc the class that the fields belong to.
     * @param header the header to write.
     */
    public void writeHeader(ClassDoc classDoc, String header);
    
    /**
     * Write the field header for the given field.
     *
     * @param field the field being documented.
     * @param isFirst the flag to indicate whether or not the field is the 
     *        first to be documented.
     */
    public void writeFieldHeader(FieldDoc field, boolean isFirst);
    
    /**
     * Write the signature for the given field.
     *
     * @param field the field being documented.
     */
    public void writeSignature(FieldDoc field);
    
    /**
     * Write the deprecated output for the given field.
     *
     * @param field the field being documented.
     */
    public void writeDeprecated(FieldDoc field);
    
    /**
     * Write the comments for the given field.
     *
     * @param field the field being documented.
     */
    public void writeComments(FieldDoc field);
    
    /**
     * Write the tag output for the given field.
     *
     * @param field the field being documented.
     */
    public void writeTags(FieldDoc field);
    
    /**
     * Write the field footer.
     */
    public void writeFieldFooter();
    
    /**
     * Write the footer for the field documentation.
     *
     * @param classDoc the class that the fields belong to.
     */
    public void writeFooter(ClassDoc classDoc);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

