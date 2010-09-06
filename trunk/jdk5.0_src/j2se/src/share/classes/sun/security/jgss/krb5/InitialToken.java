/*
 * @(#)InitialToken.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import javax.security.auth.kerberos.DelegationPermission;
import java.io.IOException;
import java.net.InetAddress;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import sun.security.krb5.*;

abstract class InitialToken extends Krb5Token {

    private static final int CHECKSUM_TYPE = 0x8003;
    
    private static final int CHECKSUM_LENGTH_SIZE     = 4;
    private static final int CHECKSUM_BINDINGS_SIZE   = 16;
    private static final int CHECKSUM_FLAGS_SIZE      = 4;
    private static final int CHECKSUM_DELEG_OPT_SIZE  = 2;
    private static final int CHECKSUM_DELEG_LGTH_SIZE = 2;
    
    private static final int CHECKSUM_DELEG_FLAG    = 1;
    private static final int CHECKSUM_MUTUAL_FLAG   = 2;
    private static final int CHECKSUM_REPLAY_FLAG   = 4;
    private static final int CHECKSUM_SEQUENCE_FLAG = 8;
    private static final int CHECKSUM_CONF_FLAG     = 16;
    private static final int CHECKSUM_INTEG_FLAG    = 32;
    
    private final byte[] CHECKSUM_FIRST_BYTES = 
    {(byte)0x10, (byte)0x00, (byte)0x00, (byte)0x00};

    private final int CHANNEL_BINDING_AF_INET = 2;
    private final int CHANNEL_BINDING_AF_NULL_ADDR = 255;

    protected class OverloadedChecksum {
	
	private byte[] checksumBytes = null;
	private Credentials delegCreds = null;
	private int flags = 0;

	/**
	 * Called on the initiator side when creating the
	 * InitSecContextToken.
	 */
	public OverloadedChecksum(Krb5Context context,
				  Credentials tgt,
				  Credentials serviceTicket) 
	    throws KrbException, IOException, GSSException {
	    
	    byte[] krbCredMessage = null;
	    int pos = 0;
	    int size = CHECKSUM_LENGTH_SIZE + CHECKSUM_BINDINGS_SIZE +
		CHECKSUM_FLAGS_SIZE;
	    
	    if (context.getCredDelegState()) {

		if (!tgt.isForwardable()) {
		    // XXX log this resetting of delegation state
		    context.setCredDelegState(false);
		} else {

		    KrbCred krbCred = new KrbCred(tgt, serviceTicket, 
						  EncryptionKey.NULL_KEY);
		    krbCredMessage = krbCred.getMessage();
		    size += CHECKSUM_DELEG_OPT_SIZE + 
			    CHECKSUM_DELEG_LGTH_SIZE +
			    krbCredMessage.length;
		}
	    }
	    
	    checksumBytes = new byte[size];
	    
	    checksumBytes[pos++] = CHECKSUM_FIRST_BYTES[0];
	    checksumBytes[pos++] = CHECKSUM_FIRST_BYTES[1];
	    checksumBytes[pos++] = CHECKSUM_FIRST_BYTES[2];
	    checksumBytes[pos++] = CHECKSUM_FIRST_BYTES[3];
	    
	    ChannelBinding localBindings = context.getChannelBinding();
	    if (localBindings != null) {
		byte[] localBindingsBytes =
		    computeChannelBinding(context.getChannelBinding());
		System.arraycopy(localBindingsBytes, 0,
			     checksumBytes, pos, localBindingsBytes.length);
		//		System.out.println("ChannelBinding hash: "
		//	   + getHexBytes(localBindingsBytes));
	    }

	    pos += CHECKSUM_BINDINGS_SIZE;
	    
	    if (context.getCredDelegState())
		flags |= CHECKSUM_DELEG_FLAG;
	    if (context.getMutualAuthState())
		flags |= CHECKSUM_MUTUAL_FLAG;
	    if (context.getReplayDetState())
		flags |= CHECKSUM_REPLAY_FLAG;
	    if (context.getSequenceDetState())
		flags |= CHECKSUM_SEQUENCE_FLAG;
	    if (context.getIntegState())
		flags |= CHECKSUM_INTEG_FLAG;
	    if (context.getConfState())
		flags |= CHECKSUM_CONF_FLAG;

	    byte[] temp = new byte[4];    
	    writeLittleEndian(flags, temp);
	    checksumBytes[pos++] = temp[0];
	    checksumBytes[pos++] = temp[1];
	    checksumBytes[pos++] = temp[2];
	    checksumBytes[pos++] = temp[3];
	    
	    if (context.getCredDelegState()) {
		
		PrincipalName delegateTo =
		    serviceTicket.getServer();
		// Cannot use '\"' instead of "\"" in constructor because
		// it is interpreted as suggested length!
		StringBuffer buf = new StringBuffer("\"");
		buf.append(delegateTo.getName()).append('\"');
		String realm = delegateTo.getRealmAsString();
		buf.append(" \"krbtgt/").append(realm).append('@');
		buf.append(realm).append('\"');
		SecurityManager sm = System.getSecurityManager();  
		if (sm != null) {  
		    DelegationPermission perm =   
			new DelegationPermission(buf.toString());
		    sm.checkPermission(perm);  
		}  


		/* 
		 * Write 1 in little endian but in two bytes
		 * for DlgOpt
		 */
		
		checksumBytes[pos++] = (byte)0x01;
		checksumBytes[pos++] = (byte)0x00;
		
		/*
		 * Write the length of the delegated credential in little
		 * endian but in two bytes for Dlgth
		 */
		
		if (krbCredMessage.length > 0x0000ffff)
		    throw new GSSException(TBD);
		
		writeLittleEndian(krbCredMessage.length, temp);
		checksumBytes[pos++] = temp[0];
		checksumBytes[pos++] = temp[1];
		System.arraycopy(krbCredMessage, 0, 
				 checksumBytes, pos, krbCredMessage.length);
	    }

	}

	/**
	 * Called on the acceptor side when reading an InitSecContextToken.
	 */
	// TBD: Maybe passing in Checksum is not required. byte[] can
	// be passed in if this checksum type denotes a
	// raw_checksum. In that case, make Checksum class krb5
	// internal.
	public OverloadedChecksum(Krb5Context context, 
				  Checksum checksum, EncryptionKey key)
	    throws GSSException, KrbException, IOException{

	    int pos = 0;
	    
	    checksumBytes = checksum.getBytes();
	    
	    if ((checksumBytes[0] != CHECKSUM_FIRST_BYTES[0]) ||
		(checksumBytes[1] != CHECKSUM_FIRST_BYTES[1]) ||
		(checksumBytes[2] != CHECKSUM_FIRST_BYTES[2]) ||
		(checksumBytes[3] != CHECKSUM_FIRST_BYTES[3])) {
		throw new GSSException(TBD);
	    }

	    byte[] remoteBindingBytes = new byte[CHECKSUM_BINDINGS_SIZE];
	    System.arraycopy(checksumBytes, 4, remoteBindingBytes, 0,
			     CHECKSUM_BINDINGS_SIZE);

	    byte[] noBindings = new byte[CHECKSUM_BINDINGS_SIZE];
	    boolean tokenContainsBindings = 
		(!java.util.Arrays.equals(noBindings, remoteBindingBytes));

	    ChannelBinding localBindings = context.getChannelBinding();

	    if (tokenContainsBindings ||
		localBindings != null) {

		boolean badBindings = false;
		String errorMessage = null;

		if (tokenContainsBindings &&
		    localBindings != null) {
		    byte[] localBindingsBytes = 
			computeChannelBinding(localBindings);
		    //		    System.out.println("ChannelBinding hash: "
		    //	       + getHexBytes(localBindingsBytes));
		    badBindings =
			(!java.util.Arrays.equals(localBindingsBytes,
						remoteBindingBytes));
		    errorMessage = "Bytes mismatch!";
		} else if (localBindings == null) {
		    errorMessage = "ChannelBinding not provided!";
		    badBindings = true;
		} else {
		    errorMessage = "Token missing ChannelBinding!";
		    badBindings = true;
		}

		if (badBindings)
		    throw new GSSException(GSSException.BAD_BINDINGS, -1,
					   errorMessage);
	    }

	    flags = readLittleEndian(checksumBytes, 20, 4);

    	    if ((flags & CHECKSUM_DELEG_FLAG) > 0) {

		/* TBD: CHECK THIS TOO?
		   if ((checksumBytes[24] != (byte)0x01) && 
		   (checksumBytes[25] != (byte)0x00))
		*/

		int credLen = readLittleEndian(checksumBytes, 26, 2);
		byte[] credBytes = new byte[credLen];
		System.arraycopy(checksumBytes, 28, credBytes, 0, credLen);
		delegCreds = 
		    new KrbCred(credBytes, EncryptionKey.NULL_KEY).
		    getDelegatedCreds()[0];

	    }
	}
	
	public Checksum getChecksum() throws KrbException {
	    return new Checksum(checksumBytes, CHECKSUM_TYPE);
	}
	
	public Credentials getDelegatedCreds() {
	    return delegCreds;
	}
	
	public void setContextFlags(Krb5Context context) {
		// default for cred delegation is false
    	    if ((flags & CHECKSUM_DELEG_FLAG) > 0)
		context.setCredDelegState(true);
		// default for the following are true
	    if ((flags & CHECKSUM_MUTUAL_FLAG) == 0) {
		context.setMutualAuthState(false);
	    }
	    if ((flags & CHECKSUM_REPLAY_FLAG) == 0) {
		context.setReplayDetState(false);
	    }
	    if ((flags & CHECKSUM_SEQUENCE_FLAG) == 0) {
		context.setSequenceDetState(false);
	    }
	    if ((flags & CHECKSUM_CONF_FLAG) == 0) {
		context.setConfState(false);
	    }
	    if ((flags & CHECKSUM_INTEG_FLAG) == 0) {
		context.setIntegState(false);
	    }
	}
    }

    private byte[] computeChannelBinding(ChannelBinding channelBinding)
	throws GSSException {

	InetAddress initiatorAddress = channelBinding.getInitiatorAddress();
	InetAddress acceptorAddress = channelBinding.getAcceptorAddress();
	byte[] initiatorAddressBytes = null;
	int initiatorAddressType = CHANNEL_BINDING_AF_NULL_ADDR;
	byte[] acceptorAddressBytes = null;
	int acceptorAddressType = CHANNEL_BINDING_AF_NULL_ADDR;

	int size = 5*4;

	if (initiatorAddress != null) {
	    initiatorAddressType = CHANNEL_BINDING_AF_INET;
	    initiatorAddressBytes = initiatorAddress.getAddress();
	    if (initiatorAddressBytes.length != 4)
		throw new GSSException(GSSException.FAILURE, -1,
  	        "Cannot handle non AF-INET addresses in ChannelBinding.");
	    size += initiatorAddressBytes.length;
	}

	if (acceptorAddress != null) {
	    acceptorAddressType = CHANNEL_BINDING_AF_INET;
	    acceptorAddressBytes = acceptorAddress.getAddress();
	    if (acceptorAddressBytes.length != 4)
		throw new GSSException(GSSException.FAILURE, -1,
  	        "Cannot handle non AF-INET addresses in ChannelBinding.");
	    size += acceptorAddressBytes.length;
	}

	byte[] appDataBytes = channelBinding.getApplicationData();
	if (appDataBytes != null) {
	    size += appDataBytes.length;
	}

	byte[] data = new byte[size];

	int pos = 0;

	writeLittleEndian(initiatorAddressType, data, pos);
	pos += 4;

	if (initiatorAddressBytes != null) {
	    writeLittleEndian(initiatorAddressBytes.length, data, pos);
	    pos += 4;
	    System.arraycopy(initiatorAddressBytes, 0, 
			     data, pos, initiatorAddressBytes.length);
	    pos += initiatorAddressBytes.length;
	} else {
	    // Write length 0
	    pos += 4;
	}

	writeLittleEndian(acceptorAddressType, data, pos);
	pos += 4;

	if (acceptorAddressBytes != null) {
	    writeLittleEndian(acceptorAddressBytes.length, data, pos);
	    pos += 4;
	    System.arraycopy(acceptorAddressBytes, 0, 
			     data, pos, acceptorAddressBytes.length);
	    pos += acceptorAddressBytes.length;
	} else {
	    // Write length 0
	    pos += 4;
	}

	if (appDataBytes != null) {
	    writeLittleEndian(appDataBytes.length, data, pos);
	    pos += 4;
	    System.arraycopy(appDataBytes, 0, data, pos,
			     appDataBytes.length);
	    pos += appDataBytes.length;
	} else {
	    // Write 0
	    pos += 4;
	}

	try {
	    MessageDigest md5 = MessageDigest.getInstance("MD5");
	    return md5.digest(data);
	} catch (NoSuchAlgorithmException e) {
		throw new GSSException(GSSException.FAILURE, -1, 
				       "Could not get MD5 Message Digest - " 
				       + e.getMessage());
	}
    }

    public abstract byte[] encode() throws IOException;

}
