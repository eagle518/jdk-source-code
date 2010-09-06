/*
 * @(#)CatchData.java	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.util.Hashtable;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public final
class CatchData {
     Object type;
     Label label;

     /**
      * Constructor
      */
     CatchData(Object type) {
	 this.type = type;
	 this.label = new Label();
     }

     /**
      * Get the label
      */
     public Label getLabel() {
	 return label;
     }

     /**
      * Get the clazz
      */
     public Object getType() {
	 return type;
     }
}
