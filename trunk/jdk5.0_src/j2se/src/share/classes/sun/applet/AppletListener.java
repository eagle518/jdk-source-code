/*
 * @(#)AppletListener.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

import java.util.EventListener;

/**
 * Applet Listener interface.  This interface is to be implemented
 * by objects interested in AppletEvents.
 * 
 * @version 1.9, 12/19/03
 * @author  Sunita Mani
 */

public interface AppletListener extends EventListener {
    public void appletStateChanged(AppletEvent e);
}
 

