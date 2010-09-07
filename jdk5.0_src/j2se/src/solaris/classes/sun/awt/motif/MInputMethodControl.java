/*
 * @(#)MInputMethodControl.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.motif.MComponentPeer;
import sun.awt.motif.MInputMethod;

/**
 * An interface for controlling containment hierarchy configuration to
 * keep track of existence of any TextArea or TextField and to manage
 * input method status area.
 *
 * @version	1.11 12/19/03
 * @auther	JavaSoft International
 */
interface MInputMethodControl {

    /**
     * Informs Frame or Dialog that a text component has been added to
     * the hierarchy.
     * @param	textComponentPeer	peer of the text component
     */
    void addTextComponent(MComponentPeer textComponentPeer);

    /**
     * Informs Frame or Dialog that a text component has been removed
     * from the hierarchy.
     * @param textComponentPeer peer of the text component
     */
    void removeTextComponent(MComponentPeer textComponentPeer);

    /**
     * Returns a text component peer in the containment hierarchy 
     * to obtain the Motif status area information
     */
    MComponentPeer getTextComponent();

    /**
     * Inform Frame or Dialog that an MInputMethod has been
     * constructed so that Frame and Dialog can invoke the method in
     * MInputMethod to reconfigure XICs.
     * @param	inputMethod	an MInputMethod instance
     */
    void addInputMethod(MInputMethod inputMethod);

    /**
     * Inform Frame or Dialog that an X11InputMethod is being destroyed.
     * @param	inputMethod	an X11InputMethod instance
     */
    void removeInputMethod(MInputMethod inputMethod);
}
