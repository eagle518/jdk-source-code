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
public class PredSuccAction extends AbstractAction {

    private boolean state;
    public static final String STATE = "state";

    public PredSuccAction() {
        state = true;
        putValue(AbstractAction.SMALL_ICON, new ImageIcon(org.openide.util.Utilities.loadImage(iconResource())));
        putValue(STATE, true);
        putValue(Action.SHORT_DESCRIPTION, "Show neighboring nodes of fully visible nodes semi-transparent");
    }

    public void actionPerformed(ActionEvent ev) {
        this.state = !state;
        this.putValue(STATE, state);
    }

    protected String iconResource() {
        return "com/sun/hotspot/igv/view/images/predsucc.gif";
    }
}
