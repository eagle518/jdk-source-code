/*
 * @(#)HTMLDocument.java	1.19 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import org.w3c.dom.stylesheets.*;
import org.w3c.dom.css.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;
import sun.plugin.dom.views.*;
import sun.plugin.dom.exception.PluginNotSupportedException;

/**
 *  An <code>HTMLDocument</code> is the root of the HTML hierarchy and holds 
 * the entire content. Besides providing access to the hierarchy, it also 
 * provides some convenience methods for accessing certain sets of 
 * information from the document.
 * <p> The following properties have been deprecated in favor of the 
 * corresponding ones for the <code>BODY</code> element: alinkColor background
 *  bgColor fgColor linkColor vlinkColor In DOM Level 2, the method 
 * <code>getElementById</code> is inherited from the <code>Document</code> 
 * interface where it was moved.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLDocument extends sun.plugin.dom.core.Document 
			  implements org.w3c.dom.html.HTMLDocument,
				     org.w3c.dom.views.DocumentView,
				     org.w3c.dom.stylesheets.DocumentStyle,
				     org.w3c.dom.css.DocumentCSS {
 
    private static final String	  TAG_HTML = "HTML";

    /**
     * Construct a HTMLDocument object.
     */
    public HTMLDocument(DOMObject obj, 
			org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }


    /**
     *  The title of a document as specified by the <code>TITLE</code> element 
     * in the head of the document. 
     */
    public String getTitle() {
	return getAttribute(HTMLConstants.ATTR_TITLE);
    }

    public void setTitle(String title) {
	setAttribute(HTMLConstants.ATTR_TITLE, title);
    }

    /**
     *  Returns the URI  of the page that linked to this page. The value is an 
     * empty string if the user navigated to the page directly (not through a 
     * link, but, for example, via a bookmark). 
     */
    public String getReferrer() {
	return getAttribute(HTMLConstants.ATTR_REFERRER);
    }

    /**
     *  The domain name of the server that served the document, or 
     * <code>null</code> if the server cannot be identified by a domain name. 
     */
    public String getDomain() {
	return getAttribute(HTMLConstants.ATTR_DOMAIN);
    }

    /**
     *  The complete URI  of the document. 
     */
    public String getURL() {
	return getAttribute(HTMLConstants.ATTR_URL);
    }

    /**
     *  The element that contains the content for the document. In documents 
     * with <code>BODY</code> contents, returns the <code>BODY</code> 
     * element. In frameset documents, this returns the outermost
     * <code>FRAMESET</code> element. 
     */
    public org.w3c.dom.html.HTMLElement getBody() {
	Object result = obj.getMember(HTMLConstants.MEMBER_BODY);
        if (result == null) {
            return null;
        }
        return DOMObjectFactory.createHTMLElement((DOMObject) result, this);
    }

    public void setBody(org.w3c.dom.html.HTMLElement body) {
        DOMObject newBody = ((sun.plugin.dom.html.HTMLElement) body).getDOMObject();
        obj.setMember(HTMLConstants.MEMBER_BODY, newBody);
    }

    /**
     *  A collection of all the <code>IMG</code> elements in a document. The 
     * behavior is limited to <code>IMG</code> elements for backwards 
     * compatibility. 
     */
    public org.w3c.dom.html.HTMLCollection getImages() {
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.MEMBER_IMAGES),
                                                     this);
    }

    /**
     *  A collection of all the <code>OBJECT</code> elements that include 
     * applets and <code>APPLET</code> ( deprecated ) elements in a document. 
     */
    public org.w3c.dom.html.HTMLCollection getApplets() {
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.MEMBER_APPLETS),
                                                     this);
    }

    /**
     *  A collection of all <code>AREA</code> elements and anchor (
     * <code>A</code> ) elements in a document with a value for the 
     * <code>href</code> attribute. 
     */
    public org.w3c.dom.html.HTMLCollection getLinks() {
	return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.MEMBER_LINKS),
                                                     this);
    }

    /**
     *  A collection of all the forms of a document. 
     */
    public org.w3c.dom.html.HTMLCollection getForms() {
	return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.MEMBER_FORMS),
                                                     this);
    }

    /**
     *  A collection of all the anchor (<code>A</code> ) elements in a document
     *  with a value for the <code>name</code> attribute. Note. For reasons 
     * of backwards compatibility, the returned set of anchors only contains 
     * those anchors created with the <code>name</code>  attribute, not those 
     * created with the <code>id</code> attribute. 
     */
    public org.w3c.dom.html.HTMLCollection getAnchors() {
        return DOMObjectFactory.createHTMLCollection(obj.getMember(HTMLConstants.MEMBER_ARCHORS),
                                                     this);
    }

    /**
     *  The cookies associated with this document. If there are none, the 
     * value is an empty string. Otherwise, the value is a string: a 
     * semicolon-delimited list of "name, value" pairs for all the cookies 
     * associated with the page. For example, 
     * <code>name=value;expires=date</code> . 
     */
    public String getCookie() {
	return getAttribute(HTMLConstants.ATTR_COOKIE);
    }

    public void setCookie(String cookie) {
	setAttribute(HTMLConstants.ATTR_COOKIE, cookie);
    }

    /**
     *  Note. This method and the ones following  allow a user to add to or 
     * replace the structure model of a document using strings of unparsed 
     * HTML. At the time of  writing alternate methods for providing similar 
     * functionality for  both HTML and XML documents were being considered. 
     * The following methods may be deprecated at some point in the future in 
     * favor of a more general-purpose mechanism.
     * <br> Open a document stream for writing. If a document exists in the 
     * target, this method clears it.
     */
    public void open() {
        throw new PluginNotSupportedException("HTMLDocument.open() is not supported");
    }

    /**
     *  Closes a document stream opened by <code>open()</code> and forces 
     * rendering.
     */
    public void close() {
        throw new PluginNotSupportedException("HTMLDocument.close() is not supported");
    }

    /**
     *  Write a string of text to a document stream opened by
     * <code>open()</code> . The text is parsed into the document's structure 
     * model.
     * @param text  The string to be parsed into some structure in the 
     *   document structure model.
     */
    public void write(String text) {
        throw new PluginNotSupportedException("HTMLDocument.write() is not supported");
    }

    /**
     *  Write a string of text followed by a newline character to a document 
     * stream opened by <code>open()</code> . The text is parsed into the 
     * document's structure model.
     * @param text  The string to be parsed into some structure in the 
     *   document structure model.
     */
    public void writeln(String text) {
        throw new PluginNotSupportedException("HTMLDocument.writeln() is not supported");
    }


    /**
     *  Returns the (possibly empty) collection of elements whose
     * <code>name</code> value is given by <code>elementName</code> .
     * @param elementName  The <code>name</code> attribute value for an 
     *   element.
     * @return  The matching elements.
     */
    public org.w3c.dom.NodeList getElementsByName(String elementName) {
        return DOMObjectFactory.createNodeList(obj.call(HTMLConstants.FUNC_GET_ELEMENTS_BY_NAME,
                                                        new Object[]{elementName}),
                                               this);
    }


    /**
     *  Returns a <code>NodeList</code> of all the <code>Elements</code> with 
     * a given tag name in the order in which they are encountered in a 
     * preorder traversal of the <code>Document</code> tree. 
     * @param tagname  The name of the tag to match on. The special value "*" 
     *   matches all tags.
     * @return  A new <code>NodeList</code> object containing all the matched 
     *   <code>Elements</code> .
     */
    public org.w3c.dom.NodeList getElementsByTagName(String tagname) {
	return DOMObjectFactory.createNodeList(obj.call(HTMLConstants.FUNC_GET_ELEMENTS_BY_TAGNAME,
                                                        new Object[]{tagname}),
                                               this);
    }

    /**
     *  This is a convenience attribute that allows direct access to the child 
     * node that is the root element of  the document. For HTML documents, 
     * this is the element with the tagName "HTML".
     */
    public org.w3c.dom.Element getDocumentElement() {
	Object result = obj.getMember(HTMLConstants.ATTR_DOCUMENT_ELEMENT);
	if (result == null) {
	    return null;
        }
        return DOMObjectFactory.createHTMLElement((DOMObject) result, this);
    }

    /**
     *  Creates an element of the type specified. Note that the instance 
     * returned implements the <code>Element</code> interface, so attributes 
     * can be specified directly  on the returned object.
     * <br> In addition, if there are known attributes with default values, 
     * <code>Attr</code> nodes representing them are automatically created and
     *  attached to the element.
     * <br> To create an element with a qualified name and namespace URI, use 
     * the <code>createElementNS</code> method.
     * @param tagName  The name of the element type to instantiate. For XML, 
     *   this is case-sensitive. For HTML, the  <code>tagName</code> 
     *   parameter may be provided in any case,  but it must be mapped to the 
     *   canonical uppercase form by  the DOM implementation. 
     * @return  A new <code>Element</code> object with the 
     *   <code>nodeName</code> attribute set to <code>tagName</code> , and 
     *   <code>localName</code> , <code>prefix</code> , and 
     *   <code>namespaceURI</code> set to <code>null</code> .
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified name contains an 
     *   illegal character.
     */
    public org.w3c.dom.Element createElement(String tagName)
	    throws DOMException {
	Object result = obj.call(HTMLConstants.FUNC_CREATE_ELEMENT, new Object[]{tagName});
        if (result == null) {
            return null;
        }
        return DOMObjectFactory.createHTMLElement((DOMObject) result, this);
    }

    /**
     *  Creates an element of the given qualified name and namespace URI. 
     * HTML-only DOM implementations do not need to implement this method.
     * @param namespaceURI  The  namespace URI of the element to create.
     * @param qualifiedName  The  qualified name of the element type to 
     *   instantiate.
     * @return  A new <code>Element</code> object with the following 
     *   attributes: Attribute Value<code>Node.nodeName</code>
     *   <code>qualifiedName</code><code>Node.namespaceURI</code>
     *   <code>namespaceURI</code><code>Node.prefix</code> prefix, extracted 
     *   from <code>qualifiedName</code> , or <code>null</code> if there is no
     *    prefix<code>Node.localName</code> local name , extracted from 
     *   <code>qualifiedName</code><code>Element.tagName</code>
     *   <code>qualifiedName</code>
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified qualified name 
     *   contains an illegal character.
     *   <br> NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is 
     *   malformed, if the <code>qualifiedName</code> has a prefix and the 
     *   <code>namespaceURI</code> is <code>null</code> or an empty string, 
     *   or if the <code>qualifiedName</code> has a prefix that is "xml" and 
     *   the <code>namespaceURI</code> is different from " 
     *   http://www.w3.org/XML/1998/namespace "  .
     * @since DOM Level 2
     */
    public org.w3c.dom.Element createElementNS(String namespaceURI, 
					       String qualifiedName)
	throws DOMException {
	Object result = obj.call(HTMLConstants.FUNC_CREATE_ELEMENT_NS, new Object[]{namespaceURI, qualifiedName});
        if (result == null) {
            return null;
        }
        return DOMObjectFactory.createHTMLElement((DOMObject) result, this);
    }

    /**
     *  Returns the <code>Element</code> whose <code>ID</code> is given by 
     * <code>elementId</code> . If no such element exists, returns 
     * <code>null</code> . Behavior is not defined if more than one element 
     * has this <code>ID</code> .  The DOM implementation must have 
     * information that says which attributes are of type ID. Attributes with 
     * the name "ID" are not of type ID unless so defined. Implementations 
     * that do not know whether attributes are of type ID or not are expected 
     * to return <code>null</code> .
     * @param elementId  The unique <code>id</code> value for an element.
     * @return  The matching element.
     * @since DOM Level 2
     */
    public org.w3c.dom.Element getElementById(String elementId) {
	Object result = obj.call(HTMLConstants.FUNC_GET_ELEMENT_BY_ID, new Object[]{elementId});
        if (result == null) {
            return null;
        }
        return DOMObjectFactory.createHTMLElement((DOMObject) result, this);
    }

    private String getAttribute(String name) {
	return DOMObjectHelper.getStringMember(obj, name);
    }


    private void setAttribute(String name, String value) {
	DOMObjectHelper.setStringMember(obj, name, value);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.views.DocumentView
    //------------------------------------------------------------

    /**
     *  The default <code>AbstractView</code> for this <code>Document</code> , 
     * or <code>null</code> if none available.
     */
    public org.w3c.dom.views.AbstractView getDefaultView() {
	return new sun.plugin.dom.css.ViewCSS(this);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.stylesheets.DocumentStyle
    //------------------------------------------------------------

    /**
     *  A list containing all the style sheets explicitly linked into or 
     * embedded in a document. For HTML documents, this includes external 
     * style sheets, included via the HTML  LINK element, and inline  STYLE 
     * elements. In XML, this includes external style sheets, included via 
     * style sheet processing instructions (see ). 
     */
    public org.w3c.dom.stylesheets.StyleSheetList getStyleSheets() {
        return DOMObjectFactory.createStyleSheetList(obj.getMember(HTMLConstants.MEMBER_STYLESHEETS),
                                                     this);
    }


    //------------------------------------------------------------
    // Method from org.w3c.dom.css.DocumentCSS
    //------------------------------------------------------------

    /**
     *  This method is used to retrieve the override style declaration for a 
     * specified element and a specified pseudo-element. 
     * @param elt The element whose style is to be modified. This parameter 
     *   cannot be null. 
     * @param pseudoElt The pseudo-element or <code>null</code> if none. 
     * @return  The override style declaration. 
     */
    public org.w3c.dom.css.CSSStyleDeclaration getOverrideStyle(org.w3c.dom.Element elt, 
                                                String pseudoElt) {
	if (elt instanceof org.w3c.dom.css.ElementCSSInlineStyle) {
	    org.w3c.dom.css.ElementCSSInlineStyle element = (org.w3c.dom.css.ElementCSSInlineStyle)elt;
	    return element.getStyle();
	} else {
	    return null;
	}
    }
    /**
     * A <code>NodeList</code> that contains all children of this node. If 
     * there are no children, this is a <code>NodeList</code> containing no 
     * nodes.
     */
    public org.w3c.dom.NodeList getChildNodes() {
	return getElementsByTagName(TAG_HTML);
    }
}
