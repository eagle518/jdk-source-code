/*
 * @(#)JarImageSource.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.jar;

import sun.awt.image.URLImageSource;
import sun.awt.image.ImageDecoder;
import java.net.URL;
import java.net.JarURLConnection;
import java.util.jar.JarFile;
import java.util.jar.JarEntry;
import java.io.InputStream;
import java.io.IOException;


public class JarImageSource extends URLImageSource {
    String mimeType;
    String entryName = null;
    URL url;
    
    /**
     * Create an image source from a Jar entry URL with the specified
     * mime type.
     */
    public JarImageSource(URL u, String type) {
	super(u);
        url = u;
	mimeType = type;
    }

    /**
     * Create an image source from a Jar file/entry URL 
     * with the specified entry name and mime type.
     */
    public JarImageSource(URL u, String name, String type) {
        this(u, type);
        this.entryName = name;
    }

    protected ImageDecoder getDecoder() {
        InputStream is = null;
        try {
            JarURLConnection c = (JarURLConnection)url.openConnection();
            JarFile f = c.getJarFile();
            JarEntry e = c.getJarEntry();
        
            if (entryName != null && e == null) {
                e = f.getJarEntry(entryName);
            } 
            if (e == null || (e != null && entryName != null 
                              && (!(entryName.equals(e.getName()))))) {
                return null;
            }
            is = f.getInputStream(e);
        } catch (IOException e) {
	    return null;
        }
        
	ImageDecoder id = decoderForType(is, mimeType);
	if (id == null) {
	    id = getDecoder(is);
	}
	return id;
    }
}
