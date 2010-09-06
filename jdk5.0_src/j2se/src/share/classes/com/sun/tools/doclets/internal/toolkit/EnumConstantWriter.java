/*
 * @(#)EnumConstantWriter.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing enum constant output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface EnumConstantWriter {
    
    /**
     * Write the header for the enum constant documentation.
     *
     * @param classDoc the class that the enum constants belong to.
     * @param header the header to write.
     */
    public void writeHeader(ClassDoc classDoc, String header);
    
    /**
     * Write the enum constant header for the given enum constant.
     *
     * @param enumConstant the enum constant being documented.
     * @param isFirst the flag to indicate whether or not the enum constant is 
     *                the first to be documented.
     */
    public void writeEnumConstantHeader(FieldDoc enumConstant, boolean isFirst);
    
    /**
     * Write the signature for the given enum constant.
     *
     * @param enumConstant the enum constant being documented.
     */
    public void writeSignature(FieldDoc enumConstant);
    
    /**
     * Write the deprecated output for the given enum constant.
     *
     * @param enumConstant the enum constant being documented.
     */
    public void writeDeprecated(FieldDoc enumConstant);
    
    /**
     * Write the comments for the given enum constant.
     *
     * @param enumConstant the enum constant being documented.
     */
    public void writeComments(FieldDoc enumConstant);
    
    /**
     * Write the tag output for the given enum constant.
     *
     * @param enumConstant the enum constant being documented.
     */
    public void writeTags(FieldDoc enumConstant);
    
    /**
     * Write the enum constant footer.
     */
    public void writeEnumConstantFooter();
    
    /**
     * Write the footer for the enum constant documentation.
     *
     * @param classDoc the class that the enum constant belong to.
     */
    public void writeFooter(ClassDoc classDoc);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

