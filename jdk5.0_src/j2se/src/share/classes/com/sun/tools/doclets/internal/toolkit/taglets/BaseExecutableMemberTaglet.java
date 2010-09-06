/*
 * @(#)BaseExecutableMemberTaglet.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

/**
 * An abstract class for that implements the {@link Taglet} interface
 * for tags in <code>ExecutableMembers</code>.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API.
 * 
 * @author Jamie Ho
 * @since 1.4
 */
public abstract class BaseExecutableMemberTaglet extends BaseTaglet {
        
    /**
     * Return true if this <code>Taglet</code>
     * is used in field documentation.
     * @return true if this <code>Taglet</code>
     * is used in field documentation and false
     * otherwise.
     */
    public boolean inField() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in overview documentation.
     * @return true if this <code>Taglet</code>
     * is used in overview documentation and false
     * otherwise.
     */
    public boolean inOverview() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in package documentation.
     * @return true if this <code>Taglet</code>
     * is used in package documentation and false
     * otherwise.
     */
    public boolean inPackage() {
        return false;
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in type documentation (classes or interfaces).
     * @return true if this <code>Taglet</code>
     * is used in type documentation and false
     * otherwise.
     */
    public boolean inType() {
        return false;
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
}
