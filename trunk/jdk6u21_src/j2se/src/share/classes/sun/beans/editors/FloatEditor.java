/*
 * @(#)FloatEditor.java	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Property editor for a java builtin "float" type.
 *
 */

import java.beans.*;

public class FloatEditor extends NumberEditor {

    public String getJavaInitializationString() {
	return (getValue() + "F");
    }

    public void setAsText(String text) throws IllegalArgumentException {
	setValue(Float.valueOf(text));
    }

}

