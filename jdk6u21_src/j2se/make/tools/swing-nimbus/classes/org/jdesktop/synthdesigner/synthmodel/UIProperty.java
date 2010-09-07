/*
 * @(#)UIProperty.java	1.5 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import org.jdesktop.beans.AbstractBean;

/**
 * UIProperty
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIProperty extends AbstractBean {
    public static enum PropertyType {
        BOOLEAN, INT, FLOAT, DOUBLE, STRING, FONT, COLOR, INSETS, DIMENSION, BORDER
    }

    private String name;
    private PropertyType type;
    private Object value;

    protected UIProperty() {
    }

    public UIProperty(String name, PropertyType type, Object value) {
        this.name = name;
        this.type = type;
        this.value = value;
    }

    // =================================================================================================================
    // Bean Methods

    public String getName() {
        return name;
    }

    public void setName(String name) {
        String old = getName();
        this.name = name;
        firePropertyChange("name", old, getName());
    }

    public PropertyType getType() {
        return type;
    }

    public void setType(PropertyType type) {
        PropertyType old = getType();
        this.type = type;
        firePropertyChange("type", old, getType());
    }

    public Object getValue() {
        return value;
    }

    public void setValue(Object value) {
        Object old = getValue();
        this.value = value;
        firePropertyChange("value", old, getValue());
    }
}
