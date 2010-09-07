/*
 * @(#)Canvas.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import org.jdesktop.beans.AbstractBean;
import org.jdesktop.swingx.designer.utils.HasResources;
import org.jdesktop.swingx.designer.utils.HasUIDefaults;
import org.jibx.runtime.IUnmarshallingContext;

import javax.swing.UIDefaults;
import java.awt.AlphaComposite;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Insets;
import java.awt.RenderingHints;
import java.awt.image.BufferedImage;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

/**
 * ComponentRegion
 *
 * @author Created by Jasper Potts (May 22, 2007)
 * @version 1.0
 */
public class Canvas extends AbstractBean implements LayerContainer, HasUIDefaults, HasResources {
    private Dimension size;
    /** list of all layers in the canvas, the first layer is painted on top */
    private List<Layer> layers;
    private int nextLayerNameIndex = 1;
    private BufferedImage buffer;
    private boolean isValid = false;
    private Insets stretchingInsets = null;
    private Layer workingLayer = null;
    private PropertyChangeListener layersPropertyChangeListener;
    private UIDefaults canvasUIDefaults = null;
    private transient File resourcesDir;
    private transient File imagesDir;
    private transient File templatesDir;

    // =================================================================================================================
    // Constructor

    /** Private constructor for JIBX */
    protected Canvas() {
        layersPropertyChangeListener = new PropertyChangeListener() {
            public void propertyChange(PropertyChangeEvent evt) {
                isValid = false;
                // pass on layer change
                int index = layers.indexOf((Layer) evt.getSource());
                if (index != -1) {
                    firePropertyChange("layers[" + index + "]." + evt.getPropertyName(), evt.getOldValue(),
                            evt.getNewValue());
                }
            }
        };
    }

    public Canvas(int width, int height) {
        this();
        stretchingInsets = new Insets(1, 1, 1, 1);
        layers = new ArrayList<Layer>();
        setSize(new Dimension(width, height));
        addLayer(new Layer());
    }

    // =================================================================================================================
    // JIBX Methods

    /**
     * Called by JIBX before all fields have been set
     *
     * @param context The JIBX Unmarshalling Context
     */
    private void preSet(IUnmarshallingContext context) {
        canvasUIDefaults = (UIDefaults) context.getUserContext();
    }

    // =================================================================================================================
    // Bean Methods

    /**
     * Get the UIDefaults for this canvas. The UIDefaults is used to store default pallet of colors, fonts etc.
     *
     * @return Canvas UIDefaults
     */
    public UIDefaults getUiDefaults() {
        return canvasUIDefaults;
    }

    /**
     * Set the UIDefaults for this canvas. The UIDefaults is used to store default pallet of colors, fonts etc.
     *
     * @param canvasUIDefaults Canvas UIDefaults
     */
    public void setUiDefaults(UIDefaults canvasUIDefaults) {
        this.canvasUIDefaults = canvasUIDefaults;
    }

    /**
     * Get the current working layer, is is the layer that new shapes will be drawn into
     *
     * @return The current working layer, may be null if there is no working layer
     */
    public Layer getWorkingLayer() {
        return workingLayer;
    }

    /**
     * Set the current working layer, is is the layer that new shapes will be drawn into
     *
     * @param workingLayer the new working layer, must be a child of this canvas
     */
    public void setWorkingLayer(Layer workingLayer) {
        Layer old = getWorkingLayer();
        this.workingLayer = workingLayer;
        firePropertyChange("workingLayer", old, getWorkingLayer());
    }

    public int getNextLayerNameIndex() {
        return nextLayerNameIndex++;
    }

    public Dimension getSize() {
        return size;
    }

    public void setSize(Dimension size) {
        Dimension old = getSize();
        this.size = size;
        buffer = new BufferedImage(this.size.width, this.size.height, BufferedImage.TYPE_INT_ARGB);
        isValid = false;
        firePropertyChange("size", old, getSize());
    }


    public Insets getStretchingInsets() {
        return stretchingInsets;
    }

