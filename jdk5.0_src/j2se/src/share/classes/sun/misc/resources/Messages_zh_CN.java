/*
 * @(#)Messages_zh_CN.java	1.3 03/12/19
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

public class Messages_zh_CN extends java.util.ListResourceBundle {

    /**
     * Returns the contents of this <code>ResourceBundle</code>.     
     * <p>   
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
	return contents;
    }
        
    private static final Object[][] contents = {
	{ "optpkg.versionerror", "\u9519\u8bef\uff1a{0} JAR \u6587\u4ef6\u4e2d\u4f7f\u7528\u7684\u7248\u672c\u683c\u5f0f\u65e0\u6548\u3002\u8bf7\u68c0\u67e5\u6587\u6863\u4ee5\u4e86\u89e3\u652f\u6301\u7684\u7248\u672c\u683c\u5f0f\u3002" },
	{ "optpkg.attributeerror", "\u9519\u8bef\uff1a\u5fc5\u8981\u7684 {0} JAR \u6807\u660e\u5c5e\u6027\u672a\u5728 {1} JAR \u6587\u4ef6\u4e2d\u8bbe\u7f6e\u3002" },
	{ "optpkg.attributeserror", "\u9519\u8bef\uff1a\u67d0\u4e9b\u5fc5\u8981\u7684 JAR \u6807\u660e\u5c5e\u6027\u672a\u5728 {0} JAR \u6587\u4ef6\u4e2d\u8bbe\u7f6e\u3002" }
    };
    
}
