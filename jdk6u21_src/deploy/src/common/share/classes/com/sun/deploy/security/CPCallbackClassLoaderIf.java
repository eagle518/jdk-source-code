/*
 * @(#)CPCallbackClassLoaderIf.java	1.2 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * @version 1.0
 */

package com.sun.deploy.security;

import java.security.CodeSource;

public interface CPCallbackClassLoaderIf {
    CodeSource[] getTrustedCodeSources(CodeSource[] sources);
}
