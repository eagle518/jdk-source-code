/*
 * @(#)LinkOutput.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.util.links;

/**
 * Stores output of a link.
 *
 * @author Jamie Ho
 * @since 1.5
 */
public interface LinkOutput {
    
    /**
     * Append the given object to the output.
     *
     * @param o the object to append.
     */
    public void append(Object o);
}

