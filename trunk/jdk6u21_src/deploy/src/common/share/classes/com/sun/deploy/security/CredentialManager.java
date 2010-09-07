/*
 * @(#)CredentialManager.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.security;

import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;

import sun.net.www.protocol.http.AuthCacheBridge;
import sun.net.www.protocol.http.AuthCacheImpl;
import sun.net.www.protocol.http.AuthCacheValue;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.EOFException;
import java.io.Externalizable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;

import java.security.AccessController;
import java.security.PrivilegedExceptionAction;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;


/**
 * This class finds user credientals for a given connection.  It can be used
 * as a convenient way for remembering usernames to autofill them in for the user
 * It can also be used to retrieve persisted passwords to perform auto
 * login for users to well known sites.
 *
 * @author Ashley Woodsom
 */
public class CredentialManager {
    public static final long LOGIN_SESSION_INVALID = -1;
    protected static CredentialManager instance = null;
    private CredentialCache credCache = new CredentialCache();
    private CredentialPersistor persistor = new CredentialPersistor();
    private Map serverMap;

    /**
     * singleton, protected constructor
     */
    protected CredentialManager() {
        serverMap = persistor.getAllPersistedCredentials();

        // Take the credentials we read from the file and write them
        // back out if there are any duplicates, this will overwrite any
        // duplicates and leave only the most recent credentials
        if (persistor.getSavedCredentialCount() > serverMap.size()) {
            persistor.persistAllCredentials(serverMap);
        }

        // Register when the singleton is created
        AuthCacheValue.setAuthCache(credCache);
    }

    /**
     * get the one and only instance of the CredentialManager
     * @return the CredentialManager
     */
    public static synchronized CredentialManager getInstance() {
        if (instance == null) {
            instance = new CredentialManager();
        }

        return instance;
    }

    /**
     * Gets the login session id for the current user
     * @return the login session ID or LOGIN_SESSION_INVALID
     */
    protected long getLoginSessionId() {
        // Not available in base class
        return LOGIN_SESSION_INVALID;
    }

    /**
     * This method indicates the availability of a cryptography
     * service for storing passwords.  An implementation on a platform
     * that can support encryption should override this method
     *
     * @return true if password persistance is available
     */
    protected boolean isPasswordEncryptionSupported() {
        // not available in base class
        return false;
    }

    /**
     * Saves a password for a given site key.
     * @param pass the password to encrypt
     * @return the encrypted password or a zero size array
     */
    protected byte[] encryptPassword(char[] pass) {
        // Base class can't encrypt passwords
        return new byte[0];
    }

    /**
     * decryptes a password
     * @param pass The password to be decrypted
     * @return the decrypted password or an array of size 0
     */
    protected char[] decryptPassword(byte[] pass) {
        // Base class always returns an empty array
        return new char[0];
    }

    /**
     * saves a copy of the credential in memory and persists it to disc.
     * The password will be saved only if the password save flag was set
     * in the CredentialInfo object.
     * @param key the server connection the Crenential is for
     * @param info credential to be saved
     */
    public void saveCredential(AuthKey key, CredentialInfo info) {
        // Update the login session ID for this credential
        info.setSessionId(getLoginSessionId());

        CredentialInfo cred = (CredentialInfo) info.clone();

        // Encrypt the password or delete it if the credential
        // shouldn't be persisted or can't be encrypted
        if (isPasswordEncryptionSupported() && cred.isPasswordSaveApproved()) {
            cred.setEncryptedPassword(encryptPassword(info.getPassword()));
        } else {
            // If platform can't encrypt passwords then remove it
            cred.setPassword(null);
        }

        String keyId = buildConnectionKey(key);

        // Save the credential in our local map
        serverMap.put(keyId, cred);

        // Save the credential to permanant storage
        persistor.persistCredential(keyId);
    }

