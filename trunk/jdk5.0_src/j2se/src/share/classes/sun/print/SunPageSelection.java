/*
 * 1.3 @(#)SunPageSelection.java	1.3
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import javax.print.attribute.PrintRequestAttribute;
import javax.print.attribute.standard.Media;

/*
 * A class used to determine the range of pages to be printed.
 */
public final class SunPageSelection implements PrintRequestAttribute {

    public static final SunPageSelection ALL = new SunPageSelection(0);	
    public static final SunPageSelection RANGE = new SunPageSelection(1);
    public static final SunPageSelection SELECTION = new SunPageSelection(2);

    private int pages;

    public SunPageSelection(int value) {
        pages = value;
    }

    public final Class getCategory() {
        return SunPageSelection.class;
    }

    public final String getName() {
        return "sun-page-selection";
    }
 
    public String toString() {
       return "page-selection: " + pages;
    }
  
}
