/*
 * @(#)UIComponent.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import org.jdesktop.swingx.designer.utils.HasUIDefaults;

import java.util.List;
import java.util.ArrayList;

/**
 * UIComponent - model node that represents the designs for a single swing component
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIComponent extends UIRegion implements HasUIDefaults {

    /** The classname of the swing component that this UIComponent represents */
    private String type;
    /** The name of the component if its a named component or null if its a generic component */
    private String componentName = null;
    /** If this components is opaque which means that when it is painted all of its bounds are filled */
    private boolean opaque = false;
    /**
     * A list of state types that are available to this region and sub regions of this component but not subcomponents
     * of this component. If this list is empty then the standard synth set of state types are assumed.
     */
    private List<UIStateType> stateTypes;

    // =================================================================================================================
    // Contructors

    /** no-args contructor for JIBX */
    protected UIComponent() {
        super();
        // create new observable list for state types so we get events for when the model changes
        stateTypes = new ArrayList<UIStateType>();
    }

    public UIComponent(String name, String type, String ui, UIRegion... subRegions) {
        super(name, ui, subRegions);
        this.type = type;
        for (UIRegion r : subRegions) {
            r.setRegion(this);
        }
        // create new observable list for state types so we get events for when the model changes
        stateTypes = new ArrayList<UIStateType>();
    }

    // =================================================================================================================
    // Bean Methods

    public String getComponentName() {
        return componentName;
    }

    public void setComponentName(String componentName) {
        String old = getComponentName();
        this.componentName = componentName;
        firePropertyChange("componentName", old, getComponentName());
    }

    public boolean isOpaque() {
        return opaque;
    }

    public void setOpaque(boolean opaque) {
        boolean old = isOpaque();
        this.opaque = opaque;
        firePropertyChange("opaque", old, isOpaque());
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        String old = getType();
        this.type = type;
        firePropertyChange("type", old, getType());
    }

    /**
     * Get the list of state types that are available to this region and sub regions of this component but not
     * subcomponents of this component. If this list is empty then the standard synth set of state types are assumed.
     *
     * @return List of available state types
     */
    public List<UIStateType> getStateTypes() {
        return stateTypes;
    }

    @Override public String getKey() {
        if (key == null || "".equals(key)) {
            if (componentName == null || "".equals(componentName)) {
                return getName();
            } else {
                return "\"" + componentName + "\"";
            }
        } else {
            return key;
        }
    }

    @Override public String getTitle() {
        if (title == null || "".equals(title)) {
            if (componentName == null || "".equals(componentName)) {
                return getName();
            } else {
                return componentName;
            }
        } else {
            return title;
        }
    }
}
