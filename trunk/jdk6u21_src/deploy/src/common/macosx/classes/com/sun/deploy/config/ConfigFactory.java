/*
 * @(#)ConfigFactory.java	1.2 10/03/24
 * 
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.config;

public class ConfigFactory {
    public static Config newInstance() {
	return new MacOSXConfig();
    }
}
