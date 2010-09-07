/*
 * @(#)MSCredentialManager.java              05/17/2005
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.security;

import com.sun.deploy.util.Trace;


/**
 * This class is the Microsoft extention of the CredentialManager class
 * It supports the secure encryption of user password credentials using the
 * Microsoft Encryption API.  It also supports retrival of the login session id.
 *
 * @author Ashley Woodsom
 */
public final class MSCredentialManager extends CredentialManager {
    private static boolean isEncryptionSupported = false;
    private static long sessionId = LOGIN_SESSION_INVALID;

    static {
        // Determine if this is a Windows 98/ME/NT platform.  If it is make
        // no native calls, otherwise initialize members
        String osName = System.getProperty("os.name").toLowerCase();

        if (!(((osName.indexOf("windows") != -1) &&
                (osName.indexOf("98") != -1)) || (osName.indexOf("me") != -1) ||
                (osName.indexOf("nt") != -1))) {
            // Make the native calls
            isEncryptionSupported = isEncryptionAvailable();
            sessionId = getLoginUID();
        }
    }

    /** Creates a new instance of MSCredentialManager */
    protected MSCredentialManager() {
        super();
    }

    /**
     * get the one and only instance of the MSCredentialManager
     * @ return a CredentialManager
     */
    public static synchronized CredentialManager getInstance() {
        if (instance == null) {
            instance = new MSCredentialManager();
        }

        return instance;
    }

    /**
     * This method indicates the availability of a cryptography
     * service for storing passwords.
     *
     * @return true if password persistance is available
     */
    protected boolean isPasswordEncryptionSupported() {
        return isEncryptionSupported;
    }

    /**
     * Encrypts a password
     * @param pass the password to encrypt
     * @return an encrypted password or and empty byte array on failure
     */
    protected byte[] encryptPassword(char[] pass) {
        byte[] buffer = new byte[0];

        try {
            if (pass.length > 0) {
                buffer = encryptMSPassword(pass);
            }
        } catch (Exception e) {
            // Problem with Encryption
            Trace.securityPrintException(e);
        }

        return buffer;
    }

    /**
     * Gets the login session id for the current user.  This method
     * is only available on Windows 2000 and newer platforms
     * @ return the login session ID or LOGIN_SESSION_INVALID
     */
    protected long getLoginSessionId() {
        return sessionId;
    }

    /**
     * decryptes an encrypted password
     * @param pass The password to be decrypted
     * @return the decrypted password or an array of size 0 if there is
     * an error during decryption
     */
    protected char[] decryptPassword(byte[] pass) {
        char[] result = null;

        try {
            result = decryptMSPassword(pass);
        } catch (Exception e) {
            Trace.securityPrintException(e);
        }

        return result;
    }

    /**
     * Encrypts a password using the user's session credentials as a key
     *
     * @return a byte array containing the encrypted password or an array of
     * size zero
     */
    private static native byte[] encryptMSPassword(char[] pass);

    /**
     * Decrypts an encrypted password
     *
     * @param inputBuff the encrypted password byte buffer
     * @return a char array with the password, or an array of size 0
     */
    private static native char[] decryptMSPassword(byte[] inputBuff);

    /**
     * Determins if the Windows Encryption APIs are available on a version
     * of Windows
     * @return true if functionality is available
     */
    private static native boolean isEncryptionAvailable();

    /**
     * Determins the unique login session ID.  This Id only changes when
     * a user logs off and logs back onto the system.  Suspend or fast
     * login switching does not change the login ID
     *
     * @return The login session Id or zero on failure
     */
    private static native long getLoginUID();
}
