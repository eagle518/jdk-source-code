/*
 * @(#)HTMLImageElement.java	1.8 10/03/24
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
 *  Embedded image. See the  IMG element definition in HTML 4.0.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public class HTMLImageElement extends sun.plugin.dom.html.HTMLElement 
			      implements org.w3c.dom.html.HTMLImageElement
{
    /**
     * Construct a HTMLImageElement object.
     */
    public HTMLImageElement(DOMObject obj, 
			    org.w3c.dom.html.HTMLDocument doc) {
	super(obj, doc);
    }

    /**
     *  URI designating the source of this image, for low-resolution output. 
     */
    public String getLowSrc() {
	return getAttribute(HTMLConstants.ATTR_LOWSRC);
    }


    public void setLowSrc(String lowSrc) {
	setAttribute(HTMLConstants.ATTR_LOWSRC, lowSrc);
    }

    /**
     *  The name of the element (for backwards compatibility). 
     */
    public String getName() {
	return getAttribute(HTMLConstants.ATTR_NAME);
    }

    public void setName(String name) {
	setAttribute(HTMLConstants.ATTR_NAME, name);
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
     *  Alternate text for user agents not rendering the normal content of 
     * this element. See the  alt attribute definition in HTML 4.0.
     */
    public String getAlt() {
	return getAttribute(HTMLConstants.ATTR_ALT);
    }

    public void setAlt(String alt) {
	setAttribute(HTMLConstants.ATTR_ALT, alt);
    }

    /**
     *  Width of border around image. See the  border attribute definition in 
     * HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getBorder() {
	return getAttribute(HTMLConstants.ATTR_BORDER);
    }

    public void setBorder(String border) {
	setAttribute(HTMLConstants.ATTR_BORDER, border);
    }

    /**
     *  Override height. See the  height attribute definition in HTML 4.0.
     */
    public String getHeight() {
	return getAttribute(HTMLConstants.ATTR_HEIGHT);
    }

    public void setHeight(String height) {
	setAttribute(HTMLConstants.ATTR_HEIGHT, height);
    }
    /**
     *  Horizontal space to the left and right of this image. See the  hspace 
     * attribute definition in HTML 4.0. This attribute is deprecated in HTML 
     * 4.0.
     */
    public String getHspace() {
	return getAttribute(HTMLConstants.ATTR_HSPACE);
    }

    public void setHspace(String hspace) {
	setAttribute(HTMLConstants.ATTR_HSPACE, hspace);
    }

    /**
     *  Use server-side image map. See the  ismap attribute definition in HTML 
     * 4.0.
     */
    public boolean getIsMap() {
	return DOMObjectHelper.getBooleanMember(obj, HTMLConstants.ATTR_IS_MAP);
    }

    public void setIsMap(boolean isMap)
    {
	DOMObjectHelper.setBooleanMember(obj, HTMLConstants.ATTR_IS_MAP, isMap);
    }

    /**
     *  URI designating a long description of this image or frame. See the  
     * longdesc attribute definition in HTML 4.0.
     */
    public String getLongDesc() {
	return getAttribute(HTMLConstants.ATTR_LONGDESC);
    }

    public void setLongDesc(String longDesc) {
	setAttribute(HTMLConstants.ATTR_LONGDESC, longDesc);
    }

    /**
     *  URI designating the source of this image. See the  src attribute 
     * definition in HTML 4.0.
     */
    public String getSrc() {
	return getAttribute(HTMLConstants.ATTR_SRC);
    }

    public void setSrc(String src) {
	setAttribute(HTMLConstants.ATTR_SRC, src);
    }

    /**
     *  Use client-side image map. See the  usemap attribute definition in 
     * HTML 4.0.
     */
    public String getUseMap() {
	return getAttribute(HTMLConstants.ATTR_USE_MAP);
    }

    public void setUseMap(String useMap) {
	setAttribute(HTMLConstants.ATTR_USE_MAP, useMap);
    }

    /**
     *  Vertical space above and below this image. See the  vspace attribute 
     * definition in HTML 4.0. This attribute is deprecated in HTML 4.0.
     */
    public String getVspace() {
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
}

