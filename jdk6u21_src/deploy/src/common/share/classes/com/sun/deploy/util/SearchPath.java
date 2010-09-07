/*
 * @(#)SearchPath.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.File;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Iterator;

public class SearchPath {  

    private static ArrayList PREFERRED_BROWSERS = new ArrayList();

    public static File findOne(String path) {
      
        File result = null;

	// initialized preferred brwosers list
	PREFERRED_BROWSERS.clear();
	PREFERRED_BROWSERS.add("mozilla");
	PREFERRED_BROWSERS.add("firefox");
	PREFERRED_BROWSERS.add("netscape");
	PREFERRED_BROWSERS.add("opera");
	PREFERRED_BROWSERS.add("konqueror");
	PREFERRED_BROWSERS.add("galeon");
	
	for (Iterator i = PREFERRED_BROWSERS.iterator(); i.hasNext();) {

	    StringTokenizer tokenizer = new StringTokenizer(path, 
							File.pathSeparator);

	    String browser = (String) i.next();
	    while (tokenizer.hasMoreTokens()) {

		File dir = new File(tokenizer.nextToken());
		
		File browserPath = new File(dir, browser);
		
		if (browserPath.exists()) {
		    return browserPath;
		}
	    }
	}

        return result;
    }   
}
