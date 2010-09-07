/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.hotspot.igv.data;

import java.io.Serializable;

/**
 *
 * @author Thomas Wuerthinger
 */
public class Property implements Serializable {

    public static final long serialVersionUID = 1L;

    private String name;
    private String value;

    private Property() {
        this(null, null);
    }

    private Property(Property p) {
        this(p.getName(), p.getValue());
    }

    private Property(String name) {
        this(name, null);
    }

    public Property(String name, String value) {
        this.name = name;
        this.value = value;
    }

    public String getName() {
        return name;
    }

    public String getValue() {
        return value;
    }

    @Override
    public String toString() {
        return name + " = " + value + "; ";
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Property)) return false;
        Property p2 = (Property)o;
        return name.equals(p2.name) && value.equals(p2.value);
    }
    @Override
    public int hashCode() {
        return name.hashCode() + value == null ? 0 : value.hashCode();
    }
}
