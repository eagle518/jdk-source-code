/*
 * @(#)X500Signer.java	1.26 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.x509;

import java.security.Signature;
import java.security.SignatureException;
import java.security.Signer;
import java.security.NoSuchAlgorithmException;

/**
 * This class provides a binding between a Signature object and an
 * authenticated X.500 name (from an X.509 certificate chain), which
 * is needed in many public key signing applications.
 *
 * <P>The name of the signer is important, both because knowing it is the
 * whole point of the signature, and because the associated X.509 certificate
 * is always used to verify the signature.
 *
 * <P><em>The X.509 certificate chain is temporarily not associated with
 * the signer, but this omission will be resolved.</em>
 *
 * @version 1.26
 *
 * @author David Brownell
 * @author Amit Kapoor
 * @author Hemma Prafullchandra
 */
public final class X500Signer extends Signer
{
    private static final long serialVersionUID = -8609982645394364834L;

    /**
     * Called for each chunk of the data being signed.  That
     * is, you can present the data in many chunks, so that
     * it doesn't need to be in a single sequential buffer.
     *
     * @param buf buffer holding the next chunk of the data to be signed
     * @param offset starting point of to-be-signed data
     * @param len how many bytes of data are to be signed
     * @exception SignatureException on errors.
     */
    public void update(byte buf[], int offset, int len)
    throws SignatureException {
        sig.update (buf, offset, len);
    }

    /**
     * Produces the signature for the data processed by update().
     *
     * @exception SignatureException on errors.
     */
    public byte[] sign() throws SignatureException {
        return sig.sign();
    }

    /**
     * Returns the algorithm used to sign.
     */
    public AlgorithmId 	getAlgorithmId() {
        return algid;
    }

    /**
     * Returns the name of the signing agent.
     */
    public X500Name	getSigner() {
        return agent;
    }

    /*
     * Constructs a binding between a signature and an X500 name
     * from an X.509 certificate.
     */
    // package private  ----hmmmmm ?????
    public X500Signer(Signature sig, X500Name agent) {
	if (sig == null || agent == null)
	    throw new IllegalArgumentException ("null parameter");

	this.sig = sig;
	this.agent = agent;

	try {
	  this.algid = AlgorithmId.getAlgorithmId(sig.getAlgorithm());

	} catch (NoSuchAlgorithmException e) {
	    throw new RuntimeException("internal error! " + e.getMessage());
	}
    }
    
    private Signature	sig;
    private X500Name	agent;		// XXX should be X509CertChain
    private AlgorithmId	algid;
}