    /**
     * Determins if the credential is valid to use for authentication.
     * to be considered valid it must have a username, password, and must
     * have a current login session id indicating the user has agreed to
     * use it during the current login session.
     *
     * @param info the Crenential to be checked
     * @return true if the credential is valid for the current login
     * session
     */
    public boolean isCredentialValid(CredentialInfo info) {
        boolean result = false;

        // Are Username and Password filled in?
        if ((info.getUserName().length() > 0) &&
                (info.getPassword().length > 0)) {
            // Is the Login session ID current
            if ((info.getSessionId() != LOGIN_SESSION_INVALID) &&
                    (info.getSessionId() == getLoginSessionId())) {
                result = true;
            }
        }

        return result;
    }

    /**
     * Removes the saved credentials from persistant storage
     */
    public static void removePersistantCredentials() {
        try {
            File f = new File(Config.getUserAuthFile());

            // Remove file
            if (f.delete() == false) {
                f.deleteOnExit();
            }
        } catch (Exception e) {
            Trace.securityPrintException(e);
        }
    }

    /**
     * Clears the password for a given credential
     *
     * @param key the connection to be cleared
     */
    public void clearCredentialPassword(AuthKey key) {
        String keyId = buildConnectionKey(key);
        CredentialInfo info = findServerCredential(keyId);

        // If the credential exists clear the password
        if (info != null) {
            info.setPassword(null);
            saveCredential(key, info);
        }

        persistor.persistCredential(keyId);
    }

    /**
     * Gets a credential with information the user has previously used
     * for authentication for a given server
     *
     * @param key the server connection to get the credential for
     *
     * @return the Crential for the given server or an empty credential
     * if no previous information was found
     */
    protected CredentialInfo getCredential(AuthKey key) {
        String keyId = buildConnectionKey(key);
        CredentialInfo info = (CredentialInfo) serverMap.get(keyId);

        // If there is no existing credential for the exact server path
        // see if there are any stored credentials for the same server
        // as they usually will use the same credentials.
        if ((info == null) || ((info != null) && info.isCredentialEmpty())) {
            // Try to find a credential from the same server
            info = findServerCredential(keyId);

            if (info != null) {
                // This credential hasn't been used for the exact site
                // before so force the user to acknowlege its use
                info.setSessionId(LOGIN_SESSION_INVALID);
            } else {
                // Make sure a valid credential was found, otherwise create one
                info = new CredentialInfo();
            }
        }

        // If there is a password make sure it is decrypted before returning
        if (info.getPassword().length == 0) {
            byte[] buff = info.getEncryptedPassword();

            if (buff.length > 0) {
                info.setPassword(decryptPassword(buff));
            }
        }

        return info;
    }

    /**
     * Finds a credential that has been used with the given
     * server.  The method should be used when an exact credential
     * match can't be found.
     *
     * @param server the server the credential is needed for
     * @return a Credential that has previously been used for the server, or
     * null if no information for the server was found
     */
    private CredentialInfo findServerCredential(String server) {
        CredentialInfo info = null;
        Set set = serverMap.keySet();
        Iterator iter = set.iterator();

        // loop until there are no more entries or a match is found with a
        // password
        while (iter.hasNext() &&
                (info == null || info.getEncryptedPassword().length == 0)) {
            String key = (String) iter.next();

            // See if the credential is for the server
            if (getServerFromKey(server).equals(getServerFromKey(key))) {
                CredentialInfo cred = (CredentialInfo) serverMap.get(key);

                // If the credential isn't empty use it
                if (!cred.isCredentialEmpty()) {
                    info = (CredentialInfo) serverMap.get(key);
                }
            }
        }

        return info;
    }

    /**
     * Returns the server element from the connection key
     *
     * @param the key produced by the buildConnectionKey method
     * @return the server name or null
     */
    private static String getServerFromKey(String conKey) {
        StringTokenizer tokenizer = new StringTokenizer(conKey, ":");

        return tokenizer.nextToken();
    }

