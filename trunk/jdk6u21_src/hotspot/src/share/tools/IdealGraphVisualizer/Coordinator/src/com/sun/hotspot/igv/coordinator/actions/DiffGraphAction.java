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

import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.NbBundle;
import org.openide.util.actions.CookieAction;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class DiffGraphAction extends CookieAction {

    protected void performAction(Node[] activatedNodes) {
        DiffGraphCookie c = activatedNodes[0].getCookie(DiffGraphCookie.class);
        c.openDiff();
    }

    protected int mode() {
        return CookieAction.MODE_EXACTLY_ONE;
    }

    public String getName() {
        return NbBundle.getMessage(DiffGraphAction.class, "CTL_DiffGraphAction");
    }

    protected Class[] cookieClasses() {
        return new Class[]{
            DiffGraphCookie.class
        };
    }

    @Override
    protected String iconResource() {
        return "com/sun/hotspot/igv/coordinator/images/diff.gif";
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }
}
