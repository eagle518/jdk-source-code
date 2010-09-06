/*
 * %W% %E%
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

import com.sun.tools.doclets.internal.toolkit.util.DocFinder;

/**
 * A taglet should implement this interface if it supports the inheritDoc
 * tag or is automatically inherited if it is missing.
 * 
 * @author Jamie Ho
 * @since 1.5 
 */
public interface InheritableTaglet extends Taglet {
    
    /**
     * Given an {@link com.sun.tools.doclets.internal.toolkit.util.DocFinder.Output} 
     * object, set its values with the appropriate information to inherit 
     * documentation.
     * 
     * @param input  the input for documentation search.
     * @param output the output for documentation search.
     */
    void inherit(DocFinder.Input input, DocFinder.Output output);
}