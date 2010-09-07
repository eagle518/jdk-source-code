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

import com.sun.hotspot.igv.filterwindow.FilterTopComponent;
import com.sun.hotspot.igv.filter.Filter;
import javax.swing.Action;
import javax.swing.JOptionPane;
import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.actions.CookieAction;
import org.openide.windows.WindowManager;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class RemoveFilterAction extends CookieAction {

    protected void performAction(Node[] activatedNodes) {
        Object[] options = {"Yes",
            "No",
            "Cancel"
        };
        int n = JOptionPane.showOptionDialog(WindowManager.getDefault().getMainWindow(),
                "Do you really want to delete " + activatedNodes.length + " filter/s?", "Delete?",
                JOptionPane.YES_NO_CANCEL_OPTION,
                JOptionPane.QUESTION_MESSAGE,
                null,
                options,
                options[2]);

        if (n == JOptionPane.YES_OPTION) {
            for (int i = 0; i < activatedNodes.length; i++) {
                FilterTopComponent.findInstance().removeFilter(activatedNodes[i].getLookup().lookup(Filter.class));
            }
        }
    }

    protected int mode() {
        return CookieAction.MODE_ALL;
    }

    public String getName() {
        return NbBundle.getMessage(RemoveFilterAction.class, "CTL_RemoveFilterAction");
    }

    public RemoveFilterAction() {
        putValue(Action.SHORT_DESCRIPTION, "Remove filter");
    }

    protected Class[] cookieClasses() {
        return new Class[]{
            Filter.class
        };
    }

    @Override
    protected void initialize() {
        super.initialize();
        putValue("noIconInMenu", Boolean.TRUE);
    }

    @Override
    protected String iconResource() {
        return "com/sun/hotspot/igv/filterwindow/images/minus.gif";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }
}
