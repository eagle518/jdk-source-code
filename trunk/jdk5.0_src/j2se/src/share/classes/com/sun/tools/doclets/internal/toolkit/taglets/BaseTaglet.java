/*
 * @(#)BaseTaglet.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;

/**
 * An abstract class for that implements the {@link Taglet} interface.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.4
 */
public abstract class BaseTaglet implements Taglet {
    
    protected String name = "Default";
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in constructor documentation.
     * @return true if this <code>Taglet</code>
     * is used in constructor documentation and false
     * otherwise.
     */
    public boolean inConstructor() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in field documentation.
     * @return true if this <code>Taglet</code>
     * is used in field documentation and false
     * otherwise.
     */
    public boolean inField() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in method documentation.
     * @return true if this <code>Taglet</code>
     * is used in method documentation and false
     * otherwise.
     */
    public boolean inMethod() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in overview documentation.
     * @return true if this <code>Taglet</code>
     * is used in method documentation and false
     * otherwise.
     */
    public boolean inOverview() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in package documentation.
     * @return true if this <code>Taglet</code>
     * is used in package documentation and false
     * otherwise.
     */
    public boolean inPackage() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in type documentation (classes or interfaces).
     * @return true if this <code>Taglet</code>
     * is used in type documentation and false
     * otherwise.
     */
    public boolean inType() {
        return true;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is an inline tag.
     * @return true if this <code>Taglet</code>
     * is an inline tag and false otherwise.
     */
    public boolean isInlineTag() {
        return false;
    }
    
    /**
     * Return the name of this custom tag.
     * @return the name of this custom tag.
     */
    public String getName() {
        return name;
    }
    
    /**
     * {@inheritDoc}
     * @throws IllegalArgumentException thrown when the method is not supported by the taglet.
     */
    public TagletOutput getTagletOutput(Tag tag, TagletWriter writer) {
        throw new IllegalArgumentException("Method not supported in taglet " + getName() + ".");
    }
    
    /**
     * {@inheritDoc}
     * @throws IllegalArgumentException thrown when the method is not supported by the taglet.
     */
    public TagletOutput getTagletOutput(Doc holder, TagletWriter writer) {
        throw new IllegalArgumentException("Method not supported in taglet " + getName() + ".");
    }
}
