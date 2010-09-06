/*
 * @(#)RemoteHost.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.monitor.remote;

import sun.jvmstat.monitor.*;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.io.IOException;

/**
 * Remote Interface for discovering and attaching to remote
 * monitorable Java Virtual Machines.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public interface RemoteHost extends Remote {

    /**
     * Remote method to attach to a remote HotSpot Java Virtual Machine
     * identified by <code>vmid</code>.
     *
     * @param vmid The identifier for the target virtual machine.
     * @return RemoteVm - A remote object for accessing the remote Java
     *                    Virtual Machine.
     *
     * @throws MonitorException Thrown when any other error is encountered
     *                          while communicating with the target virtual
     *                          machine.
     * @throws RemoteException
     *
     */
    RemoteVm attachVm(int vmid, String mode) throws RemoteException,
                                                    MonitorException;

    /**
     * Remote method to detach from a remote HotSpot Java Virtual Machine
     * identified by <code>vmid</code>.
     *
     * @param rvm The remote object for the target Java Virtual
     *            Machine.
     *
     * @throws MonitorException Thrown when any other error is encountered
     *                          while communicating with the target virtual
     *                          machine.
     * @throws RemoteException
     */
    void detachVm(RemoteVm rvm) throws RemoteException, MonitorException;

    /**
     * Get a list of Local Virtual Machine Identifiers for the active
     * Java Virtual Machine the remote system. A Local Virtual Machine
     * Identifier is also known as an <em>lvmid</em>.
     *
     * @return int[] - A array of <em>lvmid</em>s.
     * @throws MonitorException Thrown when any other error is encountered
     *                          while communicating with the target virtual
     *                          machine.
     * @throws RemoteException
     */
    int[] activeVms() throws RemoteException, MonitorException;
}
