/*
 * @(#)PrintJobFlavorException.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import javax.print.DocFlavor;
import javax.print.FlavorException;
import javax.print.PrintException;


class PrintJobFlavorException extends PrintException 
    implements FlavorException {
    
    private DocFlavor flavor;
    
    PrintJobFlavorException(String s, DocFlavor f) {
	super(s);
	flavor = f;
	}
    
    public DocFlavor[] getUnsupportedFlavors() {
	DocFlavor [] flavors = { flavor};
	    return flavors;
    }
}
