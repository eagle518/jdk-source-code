/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */
 
 /*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See W3C License http://www.w3.org/Consortium/Legal/ for more details.
 */

package sun.plugin.dom.stylesheets;

import org.w3c.dom.DOMException;
import org.w3c.dom.Node;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;


/**
 *  The <code>StyleSheet</code> interface is the abstract base interface for 
 * any type of style sheet. It represents a single style sheet associated 
 * with a structured document. In HTML, the StyleSheet interface represents 
 * either an external style sheet, included via the HTML  LINK element, or 
 * an inline  STYLE element. In XML, this interface represents an external 
 * style sheet, included via a style sheet processing instruction. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class StyleSheet implements org.w3c.dom.stylesheets.StyleSheet 
{
    // Underlying DOMObject
    protected DOMObject obj;

    // Owner document
    protected org.w3c.dom.Document  doc;
    protected org.w3c.dom.Node	    owner;
    

    /**
     * Construct a HTML StyleSheet object.
     */
    public StyleSheet(DOMObject obj, org.w3c.dom.Document doc, org.w3c.dom.Node owner) {
	this.obj = obj;
	this.doc = doc;
	this.owner = owner;
    }

    /**
     *  This specifies the style sheet language for this style sheet. The 
     * style sheet language is specified as a content type (e.g. 
     * "text/css"). The content type is often specified in the 
     * <code>ownerNode</code>. Also see the type attribute definition for 
     * the <code>LINK</code> element in HTML 4.0, and the type 
     * pseudo-attribute for the XML style sheet processing instruction. 
     */
    public String getType() {
	return DOMObjectHelper.getStringMemberNoEx(obj, SSLConstants.ATTR_TYPE);
    }

    /**
     *  <code>false</code> if the style sheet is applied to the document. 
     * <code>true</code> if it is not. Modifying this attribute may cause a 
     * new resolution of style for the document. A stylesheet only applies 
     * if both an appropriate medium definition is present and the disabled 
     * attribute is false. So, if the media doesn't apply to the current 
     * user agent, the <code>disabled</code> attribute is ignored. 
     */
    public boolean getDisabled() {
	return DOMObjectHelper.getBooleanMemberNoEx(obj, SSLConstants.ATTR_DISABLED);
    }

    public void setDisabled(boolean disabled){
	DOMObjectHelper.setBooleanMemberNoEx(obj, SSLConstants.ATTR_DISABLED, disabled);
    }

    /**
     *  The node that associates this style sheet with the document. For HTML, 
     * this may be the corresponding <code>LINK</code> or <code>STYLE</code> 
     * element. For XML, it may be the linking processing instruction. For 
     * style sheets that are included by other style sheets, the value of 
     * this attribute is <code>null</code>. 
     */
    public org.w3c.dom.Node getOwnerNode() {
	if(owner != null)
	    return owner;

	try {
	    Object result = obj.getMember(SSLConstants.ATTR_OWNER_ELEMENT);
	    if(result != null && result instanceof DOMObject) {
		return (Node)DOMObjectFactory.createHTMLElement((DOMObject)result, 
		    (org.w3c.dom.html.HTMLDocument)doc);
	    }
	}catch(DOMException e) {
	}

	return null;
    }

    /**
     *  For style sheet languages that support the concept of style sheet 
     * inclusion, this attribute represents the including style sheet, if 
     * one exists. If the style sheet is a top-level style sheet, or the 
     * style sheet language does not support inclusion, the value of this 
     * attribute is <code>null</code>. 
     */
    public org.w3c.dom.stylesheets.StyleSheet getParentStyleSheet() {
	try {
	    Object result = obj.getMember(SSLConstants.ATTR_PARENT_STYLESHEET);
	    if(result != null && result instanceof DOMObject) {
		return DOMObjectFactory.createStyleSheet((DOMObject)result, doc, getOwnerNode());
	    }
	}catch(DOMException e) {
	}
	
	return null; 
    }

    /**
     *  If the style sheet is a linked style sheet, the value of its attribute 
     * is its location. For inline style sheets, the value of this attribute 
     * is <code>null</code>. See the href attribute definition for the 
     * <code>LINK</code> element in HTML 4.0, and the href pseudo-attribute 
     * for the XML style sheet processing instruction. 
     */
    public String getHref() {
	return DOMObjectHelper.getStringMemberNoEx(obj, SSLConstants.ATTR_HREF);
    }

    /**
     *  The advisory title. The title is often specified in the 
     * <code>ownerNode</code>. See the title attribute definition for the 
     * <code>LINK</code> element in HTML 4.0, and the title pseudo-attribute 
     * for the XML style sheet processing instruction. 
     */
    public String getTitle() {
	return DOMObjectHelper.getStringMemberNoEx(obj, SSLConstants.ATTR_TITLE);
    }

    /**
     *  The intended destination media for style information. The media is 
     * often specified in the <code>ownerNode</code>. If no media has been 
     * specified, the <code>MediaList</code> will be empty. See the media 
     * attribute definition for the <code>LINK</code> element in HTML 4.0, 
     * and the media pseudo-attribute for the XML style sheet processing 
     * instruction . Modifying the media list may cause a change to the 
     * attribute <code>disabled</code>. 
     */
    public org.w3c.dom.stylesheets.MediaList getMedia() {
	try {
	    Object result = obj.getMember(SSLConstants.ATTR_MEDIA);

	    if (result != null && result instanceof DOMObject){
		Object ret = DOMObjectFactory.createStyleSheetObject((DOMObject)result, doc, owner);
		if(ret != null && ret instanceof org.w3c.dom.stylesheets.MediaList) {
		    return (org.w3c.dom.stylesheets.MediaList)ret;
		}
	    }
	}
	catch (DOMException e) {    
	}

	return null;
    }
}
