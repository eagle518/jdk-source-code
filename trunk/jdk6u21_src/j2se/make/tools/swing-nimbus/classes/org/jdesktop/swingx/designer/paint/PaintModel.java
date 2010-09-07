/*
 * @(#)PaintModel.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package org.jdesktop.swingx.designer.paint;

import org.jdesktop.beans.AbstractBean;

import java.awt.Paint;

/**
 * I'd have just called it Paint, but sadly, that name was already taken, and would have been too confusing.
 * <p/>
 * Whenever size or position values are required (for example with Texture or Gradient), they are specified in the unit
 * square: that is, between 0 and 1 inclusive. They can then later be scaled as necessary by any painting code.
 *
 * @author rbair
 */
public abstract class PaintModel extends AbstractBean implements Cloneable {
    public static enum PaintControlType {
        none, control_line, control_rect
    }

    protected PaintModel() { }

    /**
     * @return an instance of Paint that is represented by this PaintModel. This is often not a reversable operation,
     *         and hence there is no "setPaint" method. Rather, tweaking the exposed properties of the PaintModel fires,
     *         when necessary, property change events for the "paint" property, and results in different values returned
     *         from this method.
     */
    public abstract Paint getPaint();

    /**
     * Get the type of controls for this paint model
     *
     * @return The type of paint controls, one of PaintControlType.none, PaintControlType.control_line or
     *         PaintControlType.control_rect
     */
    public abstract PaintControlType getPaintControlType();


    public abstract PaintModel clone();
}
