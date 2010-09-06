/*
 * @(#)RSAKeyPairGenerator.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.rsa;

import java.math.BigInteger;

import java.security.*;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.RSAKeyGenParameterSpec;

import sun.security.jca.JCAUtil;

/**
 * RSA keypair generation. Standard algorithm, minimum key length 512 bit.
 * We generate two random primes until we find two where phi is relative
 * prime to the public exponent. Default exponent is 65537. It has only bit 0
 * and bit 4 set, which makes it particularly efficient.
 *
 * Note that this procedure may generate keys that are one bit shorter than
 * requested. Multiplying two n bit primes produces a result that is either
 * 2*n or 2*n - 1 bits long.
 *
 * @since   1.5
 * @version 1.2, 12/19/03
 * @author  Andreas Sterbenz
 */
public final class RSAKeyPairGenerator extends KeyPairGeneratorSpi {
    
    // public exponent to use
    private BigInteger publicExponent;
    
    // size of the key to generate, >= 512
    private int keySize;
    
    // PRNG to use
    private SecureRandom random;
    
    public RSAKeyPairGenerator() {
	// initialize to default in case the app does not call initialize()
	initialize(1024, null);
    }

    // initialize the generator. See JCA doc
    public void initialize(int keySize, SecureRandom random) {
	if (keySize < 512) {
	    throw new InvalidParameterException
		("Key size must be at least 512 bits");
	}
	if (keySize > 64 * 1024) {
	    // do not allow unreasonably large key sizes, probably user error
	    throw new InvalidParameterException
		("Key size must be 65536 bits or less");
	}
	this.keySize = keySize;
	this.random = random;
	this.publicExponent = RSAKeyGenParameterSpec.F4;
    }

    // second initialize method. See JCA doc.
    public void initialize(AlgorithmParameterSpec params, SecureRandom random) 
	    throws InvalidAlgorithmParameterException {
	if (params instanceof RSAKeyGenParameterSpec == false) {
	    throw new InvalidAlgorithmParameterException
	    	("Params must be instance of RSAKeyGenParameterSpec");
	}
	RSAKeyGenParameterSpec rsaSpec = (RSAKeyGenParameterSpec)params;
	keySize = rsaSpec.getKeysize();
	publicExponent = rsaSpec.getPublicExponent();
	this.random = random;
	if (keySize < 512) {
	    throw new InvalidAlgorithmParameterException
		("Key size must be at least 512 bits");
	}
	if (keySize > 64 * 1024) {
	    // do not allow unreasonably large key sizes, probably user error
	    throw new InvalidParameterException
		("Key size must be 65536 bits or less");
	}
	if (publicExponent == null) {
	    publicExponent = RSAKeyGenParameterSpec.F4;
	} else {
	    if (publicExponent.compareTo(RSAKeyGenParameterSpec.F0) < 0) {
		throw new InvalidAlgorithmParameterException
			("Public exponent must be 3 or larger");
	    }
	    if (publicExponent.bitLength() > keySize) {
		throw new InvalidAlgorithmParameterException
			("Public exponent must be smaller than key size");
	    }
	}
    }
    
    // generate the keypair. See JCA doc
    public KeyPair generateKeyPair() {
	// accomodate odd key sizes in case anybody wants to use them
	int lp = (keySize + 1) >> 1;
	int lq = keySize - lp;
	if (random == null) {
	    random = JCAUtil.getSecureRandom();
	}
	BigInteger e = publicExponent;
	while (true) {
	    // generate two random primes of size lp/lq
	    BigInteger p = BigInteger.probablePrime(lp, random);
	    BigInteger q = BigInteger.probablePrime(lq, random);
	    // convention is for p > q
	    if (p.compareTo(q) < 0) {
		BigInteger tmp = p;
		p = q;
		q = tmp;
	    }
	    // phi = (p - 1) * (q - 1) must be relative prime to e
	    // otherwise RSA just won't work ;-)
	    BigInteger p1 = p.subtract(BigInteger.ONE);
	    BigInteger q1 = q.subtract(BigInteger.ONE);
	    BigInteger phi = p1.multiply(q1);
	    // generate new p and q until they work. typically
	    // the first try will succeed when using F4
	    if (e.gcd(phi).equals(BigInteger.ONE) == false) {
		continue;
	    }
	    
	    // private exponent d is the inverse of e mod phi
	    BigInteger d = e.modInverse(phi);
	    // modulus n = p * q
	    BigInteger n = p.multiply(q);
	    
	    // 1st prime exponent pe = d mod (p - 1)
	    BigInteger pe = d.mod(p1);
	    // 2nd prime exponent qe = d mod (q - 1)
	    BigInteger qe = d.mod(q1);
	    
	    // crt coefficient coeff is the inverse of q mod p
	    BigInteger coeff = q.modInverse(p);
	    
	    try {
		PublicKey publicKey = new RSAPublicKeyImpl(n, e);
		PrivateKey privateKey = 
			new RSAPrivateCrtKeyImpl(n, e, d, p, q, pe, qe, coeff);
		return new KeyPair(publicKey, privateKey);
	    } catch (InvalidKeyException exc) {
		// invalid key exception only thrown for keys < 512 bit,
		// will not happen here
		throw new RuntimeException(exc);
	    }
	}
    }
    
}

