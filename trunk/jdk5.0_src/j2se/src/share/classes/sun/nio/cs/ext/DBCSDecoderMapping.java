/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DBCSDecoderMapping.java	1.2	03/12/19
 *
 * package private helper class which provides decoder (native->ucs) 
 * mapping capability for the benefit of compound encoders/decoders
 * whose individual component submappings do not need an association with 
 * an enclosing charset
 *
 */

package sun.nio.cs.ext;

public class DBCSDecoderMapping {

    /* 1rst level index */
    private short[] index1;
    /*
     * 2nd level index, provided by subclass
     * every string has 0x10*(end-start+1) characters.
     */
    private String[] index2;

    protected int start;
    protected int end;
    
    protected static final char REPLACE_CHAR='\uFFFD';

    public DBCSDecoderMapping(short[] index1, String[] index2,
			     int start, int end) {
	this.index1 = index1;
	this.index2 = index2;
	this.start = start;
	this.end = end;
    }

    /*
     * Can be changed by subclass
     */
    protected char decodeSingle(int b) {
	if (b >= 0)
	    return (char) b;
	return REPLACE_CHAR;
    }

    protected char decodeDouble(int byte1, int byte2) {
	if (((byte1 < 0) || (byte1 > index1.length))
	    || ((byte2 < start) || (byte2 > end)))
	    return REPLACE_CHAR;

	int n = (index1[byte1] & 0xf) * (end - start + 1) + (byte2 - start);
	return index2[index1[byte1] >> 4].charAt(n);
    }
}
