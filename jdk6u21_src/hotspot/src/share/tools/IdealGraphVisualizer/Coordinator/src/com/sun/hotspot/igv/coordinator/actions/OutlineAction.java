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

package com.sun.hotspot.igv.coordinator.actions;

import com.sun.hotspot.igv.coordinator.*;
import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import org.openide.util.NbBundle;
import org.openide.windows.TopComponent;

/**
 *
 * @author Thomas Wuerthinger
 */
public class OutlineAction extends AbstractAction {

    public OutlineAction() {
        super(NbBundle.getMessage(OutlineAction.class, "CTL_OutlineAction"));
    }

    public void actionPerformed(ActionEvent evt) {
        TopComponent win = OutlineTopComponent.findInstance();
        win.open();
        win.requestActive();
    }
}
