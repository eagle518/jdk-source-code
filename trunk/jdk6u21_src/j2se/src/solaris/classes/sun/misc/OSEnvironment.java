/*
 * @(#)OSEnvironment.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

public class OSEnvironment {
   
    /*
     * Initialize any miscellenous operating system settings that need to be set
     * for the class libraries.
     */ 
    public static void initialize() {                        
	// no-op on Solaris and Linux
    }
    
}    
