/*
 * @(#)SynthIcon.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.swing.plaf.synth;

import javax.swing.plaf.synth.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.border.Border;
import javax.swing.plaf.UIResource;

/**
 * An icon that is passed a SynthContext. Subclasses need only implement
 * the variants that take a SynthContext, but must be prepared for the
 * SynthContext to be null.
 *
 * @version 1.8, 12/19/03
 * @author Scott Violet
 */
public abstract class SynthIcon implements Icon {
    public static int getIconWidth(Icon icon, SynthContext context) {
        if (icon == null) {
            return 0;
        }
        if (icon instanceof SynthIcon) {
            return ((SynthIcon)icon).getIconWidth(context);
        }
        return icon.getIconWidth();
    }

    public static int getIconHeight(Icon icon, SynthContext context) {
        if (icon == null) {
            return 0;
        }
        if (icon instanceof SynthIcon) {
            return ((SynthIcon)icon).getIconHeight(context);
        }
        return icon.getIconHeight();
    }

    public static void paintIcon(Icon icon, SynthContext context, Graphics g,
                                 int x, int y, int w, int h) {
        if (icon instanceof SynthIcon) {
            ((SynthIcon)icon).paintIcon(context, g, x, y, w, h);
        }
        else if (icon != null) {
            icon.paintIcon(context.getComponent(), g, x, y);
        }
    }

    /**
     * Paints the icon at the specified location.
     *
     * @param context Identifies hosting region, may be null.
     * @param x x location to paint to
     * @param y y location to paint to
     * @param w Width of the region to paint to, may be 0
     * @param h Height of the region to paint to, may be 0
     */
    public abstract void paintIcon(SynthContext context, Graphics g, int x,
                                   int y, int w, int h);

    /**
     * Returns the desired width of the Icon.
     *
     * @param context SynthContext requesting the Icon, may be null.
     * @return Desired width of the icon.
     */
    public abstract int getIconWidth(SynthContext context);

    /**
     * Returns the desired height of the Icon.
     *
     * @param context SynthContext requesting the Icon, may be null.
     * @return Desired height of the icon.
     */
    public abstract int getIconHeight(SynthContext context);

    /**
     * Paints the icon. This is a cover method for
     * <code>paintIcon(null, g, x, y, 0, 0)</code>
     */
    public void paintIcon(Component c, Graphics g, int x, int y) {
        paintIcon(null, g, x, y, 0, 0);
    }
    
    /**
     * Returns the icon's width. This is a cover methods for
     * <code>getIconWidth(null)</code>.
     *
     * @return an int specifying the fixed width of the icon.
     */
    public int getIconWidth() {
        return getIconWidth(null);
    }

    /**
     * Returns the icon's height. This is a cover method for
     * <code>getIconHeight(null)</code>.
     *
     * @return an int specifying the fixed height of the icon.
     */
    public int getIconHeight() {
        return getIconHeight(null);
    }
}
