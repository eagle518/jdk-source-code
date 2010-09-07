/*
 * @(#)JDK11ClassFileTransformer.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.security;

import sun.misc.ClassFileTransformer;

/**
 * This is a class called by plugin's appletviewer to initialize the
 * JDK's ClassFileTransformer.
 *
 * The ClassFileTransformer allow Java Plug-in to have a chance 
 * to transform the byte code from one form to another if necessary. 
 * In this case, malformed JDK 1.1 class file may be transformed 
 * by JPI into a well-formed Java 2 class file on-the-fly, so JDK 
 * 1.1 applets with malformed class file in the Internet may run 
 * in Java Plug-in
 *
 * @author	Stanley Man-Kit Ho
 */
public class JDK11ClassFileTransformer
{
    /**
     * Initialize class file transformer in JDK.
     */
    public synchronized static void init()
    {
	// Add two class file transformer objects
	ClassFileTransformer.add(new Broken11Transformer_0());
	ClassFileTransformer.add(new Broken11Transformer_1());
    }

    /*
     * Fix malformed class file format error if possible. This
     * class is intended to be used by Java Plug-in to fix
     * any JDK 1.1 compatible applet which may have class
     * format error.
     */
    private static class Broken11Transformer_0 extends sun.misc.ClassFileTransformer
    {
	/**
	 * Transform a byte array from one to the other.
	 *
	 * @param b Byte array
	 * @param off Offset
	 * @param len Length of byte array
	 * @return Transformed byte array
	 */
	public byte[] transform(byte[] b, int off, int len)
		      throws ClassFormatError
	{
	    // Check class file format version
	   JDK11ClassFileTransformer.ensureClassFileVersion(b, off, len);

	    try 
	    {
		Broken11ClassFixer fixer = new Broken11ClassFixer();

		// Try to fix it
		fixer.process(b, off, len);

		byte[] processedData = fixer.getProcessedData();
		int processedOffset = fixer.getProcessedDataOffset();
		int processedLength = fixer.getProcessedDataLength();

		if (processedOffset == 0 && processedLength == processedData.length)
    		{
		    return processedData;
		}
		else
		{
		    // We need a new byte array
		    byte[] result = new byte[processedLength];

		    System.arraycopy(processedData, processedOffset, result, 0, processedLength);

		    return result;
		}
	    }  
	    catch (ThreadDeath td) 
	    {
		// rethrow
		throw td;
	    }
	    catch (Throwable e)
	    {
		// No good, throw ClassFormatError
		throw new ClassFormatError();
	    }
	}
    }

    /*
     * Fix malformed class file format error if possible. This
     * class is intended to be used by Java Plug-in to fix
     * any JDK 1.1 compatible applet which may have class
     * format error.
     */
    private static class Broken11Transformer_1 extends sun.misc.ClassFileTransformer
    {
	/**
	 * Transform a byte array from one to the other.
	 *
	 * @param b Byte array
	 * @param off Offset
	 * @param len Length of byte array
	 * @return Transformed byte array
	 */
	public byte[] transform(byte[] b, int off, int len)
		      throws ClassFormatError
	{
	    // Check class file format version
	    JDK11ClassFileTransformer.ensureClassFileVersion(b, off, len);

	    try 
	    {
		StripClassFile stripClassFile = new StripClassFile();

		return stripClassFile.strip(b);
	    }  
	    catch (ThreadDeath td) 
	    {
		// rethrow
		throw td;
	    }
	    catch (Throwable e)
	    {
		// No good, throw ClassFormatError
		throw new ClassFormatError();
	    }
	}
    }

    /**
     * Check if the class file format version is JDK 1.1 or not.
     *
     * @param b Byte array
     * @param off Offset
     * @param len Length of byte array
     */
    private static void ensureClassFileVersion(byte[] b, int off, int len)
		        throws ClassFormatError
    {
	// Bound-checking
	if (len < 8)
	    throw new ClassFormatError();
    
	// #4527533: JPI should not transform class file if version
	// is 46.0 or later.
	//
	int major_version = readShort(b, off + 6);

	if (major_version >= 46)
	    throw new ClassFormatError();	    
    }
    private static int readByte(byte b) 
    {
	return ((int)b) & 0xFF;
    }

    private static int readShort(byte[] b, int off) 
    {
	int hi = readByte(b[off]);
	int lo = readByte(b[off + 1]);
	return (hi << 8) | lo;
    }
}


