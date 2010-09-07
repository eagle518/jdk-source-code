/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.hotspot.igv.filterwindow;

import com.sun.hotspot.igv.data.ChangedEvent;
import org.openide.nodes.AbstractNode;
import org.openide.nodes.Children;
import org.openide.util.Lookup;

/**
 *
 * @author Thomas Wuerthinger
 */
public class CheckNode extends AbstractNode {

    private ChangedEvent<CheckNode> selectionChangedEvent;
    public boolean selected;
    public boolean enabled;

    public CheckNode(Children c, Lookup lookup) {
        super(c, lookup);
        selectionChangedEvent = new ChangedEvent<CheckNode>(this);
        selected = false;
        enabled = true;
    }

    public ChangedEvent<CheckNode> getSelectionChangedEvent() {
        return selectionChangedEvent;
    }

    public boolean isSelected() {
        return selected;
    }

    public void setSelected(boolean b) {
        if (b != selected) {
            selected = b;
            selectionChangedEvent.fire();
        }
    }

    public void setEnabled(boolean b) {
        enabled = b;
    }

    public boolean isEnabled() {
        return enabled;
    }
}
