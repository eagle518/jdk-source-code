/*
 * @(#)CSSValueList.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import java.util.ArrayList;
import sun.plugin.dom.*;

/**
 * The <code>CSSValueList</code> interface provides the abstraction of an 
 * ordered collection of CSS values.
 * <p> Some properties allow an empty list into their syntax. In that case, 
 * these properties take the <code>none</code> identifier. So, an empty list 
 * means that the property has the value <code>none</code>. 
 * <p> The items in the <code>CSSValueList</code> are accessible via an 
 * integral index, starting from 0. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class CSSValueList extends CSSValue 
			        implements org.w3c.dom.css.CSSValueList {
    public CSSValueList(DOMObject obj, org.w3c.dom.Document document) {
        super(obj, document);
    }

    public int getLength() {
        return ((Number) obj.getMember(CSSConstants.ATTR_LENGTH)).intValue();
    }

    public org.w3c.dom.css.CSSValue item(int index) {
        return DOMObjectFactory.createCSSValue(obj.call(CSSConstants.FUNC_ITEM,
                                                        new Object[] { new Integer(index) }),
                                               document);
    }
}
