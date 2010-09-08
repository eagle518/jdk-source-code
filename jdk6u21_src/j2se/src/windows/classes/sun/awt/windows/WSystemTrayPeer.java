/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved. Use is
 * subject to license terms.
 */

package sun.awt.windows;

import java.awt.SystemTray;
import java.awt.Dimension;
import java.awt.Toolkit;
import java.awt.peer.SystemTrayPeer;

public class WSystemTrayPeer extends WObjectPeer implements SystemTrayPeer {
    WSystemTrayPeer(SystemTray target) {
        this.target = target;
    }

    public Dimension getTrayIconSize() {
        return new Dimension(WTrayIconPeer.TRAY_ICON_WIDTH, WTrayIconPeer.TRAY_ICON_HEIGHT);
    }

    public boolean isSupported() {
        return ((WToolkit)Toolkit.getDefaultToolkit()).isTraySupported();
    }

    protected void disposeImpl() {
    }
}
