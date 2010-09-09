package com.sun.xml.internal.rngom.digested;

import com.sun.xml.internal.rngom.ast.om.ParsedElementAnnotation;
import org.w3c.dom.Element;

/**
 * @author Kohsuke Kawaguchi
 */
final class ElementWrapper implements ParsedElementAnnotation {
    final Element element;

    public ElementWrapper(Element e) {
        this.element = e;
    }
}
