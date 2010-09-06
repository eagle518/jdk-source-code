/*
 * @(#)TagletOutput.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

/**
 * The interface for taglet output.  This interface is needed because
 * different doclets work with different formats of output.  A single taglet can
 * work with any doclet that provides an implementation of taglet output.
 *
 * @author Jamie Ho
 * @since 1.5
 */
public interface TagletOutput {
    
    /**
     * Set the output for the taglet.
     * @param o an object representing the output.
     */
    public abstract void setOutput(Object o);
    
    /**
     * Append the given output to this output.
     * @param o a TagletOutput representing the output.
     */
    public abstract void appendOutput(TagletOutput o);
    
    /**
     * Return true if this output has any occurances of @inheritDoc.
     * @return true if inheritDoc tag is found.
     */
    public abstract boolean hasInheritDocTag();
}
