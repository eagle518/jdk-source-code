/*
 * @(#)Handler.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.jar;

import java.io.*;
import java.net.*;
import java.util.*;
import sun.net.www.ParseUtil;

/*
 * Jar URL Handler
 */
public class Handler extends java.net.URLStreamHandler {

    private static final String separator = "!/";

    protected java.net.URLConnection openConnection(URL u)
    throws IOException {
	return new JarURLConnection(u, this);
    }

    private int indexOfBangSlash(String spec) {
	int indexOfBang = spec.length();
	while((indexOfBang = spec.lastIndexOf('!', indexOfBang)) != -1) {
	    if ((indexOfBang != (spec.length() - 1)) &&
		(spec.charAt(indexOfBang + 1) == '/')) {
		return indexOfBang + 1;
	    } else {
		indexOfBang--;
	    }
	}
	return -1;
    }
	
    protected void parseURL(URL url, String spec, 
			    int start, int limit) {
	String file = null;
	String ref = null;
	// first figure out if there is an anchor
	int refPos = spec.indexOf('#', limit);
	boolean refOnly = refPos == start;
	if (refPos > -1) {
	    ref = spec.substring(refPos + 1, spec.length());
	    if (refOnly) {
		file = url.getFile();
	    }
	}
	// then figure out if the spec is 
	// 1. absolute (jar:)
	// 2. relative (i.e. url + foo/bar/baz.ext)
	// 3. anchor-only (i.e. url + #foo), which we already did (refOnly)
	boolean absoluteSpec = false;
	if (spec.length() >= 4) {
	    absoluteSpec = spec.substring(0, 4).equalsIgnoreCase("jar:");
	}
	spec = spec.substring(start, limit);

	if (absoluteSpec) {
	    file = parseAbsoluteSpec(spec);
	} else if (!refOnly) {
	    file = parseContextSpec(url, spec);

            // Canonize the result after the bangslash
            int bangSlash = indexOfBangSlash(file);
            String toBangSlash = file.substring(0, bangSlash);
            String afterBangSlash = file.substring(bangSlash);
            sun.net.www.ParseUtil canonizer = new ParseUtil();
            afterBangSlash = canonizer.canonizeString(afterBangSlash);
            file = toBangSlash + afterBangSlash;
	}
	setURL(url, "jar", "", -1, file, ref);	
    }

    private String parseAbsoluteSpec(String spec) {
	URL url = null;
	int index = -1;
	// check for !/
	if ((index = indexOfBangSlash(spec)) == -1) {
	    throw new NullPointerException("no !/ in spec");
	}
	// test the inner URL
	try {
	    String innerSpec = spec.substring(0, index - 1);
	    url = new URL(innerSpec);
	} catch (MalformedURLException e) {
	    throw new NullPointerException("invalid url: " + 
					   spec + " (" + e + ")");
	}
	return spec;
    }

    private String parseContextSpec(URL url, String spec) {
	String ctxFile = url.getFile();
	// if the spec begins with /, chop up the jar back !/
	if (spec.startsWith("/")) {
	    int bangSlash = indexOfBangSlash(ctxFile);
	    if (bangSlash == -1) {
		throw new NullPointerException("malformed " +
					       "context url:" +
					       url + 
					       ": no !/");
	    }
	    ctxFile = ctxFile.substring(0, bangSlash);
	}
	if (!ctxFile.endsWith("/") && (!spec.startsWith("/"))){
	    // chop up the last component
	    int lastSlash = ctxFile.lastIndexOf('/');
	    if (lastSlash == -1) {
		throw new NullPointerException("malformed " +
					       "context url:" +
					       url);
	    }
	    ctxFile = ctxFile.substring(0, lastSlash + 1);
	}
	return (ctxFile + spec);
    }
}
