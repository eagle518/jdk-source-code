/*
 * @(#)DialogListener.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

/**
 * DialogListener is an interface that does product specifc work
 * before showing any dialog (e.g hide splash screen)
 */
public interface DialogListener {
    public void beforeShow();
}
