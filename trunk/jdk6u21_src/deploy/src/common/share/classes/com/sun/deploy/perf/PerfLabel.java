/*
 * @(#)PerfLabel.java	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.perf;

import java.lang.Object;
import java.lang.String;

public class PerfLabel extends Object {
    public long getTime() {
	return (time);
    }

    public String getLabel() {
	return (label);
    }

    public PerfLabel() {
	this.time  = 0;
	this.label = "";
    }

    public PerfLabel(long time, String label) {
	this.time  = time;
	this.label = label;
    }

    private long   time;
    private String label;
}
