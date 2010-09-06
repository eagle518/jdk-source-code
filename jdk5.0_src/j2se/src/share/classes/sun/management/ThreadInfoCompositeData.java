/*
 * @(#)ThreadInfoCompositeData.java	1.5 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */ 

package sun.management;

import java.lang.management.ThreadInfo;
import javax.management.openmbean.CompositeType;
import javax.management.openmbean.CompositeData;
import javax.management.openmbean.CompositeDataSupport;
import javax.management.openmbean.OpenDataException;

/**
 * A CompositeData for ThreadInfo for the local management support.
 * This class avoids the performance penalty paid to the 
 * construction of a CompositeData use in the local case.
 */
public class ThreadInfoCompositeData extends LazyCompositeData {
    private final ThreadInfo threadInfo;

    private ThreadInfoCompositeData(ThreadInfo ti) {
	this.threadInfo = ti;
    }

    public ThreadInfo getThreadInfo() {
	return threadInfo;
    }

    public static CompositeData toCompositeData(ThreadInfo ti) {
        ThreadInfoCompositeData ticd = new ThreadInfoCompositeData(ti);
        return ticd.getCompositeData();
    }

    protected CompositeData getCompositeData() {
	final StackTraceElement[] stackTrace = threadInfo.getStackTrace();
	final CompositeData[] stackTraceData =
	    new CompositeData[stackTrace.length];
	for (int i = 0; i < stackTrace.length; i++) {
	    final StackTraceElement ste = stackTrace[i];

	    // CONTENTS OF THIS ARRAY MUST BE SYNCHRONIZED WITH
	    // stackTraceElementItemNames!
	    final Object[] stackTraceElementItemValues = {
		ste.getClassName(),
		ste.getMethodName(),
		ste.getFileName(),
		new Integer(ste.getLineNumber()),
		new Boolean(ste.isNativeMethod()),
	    };
	    try {
		stackTraceData[i] =
		    new CompositeDataSupport(stackTraceElementCompositeType,
					     stackTraceElementItemNames,
					     stackTraceElementItemValues);
	    } catch (OpenDataException e) {
                // Should never reach here
                throw Util.newInternalError(e);
	    }
	}

	// CONTENTS OF THIS ARRAY MUST BE SYNCHRONIZED WITH
	// threadInfoItemNames!
	final Object[] threadInfoItemValues = {
	    new Long(threadInfo.getThreadId()),
	    threadInfo.getThreadName(),
	    threadInfo.getThreadState().name(),
	    new Long(threadInfo.getBlockedTime()),
	    new Long(threadInfo.getBlockedCount()),
	    new Long(threadInfo.getWaitedTime()),
	    new Long(threadInfo.getWaitedCount()),
	    threadInfo.getLockName(),
	    new Long(threadInfo.getLockOwnerId()),
	    threadInfo.getLockOwnerName(),
	    stackTraceData,
	    new Boolean(threadInfo.isSuspended()),
	    new Boolean(threadInfo.isInNative()),
	};
	
	try {
	    return new CompositeDataSupport(threadInfoCompositeType,
					    threadInfoItemNames,
					    threadInfoItemValues);
	} catch (OpenDataException e) {
            // Should never reach here
            throw Util.newInternalError(e);
	}
    }

    private static final CompositeType threadInfoCompositeType;
    private static final CompositeType stackTraceElementCompositeType;
    static {
	try {
	    threadInfoCompositeType = (CompositeType)
		MappedMXBeanType.toOpenType(ThreadInfo.class);
	    stackTraceElementCompositeType = (CompositeType)
		MappedMXBeanType.toOpenType(StackTraceElement.class);
	} catch (OpenDataException e) {
            // Should never reach here
            throw Util.newInternalError(e);
	}
    }

    // Attribute names 
    private static final String THREAD_ID       = "threadId";
    private static final String THREAD_NAME     = "threadName";
    private static final String THREAD_STATE    = "threadState";
    private static final String BLOCKED_TIME    = "blockedTime";
    private static final String BLOCKED_COUNT   = "blockedCount";
    private static final String WAITED_TIME     = "waitedTime";
    private static final String WAITED_COUNT    = "waitedCount";
    private static final String LOCK_NAME       = "lockName";
    private static final String LOCK_OWNER_ID   = "lockOwnerId";
    private static final String LOCK_OWNER_NAME = "lockOwnerName";
    private static final String STACK_TRACE     = "stackTrace";
    private static final String SUSPENDED       = "suspended";
    private static final String IN_NATIVE       = "inNative";
    private static final String CLASS_NAME      = "className";
    private static final String METHOD_NAME     = "methodName";
    private static final String FILE_NAME       = "fileName";
    private static final String LINE_NUMBER     = "lineNumber";
    private static final String NATIVE_METHOD   = "nativeMethod";

