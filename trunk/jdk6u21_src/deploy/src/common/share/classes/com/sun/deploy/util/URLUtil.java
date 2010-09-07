/*
 * @(#)URLUtil.java	1.32 10/05/20
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.util;
import java.util.BitSet;
import java.io.InputStream;
import java.io.IOException;
import java.io.File;
import java.net.URL;
import java.net.URLConnection;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.WeakHashMap;
import com.sun.deploy.config.Config;

/*
 *  Handy class to add some utility methods for dealing
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

    // return true if URL is a UNC file url (file url with hostname)
    public static boolean isUNCFileURL(URL u) {
        if (u == null || u.getProtocol().equalsIgnoreCase("file") == false) {
            return false;
        }
        String urlString = u.toString();
        // Internet Explorer uses "////" for file url, which
        // java does not understand
        // Convert it back to "//" for file url
        // Mozilla Firefox does not allow file docuement base
        urlString = urlString.replaceAll("////", "//");
        try {
            URL javaUrl = new URL(urlString);
            if (javaUrl.getHost().equals("")) {
                // no hostname
                return false;
            }
        } catch (Exception e) {
            // should not happen
        }
        return true;
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


    // Use a map to store canonicalized URL for performance
    // reasons.
    private static WeakHashMap canonicalizedURLMap = new WeakHashMap();

    /**
     * Canonicalize URL
     */
    public synchronized static String canonicalize(String url)
    {
	/*
	 * Force the docbase for Internet Explorer mounted drive
	 * applet display. In IE, when displaying html pages from
	 * a mounted the drive, the document base will not be
	 *
	 *	    file://<drive>:\...
	 * but
	 *	    file://///<server>\<mount_name>\...
	 *	    file:///\\<server>\<mount_name>\...
	 *  	    file://<server>/<mount_name>\...
	 *  	    file:\\<server>\<mount_name>\...
	 *
	 * which causes the failure to load ressources or classes from
	 * a jar file. We must mangle the URL to the correct setting
	 *
	 * In the other cases, somehow IE will pass us a bogus file 
	 * URL in the form of
	 *	    file://C:\directory\file.html
	 *
         * We MUST convert it into proper form
	 */

	if (url.indexOf("file:") == -1)
	{
	    // Do nothing if this is not a file URL
	    return url;
	}    


	// Obtain canonicalized URL from map only if it is
	// file URL
	// 
	String value = (String) canonicalizedURLMap.get(url);
    
	if (value != null)
	    return value;


	// Begin canonicalization
	//
	StringBuffer urlString = new StringBuffer();

	if (url.indexOf("file://///") == 0)
	{
	    // The URL is in the form:
	    //
	    //      file://///<server>\<mount_name>\...
	    //
	    urlString.append("file:////");
	    urlString.append(url.substring(10));
	}
	else if (url.indexOf("file:///\\") == 0)
	{
	    // The URL is in the form:
	    //
	    //      file:///\\<server>\<mount_name>\...
	    //
	    urlString.append("file:////");
	    urlString.append(url.substring(9));
	}
	else if (url.indexOf("file://\\") == 0)
	{
	    // The URL is in the form:
	    //
	    //      file://\\<server>\<mount_name>\...
	    //
	    urlString.append("file:////");
	    urlString.append(url.substring(9));
	}
	else if (url.indexOf("file:\\") == 0)
	{
	    // Check if the URL is in the form
	    //
	    //	    file:\\C:\directory\file.html
	    // or
	    // 	    file:\\<server>\<mount_name>\...
	    //

	    if (url.indexOf(':', 6) != -1 || url.indexOf('|', 6) != -1)
		urlString.append("file:///");
	    else
		urlString.append("file:////");

	    urlString.append(url.substring(6));
	}
	else if (url.indexOf("file://") == 0 &&
	         url.charAt(7) != '/')		    // skip "file:///C:/..."
	{	    
	    // Check if the URL is in the form
	    //
	    //	    file://C:\directory\file.html
	    // or
	    // 	    file://<server>/<mount_name>\...
	    //

	    if (url.indexOf(':', 7) != -1 || url.indexOf('|', 7) != -1)
		urlString.append("file:///");
	    else
		urlString.append("file:////");

	    urlString.append(url.substring(7));
	}
        else
	{
	    urlString.append(url);
	} 
	              
        // Search for '\' and replace it with '/'
	// Search for first '|' and replace it with ':'
	//
	boolean found = false;

        for (int i=0; i < urlString.length(); i++)
        {
	    char c = urlString.charAt(i);

            if (c == '\\')
	    {
                urlString.setCharAt(i, '/');
	    }
	    else if (!found && c == '|')
	    {
		// only replace the first "|"
		urlString.setCharAt(i, ':');
		found = true;
	    }
        }

	// Store canonicalized URL into map
	//
	String canonicalizedURL = urlString.toString();

	canonicalizedURLMap.put(url, canonicalizedURL);

	return canonicalizedURL;
    }
    
    /**
     * Canonicalize a document base URL.
     */
    public static String canonicalizeDocumentBaseURL(String url) {
        int fromIndex=-1,lastIndex;
        
        // Strip off "#" and "?" from URL
        int fragmentIndex = url.indexOf('#');
        int queryIndex = url.indexOf('?');
        
        if(queryIndex != -1 && fragmentIndex != -1) {
            fromIndex = Math.min(fragmentIndex, queryIndex);
        } else if(fragmentIndex != -1) {
            fromIndex = fragmentIndex;
        } else if(queryIndex != -1) {
            fromIndex = queryIndex;
        }
        
        // Strip off the end of the URL
        String strippedURL;
        
        if (fromIndex == -1)
            strippedURL = url;
        else
            strippedURL = url.substring(0, fromIndex);
        
        
        // Replace "|" character with ":" 
        StringBuffer urlBuffer = new StringBuffer(strippedURL);
        int index = urlBuffer.toString().indexOf("|");
        if (index >= 0) {
            urlBuffer.setCharAt(index, ':'); 
        }
        
        if (fromIndex != -1)
	    urlBuffer.append(url.substring(fromIndex));
       
	return urlBuffer.toString();
    }

    private static String slashify(String path, boolean isDirectory) {
	String p = path;
	if (File.separatorChar != '/')
	    p = p.replace(File.separatorChar, '/');
	if (!p.startsWith("/"))
	    p = "/" + p;
	if (!p.endsWith("/") && isDirectory)
	    p = p + "/";
	return p;
    }
    
    // from java.io.File.toURI (exist since 1.4)
    public static URL fileToURL(File file) {
        if (Config.isJavaVersionAtLeast14()) {
            // URI only exists on 1.4+
            try {
                File f = file.getAbsoluteFile();
                String sp = slashify(f.getPath(), f.isDirectory());
                if (sp.startsWith("//")) {
                    sp = "//" + sp;
                }
                return new URI("file", null, sp, null).toURL();
            } catch (URISyntaxException x) {
                throw new Error(x);	// Can't happen
            } catch (MalformedURLException mue) {
                throw new Error(mue);   // should not happen
            }
        } else {
            try {
                // fall back to use file.toURL
                return file.toURL();
            } catch (MalformedURLException mue) {
                throw new Error(mue);   // should not happen
            }
        }
    }

    /**
     * This is taken from 6u19 sun.net.util.URLUtil but adapted for J2SE 1.4.
     *
     * Returns a string form of the url suitable for use as a key in HashMap/Sets.
     *
     * The string form should be behave in the same manner as the URL when
     * compared for equality in a HashMap/Set, except that no nameservice
     * lookup is done on the hostname (only string comparison), and the fragment
     * is not considered.
     *
     * @see java.net.URLStreamHandler.sameFile(java.net.URL)
     */
    public static String urlNoFragString(URL url) {
        StringBuffer strForm = new StringBuffer(128);

        String protocol = url.getProtocol();
        if (protocol != null) {
            /* protocol is compared case-insensitive, so convert to lowercase */
            protocol = protocol.toLowerCase();
            strForm.append(protocol);
            strForm.append("://");
        }

        String host = url.getHost();
        if (host != null) {
            /* host is compared case-insensitive, so convert to lowercase */
            host = host.toLowerCase();
            strForm.append(host);

            int port = url.getPort();
            if (port == -1) {
                /* if no port is specificed then use the protocols
                 * default, if there is one */
		try {
                    port = url.getDefaultPort();
		} catch (NoSuchMethodError nsme) {
                    port = url.getPort();
		}
            }
            if (port != -1) {
                strForm.append(":").append(port);
            }
        }

        String file = url.getFile();
        if (file != null) {
            strForm.append(file);
        }

        return strForm.toString();
    }
}


