/*
 * @(#)IntEditor.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Property editor for a java builtin "int" type.
 *
 */

import java.beans.*;

public class IntEditor extends NumberEditor {


    public void setAsText(String text) throws IllegalArgumentException {
	setValue(Integer.valueOf(text));
    }

}

