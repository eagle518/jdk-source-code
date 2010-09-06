/*
 * @(#)StringEditor.java	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.beans.editors;

import java.beans.*;

public class StringEditor extends PropertyEditorSupport {

    public String getJavaInitializationString() {
	// We ought to handle escapes here...
	return "\"" + getValue() + "\"";
    }

    public void setAsText(String text) {
	setValue(text);
    }

}

