/*
 * @(#)Layer.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import org.jdesktop.swingx.designer.effects.Effect;
import org.jdesktop.swingx.graphics.GraphicsUtilities;

import java.awt.AlphaComposite;
import java.awt.Composite;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.lang.ref.SoftReference;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

/**
 * Layer
 *
 * @author Created by Jasper Potts (May 22, 2007)
 * @version 1.0
 */
public class Layer extends SimpleShape implements Iterable<SimpleShape>, LayerContainer {
    public static enum LayerType {
        standard, template
    }

    private String name;
    protected LayerType type = LayerType.standard;
    /** List of shapes in this layer, first shape is painted on top */
    private List<SimpleShape> shapes = new ArrayList<SimpleShape>();
    private List<Effect> effects = new ArrayList<Effect>();
    private double opacity = 1;
    private double fillOpacity = 1;
    private BlendingMode blendingMode = BlendingMode.NORMAL;
    private boolean locked = false;
    private boolean visible = true;
    private PropertyChangeListener shapeChangeListener = new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
            int index = shapes.indexOf((SimpleShape) evt.getSource());
            firePropertyChange("shapes[" + index + "]." + evt.getPropertyName(), evt.getOldValue(), evt.getNewValue());
        }
    };
    private PropertyChangeListener effectChangeListener = new PropertyChangeListener() {
        public void propertyChange(PropertyChangeEvent evt) {
            int index = effects.indexOf((Effect) evt.getSource());
            System.out.println(
                    "Layer.propertyChange EFFECT PROPERTY CHANGED " + evt.getSource() + " -- " + evt.getPropertyName());
            firePropertyChange("effects[" + index + "]." + evt.getPropertyName(), evt.getOldValue(), evt.getNewValue());
        }
    };
    private BufferedImage buffer = null;
    // =================================================================================================================
    // Constructors

    public Layer() {
    }

    public Layer(String name) {
        this();
        this.name = name;
    }

    /** Called by JIBX after populating this layer so we can add listeners to children */
    protected void postInit() {
        for (SimpleShape shape : shapes) {
            shape.addPropertyChangeListener(shapeChangeListener);
            shape.setParent(this);
        }
        for (Effect effect : effects) {
            effect.addPropertyChangeListener(effectChangeListener);
        }
    }

    // =================================================================================================================
    // Bean Methods

    public LayerType getType() {
        return type;
    }

    public boolean isLocked() {
        return locked;
    }

    public void setLocked(boolean locked) {
        boolean old = isLocked();
        this.locked = locked;
        firePropertyChange("locked", old, isLocked());
    }

    public boolean isVisible() {
        return visible;
    }

    public void setVisible(boolean visible) {
        boolean old = isVisible();
        this.visible = visible;
        firePropertyChange("visible", old, isVisible());
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        String old = getName();
        this.name = name;
        firePropertyChange("name", old, getName());
    }

    public void setParent(LayerContainer parent) {
        super.setParent(parent);
        // generate a name if null
        if (name == null) {
            Canvas c = null;
            LayerContainer p = parent;
            while (true) {
                if (p instanceof Canvas) {
                    c = (Canvas) p;
                    break;
                } else if (p == null) {
                    break;
                }
                p = p.getParent();
            }
            if (c != null) {
                setName("Layer " + c.getNextLayerNameIndex());
            }
        }
    }

    /**
     * Add shape to top of layer so it paints above all other shapes
     *
     * @param shape The shape to add
     */
    public void add(SimpleShape shape) {
        shapes.add(0, shape);
        shape.setParent(this);
        shape.addPropertyChangeListener(shapeChangeListener);
        fireIndexedPropertyChange("shapes", 0, null, shape);
    }

    public void remove(SimpleShape shape) {
        int index = shapes.indexOf(shape);
        if (index != -1) {
            shapes.remove(shape);
            shape.setParent(null);
            fireIndexedPropertyChange("shapes", index, shape, null);
        }
    }

    /**
     * Returns an unmodifianle iterator over a set of elements of type SimpleShape.
     *
     * @return an Iterator.
     */
    public Iterator<SimpleShape> iterator() {
        return Collections.unmodifiableList(shapes).iterator();
    }


    public List<Effect> getEffects() {
        return Collections.unmodifiableList(effects);
    }

    public void addEffect(Effect effect) {
        int index = effects.size();
        effects.add(effect);
        effect.addPropertyChangeListener(effectChangeListener);
        fireIndexedPropertyChange("effects", index, null, effects);
    }

    public void removeEffect(Effect effect) {
        int index = effects.indexOf(effect);
        if (index != -1) {
            effects.remove(effect);
            effect.removePropertyChangeListener(effectChangeListener);
            fireIndexedPropertyChange("effects", index, effect, null);
        }
    }

    public double getOpacity() {
        return opacity;
    }

    public void setOpacity(double opacity) {
        if (opacity < 0 || opacity > 1) return;
        double old = getOpacity();
        this.opacity = opacity;
        firePropertyChange("opacity", old, getOpacity());
    }

    public double getFillOpacity() {
        return fillOpacity;
    }

    public void setFillOpacity(double fillOpacity) {
        if (fillOpacity < 0 || fillOpacity > 1) return;
        double old = getFillOpacity();
        this.fillOpacity = fillOpacity;
        firePropertyChange("fillOpacity", old, getFillOpacity());
    }

    public BlendingMode getBlendingMode() {
        return blendingMode;
    }

    public void setBlendingMode(BlendingMode blendingMode) {
        BlendingMode old = getBlendingMode();
        this.blendingMode = blendingMode;
        firePropertyChange("blendingMode", old, getBlendingMode());
    }

    // =================================================================================================================
    // Layer Methods

    /**
     * Get the parent canvas that contains this layer
     *
     * @return Parant canvas, or null if the layer is not in a canvas
     */
    public Canvas getCanvas() {
        LayerContainer lc = this;
        while (lc != null) {
            if (lc instanceof Canvas) return (Canvas) lc;
            lc = lc.getParent();
        }
        return null;
    }

    public List<SimpleShape> getShapes() {
        return new ArrayList<SimpleShape>(shapes);
    }

    public List<SimpleShape> getIntersectingShapes(Point2D p, double pixelSize) {
        if (isLocked() || !isVisible()) return Collections.emptyList();
        List<SimpleShape> intersectingShapes = new ArrayList<SimpleShape>();
        for (SimpleShape shape : shapes) {
            if (shape instanceof Layer) {
                intersectingShapes.addAll(((Layer) shape).getIntersectingShapes(p, pixelSize));
            } else {
                if (shape.isHit(p, pixelSize)) intersectingShapes.add(shape);
            }
        }
        return intersectingShapes;
    }

    public List<SimpleShape> getIntersectingShapes(Rectangle2D rect, double pixelSize) {
        if (isLocked() || !isVisible()) return Collections.emptyList();
        List<SimpleShape> intersectingShapes = new ArrayList<SimpleShape>();
        for (SimpleShape shape : shapes) {
            if (shape instanceof Layer) {
                intersectingShapes.addAll(((Layer) shape).getIntersectingShapes(rect, pixelSize));
            } else {
                if (shape.intersects(rect, pixelSize)) intersectingShapes.add(shape);
            }
        }
        return intersectingShapes;

    }

    public Image getBuffer(GraphicsConfiguration graphicsConfiguration) {
        Dimension rootSize = getRootSize();
        int w = Math.max(rootSize.width, 1);
        int h = Math.max(rootSize.height, 1);
        // create/recreate layerBuffer
        Graphics2D bg;
        if (buffer == null || buffer.getWidth() < w || buffer.getHeight() < h) {
            buffer = GraphicsUtilities.createCompatibleTranslucentImage(w, h);
            bg = buffer.createGraphics();
        } else {
            // clear buffer
            bg = buffer.createGraphics();
            bg.setComposite(AlphaComposite.Clear);
            bg.fillRect(0, 0, w, h);
            bg.setComposite(AlphaComposite.SrcOver);
        }
        // create layer content buffer if needed
        BufferedImage contentsBuffer = null;
        Graphics2D contentGraphics = bg;
        if (!effects.isEmpty()) {
            contentsBuffer = getTmpImg1(w, h);
            // clear buffer
            contentGraphics = contentsBuffer.createGraphics();
            contentGraphics.setComposite(AlphaComposite.Clear);
            contentGraphics.fillRect(0, 0, contentsBuffer.getWidth(), contentsBuffer.getHeight());
            contentGraphics.setComposite(AlphaComposite.SrcOver);
        }
        // paint layers contents to contentGraphics
        contentGraphics.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        for (int i = shapes.size() - 1; i >= 0; i--) {
            shapes.get(i).paint(contentGraphics, 1);
        }
        contentGraphics.dispose();
        // apply effects
        if (!effects.isEmpty()) {
            // apply blend effects to content
            BufferedImage img1 = contentsBuffer;
            BufferedImage img2 = getTmpImg2(w, h);
            for (Effect effect : effects) {
                if (effect.getEffectType() == Effect.EffectType.BLENDED && effect.isVisible()) {
                    img2 = effect.applyEffect(img1, img2, w, h);
                    // swap img
                    BufferedImage tmp = img1;
                    img1 = img2;
                    img2 = tmp;
                }
            }
            // img1 will always contain the result
            // apply under effects first
            for (Effect effect : effects) {
                if (effect.getEffectType() == Effect.EffectType.UNDER && effect.isVisible()) {
                    bg.setComposite(effect.getBlendingMode().getComposite(effect.getOpacity()));
                    bg.drawImage(effect.applyEffect(img1, img2, w, h), 0, 0, w, h, 0, 0, w, h, null);
                }
            }
            // paint blended content
            bg.setComposite(AlphaComposite.SrcOver);
            bg.drawImage(img1, 0, 0, w, h, 0, 0, w, h, null);
            // apply over effects
            for (Effect effect : effects) {
                if (effect.getEffectType() == Effect.EffectType.OVER && effect.isVisible()) {
                    bg.setComposite(effect.getBlendingMode().getComposite(effect.getOpacity()));
                    bg.drawImage(effect.applyEffect(img1, img2, w, h), 0, 0, w, h, 0, 0, w, h, null);
                }
            }
            // close buffer graphics
            bg.dispose();
        }
        return buffer;
    }


    public boolean isEmpty() {
        return shapes.isEmpty();
    }

    // =================================================================================================================
    // SimpleShape Methods

    public Rectangle2D getBounds(double pixelSize) {
        Rectangle2D.Double rect = new Rectangle2D.Double();
        for (SimpleShape shape : shapes) {
            rect.add(shape.getBounds(pixelSize));
        }
        return rect;
    }


    public Shape getShape() {
        return getBounds(0);
    }

    public boolean isHit(Point2D p, double pixelSize) {
        if (isLocked() || !isVisible()) return false;
        for (SimpleShape shape : shapes) {
            if (shape.isHit(p, pixelSize)) return true;
        }
        return false;
    }

    public boolean intersects(Rectangle2D rect, double pixelSize) {
        if (isLocked() || !isVisible()) return false;
        for (SimpleShape shape : shapes) {
            if (shape.intersects(rect, pixelSize)) return true;
        }
        return false;
    }

    public List<ControlPoint> getControlPoints() {
        return Collections.emptyList();
    }

    public void paint(Graphics2D g2, double pixelSize) {
        if (isVisible() && !shapes.isEmpty()) {
            Dimension rootSize = getRootSize();
            if (rootSize.width == 0 || rootSize.height == 0) return;
            //paint buffer to g2
            Composite oldComposite = g2.getComposite();
            g2.setComposite(blendingMode.getComposite((float) (opacity * fillOpacity)));
            g2.drawImage(getBuffer(g2.getDeviceConfiguration()), 0, 0, null);
            g2.setComposite(oldComposite);
        }
    }

    public void paintControls(Graphics2D g2, double pixelSize, boolean paintControlLines) {

    }

    public String toString() {
        return getName();
    }

    // =================================================================================================================
    // LayerContainer Methods

    public void addLayer(int i, Layer layer) {
        // get existing layer at index i
        Layer existingLayer = getLayer(i);
        if (existingLayer == null) {
            addLayer(layer);
        } else {
            int index = indexOfLayer(existingLayer);
            shapes.add(index, layer);
            layer.setParent(this);
            layer.addPropertyChangeListener(shapeChangeListener);
            fireIndexedPropertyChange("layers", index, null, layer);
        }
    }

    public void addLayer(Layer layer) {
        shapes.add(layer);
        layer.setParent(this);
        layer.addPropertyChangeListener(shapeChangeListener);
        int index = indexOfLayer(layer);
        fireIndexedPropertyChange("layers", index, null, layer);
    }

    public Layer getLayer(int index) {
        int i = -1;
        for (SimpleShape shape : shapes) {
            if (shape instanceof Layer) i++;
            if (i == index) return (Layer) shape;
        }
        return null;
    }

    public int getLayerCount() {
        int count = 0;
        for (SimpleShape shape : shapes) {
            if (shape instanceof Layer) count++;
        }
        return count;
    }


    public Collection<Layer> getLayers() {
        List<Layer> layers = new ArrayList<Layer>();
        for (SimpleShape shape : shapes) {
            if (shape instanceof Layer) layers.add((Layer) shape);
        }
        return Collections.unmodifiableList(layers);
    }

    public Iterator<Layer> getLayerIterator() {
        return new Iterator<Layer>() {
            private int index = 0;

            public boolean hasNext() {
                for (int i = index; i < shapes.size(); i++) {
                    if (shapes.get(i) instanceof Layer) {
                        return true;
                    }
                }
                return false;
            }

            public Layer next() {
                for (; index < shapes.size(); index++) {
                    if (shapes.get(index) instanceof Layer) {
                        Layer nextLayer = (Layer) shapes.get(index);
                        index++; // increment index so we don't find the same one again
                        return nextLayer;
                    }
                }
                return null;
            }

            public void remove() {
                throw new UnsupportedOperationException();
            }
        };
    }

    public int indexOfLayer(Layer layer) {
        int i = -1;
        for (SimpleShape s : shapes) {
            if (s instanceof Layer) i++;
            if (s == layer) return i;
        }
        return -1;
    }

    public void removeLayer(Layer layer) {
        int index = indexOfLayer(layer);
        if (index != -1) {
            shapes.remove(layer);
            layer.removePropertyChangeListener(shapeChangeListener);
            fireIndexedPropertyChange("layers", index, layer, null);
        }
    }


    public Dimension getRootSize() {
        return getParent().getRootSize();
    }

    // =================================================================================================================
    // Static data cache

    private static SoftReference<BufferedImage> tmpImg1 = null;
    private static SoftReference<BufferedImage> tmpImg2 = null;

    protected static BufferedImage getTmpImg1(int w, int h) {
        BufferedImage tmp;
        if (tmpImg1 == null || (tmp = tmpImg1.get()) == null || tmp.getWidth() < w || tmp.getHeight() < h) {
            // create new array
            tmp = GraphicsUtilities.createCompatibleTranslucentImage(w, h);
            tmpImg1 = new SoftReference<BufferedImage>(tmp);
        }
        return tmp;
    }

    protected static BufferedImage getTmpImg2(int w, int h) {
        BufferedImage tmp;
        if (tmpImg2 == null || (tmp = tmpImg2.get()) == null || tmp.getWidth() < w || tmp.getHeight() < h) {
            // create new array
            tmp = GraphicsUtilities.createCompatibleTranslucentImage(w, h);
            tmpImg2 = new SoftReference<BufferedImage>(tmp);
        }
        return tmp;
    }
}
