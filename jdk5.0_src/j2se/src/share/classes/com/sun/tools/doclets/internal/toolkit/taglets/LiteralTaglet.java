/*
 * @(#)LiteralTaglet.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */
package com.sun.tools.doclets.internal.toolkit.taglets;

import java.util.Map;
import com.sun.javadoc.Tag;
import com.sun.tools.doclets.Taglet;


/**
 * An inline Taglet used to denote literal text.
 * The enclosed text is interpreted as not containing HTML markup or
 * nested javadoc tags.
 * For example, the text:
 * <blockquote>  {@code {@literal a<B>c}}  </blockquote>
 * displays as:
 * <blockquote>  {@literal a<B>c}  </blockquote>
 *
 * @author Scott Seligman
 * @version 1.2 03/12/19
 * @since 1.5
 */

public class LiteralTaglet implements Taglet {

    private static final String NAME = "literal";

    public static void register(Map map) {
	   map.remove(NAME);
	   map.put(NAME, new LiteralTaglet());
    }

    public String getName() {
	return NAME;
    }

    public String toString(Tag tag) {
	return textToString(tag.text());
    }

    public String toString(Tag[] tags) { return null; }

    public boolean inField() { return false; }

    public boolean inConstructor() { return false; }

    public boolean inMethod() { return false; }

    public boolean inOverview() { return false; }

    public boolean inPackage() { return false; }

    public boolean inType() { return false; }

    public boolean isInlineTag() { return true; }

    /*
     * Replace occurrences of the following characters:  < > &
     */
    protected static String textToString(String text) {
	   StringBuffer buf = new StringBuffer();
	   for (int i = 0; i < text.length(); i++) {
	       char c = text.charAt(i);
	       switch (c) {
	           case '<':
		          buf.append("&lt;");
		          break;
	           case '>':
		          buf.append("&gt;");
		          break;
	           case '&':
		          buf.append("&amp;");
		          break;
	           default:
		          buf.append(c);
	       }
	   }
	   return buf.toString();
    }
}
