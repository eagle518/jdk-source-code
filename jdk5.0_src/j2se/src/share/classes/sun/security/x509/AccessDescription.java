/*
 * @(#)AccessDescription.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.security.cert.*;
import java.io.IOException;

import sun.security.util.*;

/**
 * @version 	1.2 12/19/03
 * @author	Ram Marti
 */

public final class AccessDescription {

    private int myhash = -1;

    private ObjectIdentifier accessMethod;

    private GeneralName accessLocation;

    public static final ObjectIdentifier Ad_OCSP_Id;
    static {
	ObjectIdentifier oid = null;
	try {
	    oid = new ObjectIdentifier("1.3.6.1.5.5.7.48.1");
	} catch (IOException ioe) {
	    // should not happen
	}
	Ad_OCSP_Id = oid;
    }
    
    public AccessDescription(DerValue derValue) throws IOException,
    CertificateParsingException {
	DerInputStream derIn = derValue.getData();
	/*
	  if (derValue.getTag() != DerValue.tag_Sequence) {
	  throw new CertificateParsingException("Missing " +
	  "sequence tag in AuthorityInformationAccessExtension");
	  }
	*/
	
	accessMethod = derIn.getOID();
	DerValue subDerValue = derIn.getDerValue();
	accessLocation = new GeneralName(subDerValue);
    }
    
    public ObjectIdentifier getAccessMethod() {
	return accessMethod;
    }
    
    public GeneralName getAccessLocation() {
	return accessLocation;
    }
    
    public int hashCode() {
        if (myhash == -1) {
            myhash = accessMethod.hashCode() + accessLocation.hashCode();
        }
        return myhash;
    }

    public boolean equals(Object obj) {
        if (obj == null || (!(obj instanceof AccessDescription))) {
            return false;
        }
        AccessDescription that = (AccessDescription)obj;

        if (this == that) {
            return true;
        }
        return (accessMethod.equals(that.getAccessMethod()) &&
            accessLocation.equals(that.getAccessLocation()));
    }
    
    public String toString() {
	return ("AccessMethod = " + accessMethod.toString() + 
		" accessLocation " + accessLocation.toString());
    }
    
}
