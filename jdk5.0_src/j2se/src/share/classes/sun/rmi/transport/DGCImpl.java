/*
 * @(#)DGCImpl.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport;

import java.rmi.*;
import java.rmi.dgc.DGC;
import java.rmi.dgc.Lease;
import java.rmi.dgc.VMID;
import java.rmi.server.LogStream;
import java.rmi.server.ObjID;
import java.rmi.server.RemoteServer;
import java.rmi.server.ServerNotActiveException;
import java.util.*;
import sun.rmi.runtime.Log;
import sun.rmi.runtime.NewThreadAction;
import sun.rmi.server.UnicastRef;
import sun.rmi.server.UnicastServerRef;
import sun.rmi.server.Util;
import sun.security.action.GetLongAction;

/**
 * This class implements the guts of the server-side distributed GC
 * algorithm
 *
 * @author Ann Wollrath
 */
final class DGCImpl implements DGC {

    /** "dgc" log level */
    static int logLevel = LogStream.parseLevel(getLogLevel());

    private static String getLogLevel() {
	return (String) java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.rmi.dgc.logLevel"));
    }

    /* dgc system log */
    static final Log dgcLog = Log.getLog("sun.rmi.dgc", "dgc",
					 DGCImpl.logLevel);

    /** general purpose timer with insulated thread context */
    static final Timer timer;

    /** lease duration to grant to clients */
    private static final long leaseValue =		// default 10 minutes
	((Long) java.security.AccessController.doPrivileged(
	    new GetLongAction("java.rmi.dgc.leaseValue",
			      600000))).longValue();

    /** lease check interval; default is half of lease grant duration */
    private static final long leaseCheckInterval =
	((Long) java.security.AccessController.doPrivileged(
	    new GetLongAction("sun.rmi.dgc.checkInterval",
			      leaseValue / 2))).longValue();

    /** remote implementation of DGC interface for this VM */
    private static DGCImpl dgc;
    /** table that maps VMID to LeaseInfo */
    private Hashtable leaseTable = new Hashtable();
    /** checks for lease expiration */
    private Thread checker = null;

    /**
     * Return the remote implementation of the DGC interface for
     * this VM.
     */
    static DGCImpl getDGCImpl()
    {
	return dgc;
    }

    /**
     * Construct a new server-side remote object collector at
     * a particular port. Disallow construction from outside.
     */
    private DGCImpl() {}

    /**
     * The dirty call adds the VMID "vmid" to the set of clients
     * that hold references to the object associated with the ObjID
     * id.  The long "sequenceNum" is used to detect late dirty calls.  If
     * the VMID "vmid" is null, a VMID will be generated on the
     * server (for use by the client in subsequent calls) and
     * returned.
     *
     * The client must call the "dirty" method to renew the lease
     * before the "lease" time expires or all references to remote
     * objects in this VM that the client holds are considered
     * "unreferenced".
     */
    public Lease dirty(ObjID[] ids, long sequenceNum, Lease lease)
    {
	VMID vmid = lease.getVMID();
	/*
	 * The server specifies the lease value; the client has
	 * no say in the matter.
	 */
	long duration = leaseValue;

	if (dgcLog.isLoggable(Log.VERBOSE)) {
	    dgcLog.log(Log.VERBOSE, "vmid = " + vmid);
	}

	// create a VMID if one wasn't supplied
	if (vmid == null) {
	    vmid = new VMID();

	    if (dgcLog.isLoggable(Log.BRIEF)) {
		String clientHost;
		try {
		    clientHost = RemoteServer.getClientHost();
		} catch (ServerNotActiveException e) {
		    clientHost = "<unknown host>";
		}
		dgcLog.log(Log.BRIEF, " assigning vmid " + vmid +
			   " to client " + clientHost);
	    }
	}

	lease = new Lease(vmid, duration);
	// record lease information
	synchronized (leaseTable) {
	    LeaseInfo info = (LeaseInfo)leaseTable.get(vmid);
	    if (info == null) {
		leaseTable.put(vmid, new LeaseInfo(vmid, duration));
		if (checker == null) {
		    checker = (Thread)
			java.security.AccessController.doPrivileged(
			    new NewThreadAction(new LeaseChecker(),
						"LeaseChecker", true));
		    checker.start();
		}
	    } else {
		info.renew(duration);
	    }
	}

	for (int i=0; i<ids.length; i++) {
	    if (dgcLog.isLoggable(Log.VERBOSE)) {
		dgcLog.log(Log.VERBOSE, "id = " + ids[i] +
			   ", vmid = " + vmid + ", duration = " + duration);
	    }

	    ObjectTable.referenced(ids[i], sequenceNum, vmid);
	}

	// return the VMID used
	return lease;
    }

    /**
     * The clean call removes the VMID from the set of clients
     * that hold references to the object associated with the LiveRef
     * ref.  The sequence number is used to detect late clean calls.  If the
     * argument "strong" is true, then the clean call is a result of a
     * failed "dirty" call, thus the sequence number for the VMID needs
     * to be remembered until the client goes away.
     */
    public void clean(ObjID[] ids, long sequenceNum, VMID vmid, boolean strong)
    {
	for (int i=0; i<ids.length; i++) {
	    if (dgcLog.isLoggable(Log.VERBOSE)) {
		dgcLog.log(Log.VERBOSE, "id = " + ids[i] +
		    ", vmid = " + vmid + ", strong = " + strong);
	    }

	    ObjectTable.unreferenced(ids[i], sequenceNum, vmid, strong);
	}
    }

