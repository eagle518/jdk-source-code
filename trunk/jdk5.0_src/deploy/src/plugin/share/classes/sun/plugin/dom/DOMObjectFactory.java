/*
 * @(#)DOMObjectFactory.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.exception.*;
import java.util.HashMap;
import java.lang.reflect.Constructor;
/**
 * A class the creates various types of DOM objects.
 */
public class DOMObjectFactory
{
    private static final String HTML_TAGNAME = "tagName";
    private static final String ATTR_TYPE = "type";
    /**
     * Create a new HTMLElement by wrapping the object.
     */
    public static org.w3c.dom.html.HTMLElement createHTMLElement(DOMObject obj, 
						org.w3c.dom.html.HTMLDocument doc) {
	Object ret = createHTMLObject(obj, doc);
	if(ret instanceof org.w3c.dom.html.HTMLElement)
	    return (org.w3c.dom.html.HTMLElement)ret;

	return null;
    }

    /**
     * <p>
     * This method DOM object that defined in <code>sun.plugin.dom.html</code> and
     * <code>sun.plugin.dom.core</code> packages
     */
    public static Object createCommonDOMObject(DOMObject obj,
					       org.w3c.dom.Document doc) {
	if(obj == null)
	    return null;
	    
	Class clazz = DOMObjectTypeHelper.getHTMLElementClass(obj);
	if(clazz == null)
	    clazz =getRealClassByTagName(obj); 
	if(clazz != null)
	    return createHTMLObject(clazz, obj, (org.w3c.dom.html.HTMLDocument)doc);	
	clazz = DOMObjectTypeHelper.getDOMCoreClass(obj);
	return createDOMCoreObject(clazz, obj, doc);
    }


    private static Object createDOMCoreObject(Class clazz, 
					      DOMObject obj,
					      org.w3c.dom.Document doc) {
	try {
	    Class[] paramClasses = {sun.plugin.dom.DOMObject.class,
				    org.w3c.dom.Document.class }; 
	    Constructor c = clazz.getConstructor(paramClasses);
	    Object[] params = {obj, doc};
	    return c.newInstance(params);
	} catch(Exception e) {
	}
		
	// Wrong type, cannot wrap - throw exception
	throw new PluginNotSupportedException("DOMObjectFactory::createDOMCoreObject() cannot wrap " + obj);
    }

    /**
     * <p>
     * This method only create HTML object that defined in <code>sun.plugin.dom.html</code> package.
     */
    public static Object createHTMLObject(DOMObject obj,
					  org.w3c.dom.html.HTMLDocument doc) {
	// Error checking
	if (obj == null)
	    return null;

	Class clazz = DOMObjectTypeHelper.getHTMLElementClass(obj);
	if(clazz == null) {
		clazz = sun.plugin.dom.html.HTMLElement.class;
	}

	return createHTMLObject(clazz, obj, doc);
    } 

    private static Object createHTMLObject(Class clazz,
					   DOMObject obj,
					   org.w3c.dom.html.HTMLDocument doc) {
	Class tmp;
	if(clazz != null) {
	    if(clazz.equals(sun.plugin.dom.html.HTMLElement.class) ||
	       clazz.equals(sun.plugin.dom.html.HTMLObjectElement.class) ||
	       clazz.equals(sun.plugin.dom.html.HTMLUListElement.class)  ||
	       clazz.equals(sun.plugin.dom.html.HTMLButtonElement.class) ||
	       clazz.equals(sun.plugin.dom.html.HTMLOptionElement.class) ||
	       clazz.equals(sun.plugin.dom.html.HTMLInputElement.class) ||
	       clazz.equals(sun.plugin.dom.html.HTMLQuoteElement.class)) {
		tmp = getRealClassByTagName((DOMObject)obj);
		if(tmp != null)
		    clazz = tmp;
	    }
	}

	// all HTML element implementation classes are required to have constructor 
	// to take DOMObject, HTMLDocument and Element as parameters
	try {
	    Class[] paramClasses = {sun.plugin.dom.DOMObject.class, 
				    org.w3c.dom.html.HTMLDocument.class};
	    Constructor c = clazz.getConstructor(paramClasses);
	    Object[] params = {obj, doc};
	    return c.newInstance(params);
	} catch(Exception e) {
	}
		
	// Wrong type, cannot wrap - throw exception
	throw new PluginNotSupportedException("DOMObjectFactory::createHTMLElement() cannot wrap " + obj);
    }

    public static Object createStyleSheetObject(DOMObject obj, 
						org.w3c.dom.Document doc, 
						org.w3c.dom.Node owner) {
	// Error checking
	if (obj == null)
	    return null;

	Class clazz = DOMObjectTypeHelper.getStyleSheetClass(obj);
	if(clazz != null) {
	    if(clazz.equals(sun.plugin.dom.stylesheets.StyleSheet.class)) {
		Class tmp = getRealClassByType(obj);
		if(tmp != null)
		    clazz = tmp;
	    }

	    try {
		Class[] paramClasses = {sun.plugin.dom.DOMObject.class,
					org.w3c.dom.Document.class, 
		    			org.w3c.dom.Node.class};
		Constructor c = clazz.getConstructor(paramClasses);
		Object[] params = {obj, doc, owner};
		return c.newInstance(params);
	    } catch(Exception e) {
	    }
	}

	// Wrong type, cannot wrap - throw exception
	throw new PluginNotSupportedException("DOMObjectFactory::createStyleSheet() cannot wrap " + obj);
    }

