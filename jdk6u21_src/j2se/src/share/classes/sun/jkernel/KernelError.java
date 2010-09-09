/*
 * @(#)KernelError.java	1.2 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jkernel;

/**
 * Thrown to indicate that Java Kernel is unable to install a required bundle
 * and the JRE is therefore not adhering to specifications.
 */
public class KernelError extends VirtualMachineError {
    /**
     * Constructs a <code>KernelError</code> with no detail message. 
     */
    public KernelError() {
	super();
    }

    /**
     * Constructs a <code>KernelError</code> with the specified 
     * detail message. 
     *
     * @param   s   the detail message.
     */
    public KernelError(String s) {
	super(s);
    }
}
