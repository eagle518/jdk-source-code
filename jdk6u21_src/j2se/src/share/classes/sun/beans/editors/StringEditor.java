/*
 * @(#)StringEditor.java	1.24 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

