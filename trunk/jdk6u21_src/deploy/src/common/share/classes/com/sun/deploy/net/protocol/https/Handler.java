/*
 * @(#)Handler.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	HTTP stream opener
 */
package com.sun.deploy.net.protocol.https;

import java.net.URL;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.SecureRandom;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.KeyManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.security.BrowserKeystore;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.security.CertificateHostnameVerifier;
import com.sun.deploy.security.X509DeployTrustManager;
import com.sun.deploy.security.X509ExtendedDeployTrustManager;
import java.io.IOException;

/** open an http input stream given a URL */
public class Handler extends sun.net.www.protocol.https.Handler {

    static class Initializer {

        //does nothing itself
        static void init() {}

        static {
            try {
                // Hook up TrustManager
                java.security.AccessController.doPrivileged(new PrivilegedExceptionAction() {

                    public Object run() throws Exception {
                        // see if user is allowed to use browser cert store
                        if (Config.getBooleanProperty(Config.SEC_USE_BROWSER_KEYSTORE_KEY)) {
                            // register security providers for browser keystore
                            BrowserKeystore.registerSecurityProviders();
                        }

                        // Get the platform dependent Random generator
                        final SecureRandom sr = ServiceManager.getService().getSecureRandom();

                        sr.nextInt();

                        // Get SSL context
                        SSLContext sslContext = SSLContext.getInstance("SSL");

                        // Create custom trust manager
                        TrustManager trustManager;

                        // Use this code only for JRE version < 6.0
                        if (!Config.isJavaVersionAtLeast16()) {
                            // Hostname verifier
                            HostnameVerifier verifier =
                                    new CertificateHostnameVerifier();

                            javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(verifier);

                            trustManager = new X509DeployTrustManager();
                        } else {
                            trustManager = new X509ExtendedDeployTrustManager();
                        }

                        TrustManager[] trustManagerArray = new TrustManager[1];
                        trustManagerArray[0] = trustManager;

                        // Create custom key manager
                        KeyManager keyManager = new com.sun.deploy.security.X509DeployKeyManager();

                        KeyManager[] keyManagerArray = new KeyManager[1];
                        keyManagerArray[0] = keyManager;

                        // Set custom keymanager and trust manager in SSL context
                        sslContext.init(keyManagerArray, trustManagerArray, sr);

                        javax.net.ssl.HttpsURLConnection.setDefaultSSLSocketFactory(sslContext.getSocketFactory());

                        return null;
                    }
                });
            } catch (PrivilegedActionException e) {
                e.printStackTrace();
            }
        }
    }

    protected java.net.URLConnection openConnection(URL u) throws IOException {
        Initializer.init();
        return super.openConnection(u);
    }
}
