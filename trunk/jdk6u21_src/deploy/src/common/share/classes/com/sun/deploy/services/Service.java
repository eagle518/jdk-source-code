/*
 * @(#)Service.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.services;

import java.security.SecureRandom;


/** 
 * Service is an interface that encapsulates the general service.
 */
public interface Service
{
    /**
     * Return cookie handler.
     */
    public com.sun.deploy.net.cookie.CookieHandler getCookieHandler();

    /**
     * Return proxy config.
     */
    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig();

    /**
     * Return system proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler();
    
    /**
     * Return auto proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler();

    /**
     * Return browser proxy handler.
     */
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler();

    /**
     * Return browser signing root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore();

    /**
     * Return browser SSL root certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore();

    /**
     * Return browser trusted signing certificate store. 
     */
    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore();

    /**
     * Return browser client authentication key store. 
     */
    public java.security.KeyStore getBrowserClientAuthKeyStore();

    /**
     * Return browser authenticator
     */
    public com.sun.deploy.security.BrowserAuthenticator getBrowserAuthenticator();
    
    /**
     * Return the Credential Manager
     */
    public com.sun.deploy.security.CredentialManager getCredentialManager();

    /**
     * Return secure random.
     */
    public SecureRandom getSecureRandom();

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer();

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape();
    
    /**
     * Return offline handler.
     */
    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler();
}



