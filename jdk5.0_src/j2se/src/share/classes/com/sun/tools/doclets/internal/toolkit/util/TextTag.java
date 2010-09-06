/*
 * @(#)TextTag.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.tools.doclets.internal.toolkit.util;
import com.sun.javadoc.*;


/**
 * A tag that holds nothing but plain text.  This is useful for passing
 * text to methods that only accept inline tags as a parameter.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */
public class TextTag implements Tag {
    protected final String text;
    protected final String name = "Text";
    protected final Doc holder;

    /**
     *  Constructor
     */
    public TextTag(Doc holder, String text) {
        super();
        this.holder = holder;
        this.text = text;
    }

    /**
     * {@inheritDoc}
     */
    public String name() {
        return name;
    }
    
    /**
     * {@inheritDoc}
     */
    public Doc holder() {
        return holder;
    }

    /**
     * {@inheritDoc}
     */
    public String kind() {
        return name;
    }

    /**
     * {@inheritDoc}
     */
    public String text() {
        return text;
    }

    /**
     * {@inheritDoc}
     */
    public String toString() {
        return name + ":" + text;
    }

    /**
     * {@inheritDoc}
     */
    public Tag[] inlineTags() {
        return new Tag[] {this};
    }

    /**
     * {@inheritDoc}
     */
    public Tag[] firstSentenceTags() {
        return new Tag[] {this};
    }

    /**
     * {@inheritDoc}
     */
    public SourcePosition position() {
        return holder.position();
    }
}
