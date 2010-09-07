/*
 * @(#)UIStateType.java	1.4 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel;

import org.jdesktop.beans.AbstractBean;

/**
 * UIStateType - A UIComponents has a collection of these which constitute the avilables states which can be chosen in
 * the components regions. A UIStateType can be either a custom state or one of the predefined standard states.
 *
 * @author  Richard Bair
 * @author  Jasper Potts
 */
public class UIStateType extends AbstractBean {
    /* Keys for standard synth states */
    public static final String ENABLED_KEY = "Enabled";
    public static final String MOUSE_OVER_KEY = "MouseOver";
    public static final String PRESSED_KEY = "Pressed";
    public static final String DISABLED_KEY = "Disabled";
    public static final String FOCUSED_KEY = "Focused";
    public static final String SELECTED_KEY = "Selected";
    public static final String DEFAULT_KEY = "Default";
    public static final String[] STANDARD_SYNTH_STATE_KEYS = new String[]{
            ENABLED_KEY, MOUSE_OVER_KEY, PRESSED_KEY, DISABLED_KEY, FOCUSED_KEY, SELECTED_KEY, DEFAULT_KEY
    };
    public static final UIStateType[] STANDARD_SYNTH_STATES = new UIStateType[]{
            new UIStateType(ENABLED_KEY),
            new UIStateType(MOUSE_OVER_KEY),
            new UIStateType(PRESSED_KEY),
            new UIStateType(DISABLED_KEY),
            new UIStateType(FOCUSED_KEY),
            new UIStateType(SELECTED_KEY),
            new UIStateType(DEFAULT_KEY)
    };

    /** Unique string for the ui key for this state, must be unique within a components set of UiStateTypes */
    private String key;
    /**
     * Snippet of java code that defines calculates the value of this state for a particular component. The varaiable
     * <code>c</code> is the component. You end with a return statement returning boolean true/false for the current
     * value of this state for this component. This can be null if the key is one of that standard synth states defined
     * in constants in this class.
     */
    private String codeSnippet;

    /** JIBX no-args contructor */
    private UIStateType() {}

    private UIStateType(String key) {
        this.key = key;
        this.codeSnippet = null;
    }

    public UIStateType(String key, String codeSnippet) {
        this.key = key;
        this.codeSnippet = codeSnippet;
    }

    // =================================================================================================================
    // Bean Methods

    /**
     * Get the ui defaults key for this state type. Unique string for the ui key for this state, must be unique within a
     * components set of UiStateTypes.
     *
     * @return Unique ui default key
     */
    public String getKey() {
        return key;
    }

    /**
     * Get the snippet of java code that defines calculates the value of this state for a particular component. The
     * varaiable <code>c</code> is the component. You end with a return statement returning boolean true/false for the
     * current value of this state for this component. This can be null if the key is one of that standard synth states
     * defined in constants in this class.
     *
     * @return Snippet of java code or null if this is a synth standard state
     */
    public String getCodeSnippet() {
        return codeSnippet;
    }

    /**
     * Set the snippet of java code that defines calculates the value of this state for a particular component. The
     * varaiable <code>c</code> is the component. You end with a return statement returning boolean true/false for the
     * current value of this state for this component. This can be null if the key is one of that standard synth states
     * defined in constants in this class.
     *
     * @param codeSnippet Snippet of java code or null if this is a synth standard state
     */
    public void setCodeSnippet(String codeSnippet) {
        this.codeSnippet = codeSnippet;
    }

    /**
     * Returns if this state type is a standard synth type and has no code snippet or a custom type that has a code
     * snippet. It is used by JIBX to determin if the code snippet should be written to XML.
     *
     * @return <code>true</code> if codeSnippet is non null
     */
    public boolean hasCodeSnippet() {
        return codeSnippet != null;
    }


}
