/*
 * @(#)ConfigFactory.java	1.3 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.config;

public class ConfigFactory {
    public static Config newInstance() {
	return new UnixConfig();
    }
}
