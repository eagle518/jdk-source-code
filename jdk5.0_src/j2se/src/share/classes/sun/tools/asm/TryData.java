/*
 * @(#)TryData.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.util.Vector;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public final
class TryData {
    Vector catches = new Vector();
    Label endLabel = new Label();
    
    /**
     * Add a label
     */
    public CatchData add(Object type) {
	CatchData cd = new CatchData(type);
	catches.addElement(cd);
	return cd;
    }

    /**
     * Get a label
     */
    public CatchData getCatch(int n) {
	return (CatchData)catches.elementAt(n);
    }

    /**
     * Get the default label
     */
    public Label getEndLabel() {
	return endLabel;
    }
}
