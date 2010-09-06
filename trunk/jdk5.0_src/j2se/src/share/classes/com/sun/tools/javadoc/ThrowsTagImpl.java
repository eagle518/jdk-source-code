/**
 * @(#)ThrowsTagImpl.java	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.javadoc.*;

/**
 * Represents a @throws or @exception documentation tag.
 * Parses and holds the exception name and exception comment.
 * The exception name my be the name of a type variable.
 * Note: @exception is a backwards compatible synonymy for @throws.
 * 
 * @author Robert Field
 * @author Atul M Dambalkar
 * @see ExecutableMemberDocImpl#throwsTags()
 *
 */
class ThrowsTagImpl extends TagImpl implements ThrowsTag {

    private final String exceptionName;
    private final String exceptionComment;

    ThrowsTagImpl(DocImpl holder, String name, String text) {
        super(holder, name, text);
	String[] sa = divideAtWhite();
	exceptionName = sa[0];
	exceptionComment = sa[1];
    }

    /**
     * Return the exception name.
     */
    public String exceptionName() {
        return exceptionName;
    }

    /**
     * Return the exception comment.
     */
    public String exceptionComment() {
        return exceptionComment;
    }

    /**
     * Return the exception as a ClassDocImpl.
     */
    public ClassDoc exception() {
	ClassDocImpl exceptionClass;
        if (!(holder instanceof ExecutableMemberDoc)) {
            exceptionClass = null;
        } else {
            ExecutableMemberDocImpl emd = (ExecutableMemberDocImpl)holder;
            ClassDocImpl con = (ClassDocImpl)emd.containingClass();
            exceptionClass = (ClassDocImpl)con.findClass(exceptionName);
        }
	return exceptionClass;
    }

    /**
     * Return the type that represents the exception.
     * This may be a <code>ClassDoc</code> or a <code>TypeVariable</code>.
     */
    public Type exceptionType() {
	//###(gj) TypeVariable not yet supported.
	return exception();
    }


    /**
     * Return the kind of this tag.  Always "@throws" for instances
     * of ThrowsTagImpl.
     */
    public String kind() {
        return "@throws";
    }

    /**
     * For the exception comment with embedded @link tags return the array of
     * TagImpls consisting of SeeTagImpl(s) and text containing TagImpl(s).
     *
     * @return TagImpl[] Array of tags with inline SeeTagImpls.
     * @see TagImpl#inlineTagImpls()
     * @see ParamTagImpl#inlineTagImpls()
     */
    public Tag[] inlineTags() {
        return Comment.getInlineTags(holder, exceptionComment());
    }
}


