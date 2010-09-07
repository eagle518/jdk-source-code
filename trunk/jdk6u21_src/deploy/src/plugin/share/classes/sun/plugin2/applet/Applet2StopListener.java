/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

/**
 * The listener can be registered to respond to applet stop related activitiy
 * For example, applet failed in stop
 */

public interface Applet2StopListener {
    public void stopFailed();
}
