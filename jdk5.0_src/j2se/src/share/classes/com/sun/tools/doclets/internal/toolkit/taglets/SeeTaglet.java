/*
 * @(#)SeeTaglet.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;

/**
 * A taglet that represents the @see tag.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.4
 */
public class SeeTaglet extends BaseTaglet implements InheritableTaglet {
    
    public SeeTaglet() {
        name = "see";
    }
    
    /**
     * {@inheritDoc}
     */
    public void inherit(DocFinder.Input input, DocFinder.Output output) {
        Tag[] tags = input.method.seeTags();
        if (tags.length > 0) {
            output.holder = input.method;
            output.holderTag = tags[0];
            output.inlineTags = input.isFirstSentence ?
                tags[0].firstSentenceTags() : tags[0].inlineTags();
        }
    }
    
    /**
     * {@inheritDoc}
     */
    public TagletOutput getTagletOutput(Doc holder, TagletWriter writer) {
        SeeTag[] tags = holder.seeTags();
        if (tags.length == 0 && holder instanceof MethodDoc) {
            DocFinder.Output inheritedDoc = 
                DocFinder.search(new DocFinder.Input((MethodDoc) holder, this));
            if (inheritedDoc.holder != null) {
                tags = inheritedDoc.holder.seeTags();
            }            
        }
        return writer.seeTagOutput(holder, tags);
    }
}

