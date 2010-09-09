package com.sun.xml.internal.rngom.digested;

import com.sun.xml.internal.rngom.nc.NameClass;

/**
 * @author Kohsuke Kawaguchi (kk@kohsuke.org)
 */
public abstract class DXmlTokenPattern extends DUnaryPattern {
    private final NameClass name;

    public DXmlTokenPattern(NameClass name) {
        this.name = name;
    }

    /**
     * Gets the name class of this element/attribute.
     */
    public NameClass getName() {
        return name;
    }

    public final boolean isNullable() {
        return false;
    }
}
