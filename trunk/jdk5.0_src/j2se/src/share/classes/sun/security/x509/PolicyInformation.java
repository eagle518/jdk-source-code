/*
 * @(#)PolicyInformation.java	1.7 04/04/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.io.IOException;
import java.security.cert.PolicyQualifierInfo;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

import sun.security.util.DerValue;
import sun.security.util.DerOutputStream;
/**
 * PolicyInformation is the class that contains a specific certificate policy
 * that is part of the CertificatePoliciesExtension. A 
 * CertificatePolicyExtension value consists of a vector of these objects.
 * <p>
 * The ASN.1 syntax for PolicyInformation (IMPLICIT tagging is defined in the
 * module definition):
 * <pre>
 *
 * PolicyInformation ::= SEQUENCE {
 *      policyIdentifier   CertPolicyId,
 *      policyQualifiers   SEQUENCE SIZE (1..MAX) OF
 *                              PolicyQualifierInfo OPTIONAL }
 *
 * CertPolicyId ::= OBJECT IDENTIFIER
 *
 * PolicyQualifierInfo ::= SEQUENCE {
 *	policyQualifierId  PolicyQualifierId,
 *	qualifier	   ANY DEFINED BY policyQualifierId }
 * </pre>
 *
 * @author Sean Mullan
 * @author Anne Anderson
 * @version 1.7, 04/01/04
 * @since	1.4
 */
public class PolicyInformation {

    // Attribute names
    public static final String NAME       = "PolicyInformation";
    public static final String ID         = "id";
    public static final String QUALIFIERS = "qualifiers";
    
    /* The policy OID */
    private CertificatePolicyId policyIdentifier;

    /* A Set of java.security.cert.PolicyQualifierInfo objects */
    private Set<PolicyQualifierInfo> policyQualifiers = 
    		new HashSet<PolicyQualifierInfo>();

    /**
     * Create an instance of PolicyInformation
     *
     * @param policyIdentifier the policyIdentifier as a 
     *		CertificatePolicyId
     * @param policyQualifiers a Set of PolicyQualifierInfo objects. 
     * 		Must not be NULL. Specify an empty Set for no qualifiers.
     * @exception IOException on decoding errors.
     */
    public PolicyInformation(CertificatePolicyId policyIdentifier,
	    Set<PolicyQualifierInfo> policyQualifiers) throws IOException {
	if (policyQualifiers == null) {
	    throw new NullPointerException("policyQualifiers is null");
	}
	policyQualifiers.addAll(policyQualifiers);
	this.policyIdentifier = policyIdentifier;
    }

    /**
     * Create an instance of PolicyInformation, decoding from
     * the passed DerValue.
     *
     * @param val the DerValue to construct the PolicyInformation from.
     * @exception IOException on decoding errors.
     */
    public PolicyInformation(DerValue val) throws IOException {
	if (val.tag != DerValue.tag_Sequence) {
	    throw new IOException("Invalid encoding of PolicyInformation");
	}
	policyIdentifier = new CertificatePolicyId(val.data.getDerValue());
	if (val.data.available() != 0) {
	    DerValue opt = val.data.getDerValue();
	    if (opt.tag != DerValue.tag_Sequence) 
		throw new IOException("Invalid encoding of PolicyInformation");
	    if (opt.data.available() == 0)
		throw new IOException("No data available in policyQualifiers");
	    while (opt.data.available() != 0)
		policyQualifiers.add(new PolicyQualifierInfo
			(opt.data.getDerValue().toByteArray()));
	}
    }

    /**
     * Compare this PolicyInformation with another object for equality
     *
     * @param other object to be compared with this
     * @return true iff the PolicyInformation objects match
     */
    public boolean equals(Object other) {
	if (!(other instanceof PolicyInformation))
	    return false;
	PolicyInformation piOther = (PolicyInformation)other;

	if (!policyIdentifier.equals(piOther.getPolicyIdentifier()))
	    return false;

	return policyQualifiers.equals(piOther.getPolicyQualifiers());
    }

    /**
     * Returns the hash code for this PolicyInformation.
     *
     * @return a hash code value.
     */
    public int hashCode() {
	int myhash = 37 + policyIdentifier.hashCode();
	myhash = 37 * myhash + policyQualifiers.hashCode();
	return myhash;
    }

    /**
     * Return the policyIdentifier value 
     *
     * @return The CertificatePolicyId object containing 
     *     the policyIdentifier (not a copy).
     */
    public CertificatePolicyId getPolicyIdentifier() {
	//XXXX May want to consider cloning this
	return policyIdentifier;
    }

