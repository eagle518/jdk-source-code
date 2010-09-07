/*
 * @(#)DialogHook.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

import java.awt.Component;

/** Provides a hook, called in the AppContext of the application or
    applet, before transferring over to the privileged AppContext for
    the display of various dialogs. This allows both notification that
    such a dialog is being raised as well as an interposition hook to
    affect the placement and modal behavior of the dialog. */

public interface DialogHook {
    /** Called in the application's AppContext before the dialog is
        raised. The given Component is the planned owner of the
        dialog; it can be replaced by returning a different Component
        from this method. */
    public Component beforeDialog(Component owner);

    /** Called in the application's AppContext after the dialog is
        dismissed. */
    public void afterDialog();

    /** Affects the placement of the dialog; return true to place the
        dialog ignoring the visibility of the dialog's owner. */
    public boolean ignoreOwnerVisibility();
}
