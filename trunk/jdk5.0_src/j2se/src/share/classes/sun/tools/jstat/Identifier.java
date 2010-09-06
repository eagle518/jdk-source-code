/*
 * @(#)Identifier.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

/**
 * An Expression subclass that describes the variable operands of an
 * expression. Objects of this type are always leaves of an expression tree.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class Identifier extends Expression {

    private String name;
    private Object value;

    public Identifier(String name) {
        super();
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void setValue(Object value) {
        this.value = value;
    }

    public Object getValue() {
        return value;
    }

    public boolean isResolved() {
        return value != null;
    }

    public String toString() {
        return name;
    }
}
