/*
 * @(#)NumberEditor.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Abstract Property editor for a java builtin number types.
 *
 */

import java.beans.*;

abstract public class NumberEditor extends PropertyEditorSupport {

    public String getJavaInitializationString() {
	return ("" + getValue());
    }

}