    /**
     * Return the policyQualifiers value 
     *
     * @return a Set of PolicyQualifierInfo objects associated
     *    with this certificate policy (not a copy). 
     *	  Returns an empty Set if there are no qualifiers.
     *    Never returns null.
     */
    public Set<PolicyQualifierInfo> getPolicyQualifiers() {
	//XXXX May want to consider cloning this
	return policyQualifiers;
    }

    /**
     * Get the attribute value.
     */
    public Object get(String name) throws IOException {
	if (name.equalsIgnoreCase(ID)) {
	    //XXXX May want to consider cloning this
	    return policyIdentifier;
	} else if (name.equalsIgnoreCase(QUALIFIERS)) {
	    //XXXX May want to consider cloning this
	    return policyQualifiers;
	} else {
	    throw new IOException("Attribute name [" + name + 
		"] not recognized by PolicyInformation.");
	}
    }

    /**
     * Set the attribute value.
     */
    public void set(String name, Object obj) throws IOException {
	if (name.equalsIgnoreCase(ID)) {
	    if (obj instanceof CertificatePolicyId)
		policyIdentifier = (CertificatePolicyId)obj;
	    else
		throw new IOException("Attribute value must be instance " +
		    "of CertificatePolicyId.");
	} else if (name.equalsIgnoreCase(QUALIFIERS)) {
	    if (policyIdentifier == null) {
		throw new IOException("Attribute must have a " +
		    "CertificatePolicyIdentifier value before " +
		    "PolicyQualifierInfo can be set.");
	    }
	    if (obj instanceof Set) {
		Iterator i = ((Set)obj).iterator();
		while (i.hasNext()) {
		    Object obj1 = i.next();
		    if (!(obj1 instanceof PolicyQualifierInfo)) {
			throw new IOException("Attribute value must be a" +
				    "Set of PolicyQualifierInfo objects.");
		    }
		}
		policyQualifiers = (Set<PolicyQualifierInfo>)obj;
	    } else {
		throw new IOException("Attribute value must be of type Set.");
	    }
	} else {
	    throw new IOException("Attribute name [" + name + 
		"] not recognized by PolicyInformation");
	}
    }

    /**
     * Delete the attribute value.
     */
    public void delete(String name) throws IOException {
	if (name.equalsIgnoreCase(QUALIFIERS)) {
	    try {
	        policyQualifiers.clear();
	    } catch (UnsupportedOperationException uoe) {
		policyQualifiers = new HashSet<PolicyQualifierInfo>();
	    }
	} else if (name.equalsIgnoreCase(ID)) {
	    throw new IOException("Attribute ID may not be deleted from " +
		"PolicyInformation.");
	} else {
	    //ID may not be deleted
	    throw new IOException("Attribute name [" + name + 
		"] not recognized by PolicyInformation.");
	}
    }

    /**
     * Return an enumeration of names of attributes existing within this
     * attribute.
     */
    public Enumeration<String> getElements() {
        AttributeNameEnumeration elements = new AttributeNameEnumeration();
        elements.addElement(ID);
        elements.addElement(QUALIFIERS);

	return (elements.elements());
    }

    /**
     * Return the name of this attribute.
     */
    public String getName() {
        return (NAME);
    }

    /**
     * Return a printable representation of the PolicyInformation.
     */
    public String toString() {
	StringBuilder s = new StringBuilder("  [" + policyIdentifier.toString());
        s.append(policyQualifiers + "  ]\n");
	return s.toString();
    }

    /**
     * Write the PolicyInformation to the DerOutputStream.
     *
     * @param out the DerOutputStream to write the extension to.
     * @exception IOException on encoding errors.
     */
    public void encode(DerOutputStream out) throws IOException {
	DerOutputStream tmp = new DerOutputStream();
	policyIdentifier.encode(tmp);
	if (!policyQualifiers.isEmpty()) {
	    Iterator i = policyQualifiers.iterator();
            DerOutputStream tmp2 = new DerOutputStream();
	    while (i.hasNext()) {
	        PolicyQualifierInfo pq = (PolicyQualifierInfo) i.next();
	        tmp2.write(pq.getEncoded());
	    }
            tmp.write(DerValue.tag_Sequence, tmp2);
	}
	out.write(DerValue.tag_Sequence, tmp);
    }
}
