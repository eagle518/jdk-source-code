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
package com.sun.hotspot.igv.filterwindow.actions;

import com.sun.hotspot.igv.filterwindow.*;
import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import org.openide.util.NbBundle;
import org.openide.windows.TopComponent;

/**
 *
 * @author Thomas Wuerthinger
 */
public class FilterAction extends AbstractAction {

    public FilterAction() {
        super(NbBundle.getMessage(FilterAction.class, "CTL_FilterAction"));
    }

    public void actionPerformed(ActionEvent evt) {
        TopComponent win = FilterTopComponent.findInstance();
        win.open();
        win.requestActive();
    }
}
