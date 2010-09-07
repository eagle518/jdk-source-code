/*
 * @(#)DOMObjectFactory.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import org.w3c.dom.*;
import org.w3c.dom.css.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.css.*;
import sun.plugin.dom.exception.*;
import sun.plugin.dom.core.CoreConstants;
import sun.plugin.dom.html.HTMLConstants;
import java.util.HashMap;
import java.lang.reflect.Constructor;
/**
 * A class the creates various types of DOM objects.
 */
public class DOMObjectFactory
{
    /**
     * Creates a new Node.
     */
    public static org.w3c.dom.Node createNode(Object o,
                                              org.w3c.dom.Document doc) {
        if (o != null && o instanceof DOMObject) {
            DOMObject obj = (DOMObject) o;
            int nodeType = ((Number) obj.getMember(CoreConstants.ATTR_NODE_TYPE)).intValue();
            switch (nodeType) {
                case org.w3c.dom.Node.ELEMENT_NODE:
                    return createHTMLElement(obj, (org.w3c.dom.html.HTMLDocument) doc);
                case org.w3c.dom.Node.ATTRIBUTE_NODE:
                    return createAttr(obj, doc);
                case org.w3c.dom.Node.TEXT_NODE:
                    return new sun.plugin.dom.core.Text((DOMObject) obj, doc);
                case org.w3c.dom.Node.CDATA_SECTION_NODE:
                    return new sun.plugin.dom.core.CDATASection((DOMObject) obj, doc);
                case org.w3c.dom.Node.ENTITY_REFERENCE_NODE:
                    throw new PluginNotSupportedException("Entity reference nodes are not supported");
                case org.w3c.dom.Node.ENTITY_NODE:
                    throw new PluginNotSupportedException("Entity nodes are not supported");
                case org.w3c.dom.Node.PROCESSING_INSTRUCTION_NODE:
                    throw new PluginNotSupportedException("Processing instruction nodes are not supported");
                case org.w3c.dom.Node.COMMENT_NODE:
                    return new sun.plugin.dom.core.Comment((DOMObject) obj, doc);
                case org.w3c.dom.Node.DOCUMENT_NODE:
                    return createHTMLElement(obj, (org.w3c.dom.html.HTMLDocument) doc);
                case org.w3c.dom.Node.DOCUMENT_TYPE_NODE:
                    throw new PluginNotSupportedException("Document type nodes are not supported");
                case org.w3c.dom.Node.DOCUMENT_FRAGMENT_NODE:
                    return new sun.plugin.dom.core.DocumentFragment((DOMObject) obj, doc);
                case org.w3c.dom.Node.NOTATION_NODE:
                    throw new PluginNotSupportedException("Notation nodes are not supported");
                default:
                    throw new PluginNotSupportedException("Unknown node type " + nodeType);
            }
        }
        return null;
    }

    /**
     * Create a new HTMLElement by wrapping the object.
     */
    public static org.w3c.dom.html.HTMLElement createHTMLElement(Object obj,
                                                                 org.w3c.dom.html.HTMLDocument doc) {
        if (obj == null || !(obj instanceof DOMObject)) {
            return null;
        }

        Class clazz = getRealClassByTagName((DOMObject) obj);
        if (clazz != null) {
            // all HTML element implementation classes are required to
            // have a constructor taking a DOMObject and HTMLDocument as
            // parameters
            try {
                Class[] paramClasses = {sun.plugin.dom.DOMObject.class,
                                        org.w3c.dom.html.HTMLDocument.class};
                Constructor c = clazz.getConstructor(paramClasses);
                Object[] params = {obj, doc};
                return (HTMLElement) c.newInstance(params);
            } catch(Exception e) {
            }
        }

        // Wrap it in a base HTMLElement if we don't otherwise know what to do with it
        return new sun.plugin.dom.html.HTMLElement((DOMObject) obj, doc);
    }

