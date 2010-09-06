/*
 * @(#)Handler.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.jar;

import java.net.URL;
import java.io.IOException;

/*
 * Jar URL Handler for plugin
 */
public class Handler extends sun.net.www.protocol.jar.Handler 
{
    protected java.net.URLConnection openConnection(URL u)
    throws IOException {
        return new CachedJarURLConnection(u, this);
    }
}
