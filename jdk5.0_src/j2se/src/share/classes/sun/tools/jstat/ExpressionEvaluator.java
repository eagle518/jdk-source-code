/*
 * @(#)ExpressionEvaluator.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import sun.jvmstat.monitor.MonitorException;

/**
 * An interface to allow an object to visit an Expression object and
 * evaluate based on some context.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
interface ExpressionEvaluator {
    Object evaluate(Expression e) throws MonitorException;
}
