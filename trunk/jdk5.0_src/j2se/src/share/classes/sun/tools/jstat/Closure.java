/*
 * @(#)Closure.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.util.List;
import sun.jvmstat.monitor.MonitorException;

/**
 * An interface for visitor object on a binary tree.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
interface Closure {
    void visit(Object o, boolean hasNext) throws MonitorException;
}
