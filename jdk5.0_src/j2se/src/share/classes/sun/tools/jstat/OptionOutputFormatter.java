/*
 * @(#)OptionOutputFormatter.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import sun.jvmstat.monitor.*;

/**
 * A class for applying an OptionFormat to a particular context, the context
 * of the available Instrumentation for a monitorable Java Virtual Machine.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class OptionOutputFormatter implements OutputFormatter {
    private OptionFormat format;
    private String header;
    private MonitoredVm vm;

    public OptionOutputFormatter(MonitoredVm vm, OptionFormat format)
           throws MonitorException {
        this.vm = vm;
        this.format = format;
        resolve();
    }

    private void resolve() throws MonitorException {
        ExpressionEvaluator ee = new ExpressionResolver(vm);
        SymbolResolutionClosure ec = new SymbolResolutionClosure(ee);
        format.apply(ec);
    }

    public String getHeader() throws MonitorException {
        if (header == null) {
            HeaderClosure hc = new HeaderClosure();
            format.apply(hc);
            header = hc.getHeader();
        }
        return header;
    }

    public String getRow() throws MonitorException {
        RowClosure rc = new RowClosure(vm);
        format.apply(rc);
        return rc.getRow();
    }
}
