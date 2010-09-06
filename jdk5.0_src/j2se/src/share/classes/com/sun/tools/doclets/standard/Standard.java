/*
 * @(#)Standard.java	1.4 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.standard;

import com.sun.javadoc.*;
import com.sun.tools.doclets.formats.html.*;


public class Standard {
    
    public static final HtmlDoclet htmlDoclet = new HtmlDoclet();
    
    public static int optionLength(String option) {
        return htmlDoclet.optionLength(option);
    }
    
    public static boolean start(RootDoc root) {
        return htmlDoclet.start(root);
    }
    
    public static boolean validOptions(String[][] options,
                                   DocErrorReporter reporter) {
        return htmlDoclet.validOptions(options, reporter);
    }

    public static LanguageVersion languageVersion() {
	return htmlDoclet.languageVersion();
    }
    
}

