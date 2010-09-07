/*
 * @(#)SimpleShape.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import org.jdesktop.beans.AbstractBean;

import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.util.List;

/**
 * SimpleShape
 *
 * @author Created by Jasper Potts (May 22, 2007)
 * @version 1.0
 */
public abstract class SimpleShape extends AbstractBean {

    protected AffineTransform transform = new AffineTransform();
    protected LayerContainer parent = null;

    public void applyTransform(AffineTransform t) {
        transform.concatenate(t);
    }

    public abstract Rectangle2D getBounds(double pixelSize);

    public abstract void paint(Graphics2D g2, double pixelSize);

    public abstract boolean isHit(Point2D p, double pixelSize);

    public boolean intersects(Rectangle2D rect, double pixelSize) {
        return getBounds(pixelSize).intersects(rect);
    }

    public abstract List<? extends ControlPoint> getControlPoints();

    public abstract void paintControls(Graphics2D g2, double pixelSize, boolean paintControlLines);

    public void move(double moveX, double moveY, boolean snapPixels) {
        for (ControlPoint controlPoint : getControlPoints()) {
            controlPoint.move(moveX, moveY, snapPixels);
        }
    }

    public LayerContainer getParent() {
        return parent;
    }

    public void setParent(LayerContainer parent) {
        LayerContainer old = getParent();
        this.parent = parent;
        firePropertyChange("parent", old, getParent());
    }

    public abstract Shape getShape();
}
