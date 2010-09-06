/*
 * @(#)RawOutputFormatter.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.util.*;
import sun.jvmstat.monitor.*;

/**
 * A class for formatting raw counter output.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class RawOutputFormatter implements OutputFormatter {
    private List logged;
    private String header;
    private boolean printStrings;

    public RawOutputFormatter(List logged, boolean printStrings) {
        this.logged = logged;
        this.printStrings = printStrings;
    }

    public String getHeader() throws MonitorException {
        if (header == null) {
            // build the header string and prune out any unwanted monitors
            StringBuilder headerBuilder = new StringBuilder();
            for (Iterator i = logged.iterator(); i.hasNext(); /* empty */ ) {
                Monitor m = (Monitor)i.next();
                headerBuilder.append(m.getName() + " ");
            }
            header = headerBuilder.toString();
        }
        return header;
    }

    public String getRow() throws MonitorException {
        StringBuilder row = new StringBuilder();
        int count = 0;
        for (Iterator i = logged.iterator(); i.hasNext(); /* empty */ ) {
            Monitor m = (Monitor)i.next();
            if (count++ > 0) {
                row.append(" ");
            }
            if (printStrings && m instanceof StringMonitor) {
                row.append("\"").append(m.getValue()).append("\"");
            } else {
                row.append(m.getValue());
            }
        }
        return row.toString();
    }
}
