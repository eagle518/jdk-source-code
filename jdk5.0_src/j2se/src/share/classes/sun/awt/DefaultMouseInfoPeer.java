/*
 * @(#)DefaultMouseInfoPeer.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.Point;
import java.awt.Window;
import java.awt.peer.MouseInfoPeer;

public class DefaultMouseInfoPeer implements MouseInfoPeer {
    
    /**
     * Package-private constructor to prevent instantiation.
     */
    DefaultMouseInfoPeer() {
    }

    public native int fillPointWithCoords(Point point);

    public native boolean isWindowUnderMouse(Window w);

}
