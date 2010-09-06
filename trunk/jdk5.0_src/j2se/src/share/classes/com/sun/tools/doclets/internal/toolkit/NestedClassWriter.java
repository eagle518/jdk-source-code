/*
 * @(#)NestedClassWriter.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit;

import java.io.*;
import com.sun.javadoc.*;

/**
 * The interface for writing class output.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */

public interface NestedClassWriter {
    
    /**
     * Write the classes summary header for the given class.
     *
     * @param nestedClass the class the summary belongs to.
     */
    public void writeNestedClassSummaryHeader(ClassDoc nestedClass);
    
    /**
     * Write the class summary for the given class and class.
     *
     * @param classDoc the class the summary belongs to.
     * @param nestedClass the nested class that I am summarizing.
     */
    public void writeNestedClassSummary(ClassDoc classDoc, ClassDoc nestedClass);
    
    /**
     * Write the classes summary footer for the given class.
     *
     * @param nestedClass the class the summary belongs to.
     */
    public void writeNestedClassSummaryFooter(ClassDoc nestedClass);
    
    /**
     * Write the inherited classes summary header for the given class.
     *
     * @param nestedClass the class the summary belongs to.
     */
    public void writeInheritedNestedClassSummaryHeader(ClassDoc nestedClass);
    
    /**
     * Write the inherited nested class summary for the given class and nested 
     * class.
     *
     * @param classDoc the class the inherited nested class belongs to.
     * @param nestedClass the inherited nested class that I am summarizing.
     * @param isFirst true if this is the first member in the list.
     */
    public void writeInheritedNestedClassSummary(ClassDoc classDoc, 
            ClassDoc nestedClass, boolean isFirst);
    
    /**
     * Write the inherited classes summary footer for the given class.
     *
     * @param nestedClass the class the summary belongs to.
     */
    public void writeInheritedNestedClassSummaryFooter(ClassDoc nestedClass);
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
}

