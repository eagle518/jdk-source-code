/*
 * @(#)Rect.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import sun.plugin.dom.*;

/**
 *  The <code>Rect</code> interface is used to represent any rect value. This 
 * interface reflects the values in the underlying style property. Hence, 
 * modifications made to the <code>CSSPrimitiveValue</code> objects modify 
 * the style property. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class Rect implements org.w3c.dom.css.Rect {
    private DOMObject obj;
    // Owner document
    private org.w3c.dom.Document document;

    public Rect(DOMObject obj, org.w3c.dom.Document document) {
        this.obj = obj;
        this.document = document;
    }

    public org.w3c.dom.css.CSSPrimitiveValue getTop() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_TOP),
                                                        document);
    }

    public org.w3c.dom.css.CSSPrimitiveValue getRight() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_RIGHT),
                                                        document);
    }

    public org.w3c.dom.css.CSSPrimitiveValue getBottom() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_BOTTOM),
                                                        document);
    }

    public org.w3c.dom.css.CSSPrimitiveValue getLeft() {
        return DOMObjectFactory.createCSSPrimitiveValue(obj.getMember(CSSConstants.ATTR_LEFT),
                                                        document);
    }

    public String toString() {
        return obj.toString();
    }
}
