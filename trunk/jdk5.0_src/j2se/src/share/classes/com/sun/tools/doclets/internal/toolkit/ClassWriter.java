/*
 * @(#)ClassWriter.java	1.4 03/12/19
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

public interface ClassWriter {
    
    /**
     * Write the header of the page.
     * @param header the header to write.
     */
    public void writeHeader(String header);
    
    /**
     * Write the class tree documentation.
     */
    public void writeClassTree();
    
    /**
     * Write all implemented interfaces if this is a class.
     */
    public void writeImplementedInterfacesInfo();
    
    /**
     * Write all super interfaces if this is an interface.
     */
    public void writeSuperInterfacesInfo();
    
    /**
     * Write the type parameter information.
     */
    public void writeTypeParamInfo();
    
    /**
     * Write all the classes that extend this one.
     */
    public void writeSubClassInfo();
    
    /**
     * Write all the interfaces that extend this one.
     */
    public void writeSubInterfacesInfo();
    
    /**
     * If this is an interface, write all classes that implement this
     * interface.
     */
    public void writeInterfaceUsageInfo ();
    
    /**
     * If this is an inner class or interface, write the enclosing class or
     * interface.
     */
    public void writeNestedClassInfo ();
    
    /**
     * If this class is deprecated, write the appropriate information.
     */
    public void writeClassDeprecationInfo ();
    
    /**
     * Write the signature of the current class.
     *
     * @param modifiers the modifiers for the signature.
     */
    public void writeClassSignature(String modifiers);  
    
    /**
     * Build the class description.
     */
    public void writeClassDescription();
    
    /**
     * Write the tag information for the current class.
     */
    public void writeClassTagInfo();
    
    /**
     * Write the footer of the page.
     */
    public void writeFooter();
    
    /**
     * Close the writer.
     */
    public void close() throws IOException;
    
    /**
     * Return the classDoc being documented.
     *
     * @return the classDoc being documented.
     */
    public ClassDoc getClassDoc();
    
    /**
     * Perform any operations that are necessary when the member summary
     * finished building.
     */
    public void completeMemberSummaryBuild();
}

