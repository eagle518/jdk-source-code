/*
 * @(#)DialogListener.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

/**
 * DialogListener is an interface that does product specifc work
 * before showing any dialog (e.g hide splash screen)
 */
public interface DialogListener {
    public void beforeShow();
}