    /**
     * Create a new StyleSheet by wrapping the object.
     */
    public static org.w3c.dom.stylesheets.StyleSheet createStyleSheet(DOMObject obj,
								      org.w3c.dom.Document doc, 
								      org.w3c.dom.Node owner) {
	Object ret = createStyleSheetObject(obj, doc, owner);
	if(ret != null && ret instanceof org.w3c.dom.stylesheets.StyleSheet)
	    return (org.w3c.dom.stylesheets.StyleSheet)ret;
	return null;
    }

    public static Object createCSSObject(DOMObject obj,
				         org.w3c.dom.Document doc,
					 org.w3c.dom.Node owner, 
					 org.w3c.dom.css.CSSStyleSheet styleSheet,
					 org.w3c.dom.css.CSSRule parentRule) {
	if(obj == null) 
	    return null;

	Class clazz = DOMObjectTypeHelper.getCSSRuleClass(obj);

	if(clazz != null) {
	    if(org.w3c.dom.stylesheets.StyleSheet.class.isAssignableFrom(clazz)) {
		return createStyleSheet(obj, doc, owner);
	    } else {
		try {
		    Class[] clsParams = {sun.plugin.dom.DOMObject.class,
					 org.w3c.dom.Document.class,
					 org.w3c.dom.Node.class,
					 org.w3c.dom.css.CSSStyleSheet.class,
					 org.w3c.dom.css.CSSRule.class};
		    Constructor c = clazz.getConstructor(clsParams);
		    Object[] params = {obj, doc, owner, styleSheet, parentRule};
		    return c.newInstance(params);
		}catch(Exception e) {
		}
	    }
	}
	throw new PluginNotSupportedException("DOMObjectFactory::createCSSRuleObject() cannot wrap " + obj);
    }


    private static Class getRealClassByType(DOMObject obj) {
	try {
	    Object result = obj.getMember(ATTR_TYPE);
	    if(result != null) {
		return (Class)getElmTypeClassMap().get(result);
	    }
	}catch(DOMException e) {
	}
	return null;
    }

    // when we get generic type, we want to find out its real type
    private static Class getRealClassByTagName(DOMObject obj) {
	try {
	    Object  result = obj.getMember(HTML_TAGNAME);
	    if(result != null) {
		return (Class)getElmTagClassMap().get(result);
	    }
	} catch(DOMException e) {
	}

	return null;
    }

