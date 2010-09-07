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
package com.sun.hotspot.igv.bytecodes;

import com.sun.hotspot.igv.data.services.InputGraphProvider;
import org.openide.nodes.Node;
import org.openide.util.HelpCtx;
import org.openide.util.Lookup;
import org.openide.util.NbBundle;
import org.openide.util.actions.CookieAction;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class SelectBytecodesAction extends CookieAction {

    protected void performAction(Node[] activatedNodes) {
        SelectBytecodesCookie c = activatedNodes[0].getCookie(SelectBytecodesCookie.class);
        InputGraphProvider p = Lookup.getDefault().lookup(InputGraphProvider.class);
        if (p != null) {
            p.setSelectedNodes(c.getNodes());
        }
    }

    protected int mode() {
        return CookieAction.MODE_EXACTLY_ONE;
    }

    public String getName() {
        return NbBundle.getMessage(SelectBytecodesAction.class, "CTL_SelectBytecodesAction");
    }

    protected Class[] cookieClasses() {
        return new Class[]{
            SelectBytecodesCookie.class
        };
    }

    @Override
    protected void initialize() {
        super.initialize();
        putValue("noIconInMenu", Boolean.TRUE);
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    protected boolean asynchronous() {
        return false;
    }
}
