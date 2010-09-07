/*
 * @(#)ConfigFactory.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.config;

public class ConfigFactory {
    public static Config newInstance() {
	return new UnixConfig();
    }
}
