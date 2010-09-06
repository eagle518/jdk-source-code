/*
 * @(#)CodeTaglet.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */
package com.sun.tools.doclets.internal.toolkit.taglets;

import java.util.Map;
import com.sun.javadoc.Tag;

/**
 * An inline Taglet used to denote literal code fragments.
 * The enclosed text is interpreted as not containing HTML markup or
 * nested javadoc tags, and is rendered in a font suitable for code.
 *
 * <p> The tag {@code {@code ...}} is equivalent to
 * {@code <code>{@literal ...}</code>}.
 * For example, the text:
 * <blockquote>  The type {@code {@code List<P>}}  </blockquote>
 * displays as:
 * <blockquote>  The type {@code List<P>}  </blockquote>
 *
 * @author Scott Seligman
 * @version 1.2 03/12/19
 * @since 1.5
 */

public class CodeTaglet extends LiteralTaglet {

	private static final String NAME = "code";

	public static void register(Map map) {
		map.remove(NAME);
		map.put(NAME, new CodeTaglet());
	}

	public String getName() {
		return NAME;
	}

	/*
	 * Wraps @literal's result in a <code> element.
	 */
	public String toString(Tag tag) {
		return "<code>" + super.toString(tag) + "</code>";
	}
}