    /**
     * Register interest in receiving a callback when this VMID
     * becomes inaccessible.
     */
    void registerTarget(VMID vmid, Target target) {
	synchronized (leaseTable) {
	    LeaseInfo info = (LeaseInfo)leaseTable.get(vmid);
	    if (info == null) {
		target.vmidDead(vmid);
	    } else {
		info.notifySet.add(target);
	    }
	}
    }

    /**
     * Remove notification request.
     */
    void unregisterTarget(VMID vmid, Target target) {
	synchronized (leaseTable) {
	    LeaseInfo info = (LeaseInfo)leaseTable.get(vmid);
	    if (info != null) {
		info.notifySet.remove(target);
	    }
	}
    }

    /**
     * Check if leases have expired.  If a lease has expired, remove
     * it from the table and notify all interested parties that the
     * VMID is essentially "dead".
     *
     * @return if true, there are leases outstanding; otherwise leases
     * no longer need to be checked
     */
    private boolean checkLeases() {
        long time = System.currentTimeMillis();
	Enumeration enum_ = null;
	LeaseInfo info = null;
	boolean leasesRemain = false;

	/* List of vmids that need to be removed from the leaseTable */
	Vector toUnregister = new Vector(5);

	/* Build a list of leaseInfo objects that need to have
	 * targets removed from their notifySet.  Remove expired
	 * leases from leaseTable.
	 */
	synchronized (leaseTable) {
	    enum_ = (Enumeration) leaseTable.elements();
	    while (enum_.hasMoreElements()) {
		info = (LeaseInfo) enum_.nextElement();
		if (info.expired(time)) {
		    toUnregister.add(info);
		    leaseTable.remove(info.vmid);			
		}
	    }
	    
	    if (leaseTable.isEmpty()) {
		checker = null;
		leasesRemain = false;
	    } else {
		leasesRemain = true;
	    }
	}
	
	/* Notify and unegister targets without holding the lock on
	 * the leaseTable so we avoid deadlock.
	 */
	Enumeration remove = toUnregister.elements();
	while (remove.hasMoreElements()) {
	    info = (LeaseInfo) remove.nextElement();
	    for (Iterator i = info.notifySet.iterator(); i.hasNext();) {
		Target target = (Target) i.next();
		target.vmidDead(info.vmid);
	    }
	}
	return leasesRemain;
    }

    static {
	/*
	 * Create shared timer here so that its thread will be isolated
	 * from the arbitrary current thread context.  Likewise, "export"
	 * the singleton DGCImpl in an isolated context.
	 */
	timer = (Timer) java.security.AccessController.doPrivileged(
	new java.security.PrivilegedAction() {
	    public Object run() {
		Timer newTimer;

		ClassLoader savedCcl =
		    Thread.currentThread().getContextClassLoader();
		try {
		    Thread.currentThread().setContextClassLoader(
			ClassLoader.getSystemClassLoader());
		    newTimer = new Timer(true);

		    /*
		     * Put remote collector object in table by hand to prevent
		     * listen on port.  (UnicastServerRef.exportObject would
		     * cause transport to listen.)
		     */
		    try {
			dgc = new DGCImpl();
			ObjID dgcID = new ObjID(ObjID.DGC_ID);
			LiveRef ref = new LiveRef(dgcID, 0);
			UnicastServerRef disp = new UnicastServerRef(ref);
			Remote stub =
			    Util.createProxy(DGCImpl.class,
					     new UnicastRef(ref), true);
			disp.setSkeleton(dgc);
			Target target =
			    new Target(dgc, disp, stub, dgcID, true);
			ObjectTable.putTarget(target);
		    } catch (RemoteException e) {
			throw new Error(
			    "exception initializing server-side DGC", e);
		    }
		} finally {
		    Thread.currentThread().setContextClassLoader(savedCcl);
		}
		return newTimer;
	    }
	});
    }

    /**
     * The lease checker runs in a thread that checks if leases have expired.
     * This thread stops when there are no more leases to be checked.
     */
    private class LeaseChecker implements Runnable {
        public void run() {
            do {
	        try {
		    Thread.sleep(leaseCheckInterval);
	        } catch (InterruptedException e) {
	        }
	    } while (checkLeases());
        }
    }

    private static class LeaseInfo {
        VMID vmid;
        long expiration;
        HashSet notifySet = new HashSet();

        LeaseInfo(VMID vmid, long lease) {
	    this.vmid = vmid;
	    expiration = System.currentTimeMillis() + lease;
        }

        synchronized void renew(long lease) {
	    long newExpiration = System.currentTimeMillis() + lease;
	    if (newExpiration > expiration)
	        expiration = newExpiration;
        }

        boolean expired(long time) {
            if (expiration < time) {
                if (dgcLog.isLoggable(Log.BRIEF)) {
                    dgcLog.log(Log.BRIEF, vmid.toString());
                }
                return true;
            } else {
                return false;
            }
        }
    }
}
