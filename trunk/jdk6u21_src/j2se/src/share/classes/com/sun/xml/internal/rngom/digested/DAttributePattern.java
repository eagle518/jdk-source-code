package com.sun.xml.internal.rngom.digested;

import com.sun.xml.internal.rngom.nc.NameClass;

/**
 * @author Kohsuke Kawaguchi (kk@kohsuke.org)
 */
public class DAttributePattern extends DXmlTokenPattern {
    public DAttributePattern(NameClass name) {
        super(name);
    }
    public Object accept( DPatternVisitor visitor ) {
        return visitor.onAttribute(this);
    }
}
