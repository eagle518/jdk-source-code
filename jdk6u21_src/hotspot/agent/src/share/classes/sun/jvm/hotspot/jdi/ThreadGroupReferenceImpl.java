/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.jdi;

import sun.jvm.hotspot.oops.Oop;
import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.oops.OopUtilities;
import sun.jvm.hotspot.runtime.JavaThread;
import com.sun.jdi.*;
import java.util.*;

public class ThreadGroupReferenceImpl extends ObjectReferenceImpl
    implements ThreadGroupReference
{
    ThreadGroupReferenceImpl(VirtualMachine aVm, sun.jvm.hotspot.oops.Oop oRef) {
        super(aVm, oRef);
    }

    protected String description() {
        return "ThreadGroupReference " + uniqueID();
    }

    public String name() {
        return OopUtilities.threadGroupOopGetName(ref());
    }

    public ThreadGroupReference parent() {
        return (ThreadGroupReferenceImpl)vm.threadGroupMirror(
               (Instance)OopUtilities.threadGroupOopGetParent(ref()));
    }

    public void suspend() {
        vm.throwNotReadOnlyException("ThreadGroupReference.suspend()");
    }

    public void resume() {
        vm.throwNotReadOnlyException("ThreadGroupReference.resume()");
    }

    public List threads() {
        // Each element of this array is the Oop for a thread;
        // NOTE it is not the JavaThread that we need to create
        // a ThreadReferenceImpl.
        Oop[] myThreads = OopUtilities.threadGroupOopGetThreads(ref());

        ArrayList myList = new ArrayList(myThreads.length);
        for (int ii = 0; ii < myThreads.length; ii++) {
            JavaThread jt = OopUtilities.threadOopGetJavaThread(myThreads[ii]);
            if (jt != null) {
                ThreadReferenceImpl xx = (ThreadReferenceImpl)vm.threadMirror(jt);
                myList.add(xx);
            }
        }
        return myList;
    }

    public List threadGroups() {
        Oop[] myGroups = OopUtilities.threadGroupOopGetGroups(ref());
        ArrayList myList = new ArrayList(myGroups.length);
        for (int ii = 0; ii < myGroups.length; ii++) {
            ThreadGroupReferenceImpl xx = (ThreadGroupReferenceImpl)vm.threadGroupMirror(
                                          (Instance)myGroups[ii]);
            myList.add(xx);

        }
        return myList;
    }

    public String toString() {
        return "instance of " + referenceType().name() +
               "(name='" + name() + "', " + "id=" + uniqueID() + ")";
    }
}