    /**
     * Create a new HTMLFormElement by wrapping the object.
     */
    public static org.w3c.dom.html.HTMLFormElement createHTMLFormElement(Object obj,
                                                                         org.w3c.dom.html.HTMLDocument doc) {
        org.w3c.dom.html.HTMLElement elem = createHTMLElement(obj, doc);
        if (elem != null && (elem instanceof org.w3c.dom.html.HTMLFormElement)) {
            return (org.w3c.dom.html.HTMLFormElement) elem;
        }
        return null;
    }

    /**
     * Create a new HTMLOptionElement by wrapping the object.
     */
    public static org.w3c.dom.html.HTMLOptionElement createHTMLOptionElement(Object obj,
                                                                             org.w3c.dom.html.HTMLDocument doc) {
        org.w3c.dom.html.HTMLElement elem = createHTMLElement(obj, doc);
        if (elem != null && (elem instanceof org.w3c.dom.html.HTMLOptionElement)) {
            return (org.w3c.dom.html.HTMLOptionElement) elem;
        }
        return null;
    }

    /**
     * Creates a node list out of the given Object, if it is a
     * non-null DOMObject, or returns null.
     */
    public static org.w3c.dom.NodeList createNodeList(Object obj,
                                                      org.w3c.dom.html.HTMLDocument doc) {
        if (obj != null && (obj instanceof DOMObject)) {
            return new sun.plugin.dom.html.HTMLCollection((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.NamedNodeMap createNamedNodeMap(Object obj, org.w3c.dom.html.HTMLDocument doc) {
        if (obj != null && (obj instanceof DOMObject)) {
            return new sun.plugin.dom.core.NamedNodeMap((DOMObject) obj, doc);
        }
        return null;
    }

    /**
     * Creates an HTMLCollection out of the given Object, if it is a
     * non-null DOMObject, or returns null.
     */
    public static org.w3c.dom.html.HTMLCollection
        createHTMLCollection(Object obj,
                             org.w3c.dom.html.HTMLDocument doc) {
        return (org.w3c.dom.html.HTMLCollection) createNodeList(obj, doc);
    }

    /**
     * Creates an Attr node out of the given Object, if it is a
     * non-null DOMObject, or returns null.
     */
    public static org.w3c.dom.Attr createAttr(Object obj, org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.core.Attr((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.stylesheets.StyleSheetList
        createStyleSheetList(Object obj, org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.stylesheets.StyleSheetList((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.stylesheets.StyleSheet
        createStyleSheet(Object obj, org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.stylesheets.StyleSheet((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.stylesheets.MediaList
        createMediaList(Object obj, org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.stylesheets.MediaList((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.css.CSSStyleSheet
        createCSSStyleSheet(Object obj, org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.css.CSSStyleSheet((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.css.CSSStyleDeclaration
        createCSSStyleDeclaration(Object obj,
                                  org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.css.CSSStyleDeclaration((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.css.CSSValue createCSSValue(Object obj,
                                                          org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            try {
                Number number = (Number)
                    ((DOMObject) obj).getMember(CSSConstants.ATTR_CSS_VALUE_TYPE);
                switch (number.intValue()) {
                    case org.w3c.dom.css.CSSValue.CSS_INHERIT:
                    case org.w3c.dom.css.CSSValue.CSS_CUSTOM:
                        return new sun.plugin.dom.css.CSSValue((DOMObject) obj, doc);

                    case org.w3c.dom.css.CSSValue.CSS_PRIMITIVE_VALUE:
                        return new sun.plugin.dom.css.CSSPrimitiveValue((DOMObject) obj, doc);

                    case org.w3c.dom.css.CSSValue.CSS_VALUE_LIST:
                        return new sun.plugin.dom.css.CSSValueList((DOMObject) obj, doc);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return null;
    }

    public static org.w3c.dom.css.CSSPrimitiveValue
        createCSSPrimitiveValue(Object obj,
                                org.w3c.dom.Document doc) {
        org.w3c.dom.css.CSSValue val = createCSSValue(obj, doc);
        if (val != null && val instanceof org.w3c.dom.css.CSSPrimitiveValue) {
            return (org.w3c.dom.css.CSSPrimitiveValue) val;
        }
        return null;
    }

    public static org.w3c.dom.css.CSSRule createCSSRule(Object obj,
                                                        org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            try {
                DOMObject dobj = (DOMObject) obj;
                int ruleKind = ((Number) dobj.getMember(HTMLConstants.ATTR_TYPE)).intValue();
                switch (ruleKind) {
                case org.w3c.dom.css.CSSRule.UNKNOWN_RULE:
                    return new sun.plugin.dom.css.CSSUnknownRule(dobj, doc);
                case org.w3c.dom.css.CSSRule.STYLE_RULE:
                    return new sun.plugin.dom.css.CSSStyleRule(dobj, doc);
                case org.w3c.dom.css.CSSRule.CHARSET_RULE:
                    return new sun.plugin.dom.css.CSSCharsetRule(dobj, doc);
                case org.w3c.dom.css.CSSRule.IMPORT_RULE:
                    return new sun.plugin.dom.css.CSSImportRule(dobj, doc);
                case org.w3c.dom.css.CSSRule.MEDIA_RULE:
                    return new sun.plugin.dom.css.CSSMediaRule(dobj, doc);
                case org.w3c.dom.css.CSSRule.FONT_FACE_RULE:
                    return new sun.plugin.dom.css.CSSFontFaceRule(dobj, doc);
                case org.w3c.dom.css.CSSRule.PAGE_RULE:
                    return new sun.plugin.dom.css.CSSPageRule(dobj, doc);
                default:
                    break;
                }
            } catch (Exception e) {
            }
        }
        return null;
    }

    public static org.w3c.dom.css.CSSRuleList createCSSRuleList(Object obj,
                                                                org.w3c.dom.Document doc) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.css.CSSRuleList((DOMObject) obj, doc);
        }
        return null;
    }

    public static org.w3c.dom.css.Counter createCSSCounter(Object obj) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.css.Counter((DOMObject) obj);
        }
        return null;
    }

    public static org.w3c.dom.css.Rect createCSSRect(Object obj, org.w3c.dom.Document document) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.css.Rect((DOMObject) obj, document);
        }
        return null;
    }

    public static org.w3c.dom.css.RGBColor createCSSRGBColor(Object obj, org.w3c.dom.Document document) {
        if (obj != null && obj instanceof DOMObject) {
            return new sun.plugin.dom.css.RGBColor((DOMObject) obj, document);
        }
        return null;
    }

    private static Class getRealClassByTagName(DOMObject obj) {
	try {
	    Object  result = obj.getMember(HTMLConstants.ATTR_TAG_NAME);
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
	    elmTagClassMap.put("TEXTAREA", sun.plugin.dom.html.HTMLTextAreaElement.class);
	    elmTagClassMap.put("TITLE", sun.plugin.dom.html.HTMLTitleElement.class);
	    elmTagClassMap.put("UL", sun.plugin.dom.html.HTMLUListElement.class);

            // Special node types represented only by the base HTMLElement class --
            // see the javadoc for the HTMLElement interface
	    elmTagClassMap.put("ACRONYM", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("ABBR", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("ADDRESS", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("B", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("BDO", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("BIG", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("CITE", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("CENTER", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("CODE", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("DD", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("DFN", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("DT", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("EM", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("I", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("KBD", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("NOFRAMES", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("NOSCRIPT", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("S", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("SAMP", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("SMALL", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("SPAN", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("STRIKE", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("STRONG", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("SUB", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("SUP", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("U", sun.plugin.dom.html.HTMLElement.class);
	    elmTagClassMap.put("VAR", sun.plugin.dom.html.HTMLElement.class);
	}

	return elmTagClassMap;
    }

    private static HashMap elmTagClassMap = null;
}
