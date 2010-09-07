/*
 * @(#)UIDefault.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import org.jdesktop.beans.AbstractBean;
import org.jdesktop.swingx.designer.utils.HasUIDefaults;
import org.jibx.runtime.IUnmarshallingContext;

import javax.swing.UIDefaults;

/**
 * Represents an entry in the UI defaults table.
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIDefault<T> extends AbstractBean implements HasUIDefaults {
    private String name;
    private T value;
    /**
     * This is a local UIDefaults that contains all the UIDefaults in the synth model. It is kept uptodate by the
     * indervidual UIDefaults nodes
     */
    private transient UIDefaults modelDefaults = null;

    public UIDefault() {
    }

    public UIDefault(String name, T value) {
        this.name = name;
        this.value = value;
    }

    public UIDefault(String name, T value, UIDefaults modelDefaults) {
        this.name = name;
        this.value = value;
        this.modelDefaults = modelDefaults;
    }

    // =================================================================================================================
    // JIBX Methods

    /**
     * Called by JIBX after all fields have been set
     *
     * @param context The JIBX Unmarshalling Context
     */
    private void postSet(IUnmarshallingContext context) {
        // walk up till we get synth model
        for (int i = 0; i < context.getStackDepth(); i++) {
            if (context.getStackObject(i) instanceof HasUIDefaults) {
                modelDefaults = ((HasUIDefaults) context.getStackObject(i)).getUiDefaults();
                if (modelDefaults != null) break;
            }
        }
    }

    // =================================================================================================================
    // Bean Methods

    /**
     * Get the local UIDefaults that contains all the UIDefaults in the synth model. It is kept uptodate by the
     * indervidual UIDefaults nodes
     *
     * @return The UIDefaults for the synth model
     */
    public UIDefaults getUiDefaults() {
        return modelDefaults;
    }

    public void setValue(T t) {
        T old = this.value;
        this.value = t;
        firePropertyChange("value", old, getValue());
    }

    public T getValue() {
        return value;
    }

    public final String getName() {
        return name;
    }

    public void setName(String name) {
        String old = this.name;
        firePropertyChange("name", old, name);
        this.name = name;
        // update model defaults
        if (old != null) modelDefaults.remove(old);
        modelDefaults.put(getName(), getValue());
    }

}
