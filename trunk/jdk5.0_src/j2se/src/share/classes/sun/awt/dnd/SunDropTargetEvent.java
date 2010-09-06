/*
 * @(#)SunDropTargetEvent.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.dnd;

import java.awt.Component;
import java.awt.dnd.InvalidDnDOperationException;
import java.awt.event.MouseEvent;

public class SunDropTargetEvent extends MouseEvent {

    public static final int MOUSE_DROPPED = MouseEvent.MOUSE_RELEASED;

    private final SunDropTargetContextPeer.EventDispatcher dispatcher;

    public SunDropTargetEvent(Component source, int id, int x, int y, 
                              SunDropTargetContextPeer.EventDispatcher d) {
        super(source, id, System.currentTimeMillis(), 0, x, y, 0, false);
        dispatcher = d;
        dispatcher.registerEvent(this);
    }
    
    public void dispatch() {
        try {
            dispatcher.dispatchEvent(this);
        } finally {
            dispatcher.unregisterEvent(this);
        }
    }

    public void consume() {
        boolean was_consumed = isConsumed();
        super.consume();
        if (!was_consumed && isConsumed()) {
            dispatcher.unregisterEvent(this);
        }
    }

    public SunDropTargetContextPeer.EventDispatcher getDispatcher() {
        return dispatcher;
    }

    public String paramString() {
        String typeStr = null;

        switch (id) {
        case MOUSE_DROPPED:           
            typeStr = "MOUSE_DROPPED"; break;
        default:
            return super.paramString();
        }
        return typeStr + ",(" + getX() + "," + getY() + ")";
    }
}