    /*
     * Builds a connection key for the server connection being authenticated
     *
     * @param con the server connection to build a key for
     * @return a key to use to identify the connection
     * The key will be in the form.. "server:protocol:host:port:path"
     *
     */

    /**
     * builds a connection key for mapping connections to credentials
     * @return A key for the connection
     * @param con The connection to build the key for
     */
    public static String buildConnectionKey(AuthKey con) {
        StringBuffer buffer = new StringBuffer();

        if (con.isProxy()) {
            buffer.append("p:");
        } else {
            buffer.append("s:");
        }

        buffer.append(con.getProtocolScheme());
        buffer.append(':');
        buffer.append(con.getHost());
        buffer.append(':');
        buffer.append(con.getPort());
        buffer.append(':');
        buffer.append(con.getPath());

        // Convert to all lowercase
        return buffer.toString().toLowerCase();
    }

    /**
     * The CredentialCache class extents the AuthCacheImpl
     * It listens for items being removed from the cache.  When an item
     * is removed it records it so the CredentialManager will know not
     * to use it then next time somone needs a credential for the same site.
     */
    private class CredentialCache extends AuthCacheImpl {
        HashMap map = new HashMap();

        public CredentialCache() {
            setMap(map);
        }

        /**
         * @param server the name of the server
         * value - a private item containing infomration about the path
         *         on the server the credential is being removed for
         */
        public void remove(String server, AuthCacheValue value) {
            try {
                super.remove(server, value);

                // Remove the credential from the credential manager as it
                // is being removed from the AuthCache
                AuthCacheBridge cacheValue = new AuthCacheBridge(value);
                CredentialManager.getInstance().clearCredentialPassword(cacheValue);
            } catch (Exception e) {
                Trace.securityPrintException(e);
            }
        }
    }

    /**
     * The credential persistor is responsible for reading and writing
     * the CrendentialManager's data to persistant storage
     */
    private class CredentialPersistor {
        private int credentialCount = 0;

        public CredentialPersistor() {
        }

        /**
         * Gets the number of saved credentials that were read from the file
         * @return the number of credentials in the file
         */
        private int getSavedCredentialCount() {
            return credentialCount;
        }

        /**
         * Saves a credential associated with a site Key to persistant storage
         *
         * @param siteKey the key to associate with the credential
         */
        private synchronized void persistCredential(String siteKey) {
            ObjectOutputStream outStream = null;

            try {
                CredentialInfo info = (CredentialInfo) serverMap.get(siteKey);

                if (info != null) {
                    OutputStream fout = openOutputFile(true);
                    outStream = new ObjectOutputStream(fout);

                    // Write the Key then the Credential
                    outStream.writeObject(siteKey);
                    info.writeExternal(outStream);

                    // Close the streams
                    outStream.flush();
                    outStream.close();
                    fout.flush();
                    fout.close();
                }
            } catch (Exception e) {
                Trace.securityPrintException(e);
            }
        }

        /**
         * removes the saved credential file
         */
        private synchronized void deleteCredentials() {
            try {
                File f = new File(Config.getUserAuthFile());

                // If delete fails, delete when the app exits
                if (f.delete() == false) {
                    f.deleteOnExit();
                }
            } catch (Exception e) {
                Trace.securityPrintException(e);
            }
        }

        /**
         * Takes each key/credential in the map and writes it to
         * persistant storage
         * @ param map the list of credentials to be savd
         */
        private synchronized void persistAllCredentials(Map map) {
            ObjectOutputStream outStream = null;
            OutputStream fout = null;

            try {
                // Overwrite the current file
                fout = openOutputFile(false);

                Set set = map.keySet();
                Iterator iter = set.iterator();

                while (iter.hasNext()) {
                    // This is putting a new Object Stream Header in front of
                    // each credential in the file.  Each saved credential has
                    // a stream header, server key, and the credential info
                    outStream = new ObjectOutputStream(fout);

                    String key = (String) iter.next();
                    CredentialInfo info = (CredentialInfo) map.get(key);

                    // Write the Key then the Credential
                    outStream.writeObject(key);
                    info.writeExternal(outStream);
                    outStream.flush();
                }
            } catch (Throwable e) {
                Trace.securityPrintException(e);
            } finally {
                try {
                    // Flush and close the output streams
                    if (outStream != null) {
                        outStream.flush();
                    }

                    fout.flush();
                    fout.close();
                } catch (Exception e) {
                    Trace.securityPrintException(e);
                }
            }
        }

