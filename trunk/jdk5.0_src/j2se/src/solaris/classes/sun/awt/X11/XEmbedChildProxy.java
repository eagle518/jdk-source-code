/*
 * @(#)XEmbedChildProxy.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Component;
import java.awt.Toolkit;

public class XEmbedChildProxy extends Component {
    long handle;
    XEmbeddingContainer container;
    public XEmbedChildProxy(XEmbeddingContainer container, long handle) {
        this.handle = handle;
        this.container = container;
    }

    public void addNotify() {
        synchronized(getTreeLock()) {
            if (ComponentAccessor.getPeer(this) == null) {
                ComponentAccessor.setPeer(this, ((XToolkit)Toolkit.getDefaultToolkit()).createEmbedProxy(this));
            }
            super.addNotify();
        }
    }

    XEmbeddingContainer getEmbeddingContainer() {
        return container;
    }
    long getHandle() {
        return handle;
    }
}

