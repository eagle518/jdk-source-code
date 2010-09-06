/*
 * @(#)BoolEditor.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.beans.editors;

/**
 * Property editor for a java builtin "boolean" type.
 */

import java.beans.*;

public class BoolEditor extends PropertyEditorSupport {


    public String getJavaInitializationString() {
	// This must return locale independnet Java.
	if (((Boolean)getValue()).booleanValue()) {
	    return ("true");
	} else {
	    return ("false");
	}
    }

    public String getAsText() {
	// Should localize this.
	if (((Boolean)getValue()).booleanValue()) {
	    return ("True");
	} else {
	    return ("False");
	}
    }

    public void setAsText(String text) throws java.lang.IllegalArgumentException {
	if (text.toLowerCase().equals("true")) {
	    setValue(Boolean.TRUE);
	} else if (text.toLowerCase().equals("false")) {
	    setValue(Boolean.FALSE);
	} else {
	    throw new java.lang.IllegalArgumentException(text);
	}
    }

    public String[] getTags() {
	String result[] = { "True", "False" };
	return result;
    }

}

