/*
 * @(#)CountedTimerTask.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.perfdata.monitor;

import java.util.*;

/**
 * A TimerTask subclass that keeps a count of the number of executions
 * of the task.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class CountedTimerTask extends TimerTask {

    volatile long executionCount;

    public long executionCount() {
        return executionCount;
    }

    public void run() {
        executionCount++;
    }
}
