/*
 * @(#)SystemIdentity.java	1.28 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider;

import java.io.Serializable;
import java.util.Enumeration;
import java.security.*;

/**
 * An identity with a very simple trust mechanism.
 *
 * @version 1.28, 12/19/03
 * @author 	Benjamin Renaud
 */

public class SystemIdentity extends Identity implements Serializable {

    /** use serialVersionUID from JDK 1.1. for interoperability */
    private static final long serialVersionUID = 9060648952088498478L;

    /* This should be changed to ACL */
    boolean trusted = false;

    /* Free form additional information about this identity. */
    private String info;

    public SystemIdentity(String name, IdentityScope scope)
    throws InvalidParameterException, KeyManagementException {
	super(name, scope);
    }

    /**
     * Is this identity trusted by sun.* facilities?
     */
    public boolean isTrusted() {
	return trusted;
    }

    /**
     * Set the trust status of this identity.
     */
    protected void setTrusted(boolean trusted) {
	this.trusted = trusted;
    }

    void setIdentityInfo(String info) {
	super.setInfo(info);
    }

    String getIndentityInfo() {
	return super.getInfo();
    }

    /**
     * Call back method into a protected method for package friends.
     */
    void setIdentityPublicKey(PublicKey key) throws KeyManagementException {
	setPublicKey(key);
    }

    /**
     * Call back method into a protected method for package friends.
     */
    void addIdentityCertificate(Certificate cert)
    throws KeyManagementException {
	addCertificate(cert);
    }

    void clearCertificates() throws KeyManagementException {
	Certificate[] certs = certificates();
	for (int i = 0; i < certs.length; i++) {
	    removeCertificate(certs[i]);
	}
    }

    public String toString() {
	String trustedString = "not trusted";
	if (trusted) {
	    trustedString = "trusted";
	}
	return super.toString() + "[" + trustedString + "]";
    }


}
