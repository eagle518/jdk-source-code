package com.sun.xml.internal.rngom.digested;

/**
 * @author Kohsuke Kawaguchi (kk@kohsuke.org)
 */
public class DZeroOrMorePattern extends DUnaryPattern {
    public boolean isNullable() {
        return true;
    }
    public Object accept( DPatternVisitor visitor ) {
        return visitor.onZeroOrMore(this);
    }
}
