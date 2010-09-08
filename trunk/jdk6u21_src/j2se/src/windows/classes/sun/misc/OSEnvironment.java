/*
 * @(#)OSEnvironment.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import sun.io.Win32ErrorMode;

public class OSEnvironment {

    /*
     * Initialize any miscellenous operating system settings that need to be set
     * for the class libraries.
     * <p>
     * At this time only the process-wide error mode needs to be set.
     */    
    public static void initialize() {                        
        Win32ErrorMode.initialize();                                       
    }
    
}    
