/*
 * @(#)EventSetImpl.java	1.26 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.jdi.*;
import com.sun.jdi.event.*;
import com.sun.jdi.request.*;

import java.util.*;

public class EventSetImpl extends ArrayList<Event> implements EventSet {
    
    private VirtualMachineImpl vm; // we implement Mirror
    private Packet pkt;
    private byte suspendPolicy;
    private boolean needsFiltering = false;

    public String toString() {
        String string = "event set, policy:" + suspendPolicy + 
                        ", count:" + this.size() + " = {";
        Iterator iter = this.iterator();
        boolean first = true;
        while (iter.hasNext()) {
            Event event = (Event)iter.next();
            if (!first) {
                string += ", ";
            }
            string += event.toString();
            first = false;
        }
        string += "}";
        return string;
    }

    abstract class EventImpl extends MirrorImpl implements Event {

        private byte eventCmd;
        private EventRequest request;
        private boolean internalEvent = false;

        /**
         * Constructor for events.
         */
        protected EventImpl(JDWP.Event.Composite.Events.EventsCommon evt,
                            int requestID) {
            super(EventSetImpl.this.vm);
            this.eventCmd = evt.eventKind();
            this.request = requestFromRequestId(requestID);
        }

        /*
         * Override superclass back to default equality
         */
        public boolean equals(Object obj) {
            return this == obj;
        }
    
        public int hashCode() {
            return System.identityHashCode(this);
        }

        /**
         * Constructor for VM disconnected events.
         */
        protected EventImpl(byte eventCmd) {
            super(EventSetImpl.this.vm);
            this.eventCmd = eventCmd;
            this.request = null;
        }

        public EventRequest request() {
            return request;
        }

        private EventRequest requestFromRequestId(int requestId) {
            if (requestId == 0) {
                return null;
            } else {
                EventRequestManagerImpl ermi = this.vm.eventRequestManagerImpl();
                EventRequest req = ermi.request(eventCmd, requestId);

                // This came from an alien (prosumably internal)
                // event request manager.  This event must be filtered
                if (req == null) {
                    internalEvent = true;
                    needsFiltering = true;
                }
                return req;
            }
        }

        abstract String eventName();

        public String toString() {
            return eventName();
        }

    }

    abstract class ThreadedEventImpl extends EventImpl {
        private ThreadReference thread;

        ThreadedEventImpl(JDWP.Event.Composite.Events.EventsCommon evt,
                          int requestID, ThreadReference thread) {
            super(evt, requestID);
            this.thread = thread;
        }

        public ThreadReference thread() {
            return thread;
        }

        public String toString() {
            return eventName() + " in thread " + thread.name();
        }
    }

    abstract class LocatableEventImpl extends ThreadedEventImpl 
                                            implements Locatable {
        private Location location;

        LocatableEventImpl(JDWP.Event.Composite.Events.EventsCommon evt,
                           int requestID, 
                           ThreadReference thread, Location location) {
            super(evt, requestID, thread);
            this.location = location;
        }

        public Location location() {
            return location;
        }

	/**
	 * For MethodEntry and MethodExit
	 */
        public Method method() {
            return location.method();
        }

        public String toString() {
            return eventName() + "@" + location().toString() + 
                                 " in thread " + thread().name();
        }
    }

    class BreakpointEventImpl extends LocatableEventImpl
                            implements BreakpointEvent {
        BreakpointEventImpl(JDWP.Event.Composite.Events.Breakpoint evt) {
            super(evt, evt.requestID, evt.thread, evt.location);
        }

        String eventName() {
            return "BreakpointEvent";
        }
    }

    class StepEventImpl extends LocatableEventImpl implements StepEvent {
        StepEventImpl(JDWP.Event.Composite.Events.SingleStep evt) {
            super(evt, evt.requestID, evt.thread, evt.location);
        }

        String eventName() {
            return "StepEvent";
        }
    }

    class MethodEntryEventImpl extends LocatableEventImpl
                            implements MethodEntryEvent {
        MethodEntryEventImpl(JDWP.Event.Composite.Events.MethodEntry evt) {
            super(evt, evt.requestID, evt.thread, evt.location);
        }

        String eventName() {
            return "MethodEntryEvent";
        }
    }

    class MethodExitEventImpl extends LocatableEventImpl
                            implements MethodExitEvent {
        MethodExitEventImpl(JDWP.Event.Composite.Events.MethodExit evt) {
            super(evt, evt.requestID, evt.thread, evt.location);
        }

        String eventName() {
            return "MethodExitEvent";
        }
    }

    class ClassPrepareEventImpl extends ThreadedEventImpl
                            implements ClassPrepareEvent {
        private ReferenceType referenceType;

        ClassPrepareEventImpl(JDWP.Event.Composite.Events.ClassPrepare evt) {
            super(evt, evt.requestID, evt.thread);
            referenceType = this.vm.referenceType(evt.typeID, evt.refTypeTag, 
                                                  evt.signature);
            ((ReferenceTypeImpl)referenceType).setStatus(evt.status);
        }

        public ReferenceType referenceType() {
            return referenceType;
        }

        String eventName() {
            return "ClassPrepareEvent";
        }
    }

    class ClassUnloadEventImpl extends EventImpl implements ClassUnloadEvent {
        private String classSignature;
    
        ClassUnloadEventImpl(JDWP.Event.Composite.Events.ClassUnload evt) {
            super(evt, evt.requestID);
            this.classSignature = evt.signature;
        }

        public String className() {
            return classSignature.substring(1, classSignature.length()-1)
                .replace('/', '.');
        }
    
        public String classSignature() {
            return classSignature;
        }

        String eventName() {
            return "ClassUnloadEvent";
        }
    }

    class ExceptionEventImpl extends LocatableEventImpl
                                             implements ExceptionEvent {
        private ObjectReference exception;
        private Location catchLocation;

        ExceptionEventImpl(JDWP.Event.Composite.Events.Exception evt) {
            super(evt, evt.requestID, evt.thread, evt.location);
            this.exception = evt.exception;
            this.catchLocation = evt.catchLocation;
        }

        public ObjectReference exception() {
            return exception;
        }

        public Location catchLocation() {
            return catchLocation;
        }

        String eventName() {
            return "ExceptionEvent";
        }
    }

    class ThreadDeathEventImpl extends ThreadedEventImpl
                                        implements ThreadDeathEvent { 
        ThreadDeathEventImpl(JDWP.Event.Composite.Events.ThreadDeath evt) {
            super(evt, evt.requestID, evt.thread);
        }

        String eventName() {
            return "ThreadDeathEvent";
        }
    }

    class ThreadStartEventImpl extends ThreadedEventImpl
                                        implements ThreadStartEvent { 
        ThreadStartEventImpl(JDWP.Event.Composite.Events.ThreadStart evt) {
            super(evt, evt.requestID, evt.thread);
        }

        String eventName() {
            return "ThreadStartEvent";
        }
    }

    class VMStartEventImpl extends ThreadedEventImpl
                                        implements VMStartEvent { 
        VMStartEventImpl(JDWP.Event.Composite.Events.VMStart evt) {
            super(evt, evt.requestID, evt.thread);
        }

        String eventName() {
            return "VMStartEvent";
        }
    }

    class VMDeathEventImpl extends EventImpl implements VMDeathEvent {

        VMDeathEventImpl(JDWP.Event.Composite.Events.VMDeath evt) {
            super(evt, evt.requestID);
        }

        String eventName() {
            return "VMDeathEvent";
        }
    }

    class VMDisconnectEventImpl extends EventImpl 
                                         implements VMDisconnectEvent {

        VMDisconnectEventImpl() {
            super((byte)JDWP.EventKind.VM_DISCONNECTED);
        }

        String eventName() {
            return "VMDisconnectEvent";
        }
    }

    abstract class WatchpointEventImpl extends LocatableEventImpl
                                            implements WatchpointEvent {
        private final ReferenceTypeImpl refType;
        private final long fieldID;
        private final ObjectReference object;
        private Field field = null;

        WatchpointEventImpl(JDWP.Event.Composite.Events.EventsCommon evt,
                            int requestID, 
                            ThreadReference thread, Location location,
                            byte refTypeTag, long typeID, long fieldID, 
                            ObjectReference object) {
            super(evt, requestID, thread, location);
            this.refType = this.vm.referenceType(typeID, refTypeTag);
            this.fieldID = fieldID;
            this.object = object;
        }

        public Field field() {
            if (field == null) {
                field = refType.getFieldMirror(fieldID);
            }
            return field;
        }

        public ObjectReference object() {
            return object;
        }

        public Value valueCurrent() {
            if (object == null) {
                return refType.getValue(field());
            } else {
                return object.getValue(field());
            }
        }
    }

    class AccessWatchpointEventImpl extends WatchpointEventImpl 
                                            implements AccessWatchpointEvent {

        AccessWatchpointEventImpl(JDWP.Event.Composite.Events.FieldAccess evt) {
            super(evt, evt.requestID, evt.thread, evt.location,
                  evt.refTypeTag, evt.typeID, evt.fieldID, evt.object);
        }

        String eventName() {
            return "AccessWatchpoint";
        }
    }

    class ModificationWatchpointEventImpl extends WatchpointEventImpl 
                           implements ModificationWatchpointEvent {                            
        Value newValue;

        ModificationWatchpointEventImpl(
                        JDWP.Event.Composite.Events.FieldModification evt) {
            super(evt, evt.requestID, evt.thread, evt.location,
                  evt.refTypeTag, evt.typeID, evt.fieldID, evt.object);
            this.newValue = evt.valueToBe;
        }

        public Value valueToBe() {
            return newValue;
        }

        String eventName() {
            return "ModificationWatchpoint";
        }
    }

    /**
     * Events are constructed on the thread which reads all data from the 
     * transport. This means that the packet cannot be converted to real 
     * JDI objects as that may involve further communications with the 
     * back end which would deadlock.
     *
     * Hence the {@link #build()} method below called by EventQueue.
     */
    EventSetImpl(VirtualMachine aVm, Packet pkt) {
        super();

        // From "MirrorImpl":
        // Yes, its a bit of a hack. But by doing it this
        // way, this is the only place we have to change
        // typing to substitute a new impl.
        vm = (VirtualMachineImpl)aVm;

        this.pkt = pkt;
    }

    /** 
     * Constructor for special events like VM disconnected
     */
    EventSetImpl(VirtualMachine aVm, byte eventCmd) {
        this(aVm, null);
        suspendPolicy = JDWP.SuspendPolicy.NONE;
        switch (eventCmd) {
            case JDWP.EventKind.VM_DISCONNECTED:
                super.add(new VMDisconnectEventImpl());
                break;

            default:
                throw new InternalException("Bad singleton event code");
        }       
    }

    /** 
     * Constructor for filtered cloning
     */
    EventSetImpl(EventSetImpl es, boolean internal) {
        this(es.vm, null);
        suspendPolicy = es.suspendPolicy;
        for (Iterator it = es.iterator(); it.hasNext();) {
            EventImpl evt = (EventImpl)it.next();
            if (evt.internalEvent == internal) {
                super.add(evt);
            }
        }
    }

    synchronized void build() {
        if (pkt == null) {
            return;
        }
        PacketStream ps = new PacketStream(vm, pkt);
        JDWP.Event.Composite compEvt = new JDWP.Event.Composite(vm, ps);
        suspendPolicy = compEvt.suspendPolicy;

        if ((vm.traceFlags & vm.TRACE_EVENTS) != 0) {
            switch(suspendPolicy) {
                case JDWP.SuspendPolicy.ALL:
                    vm.printTrace("EventSet: SUSPEND_ALL");
                    break;

                case JDWP.SuspendPolicy.EVENT_THREAD:
                    vm.printTrace("EventSet: SUSPEND_EVENT_THREAD");
                    break;

                case JDWP.SuspendPolicy.NONE:
                    vm.printTrace("EventSet: SUSPEND_NONE");
                    break;
            }
        }

        for (int i = 0; i < compEvt.events.length; i++) {
            EventImpl evt = createEvent(compEvt.events[i]);

            if ((vm.traceFlags & vm.TRACE_EVENTS) != 0) {
                vm.printTrace("Event: " + evt);
            }

            super.add(evt);
        }
        pkt = null; // No longer needed - free it up
    }

    /** 
     * Filter out internal events.
     */
    EventSet userFilter() {
        if (needsFiltering) {
            return new EventSetImpl(this, false);
        } else {
            return this;
        }
    }       
            
    /** 
     * Filter out user events.
     */
    EventSet internalFilter() {
        if (needsFiltering) {
            return new EventSetImpl(this, true);
        } else {
            return null;
        }
    }       
            
    EventImpl createEvent(JDWP.Event.Composite.Events evt) {
        JDWP.Event.Composite.Events.EventsCommon comm = evt.aEventsCommon;
        switch (evt.eventKind) {
            case JDWP.EventKind.THREAD_START: 
                return new ThreadStartEventImpl( 
                      (JDWP.Event.Composite.Events.ThreadStart)comm);
 
            case JDWP.EventKind.THREAD_END: 
                return new ThreadDeathEventImpl(  
                      (JDWP.Event.Composite.Events.ThreadDeath)comm);

            case JDWP.EventKind.EXCEPTION: 
                return new ExceptionEventImpl(  
                      (JDWP.Event.Composite.Events.Exception)comm);
 
            case JDWP.EventKind.BREAKPOINT: 
                return new BreakpointEventImpl(  
                      (JDWP.Event.Composite.Events.Breakpoint)comm);

            case JDWP.EventKind.METHOD_ENTRY: 
                return new MethodEntryEventImpl(  
                      (JDWP.Event.Composite.Events.MethodEntry)comm);

            case JDWP.EventKind.METHOD_EXIT: 
                return new MethodExitEventImpl(  
                      (JDWP.Event.Composite.Events.MethodExit)comm);

            case JDWP.EventKind.FIELD_ACCESS: 
                return new AccessWatchpointEventImpl(  
                      (JDWP.Event.Composite.Events.FieldAccess)comm);

            case JDWP.EventKind.FIELD_MODIFICATION: 
                return new ModificationWatchpointEventImpl(  
                      (JDWP.Event.Composite.Events.FieldModification)comm);

            case JDWP.EventKind.SINGLE_STEP: 
                return new StepEventImpl(  
                      (JDWP.Event.Composite.Events.SingleStep)comm);

            case JDWP.EventKind.CLASS_PREPARE: 
                return new ClassPrepareEventImpl(  
                      (JDWP.Event.Composite.Events.ClassPrepare)comm);
    
            case JDWP.EventKind.CLASS_UNLOAD: 
                return new ClassUnloadEventImpl(  
                      (JDWP.Event.Composite.Events.ClassUnload)comm);

            case JDWP.EventKind.VM_START: 
                return new VMStartEventImpl( 
                      (JDWP.Event.Composite.Events.VMStart)comm);
 
            case JDWP.EventKind.VM_DEATH:
                return new VMDeathEventImpl(  
                      (JDWP.Event.Composite.Events.VMDeath)comm);

            default:
                // Ignore unknown event types
                System.err.println("Ignoring event cmd " + 
                                   evt.eventKind + " from the VM");
                return null;
        }
    }

    public VirtualMachine virtualMachine() {
        return vm;
    }

    public int suspendPolicy() {
        return EventRequestManagerImpl.JDWPtoJDISuspendPolicy(suspendPolicy);
    }

    private ThreadReference eventThread() {
        Iterator iter = this.iterator();
        while (iter.hasNext()) {
            Event event = (Event)iter.next();
            if (event instanceof ThreadedEventImpl) {
                return ((ThreadedEventImpl)event).thread();
            }
        }
        return null;
    }

    public void resume() {
        switch (suspendPolicy()) {
            case EventRequest.SUSPEND_ALL:
                vm.resume();
                break;
            case EventRequest.SUSPEND_EVENT_THREAD:
                ThreadReference thread = eventThread();
                if (thread == null) {
                    throw new InternalException("Inconsistent suspend policy");
                }
                thread.resume();
                break;
            case EventRequest.SUSPEND_NONE:
                // Do nothing
                break;
            default:
                throw new InternalException("Invalid suspend policy");
        }
    }

    public Iterator<Event> iterator() {
        return new Itr();
    }

    public EventIterator eventIterator() {
        return new Itr();
    }

    public class Itr implements EventIterator {
	/**
	 * Index of element to be returned by subsequent call to next.
	 */
	int cursor = 0;

	public boolean hasNext() {
	    return cursor != size();
	}

	public Event next() {
	    try {
		Event nxt = get(cursor);
                ++cursor;
                return nxt;
	    } catch(IndexOutOfBoundsException e) {
		throw new NoSuchElementException();
	    }
	}

        public Event nextEvent() {
            return (Event)next();
        }

	public void remove() {
            throw new UnsupportedOperationException();
	}
    }

    /* below make this unmodifiable */

    public boolean add(Event o){
        throw new UnsupportedOperationException();
    }
    public boolean remove(Object o) {
        throw new UnsupportedOperationException();
    }
    public boolean addAll(Collection<? extends Event> coll) {
        throw new UnsupportedOperationException();
    }
    public boolean removeAll(Collection<?> coll) {
        throw new UnsupportedOperationException();
    }
    public boolean retainAll(Collection<?> coll) {
        throw new UnsupportedOperationException();
    }
    public void clear() {
        throw new UnsupportedOperationException();
    }
}

