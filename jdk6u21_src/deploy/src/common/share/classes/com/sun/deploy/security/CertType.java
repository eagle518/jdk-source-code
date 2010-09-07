/*
 * @(#)CertType.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * This class define the certType for the source
 * of client certificate, which will be either
 * from browser keystore or Java keystore
 *
 * @version 1.0
 * @author Dennis Gu
 */

package com.sun.deploy.security;

public class CertType
{
   private String type = null;

   CertType(String type) {
        this.type = type;
   }

   public String getType() {
	return type;
   }

   public static final CertType BROWSER = new CertType("browser");
   public static final CertType PLUGIN = new CertType("plugins");
}