        /**
         * Retrives a persisted credential and saves it into memory with the
         * associated site Key.
         *
         * @param in the input stream to read the credentials from
         * @param siteKey the key to associate the read in credentials with
         */
        private synchronized void getPersistedCredential(ObjectInputStream in,
            String siteKey) {
            try {
                CredentialInfo info = new CredentialInfo();
                info.readExternal(in);

                // Add the credential to the  CredentialManager's map
                serverMap.put(siteKey, info);
            } catch (Exception e) {
                Trace.securityPrintException(e);
            }
        }

        /**
         * Opens the data data file, if file doesn't exist it will be created
         *
         * @return a FileInputStream
         */
        private synchronized InputStream openInputStream() {
            InputStream instream = null;

            try {
                final File f = new File(Config.getUserAuthFile());
                instream =
                    (InputStream) AccessController.doPrivileged(
                        new PrivilegedExceptionAction() {
                            public Object run() throws IOException {
                                if (!f.exists()) {
                                    f.getParentFile().mkdirs();
                                    f.createNewFile();
                                }

                                return new BufferedInputStream(
                                        new FileInputStream(f));
                            }
                        });
            } catch (Exception e) {
                // log the error
                Trace.securityPrintException(e);
            }

            return instream;
        }

        /**
         * Opens the persistance file or creates it if it is missing.
         *
         * @param append true to append to the file, false will overwrite
         * @return an ObjectOutputStream backed by the persistance file
         */
        private synchronized OutputStream openOutputFile(final boolean append) {
            OutputStream out = null;

            try {
                final File f = new File(Config.getUserAuthFile());

                out = (OutputStream) AccessController.doPrivileged(new PrivilegedExceptionAction() {
                            public Object run() throws IOException {

                                // If the file doesn't exists create it, 
                                // otherwise read in the data
                                if (!f.exists()) {
                                    f.getParentFile().mkdirs();
                                    f.createNewFile();
                                }

                                // Open the output stream and set the 
                                // append flag from client
                                return new BufferedOutputStream(new FileOutputStream(
                                            f, append));
                            }
                        });
            } catch (Exception e) {
                Trace.securityPrintException(e);
            }

            return out;
        }

        /**
         * Loads all persisted credentials into memory from the data file
         * located in the user's java.home.security directory
         * @return a map containing the credentials
         */
        private synchronized Map getAllPersistedCredentials() {
            ObjectInputStream in = null;
            InputStream finstream = null;
            Map map = null;

            try {
                map = new HashMap();
                finstream = openInputStream();
                in = new ObjectInputStream(finstream);

                // While the stream has objects pull the credentials out
                // until the end of the stream is reached
                while (in != null) {
                    String siteKey = (String) in.readObject();
                    CredentialInfo info = new CredentialInfo();
                    info.readExternal(in);
                    map.put(siteKey, info);
                    ++credentialCount;

                    // Each Credential is saved with an object stream header
                    // Each credential read from the file must be read in
                    // with a seperate ObjectStream header
                    in = new ObjectInputStream(finstream);
                }

                finstream.close();
            } catch (java.io.EOFException e) {
                // End Of file no problem
            } catch (Exception e) {
                Trace.securityPrintException(e);

                try {
                    finstream.close();

                    // Write out the files that were read before the corrupt stream
                    // was encountered
                    if (credentialCount > 0) {
                        persistAllCredentials(map);
                    }
                } catch (Exception ex) {
                    Trace.securityPrintException(e);
                }
            }

            return map;
        }
    }
}
