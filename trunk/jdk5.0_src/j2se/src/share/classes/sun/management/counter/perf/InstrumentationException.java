/*
 * @(#)InstrumentationException.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.counter.perf;

public class InstrumentationException extends RuntimeException {
    /**
     * Constructs a <tt>InstrumentationException</tt> with no  
     * detail mesage.
     */
     public InstrumentationException() {
     }

    /**
     * Constructs a <tt>InstrumentationException</tt> with a specified
     * detail mesage.
     *
     * @param message the detail message
     */
     public InstrumentationException(String message) {
         super(message);
     }
}
