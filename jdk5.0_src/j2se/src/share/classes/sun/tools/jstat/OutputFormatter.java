/*
 * @(#)OutputFormatter.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import sun.jvmstat.monitor.MonitorException;

/**
 * An interface for the JStatLogger formatting.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public interface OutputFormatter {

    /**
     * get the header row that describes the data in the columns
     */
    String getHeader() throws MonitorException;

    /**
     * get the data row.
     */
    String getRow() throws MonitorException;
}
