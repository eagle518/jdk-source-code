/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import sun.plugin.dom.*;
import sun.plugin.dom.exception.NoModificationAllowedException;

/**
 *  The <code>CSSValue</code> interface represents a simple or a complex 
 * value. A <code>CSSValue</code> object only occurs in a context of a CSS 
 * property. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public class CSSValue implements org.w3c.dom.css.CSSValue {
    protected DOMObject obj;
    // Owner document
    protected org.w3c.dom.Document document;

    public CSSValue(DOMObject obj, org.w3c.dom.Document document) {
        this.obj = obj;
        this.document = document;
    }

    public String getCssText() {
        return DOMObjectHelper.getStringMember(obj, CSSConstants.ATTR_CSS_TEXT);

    }
    
    public void setCssText(String cssText) {
        obj.setMember(CSSConstants.ATTR_CSS_TEXT, cssText);
    }

    public short getCssValueType() {
        return ((Number) obj.getMember(CSSConstants.ATTR_CSS_VALUE_TYPE)).shortValue();
    }
}
