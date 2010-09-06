/*
 * @(#)AdaptiveCoding.java	1.4 04/01/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.util.jar.pack;

import java.util.*;
import java.io.*;

/**
 * Adaptive coding.
 * See the section "Adaptive Encodings" in the Pack200 spec.
 * @author John Rose
 * @version 1.4, 01/06/04
 */
class AdaptiveCoding implements Constants, CodingMethod {
    CodingMethod headCoding;
    int          headLength;
    CodingMethod tailCoding;

    public AdaptiveCoding(int headLength, CodingMethod headCoding, CodingMethod tailCoding) {
	assert(isCodableLength(headLength));
	this.headLength = headLength;
	this.headCoding = headCoding;
	this.tailCoding = tailCoding;
    }

    public void setHeadCoding(CodingMethod headCoding) {
	this.headCoding = headCoding;
    }
    public void setHeadLength(int headLength) {
	assert(isCodableLength(headLength));
	this.headLength = headLength;
    }
    public void setTailCoding(CodingMethod tailCoding) {
	this.tailCoding = tailCoding;
    }

    public boolean isTrivial() {
	return headCoding == tailCoding;
    }

    // CodingMethod methods.
    public void writeArrayTo(OutputStream out, int[] a, int start, int end) throws IOException {
	int mid = start+headLength;
	assert(mid <= end);
	headCoding.writeArrayTo(out, a, start, mid);
	tailCoding.writeArrayTo(out, a, mid,   end);
    }
    public void readArrayFrom(InputStream in, int[] a, int start, int end) throws IOException {
	int mid = start+headLength;
	assert(mid <= end);
	headCoding.readArrayFrom(in, a, start, mid);
	tailCoding.readArrayFrom(in, a, mid,   end);
    }

    public static final int KX_MIN = 0;
    public static final int KX_MAX = 3;
    public static final int KX_LG2BASE = 4;
    public static final int KX_BASE = 16;

    public static final int KB_MIN = 0x00;
    public static final int KB_MAX = 0xFF;
    public static final int KB_OFFSET = 1;
    public static final int KB_DEFAULT = 3;

    static int getKXOf(int K) {
	for (int KX = KX_MIN; KX <= KX_MAX; KX++) {
	    if (((K - KB_OFFSET) & ~KB_MAX) == 0)
		return KX;
	    K >>>= KX_LG2BASE;
	}
	return -1;
    }

    static int getKBOf(int K) {
	int KX = getKXOf(K);
	if (KX < 0)  return -1;
	K >>>= (KX * KX_LG2BASE);
	return K-1;
    }

    static int decodeK(int KX, int KB) {
	assert(KX_MIN <= KX && KX <= KX_MAX);
	assert(KB_MIN <= KB && KB <= KB_MAX);
	return (KB+KB_OFFSET) << (KX * KX_LG2BASE);
    }

    static int getNextK(int K) {
	if (K <= 0)  return 1;  // 1st K value
	int KX = getKXOf(K);
	if (KX < 0)  return Integer.MAX_VALUE;
	// This is the increment we expect to apply:
	int unit = 1      << (KX * KX_LG2BASE);
	int mask = KB_MAX << (KX * KX_LG2BASE);
	int K1 = K + unit;
	K1 &= ~(unit-1);  // cut off stray low-order bits
	if (((K1 - unit) & ~mask) == 0) {
	    assert(getKXOf(K1) == KX);
	    return K1;
	}
	if (KX == KX_MAX)  return Integer.MAX_VALUE;
	KX += 1;
	int unit2 = 1      << (KX * KX_LG2BASE);
	int mask2 = KB_MAX << (KX * KX_LG2BASE);
	K1 |= (mask & ~mask2);
	K1 += unit;
	assert(getKXOf(K1) == KX);
	return K1;
    }

