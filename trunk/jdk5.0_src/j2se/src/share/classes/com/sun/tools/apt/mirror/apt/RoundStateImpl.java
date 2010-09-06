/*
 * @(#)RoundStateImpl.java	1.1 04/06/25
 *
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.tools.apt.mirror.apt;

import com.sun.mirror.apt.RoundState;

public class RoundStateImpl implements RoundState {
    private final boolean finalRound;
    private final boolean errorRaised;
    private final boolean sourceFilesCreated;
    private final boolean classFilesCreated;

    public RoundStateImpl(boolean errorRaised,
			  boolean sourceFilesCreated,
			  boolean classFilesCreated) {
	 this.finalRound = errorRaised || 
	                   !(sourceFilesCreated || 
			     classFilesCreated);
	 this.errorRaised = errorRaised;
	 this.sourceFilesCreated = sourceFilesCreated;
	 this.classFilesCreated = classFilesCreated;
    }

    public boolean finalRound() {
	return finalRound;
    }
 
    public boolean errorRaised() {
	return errorRaised;
    }
 
    public boolean sourceFilesCreated() {
	return sourceFilesCreated;
    }
 
    public boolean classFilesCreated() {
	return classFilesCreated;
    }

    public String toString() {
	return 
	    "[final round: " +  finalRound + 
	    ", error raised: " +  errorRaised + 
	    ", source files created: " + sourceFilesCreated +
	    ", class files created: " + classFilesCreated + "]";
    }
}
