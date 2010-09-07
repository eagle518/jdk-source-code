/*
 * @(#)Counter.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.dom.css;

import org.w3c.dom.DOMException;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import sun.plugin.dom.*;

/**
 *  The <code>Counter</code> interface is used to represent any counter or 
 * counters function value. This interface reflects the values in the 
 * underlying style property. 
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Style-20001113'>Document Object Model (DOM) Level 2 Style Specification</a>.
 * @since DOM Level 2
 */
public final class Counter implements org.w3c.dom.css.Counter {
    private DOMObject obj;

    public Counter(DOMObject obj) {
        this.obj = obj;
    }

    public String getIdentifier() {
        return (String) obj.getMember(CSSConstants.ATTR_IDENTIFIER);
    }

    public String getListStyle() {
        return (String) obj.getMember(CSSConstants.ATTR_LIST_STYLE);
    }

    public String getSeparator() {
        return (String) obj.getMember(CSSConstants.ATTR_SEPARATOR);
    }
}
