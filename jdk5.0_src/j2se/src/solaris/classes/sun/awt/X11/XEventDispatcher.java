/*
 * @(#)XEventDispatcher.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

/**
 * Interface representing abstract event dispatchers in the system.
 * Any class implementing this interface can be installed to receive
 * event on particular window.
 */

public interface XEventDispatcher {
    void dispatchEvent(IXAnyEvent ev);
}
