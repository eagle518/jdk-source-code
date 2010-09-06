/*
 * @(#)LegacyTaglet.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

import com.sun.javadoc.*;

/**
 * This taglet acts as a wrapper to enable
 * {@link com.sun.tools.doclets.Taglet} type taglets to work
 * with the current version of Javadoc.
 * Note: this taglet only works with legacy taglets (those compatible with
 * Javadoc 1.4.x) that writes strings.
 * This taglet is able to wrap most most legacy taglets because
 * the standard doclet is the only known doclet to use legacy taglets.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @since 1.5
 * @author Jamie Ho
 */

public class LegacyTaglet implements Taglet {
    
    private com.sun.tools.doclets.Taglet legacyTaglet;
    
    public LegacyTaglet(com.sun.tools.doclets.Taglet t) {
        legacyTaglet = t;
    }
    
    /**
     * {@inheritDoc}
     */
    public boolean inField() {
        return legacyTaglet.isInlineTag() || legacyTaglet.inField();
    }
    
    /**
     * {@inheritDoc}
     */
    public boolean inConstructor() {
        return legacyTaglet.isInlineTag() || legacyTaglet.inConstructor();
    }
    
    /**
     * {@inheritDoc}
     */
    public boolean inMethod() {
        return legacyTaglet.isInlineTag() || legacyTaglet.inMethod();
    }

    /**
     * {@inheritDoc}
     */
    public boolean inOverview() {
        return legacyTaglet.isInlineTag() || legacyTaglet.inOverview();
    }
    
    /**
     * {@inheritDoc}
     */
    public boolean inPackage() {
        return legacyTaglet.isInlineTag() || legacyTaglet.inPackage();
    }

    /**
     * {@inheritDoc}
     */
    public boolean inType() {
        return legacyTaglet.isInlineTag() || legacyTaglet.inType();
    }

    /**
     * Return true if this <code>Taglet</code>
     * is an inline tag.
     * @return true if this <code>Taglet</code>
     * is an inline tag and false otherwise.
     */
    public boolean isInlineTag() {
        return legacyTaglet.isInlineTag();
    }
    
    /**
     * {@inheritDoc}
     */
    public String getName() {
        return legacyTaglet.getName();
    }
    
    /**
     * {@inheritDoc}
     */
    public TagletOutput getTagletOutput(Tag tag, TagletWriter writer) 
            throws IllegalArgumentException {
        TagletOutput output = writer.getOutputInstance();
        output.setOutput(legacyTaglet.toString(tag));
        return output;
    }

    /**
     * {@inheritDoc}
     */
    public TagletOutput getTagletOutput(Doc holder, TagletWriter writer) 
            throws IllegalArgumentException {
        TagletOutput output = writer.getOutputInstance();
        output.setOutput(legacyTaglet.toString(holder.tags(getName())));
        return output;
    }
}