    private static synchronized HashMap getElmTagClassMap() {
	if(elmTagClassMap == null) {
	    elmTagClassMap = new HashMap();
	    elmTagClassMap.put("A", sun.plugin.dom.html.HTMLAnchorElement.class);
	    elmTagClassMap.put("APPLET", sun.plugin.dom.html.HTMLAppletElement.class);
	    elmTagClassMap.put("AREA", sun.plugin.dom.html.HTMLAreaElement.class);
	    elmTagClassMap.put("BASE", sun.plugin.dom.html.HTMLBaseElement.class);
	    elmTagClassMap.put("BLOCKQUOTE", sun.plugin.dom.html.HTMLQuoteElement.class);
	    elmTagClassMap.put("BODY", sun.plugin.dom.html.HTMLBodyElement.class);
	    elmTagClassMap.put("BR", sun.plugin.dom.html.HTMLBRElement.class);
	    elmTagClassMap.put("CAPTION", sun.plugin.dom.html.HTMLTableCaptionElement.class);
	    elmTagClassMap.put("COL", sun.plugin.dom.html.HTMLTableColElement.class);
	    elmTagClassMap.put("DEL", sun.plugin.dom.html.HTMLModElement.class);
	    elmTagClassMap.put("DIR", sun.plugin.dom.html.HTMLDirectoryElement.class);
	    elmTagClassMap.put("DIV", sun.plugin.dom.html.HTMLDivElement.class);
	    elmTagClassMap.put("DL", sun.plugin.dom.html.HTMLDListElement.class);
	    elmTagClassMap.put("FIELDSET", sun.plugin.dom.html.HTMLFieldSetElement.class);
	    elmTagClassMap.put("FONT", sun.plugin.dom.html.HTMLFontElement.class);
	    elmTagClassMap.put("FORM", sun.plugin.dom.html.HTMLFormElement.class);
	    elmTagClassMap.put("FRAME", sun.plugin.dom.html.HTMLFrameElement.class);
	    elmTagClassMap.put("FRAMESET", sun.plugin.dom.html.HTMLFrameSetElement.class);
	    elmTagClassMap.put("HEAD", sun.plugin.dom.html.HTMLHeadElement.class);
	    elmTagClassMap.put("H1", sun.plugin.dom.html.HTMLHeadingElement.class);
	    elmTagClassMap.put("H2", sun.plugin.dom.html.HTMLHeadingElement.class);
	    elmTagClassMap.put("H3", sun.plugin.dom.html.HTMLHeadingElement.class);
	    elmTagClassMap.put("H4", sun.plugin.dom.html.HTMLHeadingElement.class);
	    elmTagClassMap.put("H5", sun.plugin.dom.html.HTMLHeadingElement.class);
	    elmTagClassMap.put("H6", sun.plugin.dom.html.HTMLHeadingElement.class);
	    elmTagClassMap.put("HR", sun.plugin.dom.html.HTMLHRElement.class);
	    elmTagClassMap.put("HTML", sun.plugin.dom.html.HTMLHtmlElement.class);
	    elmTagClassMap.put("IFRAME", sun.plugin.dom.html.HTMLIFrameElement.class);
	    elmTagClassMap.put("IMAGE", sun.plugin.dom.html.HTMLImageElement.class);
	    elmTagClassMap.put("INPUT", sun.plugin.dom.html.HTMLInputElement.class);
	    elmTagClassMap.put("INS", sun.plugin.dom.html.HTMLModElement.class);
	    elmTagClassMap.put("ISINDEX", sun.plugin.dom.html.HTMLIsIndexElement.class);
	    elmTagClassMap.put("LABEL", sun.plugin.dom.html.HTMLLabelElement.class);
	    elmTagClassMap.put("LEGEND", sun.plugin.dom.html.HTMLLegendElement.class);
	    elmTagClassMap.put("LI", sun.plugin.dom.html.HTMLLIElement.class);
	    elmTagClassMap.put("LINK", sun.plugin.dom.html.HTMLLinkElement.class);
	    elmTagClassMap.put("MAP", sun.plugin.dom.html.HTMLMapElement.class);
	    elmTagClassMap.put("MENU", sun.plugin.dom.html.HTMLMenuElement.class);
	    elmTagClassMap.put("META", sun.plugin.dom.html.HTMLMetaElement.class);
	    elmTagClassMap.put("MOD", sun.plugin.dom.html.HTMLModElement.class);
	    elmTagClassMap.put("OBJECT", sun.plugin.dom.html.HTMLObjectElement.class);
	    elmTagClassMap.put("OL", sun.plugin.dom.html.HTMLOListElement.class);
	    elmTagClassMap.put("OPTGROUP", sun.plugin.dom.html.HTMLOptGroupElement.class);
	    elmTagClassMap.put("OPTION", sun.plugin.dom.html.HTMLOptionElement.class);
	    elmTagClassMap.put("P", sun.plugin.dom.html.HTMLParagraphElement.class);
	    elmTagClassMap.put("PARAM", sun.plugin.dom.html.HTMLParamElement.class);
	    elmTagClassMap.put("PRE", sun.plugin.dom.html.HTMLPreElement.class);
	    elmTagClassMap.put("Q", sun.plugin.dom.html.HTMLQuoteElement.class);
	    elmTagClassMap.put("SCRIPT", sun.plugin.dom.html.HTMLScriptElement.class);
	    elmTagClassMap.put("SELECT", sun.plugin.dom.html.HTMLSelectElement.class);
	    elmTagClassMap.put("STYLE", sun.plugin.dom.html.HTMLStyleElement.class);
	    elmTagClassMap.put("TABLE", sun.plugin.dom.html.HTMLTableElement.class);
	    elmTagClassMap.put("TBODY", sun.plugin.dom.html.HTMLTableSectionElement.class);
	    elmTagClassMap.put("TD", sun.plugin.dom.html.HTMLTableCellElement.class);
	    elmTagClassMap.put("TFOOT", sun.plugin.dom.html.HTMLTableSectionElement.class);
	    elmTagClassMap.put("TH", sun.plugin.dom.html.HTMLTableCellElement.class);
	    elmTagClassMap.put("THEAD", sun.plugin.dom.html.HTMLTableSectionElement.class);
	    elmTagClassMap.put("TR", sun.plugin.dom.html.HTMLTableRowElement.class);
	    elmTagClassMap.put("TEXTARRA", sun.plugin.dom.html.HTMLTextAreaElement.class);
	    elmTagClassMap.put("TITLE", sun.plugin.dom.html.HTMLTitleElement.class);
	    elmTagClassMap.put("UL", sun.plugin.dom.html.HTMLUListElement.class);
	}

	return elmTagClassMap;
    }

    private static synchronized HashMap getElmTypeClassMap() {
	if(elmTypeClassMap == null) {
	    elmTypeClassMap = new HashMap();
	    elmTypeClassMap.put("text/css", sun.plugin.dom.css.CSSStyleSheet.class);
	}

	return elmTypeClassMap;
    }

    private static HashMap elmTagClassMap = null;
    private static HashMap elmTypeClassMap = null;

}

