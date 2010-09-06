/*
 * @(#)LinkInfo.java	1.4 04/04/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.util.links;

import com.sun.javadoc.*;
import com.sun.tools.doclets.internal.toolkit.Configuration;

/**
 * Encapsulates information about a link.
 *
 * @author Jamie Ho
 * @since 1.5
 */
public abstract class LinkInfo {
    
    /**
     * The ClassDoc we want to link to.  Null if we are not linking
     * to a ClassDoc.
     */
    public ClassDoc classDoc;
    
    /**
     * The executable member doc we want to link to.  Null if we are not linking
     * to an executable member.
     */
    public ExecutableMemberDoc executableMemberDoc;
    
    /**
     * The Type we want to link to.  Null if we are not linking to a type.
     */
    public Type type;
    
    /**
     * True if this is a link to a VarArg.
     */
    public boolean isVarArg = false;
    
    /**
     * Set this to true to indicate that you are linking to a type parameter.
     */
    public boolean isTypeBound = false;
    
    /**
     * The label for the link.
     */
    public String label;
    
    /**
     * True if the link should be bolded.
     */
    public boolean isBold = false;
    
    /**
     * True if we should include the type in the link label.  False otherwise.
     */
    public boolean includeTypeInClassLinkLabel = true;
    
    /**
     * True if we should include the type as seperate link.  False otherwise.
     */
    public boolean includeTypeAsSepLink = false;
    
    /**
     * True if we should exclude the type bounds for the type parameter.
     */
    public boolean excludeTypeBounds = false;
    
    /**
     * True if we should print the type parameters, but not link them.
     */
    public boolean excludeTypeParameterLinks = false;
    
    /**
     * True if we should print the type bounds, but not link them.
     */
    public boolean excludeTypeBoundsLinks = false;
    
    /**
     * The display length for the link.
     */
    public int displayLength = 0;
    
    /**
     * Return the id indicating where the link appears in the documentation.
     * This is used for special processing of different types of links.
     *
     * @return the id indicating where the link appears in the documentation.
     */
    public abstract int getContext();
    
    /**
     * Set the context.
     *
     * @param c the context id to set.
     */
    public abstract void setContext(int c);
    
    /**
     * Return true if this link is linkable and false if we can't link to the
     * desired place.
     *
     * @return true if this link is linkable and false if we can't link to the
     * desired place.
     */
    public abstract boolean isLinkable();
    
    /**
     * Return the label for this class link.
     *
     * @param configuration the current configuration of the doclet.
     * @return the label for this class link.
     */
    public String getClassLinkLabel(Configuration configuration) {
        if (label != null && label.length() > 0) {
            return label;
        } else if (isLinkable()) {
            return classDoc.name();
        } else {
            return configuration.getClassName(classDoc);
        }
    }
}

