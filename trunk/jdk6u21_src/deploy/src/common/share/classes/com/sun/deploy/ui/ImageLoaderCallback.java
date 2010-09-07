/*
 * @(#)ImageLoaderCallback.java	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.ui;

import java.net.URL;
import java.awt.Image;
import java.io.File;

/**
 * interface to give image loader so it can tell you
 * 1) when it has something you can display, and
 * 2) when it has the updated, cached image for you to display.
 */
public interface ImageLoaderCallback {
    public void imageAvailable(URL url, String version, Image image, File file);
    public void finalImageAvailable(URL url, String version, 
				    Image image, File file);
}
