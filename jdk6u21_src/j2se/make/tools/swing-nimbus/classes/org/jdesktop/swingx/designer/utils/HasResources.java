/*
 * @(#)HasResources.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.utils;

import java.io.File;

/**
 * HasResources - interface for model nodes that have resources
 *
 * @author Created by Jasper Potts (Jul 2, 2007)
 * @version 1.0
 */
public interface HasResources {

    public File getResourcesDir();

    public File getImagesDir();

    public File getTemplatesDir();

}
