/*
 * @(#)SunMSCAPI.java	1.7 10/03/23
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.security.mscapi;

import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.Provider;
import java.security.ProviderException;
import java.util.HashMap;
import java.util.Map;

import sun.security.action.PutAllAction;


/**
 * A Cryptographic Service Provider for the Microsoft Crypto API.
 *
 * @since 1.6
 */

public final class SunMSCAPI extends Provider {

    private static final long serialVersionUID = 8622598936488630849L; //TODO

    private static final String INFO = "Sun's Microsoft Crypto API provider";

    static {
	AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		System.loadLibrary("sunmscapi");
		return null;
	    }
	});
    }

    public SunMSCAPI() {
	super("SunMSCAPI", 1.6d, INFO);

	// if there is no security manager installed, put directly into
	// the provider. Otherwise, create a temporary map and use a
	// doPrivileged() call at the end to transfer the contents
	final Map map = (System.getSecurityManager() == null)
			? (Map)this : new HashMap();

	/*
	 * Secure random
	 */
	map.put("SecureRandom.Windows-PRNG", "sun.security.mscapi.PRNG");

        /*
         * Key store
         */
	map.put("KeyStore.Windows-MY", "sun.security.mscapi.KeyStore$MY");
	map.put("KeyStore.Windows-ROOT", "sun.security.mscapi.KeyStore$ROOT");

	/*
	 * Signature engines
	 */
	map.put("Signature.SHA1withRSA", 
	    "sun.security.mscapi.RSASignature$SHA1");
        map.put("Signature.SHA256withRSA",
            "sun.security.mscapi.RSASignature$SHA256");
        map.put("Signature.SHA384withRSA",
            "sun.security.mscapi.RSASignature$SHA384");
        map.put("Signature.SHA512withRSA",
            "sun.security.mscapi.RSASignature$SHA512");
	map.put("Signature.MD5withRSA", 
	    "sun.security.mscapi.RSASignature$MD5");
	map.put("Signature.MD2withRSA", 
	    "sun.security.mscapi.RSASignature$MD2");


	// supported key classes
	map.put("Signature.SHA1withRSA SupportedKeyClasses",
	    "sun.security.mscapi.Key");
	map.put("Signature.SHA256withRSA SupportedKeyClasses",
	    "sun.security.mscapi.Key");
	map.put("Signature.SHA384withRSA SupportedKeyClasses",
	    "sun.security.mscapi.Key");
	map.put("Signature.SHA512withRSA SupportedKeyClasses",
	    "sun.security.mscapi.Key");
	map.put("Signature.MD5withRSA SupportedKeyClasses",
	    "sun.security.mscapi.Key");
	map.put("Signature.MD2withRSA SupportedKeyClasses",
	    "sun.security.mscapi.Key");

	/*
	 * Key Pair Generator engines
	 */
	map.put("KeyPairGenerator.RSA", 
	    "sun.security.mscapi.RSAKeyPairGenerator");
	map.put("KeyPairGenerator.RSA KeySize", "1024");

	/*
	 * Cipher engines
	 */
	map.put("Cipher.RSA", "sun.security.mscapi.RSACipher");
	map.put("Cipher.RSA/ECB/PKCS1Padding", "sun.security.mscapi.RSACipher");
	map.put("Cipher.RSA SupportedModes", "ECB");
	map.put("Cipher.RSA SupportedPaddings", "PKCS1PADDING");
	map.put("Cipher.RSA SupportedKeyClasses", "sun.security.mscapi.Key");


	if (map != this) {
	    AccessController.doPrivileged(new PutAllAction(this, map));
	}
    }

    // set to true once self verification is complete
    private static volatile boolean integrityVerified;

    static void verifySelfIntegrity(Class c) {
	if (integrityVerified) {
	    return;
	}
	doVerifySelfIntegrity(c);
    }

    private static synchronized void doVerifySelfIntegrity(Class c) {
/* RSA CERTIFICATE USED TO SIGN SUNMSCAPI.JAR
Owner: CN=Sun Microsystems Inc, OU=Java Software Code Signing,
O=Sun Microsystems Inc
Issuer: CN=JCE Code Signing CA, OU=Java Software Code Signing,
O=Sun Microsystems Inc, L=Palo Alto, ST=CA, C=US
Serial number: 21f
Valid from: Wed Nov 23 14:49:41 PST 2005 until: Sat Nov 27 14:49:41 PST 2010
Certificate fingerprints:
         MD5:  8E:42:68:F7:22:2F:57:F3:F0:F0:19:CE:AE:F3:8F:60
         SHA1: CD:3E:0C:8A:32:E3:EF:40:21:C8:5B:34:98:9A:66:CF:E1:60:25:48
*/
	final String CERT =
"-----BEGIN CERTIFICATE-----\n" +
"MIICnTCCAlugAwIBAgICAh8wCwYHKoZIzjgEAwUAMIGQMQswCQYDVQQGEwJVUzEL" +
"MAkGA1UECBMCQ0ExEjAQBgNVBAcTCVBhbG8gQWx0bzEdMBsGA1UEChMUU3VuIE1p" +
"Y3Jvc3lzdGVtcyBJbmMxIzAhBgNVBAsTGkphdmEgU29mdHdhcmUgQ29kZSBTaWdu" +
"aW5nMRwwGgYDVQQDExNKQ0UgQ29kZSBTaWduaW5nIENBMB4XDTA1MTEyMzIyNDk0" +
"MVoXDTEwMTEyNzIyNDk0MVowYzEdMBsGA1UEChMUU3VuIE1pY3Jvc3lzdGVtcyBJ" +
"bmMxIzAhBgNVBAsTGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMR0wGwYDVQQD" +
"ExRTdW4gTWljcm9zeXN0ZW1zIEluYzCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkC" +
"gYEA16bKo6tC3OHFDNfPXLKXMCMtIyeubNnsEtlvrH34HhfF+ZmpSliLCvQ15ms7" +
"05vy4XgZUbZ3mgSOlLRMAGRo6596ePhc+0Z6yeKhbb3LZ8iz97ZIptkHGOshj9cf" +
"cSRPYmorUug9OsybMdIfQXazxT9mZJ9Yx5IDw6xak7kVbpUCAwEAAaOBiDCBhTAR" +
"BglghkgBhvhCAQEEBAMCBBAwDgYDVR0PAQH/BAQDAgXgMB0GA1UdDgQWBBRI319j" +
"Cbhc9DWJVltXgfrMybHNjzAfBgNVHSMEGDAWgBRl4vSGydNO8JFOWKJq9dh4WprB" +
"pjAgBgNVHREEGTAXgRV5dS1jaGluZy5wZW5nQHN1bi5jb20wCwYHKoZIzjgEAwUA" +
"Ay8AMCwCFFBFmED9s3OoN9rbXfQV3+brJPW/AhQr+Wq1MlubAvnfjrlqeksh0QaD" +
"AQ==" +
"\n-----END CERTIFICATE-----";
	integrityVerified = JarVerifierImpl.doVerification(c, CERT);
	if (integrityVerified == false) {
	    throw new ProviderException
	    	("The SunMSCAPI provider may have been tampered with.");
	}
    }
}
