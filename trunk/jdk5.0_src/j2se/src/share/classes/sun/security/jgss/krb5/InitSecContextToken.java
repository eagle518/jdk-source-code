/*
 * @(#)InitSecContextToken.java	1.8 04/04/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.IOException;
import sun.security.krb5.*;

class InitSecContextToken extends InitialToken {

    private KrbApReq apReq = null;

    /**
     * For the context initiator to call. It constructs a new
     * InitSecContextToken to send over to the peer containing the desired
     * flags and the AP-REQ. It also updates the context with the local
     * sequence number and shared context key.
     * (When mutual auth is enabled the peer has an opportunity to
     * renegotiate the session key in the followup AcceptSecContextToken
     * that it sends.)
     */
    InitSecContextToken(Krb5Context context,
			       Credentials tgt,
			       Credentials serviceTicket) 
	throws KrbException, IOException, GSSException { 
	
	boolean mutualRequired = context.getMutualAuthState();
	boolean useSubkey = true; // MIT Impl will crash if this is not set!
	boolean useSequenceNumber = true;

	OverloadedChecksum gssChecksum = 
	    new OverloadedChecksum(context, tgt, serviceTicket);

	Checksum checksum = gssChecksum.getChecksum();

	apReq = new KrbApReq(serviceTicket, 
			     mutualRequired,
			     useSubkey,
			     useSequenceNumber,
			     checksum);
	
	context.resetMySequenceNumber(apReq.getSeqNumber().intValue());

	EncryptionKey subKey = apReq.getSubKey();
	if (subKey != null)
	    context.setKey(subKey);
	else
	    context.setKey(serviceTicket.getSessionKey());

	if (!mutualRequired)
	    context.resetPeerSequenceNumber(0);
    }

    /**
     * For the context acceptor to call. It reads the bytes out of an
     * InputStream and constructs an InitSecContextToken with them.
     */
    InitSecContextToken(Krb5Context context, EncryptionKey[] keys, 
			       InputStream is) 
	throws IOException, GSSException, KrbException  {
	
	int tokenId = ((is.read()<<8) | is.read());

	if (tokenId != Krb5Token.AP_REQ_ID)
	    throw new GSSException(GSSException.DEFECTIVE_TOKEN, -1,
				   "AP_REQ token id does not match!");

	//: TBD Modify KrbApReq cons to take an InputStream
	byte[] apReqBytes = 
	    new sun.security.util.DerValue(is).toByteArray();
	//debug("=====ApReqBytes: [" + getHexBytes(apReqBytes) + "]\n");
	apReq = new KrbApReq(apReqBytes, keys);
	//debug("\nReceived AP-REQ and authenticated it.\n");

	EncryptionKey sessionKey 
	    = (EncryptionKey) apReq.getCreds().getSessionKey();

	/*
	  System.out.println("\n\nSession key from service ticket is: " +
	  getHexBytes(sessionKey.getBytes()));
	*/

	EncryptionKey subKey = apReq.getSubKey(); 
	if (subKey != null) {
	    context.setKey(subKey);
	    /*
	      System.out.println("Sub-Session key from authenticator is: " +
	      getHexBytes(subKey.getBytes()) + "\n");
	    */
	} else {
	    context.setKey(sessionKey);
	    //System.out.println("Sub-Session Key Missing in Authenticator.\n");
	}

	OverloadedChecksum gssChecksum = 
	    new OverloadedChecksum(context, apReq.getChecksum(), sessionKey);
	gssChecksum.setContextFlags(context);
	Credentials delegCred = gssChecksum.getDelegatedCreds();
	if (delegCred != null) {
	    Krb5CredElement credElement =
		Krb5InitCredential.getInstance(
				   (Krb5NameElement)context.getSrcName(), 
				   delegCred);
	    context.setDelegCred(credElement);
	}

        Integer apReqSeqNumber = apReq.getSeqNumber(); 
        int peerSeqNumber = (apReqSeqNumber != null ?  
                             apReqSeqNumber.intValue() : 
                             0);
	context.resetPeerSequenceNumber(peerSeqNumber);
	if (!context.getMutualAuthState())
	    // Use the same sequence number as the peer
	    // (Behaviour exhibited by the Windows SSPI server)
	    context.resetMySequenceNumber(peerSeqNumber);
    }
    
    // TBD: Remove this and use stream based one only
    /**
     * For the context acceptor to call.
     */
	 /*
	 // Not used
    public InitSecContextToken(Krb5Context context, EncryptionKey key, 
			       byte[] tokenBytes, int pos, int len) 
	throws IOException, GSSException, KrbException  {
	this(context, key, 
	     new java.io.ByteArrayInputStream(tokenBytes, pos, len));
    }
*/

    public final KrbApReq getKrbApReq() {
	return apReq;
    }

    public final byte[] encode() throws IOException {
	byte[] apReqBytes = apReq.getMessage();
	byte[] retVal = new byte[2 + apReqBytes.length];
	writeInt(Krb5Token.AP_REQ_ID, retVal, 0);
	System.arraycopy(apReqBytes, 0, retVal, 2, apReqBytes.length);
	//	System.out.println("GSS-Token with AP_REQ is:");
	//	System.out.println(getHexBytes(retVal));
	return retVal;
    }
}
