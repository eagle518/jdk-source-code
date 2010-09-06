/*
 * @(#)Worker.java	1.7 04/06/22
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.util.*;

public class Worker extends Thread {
    ArrayList<Runnable> jobs = new ArrayList<Runnable>();
    private boolean stopped = false;
    
    public Worker(String name) {
	super("Worker-"+name);

	setPriority(NORM_PRIORITY - 1);
    }

    public void run() {
	while (!isStopped()) {
	    Runnable job;
	    synchronized(jobs) {
		while (!isStopped() && jobs.size() == 0) {
		    try {
			jobs.wait();
		    } catch (InterruptedException ex) {
		    }
		}

		if(isStopped()) break;
		
		job = jobs.remove(0);
	    }
	    job.run();
	}
    }
    
    private synchronized boolean isStopped() {
	return stopped;
    }

    public synchronized void stopWorker() {
	stopped = true;
	synchronized(jobs) {
	    jobs.notify();
	}
    }

    public void add(Runnable job) {
	synchronized(jobs) {
	    jobs.add(job);
	    jobs.notify();
	}
    }

    public boolean queueFull() {
	synchronized(jobs) {
	    return (jobs.size() > 0);
	}
    }
}
