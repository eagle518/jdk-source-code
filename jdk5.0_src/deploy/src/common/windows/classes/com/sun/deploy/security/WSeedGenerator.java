/*
 * @(#)WSeedGenerator.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

/**
 * <p>This class provides seed for secure random generator using
 * Microsoft Cryto APIs for HTTPS support.
 *
 * By default, JSSE creates the seed for SecureRandom using
 * complicated methods to gather entropy information from
 * VM, which may cause up to 20 seconds of delay during startup.
 * Thus, it is more efficient to use native OS support if 
 * possible.
 *
 * @author Stanley Man-Kit Ho
 */
public class WSeedGenerator
{
    /**
     * Generate seed in byte array given number of bytes.
     */
    static native byte[] generateSeed(int num);
}
