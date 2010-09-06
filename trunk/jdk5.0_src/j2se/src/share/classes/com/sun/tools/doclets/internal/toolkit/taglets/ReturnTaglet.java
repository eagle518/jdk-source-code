/*
 * @(#)ReturnTaglet.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.taglets;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;

/**
 * A taglet that represents the @return tag.
 * 
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.4
 */
public class ReturnTaglet extends BaseExecutableMemberTaglet 
        implements InheritableTaglet {
    
    public ReturnTaglet() {
        name = "return";
    }
    
    /**
     * {@inheritDoc}
     */
    public void inherit(DocFinder.Input input, DocFinder.Output output) {
       Tag[] tags = input.method.tags("return");
        if (tags.length > 0) {
            output.holder = input.method;
            output.holderTag = tags[0];
            output.inlineTags = input.isFirstSentence ?
                tags[0].firstSentenceTags() : tags[0].inlineTags();
        } 
    }
    
    /**
     * Return true if this <code>Taglet</code>
     * is used in constructor documentation.
     * @return true if this <code>Taglet</code>
     * is used in constructor documentation and false
     * otherwise.
     */
    public boolean inConstructor() {
        return false;
    }
    
    /**
     * {@inheritDoc}
     */
    public TagletOutput getTagletOutput(Doc holder, TagletWriter writer) {
        Type returnType = ((MethodDoc) holder).returnType();        
        Tag[] tags = holder.tags(name);
        
        //Make sure we are not using @return tag on method with void return type.      
        if (returnType.isPrimitive() && returnType.typeName().equals("void")) {
            if (tags.length > 0) {            
                writer.getMsgRetriever().warning(holder.position(), 
                    "doclet.Return_tag_on_void_method");
            }
            return null;
        }
        //Inherit @return tag if necessary.
        if (tags.length == 0) {
            DocFinder.Output inheritedDoc = 
                DocFinder.search(new DocFinder.Input((MethodDoc) holder, this));
            tags = inheritedDoc.holderTag == null ? tags : new Tag[] {inheritedDoc.holderTag};
        }
        return tags.length > 0 ? writer.returnTagOutput(tags[0]) : null;
    }
}
