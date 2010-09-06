/*
 * @(#)Messages_zh_TW.java	1.3 03/12/19
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

public class Messages_zh_TW extends java.util.ListResourceBundle {

    /**
     * Returns the contents of this <code>ResourceBundle</code>.     
     * <p>   
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
        
    private static final Object[][] contents = {
	{ "optpkg.versionerror", "\u932f\u8aa4: {0} JAR \u6a94\u4f7f\u7528\u4e86\u7121\u6548\u7684\u7248\u672c\u683c\u5f0f\u3002\u8acb\u6aa2\u67e5\u6587\u4ef6\uff0c\u4ee5\u7372\u5f97\u652f\u63f4\u7684\u7248\u672c\u683c\u5f0f\u3002" },
	{ "optpkg.attributeerror", "\u932f\u8aa4: {1} JAR \u6a94\u4e2d\u672a\u8a2d\u5b9a\u5fc5\u8981\u7684 {0} JAR \u6a19\u660e\u5c6c\u6027\u3002" },
	{ "optpkg.attributeserror", "\u932f\u8aa4: {0} JAR \u6a94\u4e2d\u672a\u8a2d\u5b9a\u67d0\u4e9b\u5fc5\u8981\u7684 JAR \u6a19\u660e\u5c6c\u6027\u3002" }
    };
    
}
