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
package com.sun.hotspot.igv.view.actions;

import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ImageIcon;

/**
 *
 * @author Thomas Wuerthinger
 */
public class EnableBlockLayoutAction extends AbstractAction {

    private boolean state;
    public static final String STATE = "state";

    public EnableBlockLayoutAction() {
        state = true;
        putValue(AbstractAction.SMALL_ICON, new ImageIcon(org.openide.util.Utilities.loadImage(iconResource())));
        putValue(STATE, true);
        putValue(Action.SHORT_DESCRIPTION, "Cluster nodes into blocks");
    }

    public void actionPerformed(ActionEvent ev) {
        this.state = !state;
        this.putValue(STATE, state);
    }

    protected String iconResource() {
        return "com/sun/hotspot/igv/view/images/blocks.gif";
    }
}
