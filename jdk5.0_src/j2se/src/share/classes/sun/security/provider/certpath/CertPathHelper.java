/*
 * @(#)CertPathHelper.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider.certpath;

import java.lang.reflect.*;
import java.io.*;
import java.util.*;

import java.security.*;
import java.security.cert.*;
import javax.security.auth.x500.X500Principal;

import sun.security.x509.*;

/**
 * Helper class that allows access to Sun specific known-public methods in the
 * java.security.cert package. It relies on a subclass in the
 * java.security.cert packages that is initialized before any of these methods
 * are called (achieved via static initializers).
 *
 * The methods are made available in this fashion for performance reasons.
 * They are scheduled to be made public in a future release, in which case
 * this class will become obsolete.
 *
 * @author Andreas Sterbenz
 * @version 1.4, 12/19/03
 */
public abstract class CertPathHelper {
    
    /**
     * Object used to tunnel the calls. Initialized by CertPathHelperImpl.
     */
    protected static CertPathHelper instance;
    
    protected CertPathHelper() {
	// empty
    }
    
    protected abstract void implSetSubject(X509CertSelector sel, X500Principal subject);

    protected abstract X500Principal implGetSubject(X509CertSelector sel);
    
    protected abstract void implSetIssuer(X509CertSelector sel, X500Principal issuer);

    protected abstract X500Principal implGetIssuer(X509CertSelector sel);
    
    protected abstract X500Principal implGetCA(TrustAnchor anchor);
    
    protected abstract void implSetPathToNames(X509CertSelector sel, 
	    Set<GeneralNameInterface> names);
    
    protected abstract void implAddIssuer(X509CRLSelector sel, X500Principal name);
    
    protected abstract Collection<X500Principal> implGetIssuers(X509CRLSelector sel);
    
    static void setSubject(X509CertSelector sel, X500Principal subject) {
	instance.implSetSubject(sel, subject);
    }

    static X500Principal getSubject(X509CertSelector sel) {
	return instance.implGetSubject(sel);
    }
    
    static void setIssuer(X509CertSelector sel, X500Principal issuer) {
	instance.implSetIssuer(sel, issuer);
    }

    static X500Principal getIssuer(X509CertSelector sel) {
	return instance.implGetIssuer(sel);
    }
    
    static X500Principal getCA(TrustAnchor anchor) {
	return instance.implGetCA(anchor);
    }

    static void setPathToNames(X509CertSelector sel, 
	    Set<GeneralNameInterface> names) {
	instance.implSetPathToNames(sel, names);
    }
    
    static void addIssuer(X509CRLSelector sel, X500Principal name) {
	instance.implAddIssuer(sel, name);
    }
    
    static Collection<X500Principal> getIssuers(X509CRLSelector sel) {
	return instance.implGetIssuers(sel);
    }
    
}

