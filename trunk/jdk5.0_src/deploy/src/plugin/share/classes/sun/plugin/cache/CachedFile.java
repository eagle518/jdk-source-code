/*
 * @(#)CachedFile.java	1.3 02/04/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.plugin.cache;

import java.io.File;
import sun.net.www.MessageHeader;
import java.net.URL;

public class CachedFile extends File {
    MessageHeader headers;
    URL url;
    
    public CachedFile(File f, MessageHeader headers, URL url) {
	super(f.getPath());
	this.headers = headers;
	this.url = url;
    }

    public MessageHeader getHeaderFields(){
	return headers;
    }

    public URL getURL(){
	return url;
    }
}