    public void setStretchingInsets(Insets stretchingInsets) {
        Insets old = getStretchingInsets();
        this.stretchingInsets = stretchingInsets;
        firePropertyChange("stretchingInsets", old, getStretchingInsets());
    }

    public BufferedImage getRenderedImage() {
        if (!isValid) {
            Graphics2D g2 = buffer.createGraphics();
            // clear
            g2.setComposite(AlphaComposite.Clear);
            g2.fillRect(0, 0, buffer.getWidth(), buffer.getHeight());
            // paint
            g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
            g2.setComposite(AlphaComposite.SrcOver);
            for (int i = layers.size() - 1; i >= 0; i--) {
                layers.get(i).paint(g2, 1);
            }
            g2.dispose();
        }
        return buffer;
    }

    /**
     * @return true if this Canvas has not been edited.
     *         <p/>
     *         TODO Currently this is not a bound property, but should be. That is, when the Canvas becomes edited
     *         (usually due to the Layer having a shape added to it), then a property change event should be fired.
     */
    public boolean isBlank() {
        return layers.size() == 0 || (layers.size() == 1 && layers.get(0).isEmpty());
    }

    public File getResourcesDir() {
        return resourcesDir;
    }

    public void setResourcesDir(File resourcesDir) {
        File old = getResourcesDir();
        this.resourcesDir = resourcesDir;
        firePropertyChange("resourcesDir", old, getResourcesDir());
    }

    public File getImagesDir() {
        return imagesDir;
    }

    public void setImagesDir(File imagesDir) {
        File old = getImagesDir();
        this.imagesDir = imagesDir;
        firePropertyChange("imagesDir", old, getImagesDir());
    }

    public File getTemplatesDir() {
        return templatesDir;
    }

    public void setTemplatesDir(File templatesDir) {
        File old = getTemplatesDir();
        this.templatesDir = templatesDir;
        firePropertyChange("templatesDir", old, getTemplatesDir());
    }

    // =================================================================================================================
    // LayerContainer Methods

    public LayerContainer getParent() {
        // we are root so null
        return null;
    }

    public void addLayerToBottom(Layer layer) {
        layers.add(layer);
        layer.setParent(this);
        layer.addPropertyChangeListener(layersPropertyChangeListener);
        // no single layer changes so fire all changed event
        firePropertyChange("layers", null, layers);
    }

    public void addLayer(int i, Layer layer) {
        layers.add(i, layer);
        layer.setParent(this);
        layer.addPropertyChangeListener(layersPropertyChangeListener);
        // no single layer changes so fire all changed event
        firePropertyChange("layers", null, layers);
    }

    public void addLayer(Layer layer) {
        layers.add(0, layer);
        layer.setParent(this);
        layer.addPropertyChangeListener(layersPropertyChangeListener);
        // no single layer changes so fire all changed event
        firePropertyChange("layers", null, layers);
    }

    public Layer getLayer(int index) {
        return layers.get(index);
    }

    public int getLayerCount() {
        return layers.size();
    }

    public Iterator<Layer> getLayerIterator() {
        return Collections.unmodifiableList(layers).iterator();
    }

    public Collection<Layer> getLayers() {
        return Collections.unmodifiableList(layers);
    }

    public int indexOfLayer(Layer layer) {
        return layers.indexOf(layer);
    }

    public void removeLayer(Layer layer) {
        int index = layers.indexOf(layer);
        if (index != -1) {
            layers.remove(layer);
            layer.removePropertyChangeListener(layersPropertyChangeListener);
            fireIndexedPropertyChange("layers", index, layer, null);
        }
    }

    public Dimension getRootSize() {
        return getSize();
    }

    // =================================================================================================================
    // JIBX Helper Methods

    /** Called by JIBX after "layers" has been filled so we can set parents and listeners */
    private void setupLayers() {
        for (Layer layer : layers) {
            layer.setParent(this);
            layer.addPropertyChangeListener(layersPropertyChangeListener);
        }
        // no single layer changes so fire all changed event
        firePropertyChange("layers", null, layers);
    }
}
