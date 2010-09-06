/*
 * @(#)NumberEditor.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

