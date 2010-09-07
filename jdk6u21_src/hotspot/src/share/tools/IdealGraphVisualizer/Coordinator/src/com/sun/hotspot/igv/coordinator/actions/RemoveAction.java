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

import javax.swing.Action;
import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.actions.NodeAction;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class RemoveAction extends NodeAction {

    protected void performAction(Node[] activatedNodes) {
        for (Node n : activatedNodes) {
            RemoveCookie removeCookie = n.getCookie(RemoveCookie.class);
            if (removeCookie != null) {
                removeCookie.remove();
            }
        }
    }

    public RemoveAction() {
        putValue(Action.SHORT_DESCRIPTION, "Remove");
    }

    public String getName() {
        return NbBundle.getMessage(RemoveAction.class, "CTL_RemoveAction");
    }

    @Override
    protected String iconResource() {
        return "com/sun/hotspot/igv/coordinator/images/remove.gif";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }

    protected boolean enable(Node[] nodes) {
        return nodes.length > 0;
    }
}
