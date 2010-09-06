/*
 * @(#)MemoryImpl.java	1.11 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.management.MemoryMXBean;
import java.lang.management.MemoryUsage;
import java.lang.management.MemoryNotificationInfo;
import java.lang.management.MemoryManagerMXBean;
import java.lang.management.MemoryPoolMXBean;
import javax.management.ObjectName;
import javax.management.MalformedObjectNameException;
import javax.management.MBeanNotificationInfo;
import javax.management.Notification;
import javax.management.NotificationEmitter;
import javax.management.NotificationFilter;
import javax.management.NotificationListener;
import javax.management.ListenerNotFoundException;
import javax.management.openmbean.CompositeData;
import java.util.List;
import java.util.ArrayList;
import java.util.ListIterator;
import java.util.Collections;

/**
 * Implementation class for the memory subsystem.
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getMemoryMXBean() returns an instance
 * of this class.
 */
class MemoryImpl extends NotificationEmitterSupport
                 implements MemoryMXBean {

    private final VMManagement jvm;

    private static MemoryPoolMXBean[] pools = null;
    private static MemoryManagerMXBean[] mgrs = null;

    /**
     * Constructor of MemoryImpl class
     */
    MemoryImpl(VMManagement vm) {
        super(MemoryMXBean.class);
        this.jvm = vm;
    }

    public int getObjectPendingFinalizationCount() {
        return sun.misc.VM.getFinalRefCount();
    }
    
    public void gc() {
        Runtime.getRuntime().gc();
    }

    // Need to make a VM call to get coherent value
    public MemoryUsage getHeapMemoryUsage() {
        return getMemoryUsage0(true);
    }

    public MemoryUsage getNonHeapMemoryUsage() {
        return getMemoryUsage0(false);
    }

    public boolean isVerbose() {
        return jvm.getVerboseGC();
    }

    public void setVerbose(boolean value) {
        ManagementFactory.checkControlAccess();

        setVerboseGC(value);
    }

    // The current Hotspot implementation does not support 
    // dynamically add or remove memory pools & managers.
    static synchronized MemoryPoolMXBean[] getMemoryPools() {
        if (pools == null) {
            pools = getMemoryPools0();
        }  
        return pools;
    }
    static synchronized MemoryManagerMXBean[] getMemoryManagers() {
        if (mgrs == null) {
            mgrs = getMemoryManagers0();
        }  
        return mgrs;
    }
    private static native MemoryPoolMXBean[] getMemoryPools0();
    private static native MemoryManagerMXBean[] getMemoryManagers0();
    private native MemoryUsage getMemoryUsage0(boolean heap);
    private native void setVerboseGC(boolean value);

    private final static String notifName = 
        "javax.management.Notification";
    private final static String[] notifTypes = {
        MemoryNotificationInfo.MEMORY_THRESHOLD_EXCEEDED,
        MemoryNotificationInfo.MEMORY_COLLECTION_THRESHOLD_EXCEEDED
    };
    private final static String[] notifMsgs  = {
        "Memory usage exceeds usage threshold",
        "Memory usage exceeds collection usage threshold"
    };

    private MBeanNotificationInfo[] notifInfo = null;
    public MBeanNotificationInfo[] getNotificationInfo() {
        synchronized (this) {
            if (notifInfo == null) {
                 notifInfo = new MBeanNotificationInfo[1];
                 notifInfo[0] = new MBeanNotificationInfo(notifTypes,
                                                          notifName,
                                                          "Memory Notification");
            }
        }
        return notifInfo;
    }

    private static String getNotifMsg(String notifType) {
        for (int i = 0; i < notifTypes.length; i++) {
            if (notifType == notifTypes[i]) {
                return notifMsgs[i];
            } 
        }
        return "Unknown message";
    }

    private static long seqNumber = 0;
    private static long getNextSeqNumber() {
        return ++seqNumber;
    }

    private static ObjectName objname = null;
    private static synchronized ObjectName getObjectName() {
        if (objname != null) return objname;

        try {
            objname = new ObjectName(java.lang.management.ManagementFactory.MEMORY_MXBEAN_NAME);
        } catch (MalformedObjectNameException e) {
            // should never reach here
            throw Util.newInternalError(e);
        }
        return objname;
    }

    static void createNotification(String notifType,
                                   String poolName,
                                   MemoryUsage usage,
                                   long count) {
        MemoryImpl mbean = (MemoryImpl) ManagementFactory.getMemoryMXBean();
        if (!mbean.hasListeners()) {
            // if no listener is registered.
            return;
        }
        long timestamp = System.currentTimeMillis();
        String msg = getNotifMsg(notifType); 
        Notification notif = new Notification(notifType,
                                              getObjectName(),
                                              getNextSeqNumber(),
                                              timestamp,
                                              msg);
        MemoryNotificationInfo info = 
            new MemoryNotificationInfo(poolName,
                                       usage,
                                       count);
        CompositeData cd = 
            MemoryNotifInfoCompositeData.toCompositeData(info);
        notif.setUserData(cd);
        mbean.sendNotification(notif);
    }

}

