/*
 * @(#)HotspotRuntime.java	1.6 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.util.List;
import java.util.ArrayList;

/**
 * Implementation class of HotspotRuntimeMBean interface.
 *
 * Internal, uncommitted management interface for Hotspot runtime 
 * system.
 */
class HotspotRuntime 
    implements HotspotRuntimeMBean {

    private VMManagement jvm;

    /**
     * Constructor of HotspotRuntime class.
     */ 
    HotspotRuntime(VMManagement vm) {
        jvm = vm;
    }

    public long getSafepointCount() {
        return jvm.getSafepointCount();
    }

    public long getTotalSafepointTime() {
        return jvm.getTotalSafepointTime();
    }

    public long getSafepointSyncTime() {
        return jvm.getSafepointSyncTime();
    }

    public List getInternalFlagNames() {
        ManagementFactory.checkMonitorAccess();
        String[] names = getAllFlagNames();
        List result = new ArrayList(names.length);
        for (int i = 0; i < names.length; i++) {
            result.add(names[i]);
        } 
        return result;
    }

    public java.util.List getInternalFlags() {
        ManagementFactory.checkMonitorAccess();
        int numFlags = getInternalFlagCount(); 
        Flag[] flags = new Flag[numFlags];        

        // Get all internal flags with names = null
        int count = getFlags(null, flags, numFlags);
        List result = new ArrayList(count);
        for (int i = 0; i < count; i++) {
            result.add(flags[i]);
        } 
        return result;
    }
    private native int getFlags(String[] names, Flag[] flags, int count);
    private native String[] getAllFlagNames();
    private native int getInternalFlagCount();

    public Flag getFlag(String name) {
        ManagementFactory.checkMonitorAccess();
        if (name == null) {
            throw new NullPointerException();
        }
        Flag[] flags = new Flag[1];
        String[] names = new String[1];
        names[0] = name;
        int count = getFlags(names, flags, 1);
        if (count == 0) {
            return null;
        } else {
            return flags[0];
        }
    }

    // The following setter methods are not exported in the MBean interface.
    // We can define them in the MBean interface when we decide to support
    // that in Hotspot.

    /**
     * Sets <tt>LongFlag</tt> of the given name with the new value.
     * This method returns <tt>null</tt> if the flag of the given name
     * does not exist.
     *
     * @param name Name of a <tt>LongFlag</tt>
     * @param newValue new value to set
     *
     * @return a {@link LongFlag} object for the VM flag of the given name.
     *
     * @throws IllegalArgumentException if the new value is invalid.
     * @throws IllegalArgumentException if the flag is not a <tt>LongFlag</tt>.
     * @throws UnsupportedOperationException if the flag is not writeable.
     *
     * @throws  java.security.SecurityException
     *     if a security manager exists and the caller does not have
     *     ManagementPermission("control").
     */
    public LongFlag setLongFlag(String name, long newValue) {
        ManagementFactory.checkControlAccess();
        Flag flag = getFlag(name);
        if (flag == null) {
            return null; 
        }

        if (!flag.isWriteable()){
           throw new UnsupportedOperationException("Flag is not writeable");
        }

        if (flag instanceof LongFlag) {
           // Unsupported
           return null;
        } else {
           throw new IllegalArgumentException("Flag is not a LongFlag");
        }
    }

   /**
     * Sets <tt>BooleanFlag</tt> of the given name with the new value.
     * This method returns <tt>null</tt> if the flag of the given name
     * does not exist.
     *
     * @param name Name of a <tt>BooleanFlag</tt>
     * @param newValue new value to set
     *
     * @return a {@link BooleanFlag} object for the VM flag of the given name.
     *
     * @throws IllegalArgumentException if the new value is invalid.
     * @throws IllegalArgumentException if the flag is not a
     *     <tt>BooleanFlag</tt>.
     * @throws UnsupportedOperationException if the flag is not writeable.
     *
     * @throws  java.security.SecurityException
     *     if a security manager exists and the caller does not have
     *     ManagementPermission("control").
     */
    public BooleanFlag setBooleanFlag(String name, boolean newValue) {
        ManagementFactory.checkControlAccess();
        Flag flag = getFlag(name);
        if (flag == null) {
            return null; 
        }

        if (!flag.isWriteable()){
           throw new UnsupportedOperationException("Flag is not writeable");
        }

        if (flag instanceof BooleanFlag) {
           // Unsupported
           return null;
        } else {
           throw new IllegalArgumentException("Flag is not a BooleanFlag");
        }
    }

   /**
     * Sets <tt>StringFlag</tt> of the given name with the new value.
     * This method returns <tt>null</tt> if the flag of the given name
     * does not exist.
     *
     * @param name Name of a <tt>StringFlag</tt>
     * @param newValue new value to set
     *
     * @return a {@link StringFlag} object for the VM flag of the given name.
     *
     * @throws IllegalArgumentException if the new value is invalid.
     * @throws IllegalArgumentException if the flag is not a
     *      <tt>StringFlag</tt>.
     * @throws UnsupportedOperationException if the flag is not writeable.
     *
     * @throws  java.security.SecurityException
     *     if a security manager exists and the caller does not have
     *     ManagementPermission("control").
     */
    public StringFlag setStringFlag(String name, String newValue) {
        ManagementFactory.checkControlAccess();
        Flag flag = getFlag(name);
        if (flag == null) {
            return null; 
        }

        if (!flag.isWriteable()){
           throw new UnsupportedOperationException("Flag is not writeable");
        }

        if (flag instanceof StringFlag) {
           // Unsupported
           return null;
        } else {
           throw new IllegalArgumentException("Flag is not a StringFlag");
        }
    }

    // Performance counter support
    private static final String JAVA_RT          = "java.rt.";
    private static final String COM_SUN_RT       = "com.sun.rt.";
    private static final String SUN_RT           = "sun.rt.";
    private static final String JAVA_PROPERTY    = "java.property.";
    private static final String COM_SUN_PROPERTY = "com.sun.property.";
    private static final String SUN_PROPERTY     = "sun.property.";
    private static final String RT_COUNTER_NAME_PATTERN =
        JAVA_RT + "|" + COM_SUN_RT + "|" + SUN_RT + "|" +
        JAVA_PROPERTY + "|" + COM_SUN_PROPERTY + "|" + SUN_PROPERTY;

    public java.util.List getInternalRuntimeCounters() {
        return jvm.getInternalCounters(RT_COUNTER_NAME_PATTERN); 
    } 

    static {
        initialize();
    }
    private static native void initialize();
}
