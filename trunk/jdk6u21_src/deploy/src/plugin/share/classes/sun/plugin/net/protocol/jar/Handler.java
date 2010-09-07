/*
 * @(#)Handler.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.jar;

import java.net.URL;
import java.io.IOException;

/*
 * Jar URL Handler for plugin
 */
public class Handler extends sun.net.www.protocol.jar.Handler {
    protected java.net.URLConnection openConnection(URL u) throws IOException {
        return new CachedJarURLConnection(u, this);
    }
}
