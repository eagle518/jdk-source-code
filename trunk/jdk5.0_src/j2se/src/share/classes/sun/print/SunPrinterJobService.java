/*
 * @(#)SunPrinterJobService.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

 /*
  * A interface which indicates this service is implemented
  * by delegating to a subclass of PrinterJob delivered with the JRE
  * implementation
  */
public interface SunPrinterJobService {

    /**
     * This returns true if this service is implemented using the
     * platform's built-in subclass of PrinterJob.
     * ie the same class as the caller.
     */
    public boolean usesClass(Class c);
    
}
