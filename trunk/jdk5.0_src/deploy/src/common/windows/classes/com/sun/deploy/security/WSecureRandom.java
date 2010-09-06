/*
 * @(#)WSecureRandom.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandomSpi;

/**
 * <p>This class provides a crytpographically strong pseudo-random number
 * generator using native OS support.
 *
 * Most of the implementation is delegated to SUN implementation of
 * SecureRandom. The only thing that is changed is the way we obtained
 * the seed, which utilizes native OS support.
 *
 * @author Stanley Man-Kit Ho
 */

public final class WSecureRandom extends SecureRandomSpi
implements java.io.Serializable {

    // SUN implementation of SecureRandom SPI
    private sun.security.provider.SecureRandom spi = null;
    
    /**
     * Create WSecureRandom object.
     */
    public WSecureRandom() 
    {
	// Generate seed from native OS - use 128 bits
	byte[] seed = WSeedGenerator.generateSeed(128);

	try
	{
	    // Use SHA to hash the seed to make it more random
	    MessageDigest sha = MessageDigest.getInstance("SHA");

	    sha.update(seed);
	    seed = sha.digest();
	}
	catch (NoSuchAlgorithmException e)
	{
	    e.printStackTrace();
	}

	// Create SecureRandom spi for delegation
	spi = new sun.security.provider.SecureRandom();

	// Set seed
	spi.engineSetSeed(seed);
    }

    /**
     * Returns the given number of seed bytes, computed using the seed
     * generation algorithm that this class uses to seed itself.  This
     * call may be used to seed other random number generators.  While
     * we attempt to return a "truly random" sequence of bytes, we do not
     * know exactly how random the bytes returned by this call are.  (See
     * the empty constructor <a href = "#SecureRandom">SecureRandom</a>
     * for a brief description of the underlying algorithm.)
     * The prudent user will err on the side of caution and get extra
     * seed bytes, although it should be noted that seed generation is
     * somewhat costly.
     *
     * @param numBytes the number of seed bytes to generate.
     *
     * @return the seed bytes.
     */
    public byte[] engineGenerateSeed(int numBytes) 
    {
	// Delegate to SUN implementation of SecureRandom SPI
	return spi.engineGenerateSeed(numBytes);
    }

    /**
     * Reseeds this random object. The given seed supplements, rather than
     * replaces, the existing seed. Thus, repeated calls are guaranteed
     * never to reduce randomness.
     *
     * @param seed the seed.
     */
    synchronized public void engineSetSeed(byte[] seed) 
    {
	// Delegate to SUN implementation of SecureRandom SPI
	spi.engineSetSeed(seed);
    }

    /**
     * Generates a user-specified number of random bytes.
     *
     * @param bytes the array to be filled in with random bytes.
     */
    public synchronized void engineNextBytes(byte[] result) 
    {
	// Delegate to SUN implementation of SecureRandom SPI
	spi.engineNextBytes(result);
    }
  
}
