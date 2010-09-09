/*
 * @(#)DoubleEditor.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Property editor for a java builtin "double" type.
 *
 */

import java.beans.*;

public class DoubleEditor extends NumberEditor {

    public void setAsText(String text) throws IllegalArgumentException {
	setValue(Double.valueOf(text));
    }

}

