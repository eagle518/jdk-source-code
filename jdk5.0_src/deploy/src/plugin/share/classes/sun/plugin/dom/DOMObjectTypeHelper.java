/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */


package sun.plugin.dom;

import java.util.HashMap;
import sun.plugin.services.BrowserService;

/**
 * DOMObjectTypeHelper.java
 * 
 * The class helps to map DOM object type to its Java class type.
 *
 * @auther zhengyu.gu@sun.com
 * @since  1.4.2
 **/

final class DOMObjectTypeHelper {
    private static final String HTML_IMPLEMENTATION_PACKAGE = "sun.plugin.dom.html";
    private static final String SSL_IMPLEMENTATION_PACKAGE = "sun.plugin.dom.stylesheets";
    private static final String CSS_IMPLEMENTATION_PACKAGE = "sun.plugin.dom.css";
    private static final String DOM_IMPLEMENTATION_PACKAGE = "sun.plugin.dom.core";

    static Class getDOMCoreClass(DOMObject obj) {
	Class clazz = getObjectClass(obj, DOM_IMPLEMENTATION_PACKAGE);
	if(clazz == null)
	    clazz = sun.plugin.dom.core.Text.class;
	return clazz; 
    }

    static Class getHTMLElementClass(DOMObject obj) {
	return getObjectClass(obj, HTML_IMPLEMENTATION_PACKAGE);
    }

    static Class getCSSRuleClass(DOMObject obj) {
	return getObjectClass(obj, CSS_IMPLEMENTATION_PACKAGE);
    }

    static Class getStyleSheetClass(DOMObject obj) {
	Class clazz = getObjectClass(obj, CSS_IMPLEMENTATION_PACKAGE);
	if(null != clazz)
	    return clazz;

	return getObjectClass(obj, SSL_IMPLEMENTATION_PACKAGE);
    }

    private static Class getObjectClass(DOMObject obj, String packageName) {
	String objInfo = obj.toString();
	String type = getObjectType(objInfo);

	if(type == null)
	    type = objInfo;
	BrowserService service = (BrowserService) com.sun.deploy.services.ServiceManager.getService();
	type = service.mapBrowserElement(type);
	if(type != null) {
	    StringBuffer sb = new StringBuffer(packageName);
	    sb.append('.');
	    sb.append(type);
	    try {
		return Class.forName(sb.toString());
	    } catch(ClassNotFoundException e) {
	    } 
	} 

	return null;
    }

    /*
     * Expecting info string has format:
     * [object type], we need to extract <code>type</code> from
     * the string.
     */
    private static String getObjectType(String info) {
	info = info.trim();
	if(!info.endsWith("]"))
	    return null;

	int index = info.lastIndexOf(' ');
	if(index == -1)
	    return null;

	return info.substring(index + 1, info.length() - 1);
    }
}