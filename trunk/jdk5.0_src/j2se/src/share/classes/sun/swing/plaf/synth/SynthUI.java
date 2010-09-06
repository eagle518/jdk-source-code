/*
 * @(#)SynthUI.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.swing.plaf.synth;

import java.awt.Graphics;
import javax.swing.JComponent;
import javax.swing.plaf.synth.*;

/**
 * SynthUI is used to fetch the SynthContext for a particular Component.
 *
 * @version 1.11, 12/19/03
 * @author Scott Violet
 */
public interface SynthUI extends SynthConstants {
    /**
     * Returns the Context for the specified component.
     *
     * @param c Component requesting SynthContext.
     * @return SynthContext describing component.
     */
    public SynthContext getContext(JComponent c);

    /**
     * Paints the border.
     */
    public void paintBorder(SynthContext context, Graphics g, int x,
                            int y, int w, int h);
}
