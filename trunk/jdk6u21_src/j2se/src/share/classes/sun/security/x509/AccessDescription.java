/*
 * @(#)AccessDescription.java	1.8 10/03/23
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.security.cert.*;
import java.io.IOException;

import sun.security.util.*;

/**
 * @version 	1.8 03/23/10
 * @author	Ram Marti
 */

public final class AccessDescription {

    private int myhash = -1;

    private ObjectIdentifier accessMethod;

    private GeneralName accessLocation;

    public static final ObjectIdentifier Ad_OCSP_Id = 
        ObjectIdentifier.newInternal(new int[] {1, 3, 6, 1, 5, 5, 7, 48, 1});

    public static final ObjectIdentifier Ad_CAISSUERS_Id = 
        ObjectIdentifier.newInternal(new int[] {1, 3, 6, 1, 5, 5, 7, 48, 2});
    
    public AccessDescription(DerValue derValue) throws IOException {
	DerInputStream derIn = derValue.getData();
	accessMethod = derIn.getOID();
	accessLocation = new GeneralName(derIn.getDerValue());
    }
    
    public ObjectIdentifier getAccessMethod() {
	return accessMethod;
    }
    
    public GeneralName getAccessLocation() {
	return accessLocation;
    }

    public void encode(DerOutputStream out) throws IOException {
        DerOutputStream tmp = new DerOutputStream();
	tmp.putOID(accessMethod);
	accessLocation.encode(tmp);
        out.write(DerValue.tag_Sequence, tmp);
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
	return ("\n   accessMethod: " + accessMethod.toString() + 
		"\n   accessLocation: " + accessLocation.toString());
    }
}
