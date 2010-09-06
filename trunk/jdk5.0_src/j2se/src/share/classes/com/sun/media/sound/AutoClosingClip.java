/*
 * @(#)AutoClosingClip.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.media.sound;

import javax.sound.sampled.Clip;

/**
 * Interface for Clip objects that close themselves automatically
 *
 * @version 1.3, 03/12/19
 * @author Florian Bomers
 */
interface AutoClosingClip extends Clip {
    
    /**
     * Indicates whether this clip instance is auto closing.
     * When the clip is auto closing, it calls the close()
     * method automatically after a short period of inactivity.<br>
     * <br>
     *
     * @return true if this clip is auto closing
     */
    boolean isAutoClosing();
    
    /**
     * Sets whether this Clip instance is auto closing or not.
     * If true, the close() method will be called automatically
     * after a short period of inactivity.
     *
     * @param value - true to set this clip to auto closing
     */
    void setAutoClosing(boolean value);
}
