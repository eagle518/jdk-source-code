/*
 * @(#)NativePerfHelper.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.perf;

import java.lang.Object;
import java.lang.String;
import com.sun.deploy.perf.PerfHelper;
import com.sun.deploy.perf.PerfLabel;
import com.sun.deploy.config.Config;

public class NativePerfHelper extends Object
                        implements PerfHelper {
    
    static {
        Config.getInstance().loadDeployNativeLib();       
    }

    // nop functionality in respect to the extended time reference point
    public void setInitTime(long t0) { } 

    // nop functionality in respect to the extended time reference point
    public long put(long deltaStart, String label) 
    {
        put(label);
        return 0;
    }

    public native void put(String label);

    public native PerfLabel [] toArray();

    /**
     * Constructs a <code>PerfHelper</code> that uses native based storage for
     * labels.
     */
    public NativePerfHelper() {	
    }
}
