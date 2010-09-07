/*
 * @(#)AnimationPanelBase.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.util;

import java.awt.Canvas;
import java.awt.Color;
import java.lang.Runnable;

public abstract class AnimationPanelBase extends Canvas implements Runnable {
    public abstract void startAnimation();
    public abstract void stopAnimation();
    public abstract void setProgressValue(float value);
    public abstract float getProgressValue();
    public abstract void fadeAway();

    // The following two deliberately have no effect for the old
    // orange logo animation
    public void setBoxBGColor(Color bgColor) {}
    public void setBoxFGColor(Color fgColor) {}
}
