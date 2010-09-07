/*
 * @(#)LayerContainer.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import java.awt.Dimension;
import java.beans.PropertyChangeListener;
import java.util.Collection;
import java.util.Iterator;

/**
 * LayerContainer
 *
 * @author Created by Jasper Potts (May 31, 2007)
 * @version 1.0
 */
public interface LayerContainer {
    public void addPropertyChangeListener(PropertyChangeListener listener);

    public void removePropertyChangeListener(PropertyChangeListener listener);

    public LayerContainer getParent();

    public void addLayer(Layer layer);

    public void addLayer(int i, Layer layer);

    public void removeLayer(Layer layer);

    public int getLayerCount();

    public Layer getLayer(int index);

    public int indexOfLayer(Layer layer);

    public Iterator<Layer> getLayerIterator();

    public Collection<Layer> getLayers();

    /**
     * Get the size in pixels of the root of the layer tree, this is usualy a canvas
     *
     * @return The size of the whole layer tree
     */
    public Dimension getRootSize();

}
