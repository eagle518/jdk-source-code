/*
 * @(#)GarbageCollectorImpl.java	1.9 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import com.sun.management.GarbageCollectorMXBean;
import java.lang.management.ManagementFactory;
import java.lang.management.MemoryPoolMXBean;
import java.lang.management.MemoryUsage;

import com.sun.management.GcInfo;
import javax.management.openmbean.CompositeData;
import javax.management.MBeanInfo;
import javax.management.MBeanAttributeInfo;

import java.util.List;
import java.util.ListIterator;

/**
 * Implementation class for the garbage collector.
 * Standard and committed hotspot-specific metrics if any.
 *
 * ManagementFactory.getGarbageCollectorMXBeans() returns a list 
 * of instances of this class.
 */
class GarbageCollectorImpl extends MemoryManagerImpl
    implements GarbageCollectorMXBean {

    GarbageCollectorImpl(String name) {
        super(name, GarbageCollectorMXBean.class);
    }

    public native long getCollectionCount();
    public native long getCollectionTime();


    // The memory pools are static and won't be changed.
    // TODO: If the hotspot implementation begins to have pools
    // dynamically created and removed, this needs to be modified.
    private String[] poolNames = null;
    synchronized String[] getAllPoolNames() {
        if (poolNames == null) {
            List pools = ManagementFactory.getMemoryPoolMXBeans();
            poolNames = new String[pools.size()];
            int i = 0;
            for (ListIterator iter = pools.listIterator();  
                 iter.hasNext(); 
                 i++) {
                MemoryPoolMXBean p = (MemoryPoolMXBean) iter.next();
                poolNames[i] = p.getName();
            } 
        } 
        return poolNames;
    }
    
    // Sun JDK extension
    private GcInfoBuilder gcInfoBuilder;
    public GcInfo getLastGcInfo() {
        synchronized (this) {
            if (gcInfoBuilder == null) {
                 gcInfoBuilder = new GcInfoBuilder(this, getAllPoolNames());
            }
        }

        GcInfo info = gcInfoBuilder.getLastGcInfo();
        return info;
    }

}
