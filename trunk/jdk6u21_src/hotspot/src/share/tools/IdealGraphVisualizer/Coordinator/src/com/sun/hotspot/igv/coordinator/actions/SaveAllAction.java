/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
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
 *
 */
package com.sun.hotspot.igv.coordinator.actions;

import com.sun.hotspot.igv.coordinator.OutlineTopComponent;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import javax.swing.Action;
import javax.swing.KeyStroke;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.actions.CallableSystemAction;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class SaveAllAction extends CallableSystemAction {

    public void performAction() {
        final OutlineTopComponent component = OutlineTopComponent.findInstance();
        SaveAsAction.save(component.getDocument());
    }

    public String getName() {
        return NbBundle.getMessage(SaveAllAction.class, "CTL_SaveAllAction");
    }

    public SaveAllAction() {
        putValue(Action.SHORT_DESCRIPTION, "Save all methods to XML file");
        putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.CTRL_MASK));
    }

    @Override
    protected String iconResource() {
        return "com/sun/hotspot/igv/coordinator/images/saveall.gif";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }
}
