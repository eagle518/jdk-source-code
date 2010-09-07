/*
 * @(#)WGlobalCursorManager.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import sun.awt.GlobalCursorManager;

public final class WGlobalCursorManager extends GlobalCursorManager {
    private static WGlobalCursorManager manager;

    public static GlobalCursorManager getCursorManager() {
        if (manager == null) {
            manager = new WGlobalCursorManager();
        }
        return manager;
    }

    /**
     * Should be called in response to a native mouse enter or native mouse
     * button released message. Should not be called during a mouse drag.
     */
    public static void nativeUpdateCursor(Component heavy) {
	WGlobalCursorManager.getCursorManager().updateCursorLater(heavy);
    }

    protected native void setCursor(Component comp, Cursor cursor, boolean u);
    protected native void getCursorPos(Point p);
    protected native Component findComponentAt(Container con, int x, int y);
    /*
     * two native methods to call corresponding methods in Container and
     * Component
     */
    protected native Component findHeavyweightUnderCursor(boolean useCache);
    protected native Point getLocationOnScreen(Component com);
}
