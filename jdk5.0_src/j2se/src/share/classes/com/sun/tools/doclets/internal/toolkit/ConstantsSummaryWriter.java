/*
 * @(#)ConstantsSummaryWriter.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import com.sun.javadoc.*;
import java.util.*;
import java.io.*;

/**
 * The interface for writing constants summary output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface ConstantsSummaryWriter {
    
    /**
     * Write the header for the summary.
     */
    public abstract void writeHeader();
    
    /**
     * Write the footer for the summary.
     */
    public abstract void writeFooter();
    
    /**
     * Close the writer.
     */
    public abstract void close() throws IOException;
    
    /**
     * Write the header for the index.
     */
    public abstract void writeContentsHeader();
    
    /**
     * Write the footer for the index.
     */
    public abstract void writeContentsFooter();
    
    /**
     * Add the given package name to the index.
     * @param pkg                    the {@link PackageDoc} to index.
     * @param parsedPackageName      the parsed package name.  We only Write the 
     *                               first 2 directory levels of the package 
     *                               name. For example, java.lang.ref would be 
     *                               indexed as java.lang.*.
     * @param WriteedPackageHeaders the set of package headers that have already 
     *                              been indexed.  We don't want to index 
     *                              something more than once.
     */
    public abstract void writeLinkToPackageContent(PackageDoc pkg, String parsedPackageName,
        Set WriteedPackageHeaders);
    
    /**
     * Write the given package name.
     * @param pkg                    the {@link PackageDoc} to index.
     * @param parsedPackageName      the parsed package name.  We only Write the 
     *                               first 2 directory levels of the package 
     *                               name. For example, java.lang.ref would be 
     *                               indexed as java.lang.*.
     */
    public abstract void writePackageName(PackageDoc pkg, 
        String parsedPackageName);
    
    /**
     * Write the heading for the current table of constants for a given class.
     * @param cd the class whose constants are being documented.
     */
    public abstract void writeConstantMembersHeader(ClassDoc cd);
    
    /**
     * Document the given constants.
     * @param cd the class whose constants are being documented.
     * @param fields the constants being documented.
     */
    public abstract void writeConstantMembers(ClassDoc cd, List fields);
    
    /**
     * Document the given constants.
     * @param cd the class whose constants are being documented.
     */
    public abstract void writeConstantMembersFooter(ClassDoc cd);
    
}


