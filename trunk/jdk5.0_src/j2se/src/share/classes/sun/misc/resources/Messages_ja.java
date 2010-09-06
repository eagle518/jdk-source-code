/*
 * "@(#)Messages_ja.java	1.3 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc.resources;

/**
 * <p> This class represents the <code>ResourceBundle</code>
 * for sun.misc.
 * 
 * @author Michael Colburn
 * @version 1.00, 02/08/01
 */

public class Messages_ja extends java.util.ListResourceBundle {

    /**
     * Returns the contents of this <code>ResourceBundle</code>.     
     * <p>   
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
        
    private static final Object[][] contents = {
	{ "optpkg.versionerror", "\u30a8\u30e9\u30fc: JAR \u30d5\u30a1\u30a4\u30eb {0} \u3067\u7121\u52b9\u306a\u30d0\u30fc\u30b8\u30e7\u30f3\u5f62\u5f0f\u304c\u4f7f\u7528\u3055\u308c\u3066\u3044\u307e\u3059\u3002\u30b5\u30dd\u30fc\u30c8\u3055\u308c\u308b\u30d0\u30fc\u30b8\u30e7\u30f3\u5f62\u5f0f\u306b\u3064\u3044\u3066\u306e\u30c9\u30ad\u30e5\u30e1\u30f3\u30c8\u3092\u53c2\u7167\u3057\u3066\u304f\u3060\u3055\u3044\u3002" },
	{ "optpkg.attributeerror", "\u30a8\u30e9\u30fc: \u5fc5\u8981\u306a JAR \u30de\u30cb\u30d5\u30a7\u30b9\u30c8\u5c5e\u6027 {0} \u304c JAR \u30d5\u30a1\u30a4\u30eb {1} \u306b\u8a2d\u5b9a\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002" },
	{ "optpkg.attributeserror", "\u30a8\u30e9\u30fc: \u8907\u6570\u306e\u5fc5\u8981\u306a JAR \u30de\u30cb\u30d5\u30a7\u30b9\u30c8\u5c5e\u6027\u304c JAR \u30d5\u30a1\u30a4\u30eb {0} \u306b\u8a2d\u5b9a\u3055\u308c\u3066\u3044\u307e\u305b\u3093\u3002" }
    };
    
}
