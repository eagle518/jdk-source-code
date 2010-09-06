/*
 * @(#)URLUtil.java	1.22 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.util;
import java.util.BitSet;
import java.io.InputStream;
import java.io.IOException;
import java.io.File;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

/** Handy class to add some utility methods for dealing
 *  with URLs
 */
public class URLUtil {
   
    static BitSet encodedInPath;

    static {
        encodedInPath = new BitSet(256);

        // Set the bits corresponding to characters that are encoded in the
        // path component of a URI.

        // These characters are reserved in the path segment as described in
        // RFC2396 section 3.3.
        encodedInPath.set('=');
        encodedInPath.set(';');
        encodedInPath.set('?');
        encodedInPath.set('/');

        // These characters are defined as excluded in RFC2396 section 2.4.3
        // and must be escaped if they occur in the data part of a URI.
        encodedInPath.set('#');
        encodedInPath.set(' ');
        encodedInPath.set('<');
        encodedInPath.set('>');
        encodedInPath.set('%');
        encodedInPath.set('"');
        encodedInPath.set('{');
        encodedInPath.set('}');
        encodedInPath.set('|');
        encodedInPath.set('\\');
        encodedInPath.set('^');
        encodedInPath.set('[');
        encodedInPath.set(']');
        encodedInPath.set('`');

        // US ASCII control characters 00-1F and 7F.
        for (int i=0; i<32; i++)
            encodedInPath.set(i);
        encodedInPath.set(127);
    }


    public static void setHostHeader(URLConnection urlc) {

	int port = urlc.getURL().getPort();
	String host = urlc.getURL().getHost();
	if (port != -1 && port != 80) {
	    host += ":" + String.valueOf(port);
	}
	urlc.setRequestProperty("Host", host);

    }
     
    /** Given a URL to a resource, returns a URL that strips
     *  off the name part of the URL. For example,
     *  Given:
     *   http://www.mysite.com/apps/klat.jar
     *  Returns:
     *   http://www.mysite.com/apps/
     */
    public static URL getBase(URL url) {
        if (url == null) return null;
        String file = url.getFile();
        if (file != null) {
	    int idx = file.lastIndexOf('/');
	    if (idx != -1 ) {
		file = file.substring(0, idx + 1);
	    }
	    try {
		return new URL(
		    url.getProtocol(),
		    url.getHost(),
		    url.getPort(),
		    file);
	    } catch(MalformedURLException mue) {
		// Should not happen
		Trace.ignoredException(mue);
	    }
        }
        // Just return same URL
        return url;
    }
    
    /** Makes sure a URL is a path URL, i.e., ends with '/' */
    public static URL asPathURL(URL url) {
        if (url == null) return null;
        
        String path = url.getFile();
        if (path != null && !path.endsWith("/")) {
	    try {
		return new URL(url.getProtocol(),
			       url.getHost(),
			       url.getPort(),
			       url.getFile() + "/");
	    } catch(MalformedURLException mue) {
		// Should not happen
	    }
        }
        // Just return same URl
        return url;
    }
    
    /** Compares two URLs without using URL.equals since it requires DNS
     *  lookup and will not work offline
     */
    public static boolean equals(URL u1, URL u2) {
	if (u1 == null || u2 == null) return (u2 == u1);
	// Comparing HTTP urls, make sure -1 is interpreted as 80
	if ("http".equals(u1.getProtocol()) && "http".equals(u2.getProtocol()) &&
	    u1.getPort() != u2.getPort()) {
	    u1 = normalizePort(u1);
	    u2 = normalizePort(u2);
	}
	return u1.toString().equals(u2.toString());
    }
    
    public static int compareTo(URL u1, URL u2) {
	// Comparing HTTP urls, make sure -1 is interpreted as 80
	if ("http".equals(u1.getProtocol()) && "http".equals(u2.getProtocol()) &&
	    u1.getPort() != u2.getPort()) {
	    u1 = normalizePort(u1);
	    u2 = normalizePort(u2);
	}
	return u1.toString().compareTo(u2.toString());
    }
    
    private static URL normalizePort(URL url) {
	if (url.getPort() != -1) return url;
	try {
	    return new URL(url.getProtocol(), url.getHost(), 80, url.getFile());
	} catch(MalformedURLException mue) {
	    return url;
	}
    }

