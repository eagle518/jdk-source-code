/*
 * @(#)ActivationGroupImpl.java	1.37 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.server;

import java.io.ObjectInputStream;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.URL;
import java.net.MalformedURLException;

import java.rmi.MarshalledObject;
import java.rmi.NoSuchObjectException;
import java.rmi.RMISecurityManager;
import java.rmi.Remote;
import java.rmi.RemoteException;

import java.rmi.activation.*;

import java.rmi.server.RMIClassLoader;
import java.rmi.server.RemoteObject;
import java.rmi.server.RemoteServer;
import java.rmi.server.UnicastRemoteObject;

import java.security.PrivilegedExceptionAction;
import java.security.AccessController;
import java.security.PrivilegedActionException;

import java.util.Hashtable;
import java.util.ArrayList;

import sun.rmi.registry.RegistryImpl;
import sun.rmi.server.LoaderHandler;


/**
 * The default activation group implementation.
 *
 * @version	1.37, 12/19/03
 * @author	Ann Wollrath
 * @since	JDK1.2
 * @see		java.rmi.activation.ActivationGroup
 */
public class ActivationGroupImpl
	extends ActivationGroup
{
    // use serialVersionUID from JDK 1.2.2 for interoperability
    private static final long serialVersionUID = 5758693559430427303L;

    /** maps persistent IDs to activated remote objects */
    private Hashtable active = new Hashtable(101);
    private boolean groupInactive = false;
    private ActivationGroupID groupID;
    private ArrayList lockedIDs = new ArrayList();

    /** formal parameters for activating object's constructor */
    private static Class[] objConstrParams = {
	ActivationID.class, MarshalledObject.class
    };

    /**
     * Creates a default activation group implementation.  Checks to
     * ensure that a security manager is set. If no manager is set,
     * then by default will set,
     * <code>java.rmi.RMISecurityManager</code> as the system security
     * manager.  (Note: as of JDK1.2beta4, this no longer interprets
     * the <code>data</code> as a Properties list; that function is
     * now handled by the activator.)
     * 
     * @param id the group's identifier
     * @param data ignored 
     */
    public ActivationGroupImpl(ActivationGroupID id, MarshalledObject data)
	throws RemoteException
    {
	super(id);
	groupID = id;
	
	if (System.getSecurityManager() == null) {
	    try {
		// Provide a default security manager.
		System.setSecurityManager(new java.rmi.RMISecurityManager());
		
	    } catch (Exception e) {
		throw new RemoteException("unable to set security manager", e);
	    }
	}
    }

    /*
     * Obtains a lock on the ActivationID id before returning. Allows only one
     * thread at a time to hold a lock on a particular id.  If the lock for id
     * is in use, all requests for an equivalent (in the Object.equals sense)
     * id will wait for the id to be notified and use the supplied id as the
     * next lock. The caller of "acquireLock" must execute the "releaseLock"
     * method" to release the lock and "notifyAll" waiters for the id lock
     * obtained from this method.  The typical usage pattern is as follows:
     *
     * try {
     *    acquireLock(id);
     *    // do stuff pertaining to id...
     * } finally {
     *    releaseLock(id);
     *    checkInactiveGroup();
     * }
     */
    private void acquireLock(ActivationID id) {

	ActivationID waitForID;

	for (;;) {
	
	    synchronized (lockedIDs) {
		int index = lockedIDs.indexOf(id);
		if (index < 0) {
		    lockedIDs.add(id);
		    return;
		} else {
		    waitForID = (ActivationID) lockedIDs.get(index);
		}
	    }
	
	    synchronized (waitForID) {
		synchronized (lockedIDs) {
		    int index = lockedIDs.indexOf(waitForID);
		    if (index < 0) continue;
		    ActivationID actualID =
			(ActivationID) lockedIDs.get(index);
		    if (actualID != waitForID)
			/*
			 * don't wait on an id that won't be notified.
			 */
			continue;
		}
	    
		try {
		    waitForID.wait();
		} catch (InterruptedException ignore) {
		}
	    }
	}

    }

    /*
     * Releases the id lock obtained via the "acquireLock" method and then
     * notifies all threads waiting on the lock.
     */
    private void releaseLock(ActivationID id) {
	synchronized (lockedIDs) {
	    id = (ActivationID) lockedIDs.remove(lockedIDs.indexOf(id));
	}

	synchronized (id) {
	    id.notifyAll();
	}
    }
    
    /**
     * Creates a new instance of an activatable remote object. The
     * <code>Activator</code> calls this method to create an activatable
     * object in this group. This method should be idempotent; a call to
     * activate an already active object should return the previously
     * activated object.
     *
     * Note: this method assumes that the Activator will only invoke
     * newInstance for the same object in a serial fashion (i.e.,
     * the activator will not allow the group to see concurrent requests
     * to activate the same object.
     *
     * @param id the object's activation identifier
     * @param desc the object's activation descriptor
     * @return a marshalled object containing the activated object's stub
     */
    public MarshalledObject newInstance(final ActivationID id,
					final ActivationDesc desc)
	throws ActivationException, RemoteException
    {
	RegistryImpl.checkAccess("ActivationInstantiator.newInstance");
	
	if (!groupID.equals(desc.getGroupID()))
	    throw new ActivationException("newInstance in wrong group");

	try {
	    acquireLock(id);
	    synchronized (this) {
		if (groupInactive == true)
		    throw new ActivationException("group is inactive");
	    }

	    ActiveEntry entry = (ActiveEntry) active.get(id);
	    if (entry != null)
		return entry.mobj;
		
	    String className = desc.getClassName();

	    final Class cl = RMIClassLoader.loadClass(desc.getLocation(),
						      className);
	    Remote impl = null;

	    final Thread t = Thread.currentThread();
	    final ClassLoader savedCcl = t.getContextClassLoader();
	    ClassLoader objcl = cl.getClassLoader();
	    final ClassLoader ccl = covers(objcl, savedCcl) ? objcl : savedCcl;
			  
	    /*
	     * Fix for 4164971: allow non-public activatable class
	     * and/or constructor, create the activatable object in a
	     * privileged block
	     */
	    try {
		/*
		 * The code below is in a doPrivileged block to
		 * protect against user code which code might have set
		 * a global socket factory (in which case application
		 * code would be on the stack).
		 */
  		impl = (Remote) AccessController.
  		  doPrivileged(new PrivilegedExceptionAction() {
  		      public Object run() throws InstantiationException, 
  			  NoSuchMethodException, IllegalAccessException,
  			  InvocationTargetException
  		      {
			  Constructor constructor = 
			      cl.getDeclaredConstructor(objConstrParams);
			  constructor.setAccessible(true);
			  Object[] params = new Object[] {id, desc.getData()};

			  try {
			      /*
			       * Fix for 4289544: make sure to set the
			       * context class loader to be the class
			       * loader of the impl class before
			       * constructing that class.
			       */
			      t.setContextClassLoader(ccl);
			      return (Remote) constructor.newInstance(params);
			      
			  } finally {
			      t.setContextClassLoader(savedCcl);  
			  }
		      }
		  });
 	    } catch (PrivilegedActionException pae) {
 		Throwable e = pae.getException();
 
 		// narrow the exception's type and rethrow it
 		if (e instanceof InstantiationException) {
 		    throw (InstantiationException) e;
 		} else if (e instanceof NoSuchMethodException) {
 		    throw (NoSuchMethodException) e;
 		} else if (e instanceof IllegalAccessException) {
 		    throw (IllegalAccessException) e;
 		} else if (e instanceof InvocationTargetException) {
 		    throw (InvocationTargetException) e;
 		} else if (e instanceof RuntimeException) {
 		    throw (RuntimeException) e;
 		} else if (e instanceof Error) {
 		    throw (Error) e;
 		}
	    }

	    entry = new ActiveEntry(impl);
	    active.put(id, entry);
	    return entry.mobj;

	} catch (NoSuchMethodException e) {
	    /* user forgot to provide activatable constructor? */
	    throw new ActivationException
		("Activatable object must provide an activation"+
		 " constructor", e);
		
	} catch (NoSuchMethodError e) {
	    /* code recompiled and user forgot to provide
	     *  activatable constructor? 
	     */
	    throw new ActivationException
		("Activatable object must provide an activation"+
		 " constructor", e );

	} catch (InvocationTargetException e) {
	    throw new ActivationException("exception in object constructor",
					  e.getTargetException());
		    
	} catch (Exception e) {
	    throw new ActivationException("unable to activate object", e);
	} finally {
	    releaseLock(id);
	    checkInactiveGroup();
	}
    }

    
   /**
    * The group's <code>inactiveObject</code> method is called
    * indirectly via a call to the <code>Activatable.inactive</code>
    * method. A remote object implementation must call
    * <code>Activatable</code>'s <code>inactive</code> method when
    * that object deactivates (the object deems that it is no longer
    * active). If the object does not call
    * <code>Activatable.inactive</code> when it deactivates, the
    * object will never be garbage collected since the group keeps
    * strong references to the objects it creates. <p>
    *
    * The group's <code>inactiveObject</code> method 
    * unexports the remote object from the RMI runtime so that the
    * object can no longer receive incoming RMI calls. This call will
    * only succeed if the object has no pending/executing calls. If
    * the object does have pending/executing RMI calls, then false
    * will be returned.
    *
    * If the object has no pending/executing calls, the object is
    * removed from the RMI runtime and the group informs its
    * <code>ActivationMonitor</code> (via the monitor's
    * <code>inactiveObject</code> method) that the remote object is
    * not currently active so that the remote object will be
    * re-activated by the activator upon a subsequent activation
    * request.
    *
    * @param id the object's activation identifier
    * @returns true if the operation succeeds (the operation will
    * succeed if the object in currently known to be active and is
    * either already unexported or is currently exported and has no
    * pending/executing calls); false is returned if the object has
    * pending/executing calls in which case it cannot be deactivated
    * @exception UnknownObjectException if object is unknown (may already
    * be inactive)
    * @exception RemoteException if call informing monitor fails
    */
    public boolean inactiveObject(ActivationID id)
	throws ActivationException, UnknownObjectException, RemoteException
    {

	try {
	    acquireLock(id);
	    synchronized (this) {
		if (groupInactive == true)
		    throw new ActivationException("group is inactive");
	    }
	    
	    ActiveEntry entry = (ActiveEntry)active.get(id);
	    if (entry == null) {
		// REMIND: should this be silent?
		throw new UnknownObjectException("object not active");
	    }
		
	    try {
		if (Activatable.unexportObject(entry.impl, false) == false)
		    return false;
	    } catch (NoSuchObjectException allowUnexportedObjects) {
	    }
	    
	    try {
		super.inactiveObject(id);
	    } catch (UnknownObjectException allowUnregisteredObjects) {
	    }

	    active.remove(id);
		
	} finally {
	    releaseLock(id);
	    checkInactiveGroup();
	}

	return true;
    }

    /*
     * Determines if the group has become inactive and
     * marks it as such.
     */
    private void checkInactiveGroup() {
	boolean groupMarkedInactive = false;
	synchronized (this) {
	    if (active.size() == 0 && lockedIDs.size() == 0 &&
		groupInactive == false)
	    {
		groupInactive = true;
		groupMarkedInactive = true;
	    }
	}

	if (groupMarkedInactive) {
	    try {
		super.inactiveGroup();
	    } catch (Exception ignoreDeactivateFailure) {
	    }

	    try {
		UnicastRemoteObject.unexportObject(this, true);
	    } catch (NoSuchObjectException allowUnexportedGroup) {
	    }
	}
    }

    /**
     * The group's <code>activeObject</code> method is called when an
     * object is exported (either by <code>Activatable</code> object
     * construction or an explicit call to
     * <code>Activatable.exportObject<code>. The group must inform its
     * <code>ActivationMonitor</code> that the object is active (via
     * the monitor's <code>activeObject</code> method) if the group
     * hasn't already done so.
     *
     * @param id the object's identifier
     * @param obj the remote object implementation
     * @exception UnknownObjectException if object is not registered
     * @exception RemoteException if call informing monitor fails
     */
    public void activeObject(ActivationID id, Remote impl)
	throws ActivationException, UnknownObjectException, RemoteException
    {
	
	try {
	    acquireLock(id);
	    synchronized (this) {
		if (groupInactive == true)
		    throw new ActivationException("group is inactive");
	    }
	    if (!active.contains(id)) {
		ActiveEntry entry = new ActiveEntry(impl);
		active.put(id, entry);
		// created new entry, so inform monitor of active object
		super.activeObject(id, entry.mobj);
	    }
	} finally {
	    releaseLock(id);
	    checkInactiveGroup();
	}
    }

    /**
     * Entry in table for active object.
     */
    private static class ActiveEntry {
	Remote impl;
	MarshalledObject mobj;
	
	ActiveEntry(Remote impl) throws ActivationException {
	    this.impl =  impl;
	    try {
		this.mobj = new MarshalledObject(impl);
	    } catch (java.io.IOException e) {
		throw new
		    ActivationException("failed to marshal remote object", e);
	    }
	}
    }

    /**
     * Returns true if the first argument is either equal to, or is a
     * descendant of, the second argument.  Null is treated as the root of
     * the tree.
     */
    private static boolean covers(ClassLoader sub, ClassLoader sup) {
	if (sup == null) {
	    return true;
	} else if (sub == null) {
	    return false;
	}
	do {
	    if (sub == sup) {
		return true;
	    }
	    sub = sub.getParent();
	} while (sub != null);
	return false;
    }
}
