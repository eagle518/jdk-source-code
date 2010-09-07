/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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
package com.sun.hotspot.igv.filter;

import com.sun.hotspot.igv.graph.Connection;
import com.sun.hotspot.igv.graph.Diagram;
import com.sun.hotspot.igv.graph.Figure;
import com.sun.hotspot.igv.graph.InputSlot;
import com.sun.hotspot.igv.graph.OutputSlot;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class RemoveSelfLoopsFilter extends AbstractFilter {

    private String name;

    /** Creates a new instance of RemoveSelfLoops */
    public RemoveSelfLoopsFilter(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public void apply(Diagram d) {

        for (Figure f : d.getFigures()) {

            for (InputSlot is : f.getInputSlots()) {

                List<Connection> toRemove = new ArrayList<Connection>();
                for (Connection c : is.getConnections()) {

                    if (c.getOutputSlot().getFigure() == f) {
                        toRemove.add(c);
                    }
                }

                for (Connection c : toRemove) {

                    c.remove();

                    OutputSlot os = c.getOutputSlot();
                    if (os.getConnections().size() == 0) {
                        f.removeSlot(os);
                    }

                    c.getInputSlot().setShortName("O");
                    c.getInputSlot().setName("Self Loop");
                }
            }
        }
    }
}
