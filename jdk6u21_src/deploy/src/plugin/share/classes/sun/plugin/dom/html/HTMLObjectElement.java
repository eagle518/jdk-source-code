/*
 * @(#)HTMLObjectElement.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.html;

import org.w3c.dom.*;
import org.w3c.dom.html.*;
import sun.plugin.dom.*;
import sun.plugin.dom.core.*;


/**
 *  Generic embedded object.  Note. In principle, all properties on the object 
 * element are read-write but in some environments some properties may be 
 * read-only once the underlying object is instantiated. See the  OBJECT 
 * element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLObjectElement extends sun.plugin.dom.html.HTMLElement 
			       implements org.w3c.dom.html.HTMLObjectElement
{
    /**
     * Construct a HTMLObjectElement object.
     */
    public HTMLObjectElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc){
	super(obj, doc);
    }
    
    /**
     *  Returns the <code>FORM</code> element containing this control. Returns 
     * <code>null</code> if this control is not within the context of a form. 
     */
    public org.w3c.dom.html.HTMLFormElement getForm() {
    	return DOMObjectFactory.createHTMLFormElement(obj.getMember(HTMLConstants.ATTR_FORM),
                                                      (org.w3c.dom.html.HTMLDocument)getOwnerDocument());
    }

    /**
     *  Applet class file. See the <code>code</code> attribute for 
     * HTMLAppletElement. 
     */
    public String getCode() {
	return getAttribute(HTMLConstants.ATTR_CODE);
    }

    public void setCode(String code) {
	setAttribute(HTMLConstants.ATTR_CODE, code);
    }


    /**
     *  Aligns this object (vertically or horizontally)  with respect to its 
     * surrounding text. See the  align attribute definition in HTML 4.0. 
     * This attribute is deprecated in HTML 4.0.
     */
    public String getAlign() {
	return getAttribute(HTMLConstants.ATTR_ALIGN);
    }

    public void setAlign(String align) {
	setAttribute(HTMLConstants.ATTR_ALIGN, align);
    }


    /**
     *  Space-separated list of archives. See the  archive attribute definition
     *  in HTML 4.0.
     */
    public String getArchive() {
	return getAttribute(HTMLConstants.ATTR_ARCHIVE);
    }

    public void setArchive(String archive) {
	setAttribute(HTMLConstants.ATTR_ARCHIVE, archive);
    }


    /**
     *  Width of border around the object. See the  border attribute definition
     *  in HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getBorder() {
	return getAttribute(HTMLConstants.ATTR_BORDER);
    }

    public void setBorder(String border) {
	setAttribute(HTMLConstants.ATTR_BORDER, border);
    }


    /**
     *  Base URI for <code>classid</code> , <code>data</code> , and
     * <code>archive</code> attributes. See the  codebase attribute definition
     *  in HTML 4.0.
     */
    public String getCodeBase() {
	return getAttribute(HTMLConstants.ATTR_CODEBASE);
    }

    public void setCodeBase(String codeBase){
	setAttribute(HTMLConstants.ATTR_CODEBASE, codeBase);
    }

    /**
     *  Content type for data downloaded via <code>classid</code> attribute. 
     * See the  codetype attribute definition in HTML 4.0.
     */
    public String getCodeType() {
	return getAttribute(HTMLConstants.ATTR_CODE_TYPE);
    }

    public void setCodeType(String codeType) {
	setAttribute(HTMLConstants.ATTR_CODE_TYPE, codeType);
    }

    /**
     *  A URI specifying the location of the object's data.  See the  data 
     * attribute definition in HTML 4.0.
     */
    public String getData() {
	return getAttribute(HTMLConstants.ATTR_DATA);
    }

    public void setData(String data) {
	setAttribute(HTMLConstants.ATTR_DATA, data);
    }


    /**
     *  Declare (for future reference), but do not instantiate, this object. 
     * See the  declare attribute definition in HTML 4.0.
     */
    public boolean getDeclare() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_DECLARE);
    }

    public void setDeclare(boolean declare) {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_DECLARE, declare);
    }


    /**
     *  Override height. See the  height attribute definition in HTML 4.0.
     */
    public String getHeight() {
	return getAttribute(HTMLConstants.ATTR_HEIGHT);
    }

    public void setHeight(String height){
	setAttribute(HTMLConstants.ATTR_HEIGHT, height);
    }

    /**
     *  Horizontal space to the left and right of this image, applet, or 
     * object. See the  hspace attribute definition in HTML 4.0. This 
     * attribute is deprecated in HTML 4.0.
     */
    public String getHspace(){
	return getAttribute(HTMLConstants.ATTR_HSPACE);
    }

    public void setHspace(String hspace) {
	setAttribute(HTMLConstants.ATTR_HSPACE, hspace);
    }

    /**
     *  Form control or object name when submitted with a form. See the  name 
     * attribute definition in HTML 4.0.
     */
    public String getName(){
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name){
	setAttribute(HTMLConstants.ATTR_NAME, name);
    }

    /**
     *  Message to render while loading the object. See the  standby attribute 
     * definition in HTML 4.0.
     */
    public String getStandby() {
	return getAttribute(HTMLConstants.ATTR_STANDBY);
    }

    public void setStandby(String standby) {
	setAttribute(HTMLConstants.ATTR_STANDBY, standby);
    }

    /**
     *  Index that represents the element's position in the tabbing order. See 
     * the  tabindex attribute definition in HTML 4.0.
     */
    public int getTabIndex(){
	return DOMObjectHelper.getIntMember(obj, HTMLConstants.ATTR_TAB_INDEX);
    }

    public void setTabIndex(int tabIndex){
	DOMObjectHelper.setIntMember(obj, HTMLConstants.ATTR_TAB_INDEX, tabIndex);
    }

    /**
     *  Content type for data downloaded via <code>data</code> attribute. See 
     * the  type attribute definition in HTML 4.0.
     */
    public String getType(){
	return getAttribute(HTMLConstants.ATTR_TYPE);
    }

    public void setType(String type) {
	setAttribute(HTMLConstants.ATTR_TYPE, type);
    }

    /**
     *  Use client-side image map. See the  usemap attribute definition in 
     * HTML 4.0.
     */
    public String getUseMap() {
	return getAttribute(HTMLConstants.ATTR_USE_MAP);
    }

    public void setUseMap(String useMap){
	setAttribute(HTMLConstants.ATTR_USE_MAP, useMap);
    }

    /**
     *  Vertical space above and below this image, applet, or object. See the  
     * vspace attribute definition in HTML 4.0. This attribute is deprecated 
     * in HTML 4.0.
     */
    public String getVspace(){
	return getAttribute(HTMLConstants.ATTR_VSPACE);
    }

    public void setVspace(String vspace) {
	setAttribute(HTMLConstants.ATTR_VSPACE, vspace);
    }

    /**
     *  Override width. See the  width attribute definition in HTML 4.0.
     */
    public String getWidth() {
	return getAttribute(HTMLConstants.ATTR_WIDTH);
    }

    public void setWidth(String width) {
	setAttribute(HTMLConstants.ATTR_WIDTH, width);
    }

    /**
     *  The document this object contains, if there is any and it is 
     * available, or <code>null</code> otherwise.
     * @since DOM Level 2
     */
    public org.w3c.dom.Document getContentDocument()
    {
        Object res = obj.getMember(HTMLConstants.ATTR_CONTENT_DOCUMENT);
        if (res != null && res instanceof DOMObject) {
            return new HTMLDocument((DOMObject) res, null);
        }
        return null;
    }
}