    //  
    // although a year ago I put in the above "don't encode here", I
    // know from the JCP comments to bug 4754763 that we must.
    // this is only called from JNLPClassPath.JarLoader to construct a
    // file url to a file in the cache.  although the directory structure
    // we create cannot contain any illegal characters (for a url) the 
    // top lecel cache directory can (as, for example, the default on many
    // windows systems: /C:/Documents and Settings/user/.javaws/cache/...
    // we might find any char there. (from sun.net.parseUtil)
    //
    public static String encodePath(String path) {
        StringBuffer sb = new StringBuffer();
        int n = path.length();
        for (int i=0; i<n; i++) {
            char c = path.charAt(i);
            if (c == File.separatorChar)
                sb.append('/');
            else {
                if (c <= 0x007F) {
                    if (encodedInPath.get(c))
                        escape(sb, c);
                    else
                        sb.append(c);
                } else if (c > 0x07FF) {
                    escape(sb, (char)(0xE0 | ((c >> 12) & 0x0F)));
                    escape(sb, (char)(0x80 | ((c >>  6) & 0x3F)));
                    escape(sb, (char)(0x80 | ((c >>  0) & 0x3F)));
                } else {
                    escape(sb, (char)(0xC0 | ((c >>  6) & 0x1F)));
                    escape(sb, (char)(0x80 | ((c >>  0) & 0x3F)));
                }
            }
        }

	if (!path.equals(sb.toString())) {
	    Trace.println("     String: "+path, TraceLevel.BASIC);
	    Trace.println(" encoded to: "+sb.toString(), TraceLevel.BASIC);
	}

        return sb.toString();
    }
    private static void escape(StringBuffer s, char c) {
        s.append('%');
        s.append(Character.forDigit((c >> 4) & 0xF, 16));
        s.append(Character.forDigit(c & 0xF, 16));
    }

    //
    // Returns a new String constructed from the specified String by replacing
    // the URL escape sequences and UTF8 encoding with the characters they
    // represent.
    //  
    public static String decodePath(String path) {
        StringBuffer sb = new StringBuffer();
        int i=0;
        while (i<path.length()) {
            char c = path.charAt(i);
            char c2, c3;
            if (c != '%') {
                i++;
            } else {
                try {
                    c = unescape(path, i);
                    i += 3;
                    if ((c & 0x80) != 0) {
                        switch (c >> 4) {
                            case 0xC: case 0xD:
                                c2 = unescape(path, i);
                                i += 3;
                                c = (char)(((c & 0x1f) << 6) | (c2 & 0x3f));
                                break;
                            case 0xE:
                                c2 = unescape(path, i);
                                i += 3;
                                c3 = unescape(path, i);
                                i += 3;
                                c = (char)(((c & 0x0f) << 12) |
                                           ((c2 & 0x3f) << 6) |
                                            (c3 & 0x3f));
                                break;
                            default:
                                Trace.ignoredException(
					 new IllegalArgumentException());
                        }
                    }
                } catch (NumberFormatException e) {
                    Trace.ignoredException(e);
                }
            }
            sb.append(c);
        }
        
	if (!path.equals(sb.toString())) {
	    Trace.println("     String: "+path, TraceLevel.BASIC);
	    Trace.println(" decoded to: "+sb.toString(), TraceLevel.BASIC);
	}
        
        return sb.toString();
    }

    private static char unescape(String s, int i) {
        return (char) Integer.parseInt(s.substring(i+1,i+3),16);
    }

    // used in JarLoader in JNLPClassPath:
    public static String getEncodedPath(File file) {
	String path = file.getAbsolutePath();
	if (!path.endsWith(File.separator) && file.isDirectory()) {
	    path = path + File.separator;
	}
	return encodePath(path);
    }

    // decode here - we curently arn't using this except below
    public static String getDecodedPath(URL url) {
	String path = url.getFile();
	path = path.replace('/', File.separatorChar);
	return decodePath(path);
    }
    
    //  decode cache dir here, leave alone above it.
    public static String getPathFromURL(URL url) {
	return getDecodedPath(url);
    }
}


