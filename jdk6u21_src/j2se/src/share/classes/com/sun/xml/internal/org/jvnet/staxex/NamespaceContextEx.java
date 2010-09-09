/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package com.sun.xml.internal.org.jvnet.staxex;

import javax.xml.namespace.NamespaceContext;
import java.util.Iterator;

/**
 * Extended {@link NamespaceContext}.
 *
 * @author Kohsuke Kawaguchi
 * @author Paul Sandoz
 */
public interface NamespaceContextEx extends NamespaceContext, Iterable<NamespaceContextEx.Binding> {

    /**
     * Iterates all the in-scope namespace bindings.
     *
     * <p>
     * This method enumerates all the active in-scope namespace bindings.
     * This does not include implicit bindings, such as
     * <tt>"xml"->"http://www.w3.org/XML/1998/namespace"</tt>
     * or <tt>""->""</tt> (the implicit default namespace URI.)
     *
     * <p>
     * The returned iterator may not include the same prefix more than once.
     * For example, the returned iterator may only contain <tt>f=ns2</tt>
     * if the document is as follows and this method is used at the bar element.
     *
     * <pre><xmp>
     * <foo xmlns:f='ns1'>
     *   <bar xmlns:f='ns2'>
     *     ...
     * </xmp></pre>
     *
     * <p>
     * The iteration may be done in no particular order.
     *
     * @return
     *      may return an empty iterator, but never null.
     */
    Iterator<Binding> iterator();

    /**
     * Prefix to namespace URI binding.
     */
    interface Binding {
        /**
         * Gets the prefix.
         *
         * <p>
         * The default namespace URI is represented by using an
         * empty string "", not null.
         *
         * @return
         *      never null. String like "foo", "ns12", or "".
         */
        String getPrefix();

        /**
         * Gets the namespace URI.
         *
         * <p>
         * The empty namespace URI is represented by using
         * an empty string "", not null.
         *
         * @return
         *      never null. String like "http://www.w3.org/XML/1998/namespace",
         *      "urn:oasis:names:specification:docbook:dtd:xml:4.1.2", or "".
         */
        String getNamespaceURI();
    }
}
