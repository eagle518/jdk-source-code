/*
 * @(#)InheritDocTaglet.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

import com.sun.javadoc.*;
import com.sun.tools.doclets.internal.toolkit.util.*;

/**
 * An inline Taglet representing the <b>inheritDoc</b> tag. This tag should only 
 * be used with a method.  It is used to inherit documentation from overriden 
 * and implemented methods.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.4
 */

public class InheritDocTaglet extends BaseInlineTaglet {
    
    /**
     * The inline tag that would appear in the documentation if
     * the writer wanted documentation to be inherited.
     */
    public static final String INHERIT_DOC_INLINE_TAG = "{@inheritDoc}";
    
    /**
     * Construct a new InheritDocTaglet.
     */
    public InheritDocTaglet () {
        name = "inheritDoc";
    }
    
    /**
     * Will return false because this inline tag may
     * only appear in Methods.
     * @return false since this is not a method.
     */
    public boolean inField() {
        return false;
    }
    
    /**
     * Will return false because this inline tag may
     * only appear in Methods.
     * @return false since this is not a method.
     */
    public boolean inConstructor() {
        return false;
    }
    
    /**
     * Will return false because this inline tag may
     * only appear in Methods.
     * @return false since this is not a method.
     */
    public boolean inOverview() {
        return false;
    }

    /**
     * Will return false because this inline tag may
     * only appear in Methods.
     * @return false since this is not a method.
     */
    public boolean inPackage() {
        return false;
    }

    /**
     * Will return false because this inline tag may
     * only appear in Methods.
     * @return false since this is not a method.
     */
    public boolean inType() {
        return false;
    }
    
    /**
     * Given a <code>MethodDoc</code> item, a <code>Tag</code> in the
     * <code>MethodDoc</code> item and a String, replace all occurances
     * of @inheritDoc with documentation from it's superclass or superinterface.
     *
     * @param writer the writer that is writing the output.
     * @param md the {@link MethodDoc} that we are documenting.
     * @param holderTag the tag that holds the inheritDoc tag.
     * @param isFirstSentence true if we only want to inherit the first sentence.
     */
    private TagletOutput retrieveInheritedDocumentation(TagletWriter writer,
            MethodDoc md, Tag holderTag, boolean isFirstSentence) {
        TagletOutput replacement = writer.getTagletOutputInstance();
        
        Taglet inheritableTaglet = holderTag == null ? 
            null : writer.configuration().tagletManager.getTaglet(holderTag.name());
        if (inheritableTaglet != null && 
            !(inheritableTaglet instanceof InheritableTaglet)) {
                //This tag does not support inheritence.
                writer.configuration().message.warning(md.position(), 
                "doclet.noInheritedDoc", md.name() + md.flatSignature());
         }
        DocFinder.Output inheritedDoc = 
            DocFinder.search(new DocFinder.Input(md, 
                (InheritableTaglet) inheritableTaglet, holderTag, 
                isFirstSentence, true));
        if (inheritedDoc.isValidInheritDocTag == false) {
            writer.configuration().message.warning(md.position(), 
                "doclet.noInheritedDoc", md.name() + md.flatSignature());
        } else if (inheritedDoc.inlineTags.length > 0) {
            replacement = writer.commentTagsToOutput(inheritedDoc.holderTag, 
                inheritedDoc.holder, inheritedDoc.inlineTags);
        }
        return replacement;
    }

    /**
     * Given the <code>Tag</code> representation of this custom
     * tag, return its string representation, which is output
     * to the generated page.
     * @param tag the <code>Tag</code> representation of this custom tag.
     * @param tagletWriter the taglet writer for output.
     * @return the TagletOutput representation of this <code>Tag</code>.
     */
    public TagletOutput getTagletOutput(Tag tag, TagletWriter tagletWriter) {
        if (! (tag.holder() instanceof MethodDoc)) {
            return tagletWriter.getOutputInstance();
        }
        return tag.name().equals("@inheritDoc") ?
                retrieveInheritedDocumentation(tagletWriter, (MethodDoc) tag.holder(), null, tagletWriter.isFirstSentence) :
                retrieveInheritedDocumentation(tagletWriter, (MethodDoc) tag.holder(), tag, tagletWriter.isFirstSentence);
    }
}
