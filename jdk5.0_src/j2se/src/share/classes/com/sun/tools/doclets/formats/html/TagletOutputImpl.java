/*
 * @(#)TagletOutputImpl.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.taglets.*;

/**
 * The output for HTML taglets.
 *
 * @since 1.5
 * @author Jamie Ho
 */

public class TagletOutputImpl implements TagletOutput {
    
    private StringBuffer output;
    
    public TagletOutputImpl(String o) {
        setOutput(o);
    }
    
    /**
     * {@inheritDoc}
     */
    public void setOutput (Object o) {
        output = new StringBuffer(o == null ? "" : (String) o);
    }
    
    /**
     * {@inheritDoc}
     */
    public void appendOutput(TagletOutput o) {
        output.append(o.toString());
    }
    
    /**
     * {@inheritDoc}
     */
    public boolean hasInheritDocTag() {
        return output.indexOf(InheritDocTaglet.INHERIT_DOC_INLINE_TAG) != -1;
    }
    
    public String toString() {
        return output.toString();
    }
    
}
