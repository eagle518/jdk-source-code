/*
 * @(#)PackageSummaryWriter.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import com.sun.javadoc.*;
import java.io.*;

/**
 * The interface for writing package summary output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface PackageSummaryWriter {
    
    /**
     * Return the name of the output file.
     *
     * @return the name of the output file.
     */
    public abstract String getOutputFileName();
    
    /**
     * Write the header for the package summary.
     */
    public abstract void writeSummaryHeader();
    
    /**
     * Write the footer for the package summary.
     */
    public abstract void writeSummaryFooter();
    
    /**
     * Write the table of classes in this package.
     *
     * @param classes the array of classes to document.
     * @param label the label for this table.
     */
    public abstract void writeClassesSummary(ClassDoc[] classes, String label);
    
    /**
     * Write the header for the summary.
     *
     * @param heading Package name.
     */
    public abstract void writePackageHeader(String heading);
        
    /**
     * Print the package description from the "packages.html" file.
     */
    public abstract void writePackageDescription();
    
    /**
     * Print the tag information from the "packages.html" file.
     */
    public abstract void writePackageTags();
    
    /**
     * Write the footer for the summary.
     *
     */
    public abstract void writePackageFooter();
    
    /**
     * Close the writer.
     */
    public abstract void close() throws IOException;
    
}