    private static final String[] threadInfoItemNames = {
	THREAD_ID,
	THREAD_NAME,
	THREAD_STATE,
	BLOCKED_TIME,
	BLOCKED_COUNT,
	WAITED_TIME,
	WAITED_COUNT,
	LOCK_NAME,
	LOCK_OWNER_ID,
	LOCK_OWNER_NAME,
	STACK_TRACE,
	SUSPENDED,
	IN_NATIVE,
    };

    private static final String[] stackTraceElementItemNames = {
	CLASS_NAME,
	METHOD_NAME,
	FILE_NAME,
	LINE_NUMBER,
	NATIVE_METHOD,
    };

    public static long getThreadId(CompositeData cd) {
        return getLong(cd, THREAD_ID);
    }

    public static String getThreadName(CompositeData cd) {
        // The ThreadName item cannot be null so we check that 
        // it is present with a non-null value.
        String name = getString(cd, THREAD_NAME);
        if (name == null) {
            throw new IllegalArgumentException("Invalid composite data: " +
                "Attribute " + THREAD_NAME + " has null value");
        }
        return name;
    }

    public static Thread.State getThreadState(CompositeData cd) {
        return Thread.State.valueOf(getString(cd, THREAD_STATE));
    }

    public static long getBlockedTime(CompositeData cd) {
        return getLong(cd, BLOCKED_TIME); 
    }

    public static long getBlockedCount(CompositeData cd) {
        return getLong(cd, BLOCKED_COUNT);
    }

    public static long getWaitedTime(CompositeData cd) {
        return getLong(cd, WAITED_TIME);
    }

    public static long getWaitedCount(CompositeData cd) {
        return getLong(cd, WAITED_COUNT);
    }

    public static String getLockName(CompositeData cd) {
        // The LockName and LockOwnerName can legitimately be null, 
        // we don't bother to check the value
        return getString(cd, LOCK_NAME);
    }

    public static long getLockOwnerId(CompositeData cd) {
        return getLong(cd, LOCK_OWNER_ID);
    }

    public static String getLockOwnerName(CompositeData cd) {
        return getString(cd, LOCK_OWNER_NAME);
    }

    public static boolean isSuspended(CompositeData cd) {
        return getBoolean(cd, SUSPENDED);
    }

    public static boolean isInNative(CompositeData cd) {
        return getBoolean(cd, IN_NATIVE);
    }

    public static StackTraceElement[] getStackTrace(CompositeData cd) {
        CompositeData[] stackTraceData =
            (CompositeData[]) cd.get(STACK_TRACE);

        // The StackTrace item cannot be null, but if it is we will get
        // a NullPointerException when we ask for its length. 
        StackTraceElement[] stackTrace = 
            new StackTraceElement[stackTraceData.length];
        for (int i = 0; i < stackTraceData.length; i++) {
            CompositeData cdi = stackTraceData[i];
            stackTrace[i] =
                new StackTraceElement(getString(cdi, CLASS_NAME),
                                      getString(cdi, METHOD_NAME),
                                      getString(cdi, FILE_NAME),
                                      getInt(cdi, LINE_NUMBER));
        }
        return stackTrace;
    }

    /** Validate if the input CompositeData has the expected 
     * CompositeType (i.e. contain all attributes with expected
     * names and types).
     */
    public static void validateCompositeData(CompositeData cd) {
        if (cd == null) {
            throw new NullPointerException("Null CompositeData");
        }

        if (!isTypeMatched(threadInfoCompositeType, cd.getCompositeType())) {
            throw new IllegalArgumentException(
                "Unexpected composite type for ThreadInfo");
        }

        CompositeData[] stackTraceData =
            (CompositeData[]) cd.get(STACK_TRACE);
        if (stackTraceData == null) {
            throw new IllegalArgumentException(
                "StackTraceElement is missing");
        }
        if (stackTraceData.length > 0) {
            if (!isTypeMatched(stackTraceElementCompositeType, 
                         stackTraceData[0].getCompositeType())) {
                throw new IllegalArgumentException(
                    "Unexpected composite type for StackTraceElement");
            }
        }
    }
}
