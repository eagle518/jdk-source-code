/*
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

public class DeployLock {
    private Thread  lockingThread = null;

    public synchronized boolean lock() throws InterruptedException{
	boolean lockSuccessful = false;
	Thread callingThread = Thread.currentThread();
	if (lockingThread != callingThread) {
	    while(lockingThread != null && lockingThread != callingThread){
		wait();
	    }
	    lockSuccessful = true;
	}
	lockingThread = callingThread;
	return lockSuccessful;
    }

    public synchronized void unlock(){
	if(this.lockingThread != Thread.currentThread()){
	    throw new IllegalMonitorStateException(
		"Calling thread has not locked this lock");
	}
	lockingThread = null;
	notify();
    }
}