    // Is K of the form ((KB:[0..255])+1) * 16^(KX:{0..3])?
    public static boolean isCodableLength(int K) {
	int KX = getKXOf(K);
	if (KX < 0)  return false;
	int unit = 1      << (KX * KX_LG2BASE);
	int mask = KB_MAX << (KX * KX_LG2BASE);
	return ((K - unit) & ~mask) == 0;
    }

    public byte[] getMetaCoding(Coding dflt) {
	//assert(!isTrivial()); // can happen
	// See the isCodableLength restriction in CodingChooser.

	int K = headLength;
	assert(isCodableLength(K));
	int ADef   = (headCoding == dflt)?1:0;
	int BDef   = (tailCoding == dflt)?1:0;
	if (ADef+BDef > 1)  BDef = 0;  // arbitrary choice
	int ABDef  = 1*ADef + 2*BDef;
	assert(ABDef < 3);
	int KX     = getKXOf(K);
	int KB     = getKBOf(K);
	assert(decodeK(KX, KB) == K);
	int KBFlag = (KB != KB_DEFAULT)?1:0;
	ByteArrayOutputStream bytes = new ByteArrayOutputStream(10);
	bytes.write(_meta_run + KX + 4*KBFlag + 8*ABDef);
	if (KBFlag != 0)    bytes.write(KB);
	try {
	    if (ADef == 0)  bytes.write(headCoding.getMetaCoding(dflt));
	    if (BDef == 0)  bytes.write(tailCoding.getMetaCoding(dflt));
	} catch (IOException ee) {
	    throw new RuntimeException(ee);
	}
	return bytes.toByteArray();
    }
    public static int parseMetaCoding(byte[] bytes, int pos, Coding dflt, CodingMethod res[]) {
	int op = bytes[pos++] & 0xFF;
	if (op < _meta_run)  return pos-1; // backup
	op -= _meta_run;
	int KX = op % 4;
	int KBFlag = (op / 4) % 2;
	int ABDef = (op / 8);
	if (ABDef >= 3)  return pos-1;  // backup
	int ADef = (ABDef & 1);
	int BDef = (ABDef & 2);
	CodingMethod[] ACode = {dflt}, BCode = {dflt};
	int KB = KB_DEFAULT;
	if (KBFlag != 0)
	    KB = bytes[pos++] & 0xFF;
	if (ADef == 0)
	    pos = BandStructure.parseMetaCoding(bytes, pos, dflt, ACode);
	if (BDef == 0)
	    pos = BandStructure.parseMetaCoding(bytes, pos, dflt, BCode);
	res[0] = new AdaptiveCoding(decodeK(KX, KB),
				    ACode[0], BCode[0]);
	return pos;
    }

    private String keyString(CodingMethod m) {
	if (m instanceof Coding)
	    return ((Coding)m).keyString();
	return m.toString();
    }
    public String toString() {
	return
	    "run("+
	    "K="+headLength+
	    " A="+keyString(headCoding)+
	    " B="+keyString(tailCoding)+
	    ")";
    }

/*
    public static void main(String av[]) {
	int[][] samples = {
	    {1,2,3,4,5},
	    {254,255,256,256+1*16,256+2*16},
	    {0xfd,0xfe,0xff,0x100,0x110,0x120,0x130},
	    {0xfd0,0xfe0,0xff0,0x1000,0x1100,0x1200,0x1300},
	    {0xfd00,0xfe00,0xff00,0x10000,0x11000,0x12000,0x13000},
	    {0xfd000,0xfe000,0xff000,0x100000}
	};
	for (int i = 0; i < samples.length; i++) {
	    for (int j = 0; j < samples[i].length; j++) {
		int K = samples[i][j];
		int KX = getKXOf(K);
		int KB = getKBOf(K);
		System.out.println("K="+Integer.toHexString(K)+
				   " KX="+KX+" KB="+KB);
		assert(isCodableLength(K));
		assert(K == decodeK(KX, KB));
		if (j == 0)  continue;
		int K1 = samples[i][j-1];
		assert(K == getNextK(K1));
	    }
	}
    }
//*/

}
