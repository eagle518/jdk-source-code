/*
 * @(#)DefaultPerfHelper.java	1.4 10/03/24 12:07:35
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.perf;

import java.lang.String;
import java.util.ArrayList;
import com.sun.deploy.util.SystemUtils;
import com.sun.deploy.config.Config;

/**
 * we store the time in micro seconds 10 pow -6
 *
 * Implements the extended time reference point
 */
public class DefaultPerfHelper implements PerfHelper 
{
    private long initTime;
    private long lastTime;
    private static ArrayList labelList = new ArrayList();
  
    public DefaultPerfHelper() {
        setInitTime(SystemUtils.microTime());
    }
    public DefaultPerfHelper(long t0) {
        setInitTime(t0);
    }

    public void setInitTime(long t0) {
        initTime = t0;
        lastTime = 0; // relative to initTime
    }
    public void put(String label) {
        long thisTime = SystemUtils.microTime() - initTime;

        long usec   = thisTime;
        long dtusec = thisTime-lastTime;
        System.out.println("PERF: "+usec+" us, dT "+dtusec+" us :"+label);

        lastTime=thisTime;
	labelList.add(new PerfLabel(lastTime, label));
	
    }
    public long put(long deltaStart, String label) {
        long thisTime = SystemUtils.microTime() - initTime;

        long usec = thisTime;
        long dtusec = thisTime-lastTime;
        long dtusecCust = thisTime-deltaStart;
        System.out.println("PERF: "+usec+" us, dT "+dtusec+" us, user dT "+dtusecCust+" us :"+label);

        lastTime=thisTime;
	labelList.add(new PerfLabel(lastTime, label));
	
        return thisTime;
    }
    public PerfLabel [] toArray() {	
	if (labelList.size() == 0) {
	    return null;
	}

	PerfLabel [] perfLabels = new PerfLabel[labelList.size()];
	for (int i = 0; i < labelList.size(); i++) {
	    perfLabels[i] = (PerfLabel)labelList.get(i);
	}
	return perfLabels;	
    }
}

