/*
 * @(#)CredentialInfo.java              06/14/2005
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.security;

import com.sun.deploy.util.Trace;

import java.io.Externalizable;
import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;

import java.net.PasswordAuthentication;

import java.util.StringTokenizer;


/**
 *
 * This class is a container class for credential information.
 *
 * @author Ashley Woodsom
 */
public final class CredentialInfo implements Externalizable, Cloneable {
    private String userName;
    private char[] password;
    private byte[] encryptedPassword;
    private String domain;
    private long sessionId;
    private boolean isPasswordSaved;

    public CredentialInfo() {
        userName = "";
        domain = "";
        password = new char[0];
        encryptedPassword = new byte[0];
        sessionId = CredentialManager.LOGIN_SESSION_INVALID;
        isPasswordSaved = false;
    }

    /**
     * Clones the object doing a deep copy on all data members 
     *
     * @return a new CredentialInfo object
     */
    public Object clone() {
        CredentialInfo info = new CredentialInfo();
        info.userName = userName;
        info.domain = domain;
        info.sessionId = sessionId;
        info.isPasswordSaved = isPasswordSaved;
        info.password = new char[password.length];
        info.encryptedPassword = new byte[encryptedPassword.length];
        System.arraycopy(password, 0, info.password, 0, password.length);
        System.arraycopy(encryptedPassword, 0, info.encryptedPassword, 0,
            encryptedPassword.length);

        return info;
    }

    /**
     * updates the login session id of the credential
     * the session id indicates the user confirmed the use of the
     * credential during the login session specified
     * @param id the login session id the credential is valid for
     */
    public void setSessionId(long id) {
        sessionId = id;
    }

    /**
     * retrieves the last login session ID the credential was last approved in
     * @return the last valid sessionId or INVALID_SESSION_ID
     */
    public long getSessionId() {
        return sessionId;
    }

    /**
     * indicates if the password can be saved to disk
     * @return true if the password can be saved
     */
    public boolean isPasswordSaveApproved() {
        return isPasswordSaved;
    }

    /**
     * controls if a password should be persisted to disk
     * @param value of true indicates the password can be saved
     */
    public void setPasswordSaveApproval(boolean value) {
        isPasswordSaved = value;
    }

    /**
     * saves the user name in the credential.  Names can also be passed in
     * with the domain I.e. domainname\\awoodsom
     * @param name the username
     */
    public void setUserName(String name) {
        if (name == null) {
            userName = "";
	} else if (name.indexOf("\\".toString()) > -1) {
            StringTokenizer st = new StringTokenizer(name, "\\");
            domain = st.nextToken();
            userName = st.nextToken();
        } else {
            userName = name;
        }
    }

    /**
     * returns the username
     *
     * @return the username or an empty String
     */
    public String getUserName() {
        return userName;
    }

    /**
     * saves the domain name used with the credential
     *
     * @param domainName the domain name
     */
    public void setDomain(String domainName) {
        if (domainName == null) {
            domain = "";
        } else {
            domain = domainName;
        }
    }

    /**
     * retrieves the domain name
     *
     * @return the domain name or and empty string
     */
    public String getDomain() {
        return domain;
    }

    /**
     * Saves the password.  This method makes a copy of the password
     * passed into it.
     * @param pass the password to be saved
     */
    public void setPassword(char[] pass) {
        if (pass == null) {
            pass = new char[0];
            setEncryptedPassword(null);
        }

        password = new char[pass.length];
        System.arraycopy(pass, 0, password, 0, password.length);
    }

    /**
     * retrives the credential's password
     *
     * @return a copy of the saved password or an array of size zero
     */
    public char[] getPassword() {
        char[] pass = new char[password.length];
        System.arraycopy(password, 0, pass, 0, password.length);

        return pass;
    }

    /**
     * This method will save the Credential information to an output stream
     * @param out the ObjectStream to save the object to
     * @throws java.io.IOException an IO error
     */
    public void writeExternal(ObjectOutput out) throws IOException {
        try {
            // Write the object out to the stream
            out.writeObject(userName);
            out.writeLong(sessionId);
            out.writeObject(domain);
            out.writeInt(encryptedPassword.length);

            for (int i = 0; i < encryptedPassword.length; i++) {
                out.writeByte(encryptedPassword[i]);
            }
        } catch (Exception e) {
            Trace.securityPrintException(e);
        }
    }

    /** determins if a credential is empty
     *
     * @return true if credential is empty
     */
    public boolean isCredentialEmpty() {
        return !((userName.length() > 0) || (domain.length() > 0));
    }

    /**
     * This method will restore the Credential information from an input stream
     * @param in the stream to read the saved object data from
     * @throws java.io.IOException An io error
     */
    public void readExternal(ObjectInput in)
        throws IOException, ClassNotFoundException {
        try {
            // Read the object in from the stream
            userName = (String) in.readObject();
            sessionId = in.readLong();
            domain = (String) in.readObject();
            encryptedPassword = new byte[in.readInt()];

            for (int i = 0; i < encryptedPassword.length; i++) {
                encryptedPassword[i] = in.readByte();
            }
        } catch (Exception e) {
            Trace.securityPrintException(e);
        }
    }

    /**
     * retrieves the encrypted password
     *
     * @return the encrypted password buffer or a buffer of size zero
     */
    protected byte[] getEncryptedPassword() {
        byte[] pass = new byte[encryptedPassword.length];
        System.arraycopy(encryptedPassword, 0, pass, 0, encryptedPassword.length);

        return pass;
    }

    /**
     * Saves a copy of the encrypted password for the credential.
     *
     * @param buffer the encrypted password
     */
    protected void setEncryptedPassword(byte[] buffer) {
        if (buffer == null) {
            buffer = new byte[0];
        }

        encryptedPassword = new byte[buffer.length];
        System.arraycopy(buffer, 0, encryptedPassword, 0, buffer.length);
    }

    /**
     * This is a convience method that converts a PasswordAunthentication object
     * into a CredentialInfo object
     * @param pa the PasswordAuthentication to convert
     * @return a CredentialInfo object
     */
    public static CredentialInfo passAuthToCredentialInfo(
        PasswordAuthentication pa) {
        CredentialInfo info = new CredentialInfo();

        if (pa != null) {
            // Check to see if there is a domain with the user name
            // if there is parse it apart
            if (pa.getUserName().contains("\\")) {
                StringTokenizer st = new StringTokenizer(pa.getUserName(), "\\");
                info.domain = st.nextToken();
                info.userName = st.nextToken();
            } else {
                info.userName = pa.getUserName();
            }

            info.password = pa.getPassword();
        }

        return info;
    }

    /**
     * This is a convience method that converts a PasswordAunthentication object
     * into a CredentialInfo object
     * @return a password authentication object
     */
    public PasswordAuthentication getPasswordAuthentication() {
        String name = userName;

        // If there is a domain, format the name as domain//name
        if ((domain != null) && !domain.trim().equals("")) {
            name = domain + '\\' + userName;
        }

        return new PasswordAuthentication(name, password);
    }
}
