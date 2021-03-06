/*
 * @(#)ThreadReferenceImpl.java	1.45 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;
import com.sun.jdi.request.BreakpointRequest;
import java.util.*;
import java.lang.ref.WeakReference;

public class ThreadReferenceImpl extends ObjectReferenceImpl
             implements ThreadReference, VMListener {
    static final int SUSPEND_STATUS_SUSPENDED = 0x1;
    static final int SUSPEND_STATUS_BREAK = 0x2;

    private ThreadGroupReference threadGroup;
    private int suspendedZombieCount = 0;

    // This is cached only while the VM is suspended
    private static class Cache extends ObjectReferenceImpl.Cache {
        String name = null;
        JDWP.ThreadReference.Status status = null;
        List frames = null;
        int framesStart = -1;
        int framesLength = 0;
        int frameCount = -1;
        List ownedMonitors = null;
        ObjectReference contendedMonitor = null;
        boolean triedCurrentContended = false;
    }

    protected ObjectReferenceImpl.Cache newCache() {
        return new Cache();
    }

    // Listeners - synchronized on vm.state() 
    private List listeners = new ArrayList();

    ThreadReferenceImpl(VirtualMachine aVm, long aRef) {
        super(aVm,aRef);
        vm.state().addListener(this);
    }

    protected String description() {
        return "ThreadReference " + uniqueID();
    }

    /*
     * VMListener implementation
     */
    public boolean vmNotSuspended(VMAction action) {
        synchronized (vm.state()) {
            processThreadAction(new ThreadAction(this, 
                                   ThreadAction.THREAD_RESUMABLE));
        }
        return super.vmNotSuspended(action);
    }

    /**
     * Note that we only cache the name string while suspended because 
     * it can change via Thread.setName arbitrarily
     */
    public String name() {
        String name = null;
        try {
            Cache local = (Cache)getCache();

            if (local != null) {
                name = local.name;
            }
            if (name == null) {
                name = JDWP.ThreadReference.Name.process(vm, this)
                                                             .threadName;
                if (local != null) {
                    local.name = name;
                }
            }
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
        return name;
    }

    /*
     * Sends a command to the back end which is defined to do an
     * implicit vm-wide resume.
     */
    PacketStream sendResumingCommand(CommandSender sender) {
        synchronized (vm.state()) {
            processThreadAction(new ThreadAction(this, 
                                        ThreadAction.THREAD_RESUMABLE));
            return sender.send();
        }
    }

    public void suspend() {
        try {
            JDWP.ThreadReference.Suspend.process(vm, this);
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
        // Don't consider the thread suspended yet. On reply, notifySuspend()
        // will be called.
    }

    public void resume() {
        /*
         * If it's a zombie, we can just update internal state without
         * going to back end.
         */
        if (suspendedZombieCount > 0) {
            suspendedZombieCount--;
            return;
        }

        PacketStream stream;
        synchronized (vm.state()) {
            processThreadAction(new ThreadAction(this, 
                                      ThreadAction.THREAD_RESUMABLE));
            stream = JDWP.ThreadReference.Resume.enqueueCommand(vm, this);
        }
        try {
            JDWP.ThreadReference.Resume.waitForReply(vm, stream);
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    public int suspendCount() {
        /*
         * If it's a zombie, we maintain the count in the front end.
         */
        if (suspendedZombieCount > 0) {
            return suspendedZombieCount;
        }

        try {
            return JDWP.ThreadReference.SuspendCount.process(vm, this).suspendCount;
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    public void stop(ObjectReference throwable) throws InvalidTypeException {

        // Verify that the given object is a Throwable instance
        List list = vm.classesByName("java.lang.Throwable");
        ClassTypeImpl throwableClass = (ClassTypeImpl)list.get(0);

        if ((throwable == null) || 
            !throwableClass.isAssignableFrom(throwable)) {
             throw new InvalidTypeException("Not an instance of Throwable");
        }

        try {
            JDWP.ThreadReference.Stop.process(vm, this, 
                                         (ObjectReferenceImpl)throwable);
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    public void interrupt() {
        try {
            JDWP.ThreadReference.Interrupt.process(vm, this);
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
    }

    private JDWP.ThreadReference.Status jdwpStatus() {
        JDWP.ThreadReference.Status status = null;
        try {
            Cache local = (Cache)getCache();

            if (local != null) {
                status = local.status;
            }
            if (status == null) {
                status = JDWP.ThreadReference.Status.process(vm, this);
                if (local != null) {
                    local.status = status;
                }
            }
        } catch (JDWPException exc) {
            throw exc.toJDIException();
        }
        return status;
    }

    public int status() {
        return jdwpStatus().threadStatus;
    }

    public boolean isSuspended() {
        return ((suspendedZombieCount > 0) ||
                ((jdwpStatus().suspendStatus & SUSPEND_STATUS_SUSPENDED) != 0));
    }

    public boolean isAtBreakpoint() {
        /*
         * TO DO: This fails to take filters into account.
         */
        try {
            StackFrame frame = frame(0);
            Location location = frame.location();
            List requests = vm.eventRequestManager().breakpointRequests();
            Iterator iter = requests.iterator();
            while (iter.hasNext()) {
                BreakpointRequest request = (BreakpointRequest)iter.next();
                if (location.equals(request.location())) {
                    return true;
                }
            }
            return false;
        } catch (IndexOutOfBoundsException iobe) {
            return false;  // no frames on stack => not at breakpoint
        } catch (IncompatibleThreadStateException itse) {
            // Per the javadoc, not suspended => return false
            return false; 
        }
    }

    public ThreadGroupReference threadGroup() {
        /*
         * Thread group can't change, so it's cached more conventionally
         * than other things in this class.
         */
        if (threadGroup == null) {
            try {
                threadGroup = JDWP.ThreadReference.ThreadGroup.
                    process(vm, this).group;
            } catch (JDWPException exc) {
                throw exc.toJDIException();
            }
        }
        return threadGroup;
    }

    public int frameCount() throws IncompatibleThreadStateException  {
        int frameCount = -1;
        try {
            Cache local = (Cache)getCache();

            if (local != null) {
                frameCount = local.frameCount;
            }
            if (frameCount == -1) {
                frameCount = JDWP.ThreadReference.FrameCount
                                          .process(vm, this).frameCount;
                if (local != null) {
                    local.frameCount = frameCount;
                }
            }
        } catch (JDWPException exc) {
            switch (exc.errorCode()) {
            case JDWP.Error.THREAD_NOT_SUSPENDED:
            case JDWP.Error.INVALID_THREAD:   /* zombie */
                throw new IncompatibleThreadStateException();
            default:
                throw exc.toJDIException();
            }
        }
        return frameCount;
    }

    public List frames() throws IncompatibleThreadStateException  {
        return privateFrames(0, -1);
    }

    public StackFrame frame(int index) throws IncompatibleThreadStateException  {
        List list = privateFrames(index, 1);
        return (StackFrame)list.get(0);
    }

    /**
     * Is the requested subrange within what has been retrieved?
     * local is known to be non-null
     */
    private boolean isSubrange(Cache local,  
                               int start, int length, List frames) {
        if (start < local.framesStart) {
            return false;
        }
        if (length == -1) {
            return (local.framesLength == -1);
        }
        if (local.framesLength == -1) {
            if ((start + length) > (local.framesStart + frames.size())) {
                throw new IndexOutOfBoundsException();
            }
            return true;
        }
        return ((start + length) <= (local.framesStart + local.framesLength));
    }

    public List frames(int start, int length) 
                              throws IncompatibleThreadStateException  {
        if (length < 0) {
            throw new IndexOutOfBoundsException(
                "length must be greater than or equal to zero");
        }
        return privateFrames(start, length);
    }

    /**
     * Private version of frames() allows "-1" to specify all 
     * remaining frames.
     */
    private List privateFrames(int start, int length) 
                              throws IncompatibleThreadStateException  {
        List frames = null;
        try {
            Cache local = (Cache)getCache();

            if (local != null) {
                frames = local.frames;
            }
            if (frames == null || !isSubrange(local, start, length, frames)) {
                JDWP.ThreadReference.Frames.Frame[] jdwpFrames
                    = JDWP.ThreadReference.Frames.
                          process(vm, this, start, length).frames;
                int count = jdwpFrames.length;
                frames = new ArrayList(count);

                // Lock must be held while creating stack frames.
                // so that a resume will not resume a partially 
                // created stack.
                synchronized (vm.state()) {
                    for (int i = 0; i<count; i++) {
                        if (jdwpFrames[i].location == null) {
                            throw new InternalException("Invalid frame location");
                        }
                        StackFrame frame = new StackFrameImpl(vm, this, 
                                            jdwpFrames[i].frameID, 
                                            jdwpFrames[i].location);
                        // Add to the frame list
                        frames.add(frame);
                    }
                }
                if (local != null) {
                    local.frames = frames;
                    local.framesStart = start;
                    local.framesLength = length;
                }
            } else {
                int fromIndex = start - local.framesStart;
                int toIndex;
                if (length == -1) {
                    toIndex = frames.size() - fromIndex;
                } else {
                    toIndex = fromIndex + length;
                }
                frames = frames.subList(fromIndex, toIndex);
            }
        } catch (JDWPException exc) {
            switch (exc.errorCode()) {
            case JDWP.Error.THREAD_NOT_SUSPENDED:
            case JDWP.Error.INVALID_THREAD:   /* zombie */
                throw new IncompatibleThreadStateException();
            default:
                throw exc.toJDIException();
            }
        }
        return Collections.unmodifiableList(frames);
    }

    public List ownedMonitors()  throws IncompatibleThreadStateException  {
        List monitors = null;
        try {
            Cache local = (Cache)getCache();

            if (local != null) {
                monitors = local.ownedMonitors;
            }
            if (monitors == null) {
                monitors = Arrays.asList(
                                 JDWP.ThreadReference.OwnedMonitors.
                                         process(vm, this).owned);
                if (local != null) {
                    local.ownedMonitors = monitors;
                    if ((vm.traceFlags & vm.TRACE_OBJREFS) != 0) {
                        vm.printTrace(description() + 
                                      " temporarily caching owned monitors"+
                                      " (count = " + monitors.size() + ")");
                    }
                }
            }
        } catch (JDWPException exc) {
            switch (exc.errorCode()) {
            case JDWP.Error.THREAD_NOT_SUSPENDED:
            case JDWP.Error.INVALID_THREAD:   /* zombie */
                throw new IncompatibleThreadStateException();
            default:
                throw exc.toJDIException();
            }
        }
        return monitors;
    }

    public ObjectReference currentContendedMonitor() 
                              throws IncompatibleThreadStateException  {
        ObjectReference monitor = null;
        try {
            Cache local = (Cache)getCache();

            if (local != null && local.triedCurrentContended) {
                monitor = local.contendedMonitor;
            } else {
                monitor = JDWP.ThreadReference.CurrentContendedMonitor.
                    process(vm, this).monitor;
                if (local != null) {
                    local.triedCurrentContended = true;
                    local.contendedMonitor = monitor;
                    if ((monitor != null) &&
                        ((vm.traceFlags & vm.TRACE_OBJREFS) != 0)) {
                        vm.printTrace(description() + 
                              " temporarily caching contended monitor"+
                              " (id = " + monitor.uniqueID() + ")");
                    }
                }
            }
        } catch (JDWPException exc) {
            switch (exc.errorCode()) {
            case JDWP.Error.THREAD_NOT_SUSPENDED:
            case JDWP.Error.INVALID_THREAD:   /* zombie */
                throw new IncompatibleThreadStateException();
            default:
                throw exc.toJDIException();
            }
        }
        return monitor;
    }

    public void popFrames(StackFrame frame) throws IncompatibleThreadStateException {
        // Note that interface-wise this functionality belongs
        // here in ThreadReference, but implementation-wise it
        // belongs in StackFrame, so we just forward it.
        if (!frame.thread().equals(this)) {
            throw new IllegalArgumentException("frame does not belong to this thread");
        }
        if (!vm.canPopFrames()) {
            throw new UnsupportedOperationException(
                "target does not support popping frames");
        }
        ((StackFrameImpl)frame).pop();
    }

    public String toString() {
        return "instance of " + referenceType().name() + 
               "(name='" + name() + "', " + "id=" + uniqueID() + ")";
    }

    byte typeValueKey() {
        return JDWP.Tag.THREAD;
    }

    void addListener(ThreadListener listener) {
        synchronized (vm.state()) {
            listeners.add(new WeakReference(listener));
        }
    }

    void removeListener(ThreadListener listener) {
        synchronized (vm.state()) {
            Iterator iter = listeners.iterator();
            while (iter.hasNext()) {
                WeakReference ref = (WeakReference)iter.next();
                if (listener.equals(ref.get())) {
                    iter.remove();
                    break;
                }
            }
        }
    }

    /**
     * Propagate the the thread state change information
     * to registered listeners.
     * Must be entered while synchronized on vm.state()
     */
    private void processThreadAction(ThreadAction action) {
        synchronized (vm.state()) {
            Iterator iter = listeners.iterator();
            while (iter.hasNext()) {
                WeakReference ref = (WeakReference)iter.next();
                ThreadListener listener = (ThreadListener)ref.get();
                if (listener != null) {
                    switch (action.id()) {
                        case ThreadAction.THREAD_RESUMABLE:
                            if (!listener.threadResumable(action)) {
                                iter.remove();
                            }
                            break;
                    }
                } else {
                    // Listener is unreachable; clean up
                    iter.remove();
                }
            }
        }
    }
}
